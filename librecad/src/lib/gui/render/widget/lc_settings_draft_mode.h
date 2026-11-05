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

#ifndef LC_SETTINGS_DRAFT_MODE_H
#define LC_SETTINGS_DRAFT_MODE_H
#include "lc_setting.h"

namespace CFG_DraftMode {
    inline const LC_SettingsGroupBase Group("DraftMode");

    inline const LC_Setting<bool> o_DraftMode(&Group, "DraftMode", false);
    inline const LC_Setting<bool> o_DraftLinesMode(&Group, "DraftLinesMode", false);

    // --- Draft Mode Marker ---
    inline const LC_Setting<QString> o_DraftMarkerFontName(&Group, "DraftMarkerFontName", "Verdana");
    inline const LC_Setting<int> o_DraftMarkerFontSize(&Group, "DraftMarkerFontSize", 10);
    inline const LC_Setting<bool> o_ShowDraftModeMarker(&Group, "ShowDraftModeMarker", true);
    inline const LC_Setting<bool> o_IgnoreDraftForHighlight(&Group, "IgnoreDraftForHighlight", false);

    inline const LC_Setting<QColor> o_ColorDraftModeMarker(&Group, "draft_mode_marker", QColor("#A54747"));

    // --- draft mode
    inline const LC_Setting<double> o_DraftModeScreenLineWidth(&Group, "DraftScreenLineWidth", 1);
}

#endif
