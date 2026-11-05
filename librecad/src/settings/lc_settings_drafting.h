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

namespace CFG_DraftingAreaStyling {
    inline const LC_SettingsGroupBase Group("DraftingArea");

    inline const LC_Setting<bool> o_ScrollBarsVisibleForDraftingArea(&Group, "ScrollBars", true);
    inline const LC_Setting<bool> o_ScrollBarsContentBand(&Group, "ScrollBarContentBand", true);
    inline const LC_Setting<bool> o_ScrollBarsContentToolTip(&Group, "ScrollBarContentTooltip", true);

    inline const LC_Setting<bool> o_Antialiasing(&Group, "Antialiasing", false);
    inline const LC_Setting<bool> o_ClassicRenderer(&Group, "ClassicRenderer", true);

    inline const LC_Setting<int> o_EntityHandleSize(&Group, "EntityHandleSize", 8);
    inline const LC_Setting<double> o_OverlaysScreenLineWidth(&Group, "OverlayScreenLineWidth", 1.0);

    // selection and highlight
    inline const LC_Setting<double> o_SelectedEntityScreenLineWidth(&Group, "SelectedScreenLineWidth", 0.0);
    inline const LC_Setting<double> o_HighlightedEntityScreenLineWidth(&Group, "HighlightedScreenLineWidth", 0.0);

    inline const LC_Setting<RS2::LineType> o_SelectedEntityLineType(&Group, "SelectedEntityLineType", RS2::DashLineTiny);
    inline const LC_Setting<RS2::LineType> o_HighlightedEntityLineType(&Group, "HighlightedEntityLineType", RS2::SolidLine);

    inline const LC_Setting<QColor> o_ColorBackground(&Group, "Background", QColor("#000000"));
    inline const LC_Setting<QColor> o_ColorForegroundBW(&Group, "Foreground", QColor("#FFFFFF"));
    inline const LC_Setting<QColor> o_ColorSelectedEntity(&Group, "SelectedEntity", QColor("#A54747"));
    inline const LC_Setting<QColor> o_ColorHighlightedEntity(&Group, "HighlightedEntity", QColor("#739373"));

    // Endpoint handles
    inline const LC_Setting<QColor> o_ColorEntityHandleStart(&Group, "HandleStart", QColor("Cyan"));
    inline const LC_Setting<QColor> o_ColorEntityHandleInternal(&Group, "HandleInternal", QColor("Blue"));
    inline const LC_Setting<QColor> o_ColorEntityHandleEnd(&Group, "HandleEnd", QColor("Blue"));
}

namespace CFG_DraftingOperations {
    inline const LC_SettingsGroupBase Group("DraftingOperations");
    // operations
    inline const LC_Setting<bool> o_ModifyDocumentOnViewOnlyChange(&Group, "ModifyDocumentOnViewOnlyChange", true);
    inline const LC_Setting<bool> o_AdHockMovingEndsByMouseClick(&Group, "AdHockMovingEndsByMouseClick", true);
    inline const LC_Setting<bool> o_AutoZoomDrawingAfterOpening(&Group, "AutoZoomDrawingAfterOpening", false);

    inline const LC_Setting<bool> o_ModifyEntitiesToActiveLayer(&Group, "ModifyEntitiesToActiveLayer", false);
    inline const LC_Setting<bool> o_KeepModifiedEntitiesSelected(&Group, "KeepModifiedEntitiesSelected", true);

    // ucs
    inline const LC_Setting<int> o_UCSApplyPolicy(&Group, "UCSApplyPolicy", 0);
}

namespace CFG_DraftingMouseWheel {
    inline const LC_SettingsGroupBase Group("DraftingMouseWheel");

    inline const LC_Setting<bool> o_WheelScrollInvertH(&Group, "WheelScrollInvertH", false);
    inline const LC_Setting<bool> o_WheelScrollInvertV(&Group, "WheelScrollInvertV", false);
    inline const LC_Setting<bool> o_InvertZoomDirection(&Group, "InvertZoomDirection", false);
    inline const LC_Setting<bool> o_NonLinearZoomFactor(&Group, "NonLinearZoomFactor", false);
    inline const LC_Setting<int>  o_ScrollZoomFactor(&Group, "ScrollZoomFactor", 1137);
}

namespace CFG_DraftingZoomAndPan {
    inline const LC_SettingsGroupBase Group("DraftingScrollAnPan");

    inline const LC_Setting<bool> o_FirstTimeNoZoom(&Group, "FirstTimeNoZoom", false);
    inline const LC_Setting<bool> o_PanOnZoom(&Group, "PanOnZoom", false);
    inline const LC_Setting<bool> o_Autopanning(&Group, "Autopanning", false);
}

#endif
