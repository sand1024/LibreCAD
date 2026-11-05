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

#ifndef LC_SETTINGS_SNAP_H
#define LC_SETTINGS_SNAP_H

#include "lc_setting.h"
#include "rs.h"

namespace CFG_Snap {
    inline const LC_SettingsGroupBase Group("Snap");

    inline const LC_Setting<bool> o_ShowSnapOptionsInSnapToolbar(&Group, "showSnapOptionsInSnapToolbar", false);

    // RS_SnapMode::SnapEndpoint | RS_SnapMode::SnapMiddle  | RS_SnapMode::SnapCenter | RS_SnapMode::SnapIntersection
    inline const LC_Setting<int> o_SnapMode(&Group, "SnapMode",  (1 << 5) | (1 << 4)  | (1 << 2) | (1 << 0));

    inline const LC_Setting<int> o_SnapMiddlePointsCount(&Group, "MiddlePoints", 1);
    inline const LC_Setting<QString> o_SnapDistanceValue(&Group, "Distance", "1.0");

    inline const LC_Setting<int> o_SnapAngleAnglesStep(&Group, "AngleSnapStep", 3);

    inline const LC_Setting<bool> o_AngleSnapToLinesIfGrid(&Group, "AngleSnapToLinesIfGrid", true);

    inline const LC_Setting<int> o_AdvSnapOnEntitySwitchToFreeDistance(&Group, "AdvSnapOnEntitySwitchToFreeDistance", 500);
    inline const LC_Setting<int> o_AdvSnapEntityCatchRange(&Group, "AdvSnapEntityCatchRange", 32);
    inline const LC_Setting<int> o_AdvSnapGridCellSnapFactor(&Group, "AdvSnapGridCellSnapFactor", 25);

    inline const LC_Setting<bool> o_SnapGridIgnoreIfNoGrid(&Group, "SnapGridIgnoreIfNoGrid", false);

    inline const LC_Setting<bool> o_CursorHidingWhenSnapping(&Group, "CursorHideOnSnap", true);
}

#endif
