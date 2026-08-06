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

#include "lc_settings_appearance.h"
#include "lc_settings_colors.h"
#include "lc_settings_grid.h"
#include "rs_color.h"

void LC_GridOptions::loadSettings() {
    int gridType = CFG_Grid::o_GridType;;
    if (forcedGridType > -1) {
        gridType = forcedGridType;
    }
    const bool linesGrid = gridType == 1;
    drawMetaGrid = CFG_Appearance::o_metaGridDraw;
    simpleGridRendering = CFG_Appearance::o_GridRenderSimple;
    gridWidthPx = CFG_Grid::o_GridLinesLineWidth;;
    gridLineType = CFG_Grid::o_GridLinesLineType;

    drawGrid = CFG_Appearance::o_GridDraw;

    drawLines = linesGrid;
    gridColorPoint = RS_Color(CFG_Colors::o_Grid);;
    gridColorLine = RS_Color(CFG_Colors::o_GridLines);;
    if (linesGrid){
        metaGridLineType =  CFG_Grid::o_metaGridLinesLineType;
        metaGridLineWidthPx = CFG_Grid::o_metaGridLinesLineWidth;
    }
    else{
        metaGridLineType =  CFG_Grid::o_metaGridPointsLineType;
        metaGridLineWidthPx =  CFG_Grid::o_metaGridPointsLineWidth;
    }
    if (linesGrid) {
        metaGridColor= RS_Color(CFG_Colors::o_MetaGridLines);
    }
    else{
        metaGridColor= RS_Color(CFG_Colors::o_MetaGrid);
    }
    disableGridOnPanning = CFG_Appearance::o_GridDisableWithinPan;
    drawIsometricVerticalsAlways = CFG_Appearance::o_GridDrawIsoVerticalForTop;
    gridPointSize = CFG_Grid::o_GridPointsPointSize;
}
