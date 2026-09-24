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


#include "lc_typography_utils.h"

#include <QApplication>
#include <QFont>

#include "lc_settings_app_state.h"

void LC_TypographyUtils::initializeDefaultConfig(FontConfig& config) {
    config.name = CFG_AppState::DEFAULT_THEME_KEY;
    config.mainFamily = "Segoe UI";
    config.mainSize   = 10;

    config.headings.setup(0, false, false);
    config.menuBar.setup(0, false, false);
    config.menus.setup(0, false, false);
    config.buttons.setup(0, false, false);
    config.inputs.setup(0, false, false);

    config.genericDockTitle.setup(0, false, false);
    config.specialDockTitle.setup(0, false, false);
    config.propertiesWidget.setup(0, false, false);

    config.techFamily = "Consolas";
    config.technical.setup(0, false, false);
}
