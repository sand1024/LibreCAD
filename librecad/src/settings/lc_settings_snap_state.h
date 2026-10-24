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

#ifndef LC_SETTINGS_SNAP_STATE_H
#define LC_SETTINGS_SNAP_STATE_H

#include "lc_setting.h"

namespace CFG_SnapState {
    inline const LC_SettingsGroupBase Group("SnapState");

    // RS_SnapMode::SnapEndpoint | RS_SnapMode::SnapMiddle  | RS_SnapMode::SnapCenter | RS_SnapMode::SnapIntersection
    inline const LC_Setting<int> o_SnapMode(&Group, "SnapMode",  (1 << 5) | (1 << 4)  | (1 << 2) | (1 << 0));
    inline const LC_Setting<int> o_MiddlePoints(&Group, "MiddlePoints", 1);
    inline const LC_Setting<QString> o_Distance(&Group, "Distance", "1.0");
}


#endif
