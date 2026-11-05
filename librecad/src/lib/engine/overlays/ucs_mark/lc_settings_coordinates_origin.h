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

#ifndef LC_SETTINGS_ORIGINS_H
#define LC_SETTINGS_ORIGINS_H
#include "lc_setting.h"

namespace CFG_CoordinatesOrigin {
    inline const LC_SettingsGroupBase Group("CoordinateOrigins");

    inline const LC_Setting<QColor> o_ColorXAxis(&Group, "grid_x_axisColor", QColor("red"));
    inline const LC_Setting<QColor> o_ColorYAxis(&Group, "grid_y_axisColor", QColor("green"));

    // --- Relative Zero and Axis Line Extensions ---
    inline const LC_Setting<bool> o_HideRelativeZero(&Group, "HideRelativeZero", false);
    inline const LC_Setting<bool> o_ExtendAxisLines(&Group, "ExtendAxisLines", false);

    inline const LC_Setting<int> o_ExtendModeXAxis(&Group, "ExtendModeXAxis", 0);
    inline const LC_Setting<int> o_ExtendModeYAxis(&Group, "ExtendModeYAxis", 0);

    inline const LC_Setting<bool> o_ShowAxisLines(&Group, "ShowAxisLines", true);

    // --- Coordinate Origin Zero Markers ---
    inline const LC_Setting<bool> o_ShowUCSZeroMarker(&Group, "ShowUCSZeroMarker", false);
    inline const LC_Setting<bool> o_ShowWCSZeroMarker(&Group, "ShowWCSZeroMarker", true);

    inline const LC_Setting<int> o_RelZeroMarkerRadius(&Group, "RelZeroMarkerRadius", 8);
    inline const LC_Setting<int> o_ZeroShortAxisMarkSize(&Group, "ZeroShortAxisMarkSize", 45);
    inline const LC_Setting<int> o_ZeroMarkerSize(&Group, "ZeroMarkerSize", 30);
    inline const LC_Setting<int> o_ZeroMarkerFontSize(&Group, "ZeroMarkerFontSize", 9);
    inline const LC_Setting<QString> o_ZeroMarkerFontName(&Group, "ZeroMarkerFontName", "Verdana");

    inline const LC_Setting<QColor> o_ColorRelativeZero(&Group, "relativeZeroColor", QColor("Red"));
}


#endif
