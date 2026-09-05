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

#ifndef LC_SETTINGS_APPEARANCE_H
#define LC_SETTINGS_APPEARANCE_H

#include "dxf_format.h"
#include "lc_setting.h"
#include "rs.h"


namespace CFG_Appearance {
    inline const LC_SettingsGroupBase Group("Appearance");

    inline const LC_Setting<bool> o_DraftMode(&Group, "DraftMode", false);
    inline const LC_Setting<bool> o_DraftLinesMode(&Group, "DraftLinesMode", false);
    inline const LC_Setting<bool> o_StatusBarVisible(&Group, "StatusBarVisible", false);
    inline const LC_Setting<bool> o_FullscreenMode(&Group, "FullscreenMode", false);

    inline const LC_Setting<bool> o_ScrollBars(&Group, "ScrollBars", true);
    inline const LC_Setting<bool> o_Antialiasing(&Group, "Antialiasing", false);
    inline const LC_Setting<bool> o_ClassicRenderer(&Group, "ClassicRenderer", true);
    inline const LC_Setting<bool> o_Autopanning(&Group, "Autopanning", false);
    inline const LC_Setting<bool> o_FirstTimeNoZoom(&Group, "FirstTimeNoZoom", false);
    inline const LC_Setting<bool> o_PanOnZoom(&Group, "PanOnZoom", false);
    inline const LC_Setting<int> o_ScrollZoomFactor(&Group, "ScrollZoomFactor", 1137);
    inline const LC_Setting<bool> o_VisualizeHovering(&Group, "VisualizeHovering", true);
    inline const LC_Setting<bool> o_VisualizeHoveringRefPoints(&Group, "VisualizeHoveringRefPoints", true);
    inline const LC_Setting<bool> o_VisualizePreviewRefPoints(&Group, "VisualizePreviewRefPoints", true);

    // --- Dialogs & Tabs ---
    inline const LC_Setting<bool> o_PersistDialogPositions(&Group, "PersistDialogPositions", true);
    inline const LC_Setting<bool> o_PersistDialogRestoreSizeOnly(&Group, "PersistDialogRestoreSizeOnly", false);
    inline const LC_Setting<bool> o_ShowCloseButton(&Group, "ShowCloseButton", true);
    inline const LC_Setting<bool> o_ShowCloseButtonActiveOnly(&Group, "ShowCloseButtonActiveOnly", true);
    inline const LC_Setting<bool> o_ShowActionIconInOptions(&Group, "ShowActionIconInOptions", true);
    inline const LC_Setting<bool> o_ShowEntityIDs(&Group, "ShowEntityIDs", false);
    inline const LC_Setting<bool> o_ShowKeyboardShortcutsInTooltips(&Group, "ShowKeyboardShortcutsInTooltips", true);
    // - menus
    inline const LC_Setting<bool> o_AllowMenusTearOff(&Group, "AllowMenusTearOff", true);
    inline const LC_Setting<bool> o_MainMenuVisible(&Group, "MainMenuVisible", true);

    // --- Sizing & Metrics ---
    inline const LC_Setting<int> o_EntityHandleSize(&Group, "EntityHandleSize", 4);
    inline const LC_Setting<int> o_RelZeroMarkerRadius(&Group, "RelZeroMarkerRadius", 5);
    inline const LC_Setting<int> o_ZeroShortAxisMarkSize(&Group, "ZeroShortAxisMarkSize", 20);
    inline const LC_Setting<int> o_ZeroMarkerSize(&Group, "ZeroMarkerSize", 30);
    inline const LC_Setting<int> o_ZeroMarkerFontSize(&Group, "ZeroMarkerFontSize", 10);
    inline const LC_Setting<QString> o_ZeroMarkerFontName(&Group, "ZeroMarkerFontName", "Verdana");

    // --- Draft Mode Marker ---
    inline const LC_Setting<bool> o_ShowDraftModeMarker(&Group, "ShowDraftModeMarker", true);
    inline const LC_Setting<QString> o_DraftMarkerFontName(&Group, "DraftMarkerFontName", "Verdana");
    inline const LC_Setting<int> o_DraftMarkerFontSize(&Group, "DraftMarkerFontSize", 10);
    inline const LC_Setting<bool> o_IgnoreDraftForHighlight(&Group, "IgnoreDraftForHighlight", false);

    // --- Coordinate & Angles System ---
    inline const LC_Setting<bool> o_AnglesBasisMarkEnabled(&Group, "AnglesBasisMarkEnabled", true);

    enum ShowAnglesBaseMarkType{
        SHOW_ALWAYS = 0,
        SHOW_IF_NON_DEFAULTS
    };
    inline const LC_Setting<int> o_AnglesBasisMarkPolicy(&Group, "AnglesBasisMarkPolicy", SHOW_ALWAYS);
    inline const LC_Setting<int> o_AngleSnapMarkerSize(&Group, "AngleSnapMarkerSize", 20);
    inline const LC_Setting<bool> o_ModifyOnViewChange(&Group, "ModifyOnViewChange", true);
    inline const LC_Setting<bool> o_SnapGridIgnoreIfNoGrid(&Group, "SnapGridIgnoreIfNoGrid", false);

    inline const LC_Setting<int> o_RefPointType(&Group, "RefPointType", DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
    inline const LC_Setting<QString> o_RefPointSize(&Group, "RefPointSize", "2.0");
    inline const LC_Setting<RS2::LineType> o_SelectionOverlayLineType(&Group, "selection_overlay_line_type", RS2::SolidLine);
    inline const LC_Setting<RS2::LineType> o_SelectionOverlayInvertedLineType(&Group, "selection_overlay_inverted_line_type", RS2::DashLine);

    // --- Languages ---
    inline const LC_Setting<QString> o_Language(&Group, "Language", "en");
    inline const LC_Setting<QString> o_LanguageCmd(&Group, "LanguageCmd", "en");

    // --- Snapping Indicators ---
    inline const LC_Setting<bool> o_CursorHidingWhenSnapping(&Group, "cursor_hiding", false);
    inline const LC_Setting<bool> o_showSnapOptionsInSnapToolbar(&Group, "showSnapOptionsInSnapToolbar", false);

    // --- Coordinate Origin Zero Markers ---
    inline const LC_Setting<bool> o_ShowUCSZeroMarker(&Group, "ShowUCSZeroMarker", false);
    inline const LC_Setting<bool> o_ShowWCSZeroMarker(&Group, "ShowWCSZeroMarker", true);

    // --- Relative Zero and Axis Line Extensions ---
    inline const LC_Setting<bool> o_HideRelativeZero(&Group, "hideRelativeZero", false);
    inline const LC_Setting<bool> o_ExtendAxisLines(&Group, "ExtendAxisLines", false);
    inline const LC_Setting<int> o_ExtendModeXAxis(&Group, "ExtendModeXAxis", 0);
    inline const LC_Setting<int> o_ExtendModeYAxis(&Group, "ExtendModeYAxis", 0);

    // --- Grid General Settings ---
    inline const LC_Setting<bool> o_UnitlessGrid(&Group, "UnitlessGrid", false);
    inline const LC_Setting<bool> o_GridRenderSimple(&Group, "GridRenderSimple", false);
    inline const LC_Setting<bool> o_GridDisableWithinPan(&Group, "GridDisableWithinPan", false);
    inline const LC_Setting<bool> o_ScaleGrid(&Group, "ScaleGrid", true);
    inline const LC_Setting<bool> o_GridDraw(&Group, "GridDraw", true);
    inline const LC_Setting<bool> o_metaGridDraw(&Group, "metaGridDraw", true);
    inline const LC_Setting<bool> o_GridDrawIsoVerticalForTop(&Group, "GridDrawIsoVerticalForTop", true);
    inline const LC_Setting<int> o_MetaGridEvery(&Group, "MetaGridEvery", 10);
    inline const LC_Setting<int> o_MinGridSpacing(&Group, "MinGridSpacing", 10);

    // --- Preview Option Limits ---
    inline const LC_Setting<int> o_MaxPreview(&Group, "MaxPreview", 100);

    inline const LC_Setting<int> o_OverlaysScreenLineWidth(&Group, "overlayScreenLineWidth", 1);

    // ucs
    inline const LC_Setting<int> o_UCSApplyPolicy(&Group, "UCSApplyPolicy", 0);
    inline const LC_Setting<int> o_UCSHighlightBlinkCount(&Group, "UCSHighlightBlinkCount", 10);
    inline const LC_Setting<int> o_UCSHighlightBlinkDelay(&Group, "UCSHighlightBlinkDelay", 250);

    inline const LC_Setting<QString> o_ActiveShortcutsScheme(&Group, "ActiveShortcutsScheme", "");
}

#endif
