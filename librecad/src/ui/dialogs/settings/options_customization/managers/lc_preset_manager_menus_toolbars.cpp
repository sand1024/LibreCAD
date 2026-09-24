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

#include "lc_action_group_manager.h"
#include "lc_default_navigation_layout_builder.h"
#include "lc_navigation_creator.h"
#include "lc_settings_app_state.h"

LC_PresetManagerMenusToolbars::LC_PresetManagerMenusToolbars(LC_NavigationControlsCreator* controlsCreator,
                                                             LC_ActionFactory* actionFactory,
                                                             QObject* parent)
    : LC_PresetManagerConfigBase<NavigationLayoutConfig, LC_RepositoryMenuBarAndToolbars>(
          controlsCreator != nullptr ? controlsCreator->getRepository() : nullptr,
          CFG_AppState::o_ActiveNavigationLayoutScheme, parent)
    , m_creatorInvoker(controlsCreator)
    , m_actionFactory(actionFactory)
    , m_actionGroupManager(controlsCreator != nullptr ? controlsCreator->getActionGroupManager() : nullptr) {

    m_workingConfig = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
}

LC_PresetManagerUIStrings LC_PresetManagerMenusToolbars::presetStrings() const {
    LC_PresetManagerUIStrings s;
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
    auto extension = m_repository != nullptr ? m_repository->getFileExtension() : QString(".json");
    s.presetFileFilter = tr("Navigation Layout Scheme Files (*%1);All Files (*.*)")
                             .arg(extension);

    s.defaultReadOnlyMessage = tr("The Default workspace scheme is a read-only template. To customize menus or add toolbars, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");
    s.saveModifiedPromptTitle = tr("Save Workspace Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to workspace scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to workspace scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerMenusToolbars::loadPreset(const QString& key) {
    if (isDefaultPreset(key)) {
        m_workingConfig = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
        setActivePresetKeyDefault();
        setDirtyState(false);
        return true;
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        setActivePresetKey(key);
        setDirtyState(false);
        return true;
    }
    // Fallback: reset to default navigation layout and default key
    m_workingConfig = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
    setActivePresetKeyDefault();
    setDirtyState(false);
    return false;
}

void LC_PresetManagerMenusToolbars::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppState::o_ActiveNavigationLayoutScheme = activeKey;
}

void LC_PresetManagerMenusToolbars::onPostApplyPreset() {
    if (m_creatorInvoker != nullptr) {
        m_creatorInvoker->applyMenusToolbarsScheme(m_workingConfig);
    }
}

void LC_PresetManagerMenusToolbars::rollbackState() {
    LC_PresetManagerConfigBase::rollbackState();
    if (m_creatorInvoker != nullptr) {
        m_creatorInvoker->applyMenusToolbarsScheme(m_workingConfig);
    }
}
