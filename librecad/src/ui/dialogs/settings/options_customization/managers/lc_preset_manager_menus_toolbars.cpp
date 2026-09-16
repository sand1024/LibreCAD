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

#include "lc_preset_manager_menus_toolbars.h"

#include <QFile>
#include <QJsonDocument>

#include "lc_action_group_manager.h"
#include "lc_navigation_creator.h"
#include "lc_default_navigation_layout_builder.h"
#include "lc_settings_app_state.h"
#include "lc_settings_paths.h"
#include "lc_settings_startup.h"
#include "rs_settings.h"

LC_PresetManagerMenusToolbars::LC_PresetManagerMenusToolbars(LC_NavigationControlsCreator* invoker,
                                                             LC_ActionFactory* actionFactory,
                                                             QObject* parent)
    : LC_AbstractPresetManager(CFG_AppState::o_ActiveNavigationLayoutScheme, parent)
    , m_creatorInvoker(invoker)
    , m_actionFactory(actionFactory)
    , m_actionGroupManager(invoker->getActionGroupManager())
    , m_repository(invoker->getToolbarsRepository()) {

    // Initialize baseline
    m_workingConfig = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
    m_repository->migrateLegacyToolbarsIfNeeded(m_workingConfig);
}

LC_PresetManagerUIStrings LC_PresetManagerMenusToolbars::presetStrings() const {
    LC_PresetManagerUIStrings s; // fixme - sand - update wording
    s.defaultPresetName = tr("Default Workspace Suite");
    s.labelText = tr("Workspace Scheme:");
    s.selectToolTip = tr("Select a workspace scheme for main menus and toolbars.");
    s.saveToolTip = tr("Save changes directly to active scheme.");
    s.saveAsToolTip = tr("Save current menus and toolbars as a new scheme.");
    s.deleteToolTip = tr("Permanently delete the selected custom scheme from disk.");
    s.applyToolTip = tr("Apply the active menus and toolbars scheme to application window.");
    s.revertToolTip = tr("Discard modifications and reload scheme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Workspace Scheme As");
    s.saveAsDialogLabel = tr("Enter unique workspace scheme name:");
    s.defaultNewPresetName = tr("Custom Workspace");
    s.deleteConfirmTitle = tr("Delete Workspace Scheme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the workspace scheme '%1'?");
    s.exportDialogTitle = tr("Export Workspace Scheme");
    s.importDialogTitle = tr("Import Navigation Layout Scheme");
    s.presetFileFilter = tr("Navigation Layout Scheme Files (*.lcnl);;All Files (*.*)");

    s.defaultReadOnlyMessage = tr("The Default workspace scheme is a read-only template. To customize menus or add toolbars, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");
    s.saveModifiedPromptTitle = tr("Save Workspace Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to workspace scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to workspace scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerMenusToolbars::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        m_workingConfig = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
        m_activeKey = DEFAULT_THEME_KEY;
        setDirtyState(false);
        return true;
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        m_activeKey = key;
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerMenusToolbars::saveCurrentPreset() {
    if (isReadOnlyDefault() || m_repository == nullptr) {
        return false;
    }

    QString outKey;
    m_workingConfig.name = currentPresetDisplayName();
    if (m_repository->save(m_workingConfig.name, m_workingConfig, outKey)) {
        m_activeKey = outKey;
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerMenusToolbars::savePresetAs(const QString& name, QString& outKey) {
    if (m_repository == nullptr) {
        return false;
    }

    m_workingConfig.name = name;
    if (m_repository->save(name, m_workingConfig, outKey)) {
        m_activeKey = outKey;
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerMenusToolbars::doDeletePreset(const QString& key) {
    return m_repository->deleteByKey(key);
}

void LC_PresetManagerMenusToolbars::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppState::o_ActiveNavigationLayoutScheme =  activeKey;

    // Synchronize startup flags from working config
    CFG_Startup::o_ExpandedToolsMenu.set(m_workingConfig.activeMenuVariant >= 1); // fixme - review this!!!! why?
    CFG_Startup::o_ExpandedToolsMenuTillEntity.set(m_workingConfig.activeMenuVariant == 2);

    if (m_creatorInvoker != nullptr) {
        m_creatorInvoker->applyMenusToolbarsScheme(m_workingConfig);
    }
}

void LC_PresetManagerMenusToolbars::applyCurrentPreset() {
    applyActiveConfigToSystem(m_activeKey);
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}

void LC_PresetManagerMenusToolbars::rollbackState() {
    loadPreset(m_originalActiveKey);
    if (m_creatorInvoker != nullptr) {
        m_creatorInvoker->applyMenusToolbarsScheme(m_workingConfig);
    }
}

QList<QPair<QString, QString>> LC_PresetManagerMenusToolbars::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.prepend(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

bool LC_PresetManagerMenusToolbars::importPresetFromFile(const QString& filePath, QWidget*) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject() || m_repository == nullptr) {
        return false;
    }

    if (!m_repository->configFromJson(doc.object(), m_workingConfig)) {
        return false;
    }

    setDirtyState(true);
    return true;
}

bool LC_PresetManagerMenusToolbars::exportPresetToFile(const QString&, const QString& filePath, QWidget*) {
    if (m_repository == nullptr) {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonObject obj = m_repository->configToJson(m_workingConfig);
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}
