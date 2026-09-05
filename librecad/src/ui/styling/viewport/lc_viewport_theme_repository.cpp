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

#include "lc_viewport_theme_repository.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>

#include "lc_settings_appearance.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_colors.h"
#include "lc_settings_defaults.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "lc_settings_render.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "lc_settings_types.h"
#include "rs_system.h"
#include "rs_settings.h"

 // namespace

LC_ViewportThemeRepository::LC_ViewportThemeRepository()
    : LC_StyleRepositoryBase<LC_ViewportThemeConfig>(RS_System::instance()->getAppDataDir() + "/themes", ".theme.json", "viewport_theme",
                                                     "themes_index.json") { // fixme - fixme filesystem
}

QJsonObject LC_ViewportThemeRepository::configToJson(const LC_ViewportThemeConfig& config) const {
    return config.rootObject;
}

bool LC_ViewportThemeRepository::configFromJson(const QJsonObject& json, LC_ViewportThemeConfig& config) const {
    config.rootObject = json;
    return true;
}
