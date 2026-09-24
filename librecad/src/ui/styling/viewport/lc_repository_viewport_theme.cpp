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

#include "lc_repository_viewport_theme.h"

#include "lc_settings_app_styling.h"
#include "lc_settings_types.h"
#include "rs_system.h"

// namespace

LC_RepositoryViewportTheme::LC_RepositoryViewportTheme(const QString& configDir)
    : LC_PresetRepositoryBase<LC_ViewportThemeConfig>(configDir,".theme.json", "viewport_theme",
                                                     "themes_index.json") {
}

QJsonObject LC_RepositoryViewportTheme::configToJson(const LC_ViewportThemeConfig& config) const {
    return config.rootObject;
}

bool LC_RepositoryViewportTheme::configFromJson(const QJsonObject& json, LC_ViewportThemeConfig& config) const {
    config.rootObject = json;
    return true;
}
