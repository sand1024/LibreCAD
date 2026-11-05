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


#include "lc_grid_options.h"

#include "lc_settings_grid.h"
#include "rs_color.h"

void LC_GridOptions::loadSettings() {
    {
        using namespace  CFG_Grid;

        int gridType =o_GridType;;
        if (forcedGridType > -1) {
            gridType = forcedGridType;
        }
        const bool linesGrid = gridType == 1;

        gridLineScreenWidthPx = o_LinesGridScreenLineWidth;
        gridLineType = o_LinesGridLineType;

        drawLines = linesGrid;
        gridColorPoint = RS_Color(o_ColorGridPoints);
        gridColorLine = RS_Color(o_ColorGridLines);
        if (linesGrid){
            metaGridLineType =  o_LinesMetaGridLineType;
            metaGridScreenLineWidthPx = o_LinesMetaGridScreenLineWidth;
        }
        else{
            metaGridLineType =  o_PointsMetaGridLineType;
            metaGridScreenLineWidthPx =  o_PointsMetaGridScreenLineWidth;
        }
        if (linesGrid) {
            metaGridColor= RS_Color(o_ColorMetaGridLines);
        }
        else{
            metaGridColor= RS_Color(o_ColorMetaGrid);
        }

        gridPointSize = o_PointsGridPointScreenSize;
    }

    {
        using namespace CFG_Grid;

        disableGridOnPanning = o_GridDisableWithinPan;
        drawIsometricVerticalsAlways = o_GridDrawIsoVerticalForTop;
        simpleGridRendering = o_GridRenderSimple;
        drawMetaGrid = o_MetaGridDraw;
        drawGrid = o_GridDraw;
    }
}
