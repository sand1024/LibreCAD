/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_visual_snap_options.h"

#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_snap_visual.h"

void LC_VisualSnapOptions::load() {
    {
        using namespace CFG_VisualSnap;
        vertexSizeNormal = o_VSVertexSize;
        vertexSizeProjected = o_VSProjectedSnapSize;
        vertexSizeHighlighted = o_VSHighlightedVertexSize;
        delayMsSnapVertex = o_VSSnapPointAddingDelay;
        delayMsProjectedSnap = o_VSVertexAddingDelay;
        delayMsDocumentEntity = o_VSDocEntityAddingDelay;

        createAngleStepRaysForVertexes = o_VSAngleSnapStepRaysVertexes;
        createAngleStepRaysForEntitiesEndpoints = o_VSAngleSnapStepRaysRelative;
        createVertexVertexDistanceCircles = o_VSVertexVertexDistanceCircles;
        createVertexVertexDistanceCirclesTangents = o_VSVertexVertexDistanceTangents;

        autoAddSnappedPointToVisualSnap = o_VSSnapAutoAddSnapPoint;
        autoAddGuidesForLastSnapOnly = o_VSSnapAutoAddLastSnapPointOnly;
        manualVertexAddingRequiresCTRL = o_VSSnapManualAddingWithCTRL;
        guidingEntitiesSnapDistance = o_VSGuidingEntitiesCatchDistance;

        showGuidingEntitiesLabels = o_VSGuidingEntitiesShowLabels;
        guidingEntitiesFontSize = o_VSGuidingEntityLabelFontSize;

        allowClearingVisualSnapByRMB = o_VSClearSolutionByRMB;

        baseLabelOffsetPx = o_VSGuidingLabelOffsetPx;
        showNotSnappableGuides = o_VSShowNotSnappableGuides;
    }
    {
        using namespace CFG_InfoOverlayCursor;
        const QString fontName = o_FontName;
        guidingEntitiesFont = QFont(fontName, guidingEntitiesFontSize);

        guidingEntitiesFontActive = QFont(fontName, guidingEntitiesFontSize + 2);
        guidingEntitiesFontActive.setBold(true);
    }
}
