/*******************************************************************************
 *
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 * Copyright (C) 2026 sand1024
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "lc_preset_manager_commands.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

#include "lc_action_command_updater.h"
#include "lc_action_group_manager.h"
#include "lc_default_commands_builder.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_state.h"
#include "lc_settings_commands_promotion.h"

LC_PresetManagerCommands::LC_PresetManagerCommands(LC_CommandManager* commandManager, LC_ActionGroupManager* agm, QObject* parent)
    : LC_PresetManagerConfigBase<CommandsConfig, LC_RepositoryCommands>(
          commandManager != nullptr ? commandManager->getRepository() : nullptr,
          CFG_AppState::o_ActiveCommandsScheme, parent)
    , m_commandManager(commandManager)
    , m_actionGroupManager(agm)
    , m_actionTypeMapper(std::make_unique<LC_ActionTypeMapper>(agm)) {

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerCommands::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Command Aliases");
    s.labelText = tr("Aliases Scheme:");
    s.selectToolTip = tr("Select a command aliases scheme or load standard defaults.");
    s.saveToolTip = tr("Save changes directly to active aliases scheme.");
    s.saveAsToolTip = tr("Save current command aliases as a new scheme.");
    s.deleteToolTip = tr("Permanently delete the selected custom command aliases scheme from disk.");
    s.applyToolTip = tr("Apply the active command aliases scheme to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload scheme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Command Aliases As");
    s.saveAsDialogLabel = tr("Enter unique command aliases scheme name:");
    s.defaultNewPresetName = tr("Custom Aliases");
    s.deleteConfirmTitle = tr("Delete Command Aliases Scheme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the command aliases scheme '%1'?");
    s.exportDialogTitle = tr("Export Command Aliases");
    s.importDialogTitle = tr("Import Command Aliases");
    s.presetFileFilter = tr("Command Aliases Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".lci"));

    s.defaultReadOnlyMessage = tr(
        "The Default command aliases scheme is a read-only template. To customize commands or aliases, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");
    s.saveModifiedPromptTitle = tr("Save Command Aliases Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to command aliases scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to command aliases scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerCommands::loadPreset(const QString& key) {
    if (isDefaultPreset(key)) {
        m_workingConfig = LC_DefaultCommandsBuilder::createDefaultConfig(m_actionTypeMapper.get());
        setActivePresetKeyDefault();
        setDirtyState(false);
        return true;
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        setActivePresetKey(key);
        setDirtyState(false);
        return true;
    }

    // Fallback: reset to default commands configuration and default key
    m_workingConfig = LC_DefaultCommandsBuilder::createDefaultConfig(m_actionTypeMapper.get());
    setActivePresetKeyDefault();
    setDirtyState(false);
    return false;
}

void LC_PresetManagerCommands::updateActionForCommandsInMenu() {
    const bool clearTooltips = !(CFG_Appearance::o_ShowKeyboardShortcutsInTooltips || !CFG_CommandsPromotion::o_ShowCommandInMenu);
    if (clearTooltips) {
        LC_ActionCommandUpdater::clearActions(m_actionGroupManager);
    }
    else {
        const bool keycodeMode = CFG_AppState::o_KeycodeMode;
        LC_ActionCommandUpdater::updateActionsForScheme(m_actionGroupManager, m_workingConfig,
                                                        m_actionTypeMapper.get(), keycodeMode);
    }
}

void LC_PresetManagerCommands::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppState::o_ActiveCommandsScheme = activeKey;
}

void LC_PresetManagerCommands::onPostApplyPreset() {
    if (m_commandManager != nullptr) {
        m_commandManager->applyCommandsScheme(m_workingConfig, m_actionTypeMapper.get());
        updateActionForCommandsInMenu();
    }
}

void LC_PresetManagerCommands::rollbackState() {
    LC_PresetManagerConfigBase::rollbackState();
    if (m_commandManager != nullptr) {
        m_commandManager->applyCommandsScheme(m_workingConfig, m_actionTypeMapper.get());
        updateActionForCommandsInMenu();
    }
}

void LC_PresetManagerCommands::notifyConfigChanged() {
    setDirtyState(true);
}

bool LC_PresetManagerCommands::importLegacyAliasFile(const QString& filePath) {
    clearLastError();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLastError(LC_PresetError::fromCode(
            LC_PresetErrorCode::FileReadFailed,
            tr("Cannot open legacy aliases file '%1' for reading.").arg(filePath)
        ));
        return false;
    }

    m_workingConfig = LC_DefaultCommandsBuilder::createDefaultConfig(m_actionTypeMapper.get());

    QMap<QString, QString> cmdToName;
    for (const auto& item : g_commandList) {
        const QString actionName = m_actionTypeMapper->actionNameFromType(item.actionType);
        if (actionName.isEmpty()) {
            continue;
        }
        for (const auto& [fullCmd, trans] : item.fullCmdList) {
            cmdToName.insert(fullCmd.toLower(), actionName);
        }
        for (const auto& [shortCmd, trans] : item.shortCmdList) {
            cmdToName.insert(shortCmd.toLower(), actionName);
        }
    }

    QMap<QString, int> actionAliasCount;
    QTextStream ts(&file);
    static const QRegularExpression wsRe(R"(\s+)");

    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QStringList tokens = line.split(wsRe, Qt::SkipEmptyParts);
        if (tokens.size() < 2) {
            continue;
        }

        const QString alias = tokens[0].toLower();
        const QString targetCmd = tokens[1].toLower();

        const auto it = cmdToName.find(targetCmd);
        if (it == cmdToName.end()) {
            continue;
        }

        const QString actionName = it.value();
        for (auto& def : m_workingConfig.commands) {
            if (def.actionName == actionName) {
                const int count = actionAliasCount.value(actionName, 0);
                if (count == 0) {
                    def.customKeycode = alias;
                    actionAliasCount[actionName] = 1;
                }
                else if (count == 1) {
                    def.customAlias = alias;
                    actionAliasCount[actionName] = 2;
                }
                break;
            }
        }
    }

    m_workingConfig.name = QFileInfo(filePath).baseName();
    setDirtyState(true);
    return true;
}

bool LC_PresetManagerCommands::importPresetFromFile(const QString& filePath, QWidget* parent) {
    if (filePath.endsWith(".alias", Qt::CaseInsensitive)) {
        return importLegacyAliasFile(filePath);
    }
    return LC_PresetManagerConfigBase::importPresetFromFile(filePath, parent);
}
