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

#include "lc_preset_manager_shortcuts.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "lc_actiongroupmanager.h"
#include "lc_settings_appearance.h"
#include "lc_shortcutinfo.h"
#include "lc_shortcuts_manager.h"
#include "lc_shortcutsstorage.h"
#include "lc_shortcutstreemodel.h"

LC_PresetManagerShortcuts::LC_PresetManagerShortcuts(LC_ActionGroupManager* groupMgr, QObject* parent)
    : LC_AbstractPresetManager(CFG_Appearance::o_ActiveShortcutsScheme.get(), parent), m_groupManager(groupMgr),
      m_shortcutsManager(groupMgr->getShortcutsManager()), m_repository(groupMgr->getShortcutsManager()->getRepository()) {
}

void LC_PresetManagerShortcuts::setTreeModel(LC_ShortcutsTreeModel* model) {
    m_treeModel = model;
}

LC_PresetManagerUIStrings LC_PresetManagerShortcuts::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Keymap");
    s.labelText = tr("Shortcuts scheme:");
    s.selectToolTip = tr("Select a keyboard shortcuts scheme or load application defaults.");
    s.saveToolTip = tr("Save changes directly to the active shortcuts scheme.");
    s.saveAsToolTip = tr("Save current shortcuts as a new scheme.");
    s.deleteToolTip = tr("Permanently delete the selected custom shortcuts scheme from disk.");
    s.applyToolTip = tr("Apply the active shortcut mappings globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the scheme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Shortcuts Scheme As");
    s.saveAsDialogLabel = tr("Enter unique shortcuts scheme name:");
    s.defaultNewPresetName = tr("Custom Shortcuts");
    s.deleteConfirmTitle = tr("Delete Shortcuts Scheme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the shortcuts scheme '%1'?");
    s.exportDialogTitle = tr("Export Shortcuts Scheme");
    s.importDialogTitle = tr("Import Shortcuts Scheme");
    s.presetFileFilter = tr("LibreCAD Shortcuts Files (*.lcsc *.lcs);;All Files (*.*)");

    s.defaultReadOnlyMessage = tr(
        "The Default shortcuts scheme is a read-only template. To customize key bindings, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");

    s.saveModifiedPromptTitle = tr("Save Modified Shortcuts");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to shortcuts scheme '%1'.\n\nDo you want to save them before closing?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to shortcuts scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerShortcuts::loadPreset(const QString& key) {
    if (m_treeModel == nullptr) {
        return false;
    }

    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        m_treeModel->resetAllToDefault();
        m_treeModel->commitBaseline();
        m_activeKey = DEFAULT_THEME_KEY;
        setDirtyState(false);
        return true;
    }

    if (m_repository != nullptr) {
        ShortcutsConfig config;
        if (m_repository->loadByKey(key, config)) {
            m_treeModel->applyShortcuts(config.shortcuts, /*replace=*/true);
            m_activeKey = key;
            setDirtyState(false);
            return true;
        }
    }
    return false;
}

ShortcutsConfig LC_PresetManagerShortcuts::collectCurrentConfig(const QString& name) const {
    ShortcutsConfig config;
    config.name = name;
    if (m_treeModel != nullptr) {
        for (const auto* info : m_treeModel->getShortcuts()) {
            if (info != nullptr && !info->getKey().isEmpty()) {
                config.shortcuts.insert(info->getName(), info->getKey());
            }
        }
    }
    return config;
}

bool LC_PresetManagerShortcuts::saveCurrentPreset() {
    if (isReadOnlyDefault() || m_repository == nullptr) {
        return false;
    }

    ShortcutsConfig config = collectCurrentConfig(currentPresetDisplayName());
    QString outKey;
    if (m_repository->save(config.name, config, outKey)) {
        m_activeKey = outKey;
        CFG_Appearance::o_ActiveShortcutsScheme.set(outKey);
        if (m_treeModel != nullptr) {
            m_treeModel->commitBaseline();
        }
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerShortcuts::savePresetAs(const QString& name, QString& outKey) {
    if (m_repository == nullptr) {
        return false;
    }

    ShortcutsConfig config = collectCurrentConfig(name);
    if (m_repository->save(name, config, outKey)) {
        m_activeKey = outKey;
        CFG_Appearance::o_ActiveShortcutsScheme.set(outKey);
        if (m_treeModel != nullptr) {
            m_treeModel->commitBaseline();
        }
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerShortcuts::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        return false;
    }
    return m_repository->removeByKey(key);
}

void LC_PresetManagerShortcuts::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_Appearance::o_ActiveShortcutsScheme.set(activeKey);

    if (m_treeModel != nullptr && m_groupManager != nullptr && m_shortcutsManager != nullptr) {
        QMap<QString, LC_ShortcutInfo*> currentMap = m_treeModel->getShortcuts();
        auto actionsMap = m_groupManager->getActionsMap();
        m_shortcutsManager->applyShortcutsMapToActionsMap(currentMap, actionsMap);
        m_shortcutsManager->updateActionTooltips(actionsMap);
    }
}

void LC_PresetManagerShortcuts::applyCurrentPreset() {
    applyActiveConfigToSystem(m_activeKey);

    if (m_treeModel != nullptr) {
        m_treeModel->commitBaseline();
    }
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}

bool LC_PresetManagerShortcuts::isPresetModified() {
    if (m_treeModel != nullptr) {
        return m_treeModel->isModified();
    }
    return m_isDirty;
}

QList<QPair<QString, QString>> LC_PresetManagerShortcuts::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.prepend(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

bool LC_PresetManagerShortcuts::importPresetFromFile(const QString& filePath, QWidget*) {
    if (m_treeModel == nullptr) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QByteArray data = file.readAll().trimmed();
    file.close();

    QMap<QString, QKeySequence> importedMap;

    if (data.startsWith('<')) {
        // Legacy XML format
        const int res = LC_ShortcutsStorage::loadShortcuts(filePath, &importedMap);
        if (res != LC_ShortcutsStorage::OK)
            return false;
    }
    else {
        // JSON format
        ShortcutsConfig config;
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject() || !m_repository->configFromJson(doc.object(), config))
            return false;
        importedMap = config.shortcuts;
    }

    m_treeModel->applyShortcuts(importedMap, /*replace=*/false);
    setDirtyState(true);
    return true;
}

bool LC_PresetManagerShortcuts::exportPresetToFile(const QString& key, const QString& filePath, QWidget*) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    ShortcutsConfig config = collectCurrentConfig(key);
    QJsonObject obj = m_repository->configToJson(config);
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}
