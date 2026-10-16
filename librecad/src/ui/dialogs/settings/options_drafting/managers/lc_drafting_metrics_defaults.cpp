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

#include "lc_drafting_metrics_defaults.h"

#include <initializer_list>
#include <utility>

#include "lc_setting.h"
#include "lc_settings_appearance.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "rs_settings.h"

void LC_DraftingMetricsDefaults::applyStandardDefaults() {
    using namespace CFG_Appearance;
    using namespace CFG_Grid;
    using namespace CFG_Snap;
    using namespace CFG_VisualSnap;
    using namespace CFG_InfoOverlayCursor;
    using namespace CFG_RelativePositionAssistant;

    const std::initializer_list<const LC_SettingBase*> settings = {
        &o_EntityHandleSize,
        &o_ZeroMarkerSize,
        &o_ZeroMarkerFontSize,
        &o_ZeroShortAxisMarkSize,
        &o_RelZeroMarkerRadius,
        &o_AngleSnapMarkerSize,
        &o_DraftMarkerFontSize,
        &o_IndicatorShapeSize,
        &o_IndicatorScreenLinesLineWidth,
        &o_RefPointSize,
        &o_VSProjectedSnapSize,
        &o_VSHighlightedVertexSize,
        &o_VSVertexSize,
        &o_VSGuidingEntitiesLineWidth,
        &o_VSGuidingPointsLineWidth,
        &o_VSGuidingEntityLabelFontSize,
        &o_VSGuidingLabelOffsetPx,
        &o_OverlaysScreenLineWidth,
        &o_GridLinesLineWidth,
        &o_MetaGridLinesLineWidth,
        &o_MetaGridPointsLineWidth,
        &o_GridPointsPointSize,
        &o_FontSize,
        &o_OffsetFromCursor,
        &o_AssistantFontSize
    };

    for (const auto* setting : settings) {
        if (setting != nullptr) {
            setting->setVariant(setting->defaultVariant());
        }
    }

    RS_Settings::instance()->emitOptionsChanged();
}

void LC_DraftingMetricsDefaults::applyHiDpiDefaults() {
    using namespace CFG_Appearance;
    using namespace CFG_Grid;
    using namespace CFG_Snap;
    using namespace CFG_VisualSnap;
    using namespace CFG_InfoOverlayCursor;
    using namespace CFG_RelativePositionAssistant;

    const std::initializer_list<std::pair<const LC_SettingBase*, QVariant>> hiDpiTable = {
        // Interactive Handles & Markers
        { &o_EntityHandleSize, 10 },
        { &o_ZeroMarkerSize, 18 },
        { &o_ZeroMarkerFontSize, 13 },
        { &o_ZeroShortAxisMarkSize, 10 },
        { &o_RelZeroMarkerRadius, 10 },
        { &o_AngleSnapMarkerSize, 24 },
        { &o_DraftMarkerFontSize, 20 },

        // Snap & Visual Snap Indicators
        { &o_IndicatorShapeSize, 18 },
        { &o_IndicatorScreenLinesLineWidth, 2 },
        { &o_RefPointSize, "4.0" },
        { &o_VSProjectedSnapSize, 14 },
        { &o_VSHighlightedVertexSize, 14 },
        { &o_VSVertexSize, 10 },
        { &o_VSGuidingEntitiesLineWidth, 2 },
        { &o_VSGuidingPointsLineWidth, 4 },
        { &o_VSGuidingEntityLabelFontSize, 13 },
        { &o_VSGuidingLabelOffsetPx, 16 },

        // Screen Line Widths & Grid Points
        { &o_OverlaysScreenLineWidth, 2 },
        { &o_GridLinesLineWidth, 2 },
        { &o_MetaGridLinesLineWidth, 3 },
        { &o_MetaGridPointsLineWidth, 3 },
        { &o_GridPointsPointSize, 4 },

        // Overlay Fonts & Offsets
        { &o_FontSize, 14 },
        { &o_OffsetFromCursor, 24 },
        { &o_AssistantFontSize, 14 }
    };

    for (const auto& entry : hiDpiTable) {
        if (entry.first != nullptr) {
            entry.first->setVariant(entry.second);
        }
    }

    RS_Settings::instance()->emitOptionsChanged();
}
