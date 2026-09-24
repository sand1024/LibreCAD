
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

#ifndef LC_MENUS_TOOLBARS_REPOSITORY_H
#define LC_MENUS_TOOLBARS_REPOSITORY_H

#include "lc_action_node.h"
#include "lc_preset_repository_base.h"

inline const QString MENUS_TOOLBARS_EXTENSION = ".lcmt";
inline const QString MENUS_TOOLBARS_FILE_IDENTIFIER = "LibreCAD Config: MenuBar and Toolbars";

class LC_RepositoryMenuBarAndToolbars : public LC_PresetRepositoryBase<NavigationLayoutConfig> {
public:
    LC_RepositoryMenuBarAndToolbars(const QString& configDir)
        : LC_PresetRepositoryBase<NavigationLayoutConfig>(configDir, MENUS_TOOLBARS_EXTENSION, MENUS_TOOLBARS_FILE_IDENTIFIER,
                                                          "toolbars_menus_index.lcix") {
    }
    ~LC_RepositoryMenuBarAndToolbars() override = default;

    QJsonObject configToJson(const NavigationLayoutConfig& config) const override;
    bool configFromJson(const QJsonObject& json, NavigationLayoutConfig& config) const override;

private:
    static QJsonArray serializeNodes(const QList<ActionNode>& nodes);
    static QList<ActionNode> deserializeNodes(const QJsonArray& arr);
};

#endif
