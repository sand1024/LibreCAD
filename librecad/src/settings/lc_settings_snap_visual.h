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

#ifndef LC_SETTINGS_SNAP_VISUAL_H
#define LC_SETTINGS_SNAP_VISUAL_H

#include "lc_setting.h"
#include "rs.h"

namespace CFG_VisualSnap {
    inline const LC_SettingsGroupBase Group("VisualSnap");
    inline const LC_Setting<int> o_VSVertexSize(&Group, "VSVertexSize", 6);
    inline const LC_Setting<int> o_VSProjectedSnapSize(&Group, "VSProjectedSnapSize", 8);
    inline const LC_Setting<int> o_VSHighlightedVertexSize(&Group, "VSHighlightedVertexSize", 10);
    inline const LC_Setting<int> o_VSSnapPointAddingDelay(&Group, "VSSnapPointAddingDelay", 300);
    inline const LC_Setting<int> o_VSVertexAddingDelay(&Group, "VSVertexAddingDelay", 1200);
    inline const LC_Setting<int> o_VSDocEntityAddingDelay(&Group, "VSDocEntityAddingDelay", 1500);
    inline const LC_Setting<int> o_VSGuidingEntitiesCatchDistance(&Group, "VSGuidingEntitiesCatchDistance", 24);
    inline const LC_Setting<bool> o_VSAngleSnapStepRaysVertexes(&Group, "VSAngleSnapStepRaysVertexes", true);
    inline const LC_Setting<bool> o_VSAngleSnapStepRaysRelative(&Group, "VSAngleSnapStepRaysRelative", true);
    inline const LC_Setting<bool> o_VSSnapAutoAddSnapPoint(&Group, "VSSnapAutoAddSnapPoint", true);
    inline const LC_Setting<bool> o_VSSnapAutoAddLastSnapPointOnly(&Group, "VSSnapAutoAddLastSnapPointOnly", true);
    inline const LC_Setting<bool> o_VSSnapManualAddingWithCTRL(&Group, "VSSnapManualAddingWithCTRL", false);
    inline const LC_Setting<bool> o_VSVertexVertexDistanceCircles(&Group, "VSVertexVertexDistanceCircles", true);
    inline const LC_Setting<bool> o_VSVertexVertexDistanceTangents(&Group, "VSVertexVertexDistanceTangents", true);
    inline const LC_Setting<bool> o_VSGuidingEntitiesShowLabels(&Group, "VSGuidingEntitiesShowLabels", true);
    inline const LC_Setting<int> o_VSGuidingEntityLabelFontSize(&Group, "VSGuidingEntityLabelFontSize", 10);
    inline const LC_Setting<bool> o_VSClearSolutionByRMB(&Group, "VSClearSolutionByRMB", false);
    inline const LC_Setting<int> o_VSGuidingLabelOffsetPx(&Group, "VSGuidingLabelOffsetPx", 50);
    inline const LC_Setting<bool> o_VSShowNotSnappableGuides(&Group, "VSShowNotSnappableGuides", false);
    inline const LC_Setting<RS2::LineType> o_VSGuidingEntitiesLineType(&Group, "VSGuidingEntitiesLineType", RS2::DashLine2);
    inline const LC_Setting<RS2::LineType> o_VSActiveGuidingEntityLineType(&Group, "VSActiveGuidingEntitiesLineType", RS2::DashLineTiny);
    inline const LC_Setting<RS2::LineType> o_VSDocGuidingEntitiesLineType(&Group, "VSDocGuidingEntitiesLineType", RS2::SolidLine);
    inline const LC_Setting<int> o_VSGuidingEntitiesLineWidth(&Group, "VSGuidingEntitiesScreenWidth", 2);
    inline const LC_Setting<int> o_VSGuidingPointsLineWidth(&Group, "VSGuidingPointsScreenWidth", 2);
}


#endif
