
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

#include "lc_document_settings_backend.h"
#include "lc_settings_manager_base.h"

class RS_Graphic;

namespace LC_SettingsPagesDrawing {
    inline const QString DLG_DOCUMENT_PREFERENCES = "DlgDocumentPreferences";

    inline const QString PAGE_PAPER      = "drawing.paper";
    inline const QString PAGE_UNITS      = "drawing.units";
    inline const QString PAGE_GRID       = "drawing.grid";
    inline const QString PAGE_DIMENSIONS = "drawing.dimensions";
    inline const QString PAGE_POINTS     = "drawing.points";
    inline const QString PAGE_SPLINES    = "drawing.splines";
    inline const QString PAGE_META       = "drawing.meta";
    inline const QString PAGE_USER_DATA  = "drawing.user_data";
    inline const QString PAGE_VARIABLES  = "drawing.variables";
}

class LC_SettingsManagerDrawing: public LC_SettingsManagerBase {
public:
    static void initialize();
    static bool show(RS_Graphic* graphic, QWidget* parent, const QString& initialPage = "");
};

#endif
