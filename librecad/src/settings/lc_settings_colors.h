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


#ifndef LC_SETTINGS_COLORS_H
#define LC_SETTINGS_COLORS_H

#include "lc_setting.h"

namespace CFG_Colors {
    // Symmetrical Group Instance - declared exactly once
    inline const LC_SettingsGroupBase Group("Colors");

    // Decoupled, type-safe QColor settings with the 'o_' prefix
    inline const LC_Setting<QColor> o_Background(&Group, "background", QColor("#000000"));
    inline const LC_Setting<QColor> o_Foreground(&Group, "foreground", QColor("#FFFFFF"));
    inline const LC_Setting<QColor> o_Grid(&Group, "grid", QColor("#404040" /*#736F6E"*/));
    inline const LC_Setting<QColor> o_GridLines(&Group, "gridLines", "#55557F");
    inline const LC_Setting<QColor> o_MetaGrid(&Group, "meta_grid", QColor("#404040"));
    inline const LC_Setting<QColor> o_MetaGridLines(&Group, "meta_grid_lines", QColor("#55557f"));
    inline const LC_Setting<QColor> o_Select(&Group, "select", QColor("#A54747"));
    inline const LC_Setting<QColor> o_Highlight(&Group, "highlight", QColor("#739373"));

    // Endpoint handles
    inline const LC_Setting<QColor> o_StartHandle(&Group, "start_handle", QColor("Cyan"));
    inline const LC_Setting<QColor> o_Handle(&Group, "handle", QColor("Blue"));
    inline const LC_Setting<QColor> o_EndHandle(&Group, "end_handle", QColor("Blue"));

    inline const LC_Setting<QColor> o_RelativeZeroColor(&Group, "relativeZeroColor", QColor("Red"));
    inline const LC_Setting<QColor> o_PreviewEntitiesColor(&Group, "previewEntitiesColor", QColor("739373"));
    inline const LC_Setting<QColor> o_PreviewReferencesColor(&Group, "previewReferencesColor", QColor("Yellow"));
    inline const LC_Setting<QColor> o_PreviewReferencesHighlightColor(&Group, "previewReferencesHighlightColor", QColor("Green"));
    inline const LC_Setting<QColor> o_SnapIndicator(&Group, "snap_indicator", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_SnapIndicatorLines(&Group, "snap_indicator_lines", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_GridXAxis(&Group, "grid_x_axisColor", QColor("red"));
    inline const LC_Setting<QColor> o_GridYAxis(&Group, "grid_y_axisColor", QColor("green"));

    // Selection Overlay Box
    inline const LC_Setting<QColor> o_OverlayBoxLine(&Group, "overlay_box_line", QColor("#3232ff"));
    inline const LC_Setting<QColor> o_OverlayBoxFill(&Group, "overlay_box_fill", QColor("#0909ff"));
    inline const LC_Setting<QColor> o_OverlayBoxLineInverted(&Group, "overlay_box_line_inv", QColor("#32ff32"));
    inline const LC_Setting<QColor> o_OverlayBoxFillInverted(&Group, "overlay_box_fill_inv", QColor("#09ff09"));
    inline const LC_Setting<int> o_OverlayBoxTransparency(&Group, "overlay_box_transparency", 90);

    // Informational Cursor overlays
    inline const LC_Setting<QColor> o_InfoOverlayAbsolute(&Group, "info_overlay_absolute", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_InfoOverlaySnap(&Group, "info_overlay_snap", QColor("Cyan"));
    inline const LC_Setting<QColor> o_InfoOverlayPrompt(&Group, "info_overlay_prompt", QColor("Gray"));
    inline const LC_Setting<QColor> o_InfoOverlayRelative(&Group, "info_overlay_relative", QColor("Orange"));

    // Angle Basis Ray
    inline const LC_Setting<QColor> o_AnglesBasisDirection(&Group, "angles_basis_direction", QColor("#017CFF"));
    inline const LC_Setting<QColor> o_AnglesBasisAngleRay(&Group, "angles_basis_angleray", QColor("#00FFFF"));

    inline const LC_Setting<QColor> o_DraftModeMarker(&Group, "draft_mode_marker", QColor("#A54747"));

    // Visual Snaps
    inline const LC_Setting<QColor> o_VisualSnapGuideEntitiesColor(&Group, "VisualSnapGuideEntitiesColor", QColor("#00FF00"));
    inline const LC_Setting<QColor> o_VisualSnapVertexesColor(&Group, "VisualSnapVertexesColor", QColor("#00ff00"));
    inline const LC_Setting<QColor> o_VisualSnapProjectedSnapColor(&Group, "VisualSnapProjectedSnapColor", QColor("#00ffff"));
    inline const LC_Setting<QColor> o_VisualSnapDocumentEntitiesColor(&Group, "VisualSnapDocumentEntitiesColor", QColor("#FF00FF"));

    // Relative Assistant Popup
    inline const LC_Setting<QColor> o_RelativePositionAssistantBackground(&Group, "RelativePositionAssistantBackground", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_RelativePositionAssistantText(&Group, "RelativePositionAssistantText", QColor("#1E90FF"));

}

#endif
