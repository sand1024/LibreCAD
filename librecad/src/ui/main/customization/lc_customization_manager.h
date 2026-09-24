
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

#ifndef LC_CUSTOMIZATIONMANAGER_H
#define LC_CUSTOMIZATIONMANAGER_H

#include <QString>

class LC_RepositoryGraphicViewContextMenus;
class LC_RepositoryKeymaps;
class LC_RepositoryMenuBarAndToolbars;
class LC_RepositoryCommands;

class LC_CustomizationManager {
public:
    LC_CustomizationManager();
    ~LC_CustomizationManager();
    void initRepositories();
    LC_RepositoryCommands* getCommandsRepository() const {return m_commandsRepository.get();}
    LC_RepositoryMenuBarAndToolbars* getMenuAndToolbarsRepository() const {return m_menuAndToolBarRepository.get();}
    LC_RepositoryKeymaps* getKeymapsRepository() const {return m_keymapsRepository.get();}
    LC_RepositoryGraphicViewContextMenus* getGraphicViewContextMenusRepository() const {return m_graphiViewContextMenusRepository.get();}

protected:
    QString getBaseCustomizationRepositoriesFolder();
    void createRepositories();
private:
    std::unique_ptr<LC_RepositoryCommands>  m_commandsRepository;
    std::unique_ptr<LC_RepositoryMenuBarAndToolbars>  m_menuAndToolBarRepository;
    std::unique_ptr<LC_RepositoryKeymaps>  m_keymapsRepository;
    std::unique_ptr<LC_RepositoryGraphicViewContextMenus>  m_graphiViewContextMenusRepository;
};

#endif
