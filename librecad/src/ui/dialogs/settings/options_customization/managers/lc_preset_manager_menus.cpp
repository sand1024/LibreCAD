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

#include "lc_default_context_menus_builder.h"
#include "lc_graphic_view_context_menu_provider.h"
#include "lc_settings_app_state.h"

LC_PresetManagerMenus::LC_PresetManagerMenus(LC_GraphicViewContextMenuProvider* provider,
                                             LC_ActionFactory* actionFactory,
                                             LC_ActionGroupManager* groupManager,
                                             QObject* parent)
    : LC_PresetManagerConfigBase<ContextMenusConfig, LC_RepositoryGraphicViewContextMenus>(
          provider != nullptr ? provider->getMenusRepository() : nullptr,
          CFG_AppState::o_ActiveContextMenusScheme, parent)
    , m_contextMenuProvider(provider)
    , m_actionFactory(actionFactory)
    , m_actionGroupManager(groupManager) {
}

LC_PresetManagerUIStrings LC_PresetManagerMenus::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Compact");
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
    s.presetFileFilter = tr("Menus Scheme Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".lcix"));

    s.defaultReadOnlyMessage = tr(
        "The default menus scheme is a read-only template. To customize popup menus, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");
    s.saveModifiedPromptTitle = tr("Save Menus Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to menus scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to menus scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerMenus::isDefaultPreset(const QString& key) const {
    return (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || key == THEME_EXTENDED_KEY);
}

bool LC_PresetManagerMenus::loadPreset(const QString& key) {
    using namespace CFG_AppState;
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        m_workingConfig = LC_DefaultContextMenusBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
        setActivePresetKeyDefault();
        setDirtyState(false);
        return true;
    }
    if (key == THEME_EXTENDED_KEY) {
        m_workingConfig = LC_DefaultContextMenusBuilder::createExtendedConfig(m_actionFactory, m_actionGroupManager);
        setActivePresetKey(THEME_EXTENDED_KEY);
        setDirtyState(false);
        return true;
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        setActivePresetKey(key);
        setDirtyState(false);
        return true;
    }

    // Fallback: reset to compact default context menus and default key
    m_workingConfig = LC_DefaultContextMenusBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
    setActivePresetKeyDefault();
    setDirtyState(false);
    return false;
}

void LC_PresetManagerMenus::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppState::o_ActiveContextMenusScheme = activeKey;
}

void LC_PresetManagerMenus::onPostApplyPreset() {
    if (m_contextMenuProvider != nullptr) {
        m_contextMenuProvider->applyCustomMenusScheme(m_workingConfig);
    }
}

void LC_PresetManagerMenus::prependDefaultPresets(QList<QPair<QString, QString>>& choices) const {
    choices.prepend(qMakePair(tr("Default - Extended"), QString(THEME_EXTENDED_KEY)));
    choices.prepend(qMakePair(tr("Default - Compact"), CFG_AppState::DEFAULT_THEME_KEY));
}

void LC_PresetManagerMenus::rollbackState() {
    LC_PresetManagerConfigBase::rollbackState();
    if (m_contextMenuProvider != nullptr) {
        m_contextMenuProvider->applyCustomMenusScheme(m_workingConfig);
    }
}
