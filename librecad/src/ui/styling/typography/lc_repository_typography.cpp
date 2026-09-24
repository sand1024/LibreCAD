/*******************************************************************************
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

#include "lc_repository_typography.h"

QJsonObject LC_RepositoryTypography::configToJson(const FontConfig& config) const {
    QJsonObject root;

    root["main_family"] = config.mainFamily;
    root["main_size"] = config.mainSize;
    root["tech_family"] = config.techFamily;

    auto serializeRole = [](const FontRoleConfig& role) {
        QJsonObject obj;
        obj["offset"] = role.sizeOffset;
        obj["bold"] = role.bold;
        obj["italic"] = role.italic;
        return obj;
    };

    root["headings"] = serializeRole(config.headings);
    root["menu_bar"] = serializeRole(config.menuBar);
    root["menus"] = serializeRole(config.menus);
    root["buttons"] = serializeRole(config.buttons);
    root["inputs"] = serializeRole(config.inputs);
    root["generic_dock"] = serializeRole(config.genericDockTitle);
    root["special_dock"] = serializeRole(config.specialDockTitle);
    root["properties_dock"] = serializeRole(config.propertiesWidget);
    root["technical"] = serializeRole(config.technical);

    return root;
}

bool LC_RepositoryTypography::configFromJson(const QJsonObject& json, FontConfig& config) const {
    config.mainFamily = json["main_family"].toString();
    config.mainSize = json["main_size"].toInt(10);
    config.techFamily = json["tech_family"].toString();

    auto loadRole = [&](const QString& key, FontRoleConfig& role) {
        QJsonObject obj = json[key].toObject();
        role.sizeOffset = obj["offset"].toInt(0);
        role.bold = obj["bold"].toBool(false);
        role.italic = obj["italic"].toBool(false);
    };

    loadRole("headings", config.headings);
    loadRole("menu_bar", config.menuBar);
    loadRole("menus", config.menus);
    loadRole("buttons", config.buttons);
    loadRole("inputs", config.inputs);
    loadRole("generic_dock", config.genericDockTitle);
    loadRole("special_dock", config.specialDockTitle);
    loadRole("properties_dock", config.propertiesWidget);
    loadRole("technical", config.technical);
    return true;
}
