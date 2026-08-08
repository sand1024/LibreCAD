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
    inline const LC_Setting<QColor> o_BackgroundColor(&Group, "background", QColor("#000000"));
    inline const LC_Setting<QColor> o_ForegroundBWColor(&Group, "foreground", QColor("#FFFFFF"));
    inline const LC_Setting<QColor> o_GridPointsColor(&Group, "grid", QColor("#404040" /*#736F6E"*/));
    inline const LC_Setting<QColor> o_GridLinesColor(&Group, "gridLines", "#55557F");
    inline const LC_Setting<QColor> o_MetaGridColor(&Group, "meta_grid", QColor("#404040"));
    inline const LC_Setting<QColor> o_MetaGridLinesColor(&Group, "meta_grid_lines", QColor("#55557f"));
    inline const LC_Setting<QColor> o_SelectedEntityColor(&Group, "select", QColor("#A54747"));
    inline const LC_Setting<QColor> o_HighlightedEntityColor(&Group, "highlight", QColor("#739373"));

    // Endpoint handles
    inline const LC_Setting<QColor> o_EntityHandleStartColor(&Group, "start_handle", QColor("Cyan"));
    inline const LC_Setting<QColor> o_EntityHandleInternalColor(&Group, "handle", QColor("Blue"));
    inline const LC_Setting<QColor> o_EntityHandleEndColor(&Group, "end_handle", QColor("Blue"));

    inline const LC_Setting<QColor> o_RelativeZeroColor(&Group, "relativeZeroColor", QColor("Red"));
    inline const LC_Setting<QColor> o_PreviewEntitiesColor(&Group, "previewEntitiesColor", QColor("739373"));
    inline const LC_Setting<QColor> o_PreviewReferencesColor(&Group, "previewReferencesColor", QColor("Yellow"));
    inline const LC_Setting<QColor> o_PreviewReferencesHighlightColor(&Group, "previewReferencesHighlightColor", QColor("Green"));
    inline const LC_Setting<QColor> o_SnapIndicator(&Group, "snap_indicator", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_SnapIndicatorLines(&Group, "snap_indicator_lines", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_XAxisColor(&Group, "grid_x_axisColor", QColor("red"));
    inline const LC_Setting<QColor> o_YAxisColor(&Group, "grid_y_axisColor", QColor("green"));

    // Selection Overlay Box
    inline const LC_Setting<QColor> o_OverlayBoxLineColor(&Group, "overlay_box_line", QColor("#3232ff"));
    inline const LC_Setting<QColor> o_OverlayBoxFillColor(&Group, "overlay_box_fill", QColor("#0909ff"));
    inline const LC_Setting<QColor> o_OverlayBoxLineInvertedColor(&Group, "overlay_box_line_inv", QColor("#32ff32"));
    inline const LC_Setting<QColor> o_OverlayBoxFillInvertedColor(&Group, "overlay_box_fill_inv", QColor("#09ff09"));
    inline const LC_Setting<int> o_OverlayBoxTransparency(&Group, "overlay_box_transparency", 90);

    // Informational Cursor overlays
    inline const LC_Setting<QColor> o_InfoOverlayAbsoluteCoordinatesColor(&Group, "info_overlay_absolute", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_InfoOverlaySnapInfoColor(&Group, "info_overlay_snap", QColor("Cyan"));
    inline const LC_Setting<QColor> o_InfoOverlayPromptColor(&Group, "info_overlay_prompt", QColor("Gray"));
    inline const LC_Setting<QColor> o_InfoOverlayRelativeColor(&Group, "info_overlay_relative", QColor("Orange"));

    // Angle Basis Ray
    inline const LC_Setting<QColor> o_AnglesBasisDirectionIndicatorColor(&Group, "angles_basis_direction", QColor("#017CFF"));
    inline const LC_Setting<QColor> o_AnglesBasisAngleRayColor(&Group, "angles_basis_angleray", QColor("#00FFFF"));

    inline const LC_Setting<QColor> o_DraftModeMarkerColor(&Group, "draft_mode_marker", QColor("#A54747"));

    // Visual Snaps
    inline const LC_Setting<QColor> o_VisualSnapGuideEntitiesColor(&Group, "VisualSnapGuideEntitiesColor", QColor("#00FF00"));
    inline const LC_Setting<QColor> o_VisualSnapVertexesColor(&Group, "VisualSnapVertexesColor", QColor("#00ff00"));
    inline const LC_Setting<QColor> o_VisualSnapProjectedSnapColor(&Group, "VisualSnapProjectedSnapColor", QColor("#00ffff"));
    inline const LC_Setting<QColor> o_VisualSnapDocumentEntitiesColor(&Group, "VisualSnapDocumentEntitiesColor", QColor("#FF00FF"));

    // Relative Assistant Popup
    inline const LC_Setting<QColor> o_RelativePositionAssistantBackgroundColor(&Group, "RelativePositionAssistantBackground", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_RelativePositionAssistantTextColor(&Group, "RelativePositionAssistantText", QColor("#1E90FF"));

}

#endif
