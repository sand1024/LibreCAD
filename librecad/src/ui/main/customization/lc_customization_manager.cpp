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

#include "lc_customization_manager.h"

#include "lc_repository_commands.h"
#include "lc_repository_menu_bar_and_toolbars.h"
#include "lc_repository_shortcuts.h"
#include "lc_repository_graphic_view_context_menus.h"
#include "lc_settings_paths.h"

LC_CustomizationManager::LC_CustomizationManager(){
    createRepositories();
    initRepositories();
}

LC_CustomizationManager::~LC_CustomizationManager() {
}

void LC_CustomizationManager::createRepositories() {
    m_commandsRepository = std::make_unique<LC_RepositoryCommands>("");
    m_keymapsRepository = std::make_unique<LC_RepositoryKeymaps>("");
    m_graphiViewContextMenusRepository = std::make_unique<LC_RepositoryGraphicViewContextMenus>("");
    m_menuAndToolBarRepository = std::make_unique<LC_RepositoryMenuBarAndToolbars>("");
}

void LC_CustomizationManager::initRepositories() {
    QString baseFolder = getBaseCustomizationRepositoriesFolder();
    m_graphiViewContextMenusRepository->setConfigDir(baseFolder + "/context_menus");
    m_commandsRepository->setConfigDir(baseFolder + "/command_aliases");
    m_keymapsRepository->setConfigDir(baseFolder + "/keymaps");
    m_menuAndToolBarRepository->setConfigDir(baseFolder + "/menus_toolbars");
}

QString LC_CustomizationManager::getBaseCustomizationRepositoriesFolder() {
    const QString settingsDir = CFG_Paths::o_OtherSettingsDir;
    const QString customizationBaseDir = settingsDir + "/customization";
    return customizationBaseDir;
}
