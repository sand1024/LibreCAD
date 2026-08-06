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


#ifndef LC_SETTINGS_GRID_H
#define LC_SETTINGS_GRID_H

#include "lc_setting.h"
#include "rs.h"

namespace CFG_Grid {
    inline const LC_SettingsGroupBase Group("Grid");

    inline const LC_Setting<int> o_GridType(&Group, "GridType", 0);
    inline const LC_Setting<RS2::LineType> o_metaGridPointsLineType(&Group, "metaGridPointsLineType", RS2::DotLineTiny);
    inline const LC_Setting<RS2::LineType> o_metaGridLinesLineType(&Group, "metaGridLinesLineType", RS2::SolidLine);
    inline const LC_Setting<RS2::LineType> o_GridLinesLineType(&Group, "GridLinesLineType", RS2::SolidLine);
    inline const LC_Setting<int> o_metaGridPointsLineWidth(&Group, "metaGridPointsLineWidth", 1);
    inline const LC_Setting<int> o_GridPointsPointSize(&Group, "GridPointsPointSize", 1);
    inline const LC_Setting<int> o_metaGridLinesLineWidth(&Group, "metaGridLinesLineWidth", 1);
    inline const LC_Setting<int> o_GridLinesLineWidth(&Group, "GridLinesLineWidth", 1);
}

#endif
