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

#include "lc_preset_manager_menus.h"

#include <QFile>
#include <QJsonDocument>

#include "lc_default_context_menus_builder.h"
#include "lc_graphic_view_context_menu_provider.h"
#include "lc_navigation_creator.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_state.h"
#include "rs_settings.h"

LC_PresetManagerMenus::LC_PresetManagerMenus(LC_GraphicViewContextMenuProvider* provider, QObject* parent)
    : LC_AbstractPresetManager(CFG_AppState::o_ActiveContextMenusScheme, parent)
    , m_contextMenuProvider(provider)
    , m_repository(provider != nullptr ? provider->getMenusRepository() : nullptr) {
}

LC_PresetManagerUIStrings LC_PresetManagerMenus::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Menus Suite");
    s.labelText = tr("Menus scheme:");
    s.selectToolTip = tr("Select a custom popup menus scheme or load defaults.");
    s.saveToolTip = tr("Save changes directly to active menus scheme.");
    s.saveAsToolTip = tr("Save current menus as a new scheme.");
    s.deleteToolTip = tr("Permanently delete the selected menus scheme from disk.");
    s.applyToolTip = tr("Apply the active menus scheme to workspace.");
    s.revertToolTip = tr("Discard modifications and reload scheme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Menus Scheme As");
    s.saveAsDialogLabel = tr("Enter unique menus scheme name:");
    s.defaultNewPresetName = tr("Custom Menus");
    s.deleteConfirmTitle = tr("Delete Menus Scheme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the menus scheme '%1'?");
    s.exportDialogTitle = tr("Export Menus Scheme");
    s.importDialogTitle = tr("Import Menus Scheme");
    s.presetFileFilter = tr("Menus Scheme Files (*.lcmn);;All Files (*.*)");

    s.defaultReadOnlyMessage = tr(
        "The Default menus scheme is a read-only template. To customize popup menus, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");
    s.saveModifiedPromptTitle = tr("Save Menus Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to menus scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to menus scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerMenus::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        m_workingConfig = LC_DefaultContextMenusBuilder::createDefaultConfig();
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

bool LC_PresetManagerMenus::saveCurrentPreset() {
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

bool LC_PresetManagerMenus::savePresetAs(const QString& name, QString& outKey) {
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

void LC_PresetManagerMenus::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppState::o_ActiveContextMenusScheme = activeKey;
    if (m_contextMenuProvider != nullptr) {
        m_contextMenuProvider->applyCustomMenusScheme(m_workingConfig);
    }
}

void LC_PresetManagerMenus::rollbackState() {
    loadPreset(m_originalActiveKey);
    if (m_contextMenuProvider != nullptr) {
        m_contextMenuProvider->applyCustomMenusScheme(m_workingConfig);
    }
}

void LC_PresetManagerMenus::applyCurrentPreset() {
    applyActiveConfigToSystem(m_activeKey);
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}


QList<QPair<QString, QString>> LC_PresetManagerMenus::getAvailablePresets() const {
    auto choices = m_repository->getPresetChoices();
    choices.prepend(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
    return choices;
}

bool LC_PresetManagerMenus::importPresetFromFile(const QString& filePath, QWidget*) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return false;
    }

    if (!m_repository->configFromJson(doc.object(), m_workingConfig)) {
        return false;
    }

    setDirtyState(true);
    return true;
}

bool LC_PresetManagerMenus::exportPresetToFile(const QString&, const QString& filePath, QWidget*) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonObject obj = m_repository->configToJson(m_workingConfig);
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}

bool LC_PresetManagerMenus::doDeletePreset(const QString& key) {
    const bool ok = m_repository->removeByKey(key);
    return ok;
}
