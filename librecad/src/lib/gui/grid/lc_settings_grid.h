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

    inline const LC_Setting<RS2::LineType> o_PointsMetaGridLineType(&Group, "PointsMetaGridLineType", RS2::DotLineTiny);
    inline const LC_Setting<RS2::LineType> o_LinesMetaGridLineType(&Group, "LinesMetaGridLineType", RS2::SolidLine);
    inline const LC_Setting<RS2::LineType> o_LinesGridLineType(&Group, "LinesGridLineType", RS2::SolidLine);

    inline const LC_Setting<double> o_PointsMetaGridScreenLineWidth(&Group, "PointsMetaGridLineWidth", 1.0);
    inline const LC_Setting<double> o_PointsGridPointScreenSize(&Group, "PointsGridPointSize", 1.0);
    inline const LC_Setting<double> o_LinesMetaGridScreenLineWidth(&Group, "LinesMetaGridScreenLineWidth", 1.0);
    inline const LC_Setting<double> o_LinesGridScreenLineWidth(&Group, "LinesGridScreenLineWidth", 1.0);

    inline const LC_Setting<QColor> o_ColorGridPoints(&Group, "ColorGridPoints", QColor("#404040" /*#736F6E"*/));
    inline const LC_Setting<QColor> o_ColorGridLines(&Group, "ColorGridLines", "#55557F");
    inline const LC_Setting<QColor> o_ColorMetaGrid(&Group, "ColorMetaGrid", QColor("#404040"));
    inline const LC_Setting<QColor> o_ColorMetaGridLines(&Group, "ColorMetaGridLines", QColor("#55557f"));

    // --- Grid General Settings ---
    inline const LC_Setting<bool> o_UnitlessGrid(&Group, "UnitlessGrid", false);
    inline const LC_Setting<bool> o_GridRenderSimple(&Group, "GridRenderSimple", false);
    inline const LC_Setting<bool> o_GridDisableWithinPan(&Group, "GridDisableWithinPan", false);
    inline const LC_Setting<bool> o_ScaleGrid(&Group, "ScaleGrid", true);
    inline const LC_Setting<bool> o_GridDraw(&Group, "DrawGrid", true);
    inline const LC_Setting<bool> o_MetaGridDraw(&Group, "DrawMetaGrid", true);
    inline const LC_Setting<bool> o_GridDrawIsoVerticalForTop(&Group, "GridDrawIsoVerticalForTop", true);

    inline const LC_Setting<int> o_MetaGridEvery(&Group, "MetaGridEvery", 10);
    inline const LC_Setting<int> o_MinGridSpacing(&Group, "MinGridSpacing", 10);

}

#endif
