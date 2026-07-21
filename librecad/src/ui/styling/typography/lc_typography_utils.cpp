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

void LC_TypographyUtils::initializeDefaultConfig(FontConfig& config) {
    config.name       = "Default Typography";
    config.mainFamily = "Segoe UI";
    config.mainSize   = 10;

    config.headings.setup(2, true, false);
    config.menuBar.setup(1, true, false);
    config.menus.setup(0, false, false);
    config.buttons.setup(-1, false, false);
    config.inputs.setup(0, false, false);

    config.genericDockTitle.setup(0, false, false);
    config.specialDockTitle.setup(0, true, false);

    config.techFamily = "Consolas";
    config.technical.setup(0, false, false);
}


void LC_TypographyUtils::initializeWithSystem(FontConfig &config) {
    config.name = "System Standard Typography";

    // Query active desktop window manager baseline typography on-the-fly
    const QFont currentFont = QApplication::font();
    config.mainFamily = currentFont.family();
    config.mainSize   = currentFont.pointSize() > 0 ? currentFont.pointSize() : 10;

    // Apply flat, safe zero-offsets for standard native integration
    config.headings.setup(0, false, false);
    config.menuBar.setup(0, false, false);
    config.menus.setup(0, false, false);
    config.buttons.setup(0, false, false);
    config.inputs.setup(0, false, false);
    config.genericDockTitle.setup(0, false, false);
    config.specialDockTitle.setup(0, false, false);


    // Fallback monospace technical/log font
    config.techFamily = "Consolas";
    config.technical.setup(0, false, false);
}
