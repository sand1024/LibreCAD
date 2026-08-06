
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

#ifndef LC_GridOptions_H
#define LC_GridOptions_H
#include "rs_color.h"

struct LC_GridOptions{
    RS_Color gridColorLine;
    RS_Color gridColorPoint;
    RS_Color metaGridColor;
    RS2::LineType gridLineType{};
    RS2::LineType metaGridLineType{};
    int gridWidthPx = 1;
    int metaGridLineWidthPx = 1;
    bool drawMetaGrid = true;
    bool disableGridOnPanning = false;
    bool drawIsometricVerticalsAlways = true; // fixme - complete initialization
    bool simpleGridRendering = false;
    bool drawLines = false;
    bool drawGrid = true;
    int gridPointSize = 2;
    Qt::PenCapStyle capStyle = Qt::PenCapStyle::SquareCap; //Qt::PenCapStyle::RoundCap - is too slow;

    int forcedGridType = -1; // ability to override setting for displaying grid in preview
    void loadSettings();
};

#endif
