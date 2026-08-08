
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

#ifndef LC_SETTINGS_MANAGER_DRAWING_H
#define LC_SETTINGS_MANAGER_DRAWING_H

#include "lc_settings_registry.h"
#include "lc_document_settings_backend.h"

// Forward declarations of pages
class RS_Graphic;

namespace LC_SettingsPagesDrawing {
    inline const QString Paper      = "drawing.paper";
    inline const QString Units      = "drawing.units";
    inline const QString Grid       = "drawing.grid";
    inline const QString Dimensions = "drawing.dimensions";
    inline const QString Points     = "drawing.points";
    inline const QString Splines    = "drawing.splines";
    inline const QString Meta       = "drawing.meta";
    inline const QString UserData   = "drawing.user_data";
    inline const QString Variables  = "drawing.variables";
}

class LC_SettingsManagerDrawing {
public:
    static void initialize();
    static bool show(RS_Graphic* graphic, QWidget* parent, const QString& initialPage = "");
};

#endif
