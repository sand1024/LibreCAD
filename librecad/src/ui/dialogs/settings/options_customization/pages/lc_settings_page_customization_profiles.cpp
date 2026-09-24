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

#include "lc_settings_page_customization_profiles.h"

#include "lc_action_group_manager.h"
#include "lc_command_manager.h"
#include "lc_graphic_view_context_menu_provider.h"
#include "lc_navigation_creator.h"
#include "lc_settings_app_state.h"
#include "lc_shortcuts_manager.h"

LC_SettingsPageCustomizationProfiles::LC_SettingsPageCustomizationProfiles(
    LC_ActionGroupManager* groupManager,
    LC_ShortcutsManager* shortcutsManager,
    LC_CommandManager* commandManager,
    LC_NavigationControlsCreator* navCreator,
    LC_GraphicViewContextMenuProvider* contextMenuProvider,
    QObject* parent)
    : LC_SettingsPageProfileExchangeBase(tr("Profiles Exchange"), parent)
    ,m_shortcutsManager(shortcutsManager)
    , m_groupManager(groupManager)
    , m_commandManager(commandManager)
    , m_navCreator(navCreator)
    , m_contextMenuProvider(contextMenuProvider) {
    setSortWeight(50);

    auto* shortcutsRepo = (m_shortcutsManager != nullptr) ? m_shortcutsManager->getRepository() : nullptr;
    auto* commandsRepo = (m_commandManager != nullptr) ? m_commandManager->getRepository() : nullptr;
    auto* navRepo      = (m_navCreator != nullptr) ? m_navCreator->getRepository() : nullptr;
    auto* menusRepo    = (m_contextMenuProvider != nullptr) ? m_contextMenuProvider->getMenusRepository() : nullptr;

    m_helper = std::make_unique<LC_CustomizationImportExportHelper>(shortcutsRepo, commandsRepo, navRepo, menusRepo);
}

void LC_SettingsPageCustomizationProfiles::setupUi() {
    setupExchangeUi(m_widget);
}

QString LC_SettingsPageCustomizationProfiles::profileFileFilter() const {
    return tr("LibreCAD Customization Profiles (*.lccp);;All Files (*.*)");
}

QString LC_SettingsPageCustomizationProfiles::profileDefaultName() const {
    return tr("My Custom Profile");
}

QString LC_SettingsPageCustomizationProfiles::profileDescriptionText() const {
    return tr("A Customization Profile (.lccp) bundles keyboard shortcuts, command-line aliases, "
              "navigation layout & toolbars, and context menus into a single shareable package.");
}

QString LC_SettingsPageCustomizationProfiles::profileBoxTitle() const {
    return tr("Consolidated Customization Profiles (.lccp)");
}

QList<LC_ChecklistChoice> LC_SettingsPageCustomizationProfiles::getExportChoices() const {
    return {
        {"shortcuts",     tr("Keyboard Shortcuts"),   tr("Export active keymap scheme bindings"),            true, true},
        {"commands",      tr("Command Aliases"),      tr("Export active command-line aliases and keycodes"),  true, true},
        {"navigation",    tr("Menus & Toolbars"),     tr("Export active navigation layout and toolbars"),     true, true},
        {"context_menus", tr("Context Menus Suite"),  tr("Export active drawing area context menus"),         true, true}
    };
}

bool LC_SettingsPageCustomizationProfiles::inspectProfileFile(const QString& filePath,
                                                              QString& outProfileName,
                                                              QList<LC_ChecklistChoice>& outChoices) const {
    return m_helper ? m_helper->inspectProfile(filePath, outProfileName, outChoices) : false;
}

bool LC_SettingsPageCustomizationProfiles::executeExport(const QString& filePath,
                                                         const QString& profileName,
                                                         const QList<LC_ChecklistChoice>& choices) {
    if (m_helper == nullptr) {
        return false;
    }

    auto isChecked = [&choices](const QString& id) -> bool {
        for (const auto& c : choices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    ShortcutsConfig shortcuts;
    CommandsConfig commands;
    NavigationLayoutConfig navigation;
    ContextMenusConfig contextMenus;

    const ShortcutsConfig* pShortcuts = nullptr;
    const CommandsConfig* pCommands = nullptr;
    const NavigationLayoutConfig* pNav = nullptr;
    const ContextMenusConfig* pMenus = nullptr;

    if (isChecked("shortcuts") && m_shortcutsManager != nullptr) {
        auto* repo = m_shortcutsManager->getRepository();
        if (repo != nullptr) {
            repo->loadByKey(CFG_AppState::o_ActiveShortcutsScheme, shortcuts);
            pShortcuts = &shortcuts;
        }
    }
    if (isChecked("commands") && m_commandManager != nullptr && m_commandManager->getRepository() != nullptr) {
        m_commandManager->getRepository()->loadByKey(CFG_AppState::o_ActiveCommandsScheme, commands);
        pCommands = &commands;
    }
    if (isChecked("navigation") && m_navCreator != nullptr && m_navCreator->getRepository() != nullptr) {
        m_navCreator->getRepository()->loadByKey(CFG_AppState::o_ActiveNavigationLayoutScheme, navigation);
        pNav = &navigation;
    }
    if (isChecked("context_menus") && m_contextMenuProvider != nullptr && m_contextMenuProvider->getMenusRepository() != nullptr) {
        m_contextMenuProvider->getMenusRepository()->loadByKey(CFG_AppState::o_ActiveContextMenusScheme, contextMenus);
        pMenus = &contextMenus;
    }

    return m_helper->exportProfile(filePath, profileName, pShortcuts, pCommands, pNav, pMenus);
}

bool LC_SettingsPageCustomizationProfiles::executeImport(const QString& filePath,
                                                         const QList<LC_ChecklistChoice>& choices,
                                                         QString& outProfileName) {
    if (m_helper == nullptr) {
        return false;
    }

    m_pendingShortcutsKey.clear();
    m_pendingCommandsKey.clear();
    m_pendingNavigationKey.clear();
    m_pendingMenusKey.clear();

    return m_helper->importProfile(filePath, choices, outProfileName,
                                   m_pendingShortcutsKey,
                                   m_pendingCommandsKey,
                                   m_pendingNavigationKey,
                                   m_pendingMenusKey);
}

void LC_SettingsPageCustomizationProfiles::activateImportedProfile() {
    if (!m_pendingShortcutsKey.isEmpty()) {
        CFG_AppState::o_ActiveShortcutsScheme = m_pendingShortcutsKey;
        if (m_shortcutsManager != nullptr) {
            m_shortcutsManager->loadActiveScheme(m_groupManager->getActionsMap());
        }
    }
    if (!m_pendingCommandsKey.isEmpty() && m_commandManager != nullptr) {
        CFG_AppState::o_ActiveCommandsScheme = m_pendingCommandsKey;
        m_commandManager->loadActiveScheme(nullptr);
    }
    if (!m_pendingNavigationKey.isEmpty() && m_navCreator != nullptr) {
        CFG_AppState::o_ActiveNavigationLayoutScheme = m_pendingNavigationKey;
        m_navCreator->applyActiveLayoutScheme();
    }
    if (!m_pendingMenusKey.isEmpty() && m_contextMenuProvider != nullptr) {
        CFG_AppState::o_ActiveContextMenusScheme = m_pendingMenusKey;
        m_contextMenuProvider->loadActiveScheme();
    }
}
