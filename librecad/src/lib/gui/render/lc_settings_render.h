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

#ifndef LC_SETTINGS_RENDER_H
#define LC_SETTINGS_RENDER_H

#include "lc_setting.h"

namespace CFG_Render {
    inline const LC_SettingsGroupBase Group("Render");

    inline const LC_Setting<int> o_MinRenderableTextHeightPx(&Group, "MinRenderableTextHeightPx", 4);
    inline const LC_Setting<int> o_MinArcRadius(&Group, "MinArcRadius", 80);
    inline const LC_Setting<int> o_MinCircleRadius(&Group, "MinCircleRadius", 200);
    inline const LC_Setting<int> o_MinLineLen(&Group, "MinLineLen", 200);
    inline const LC_Setting<int> o_MinEllipseMajor(&Group, "MinEllipseMajor", 200);
    inline const LC_Setting<int> o_MinEllipseMinor(&Group, "MinEllipseMinor", 200);
    inline const LC_Setting<bool> o_DrawTextsAsDraftInPanning(&Group, "DrawTextsAsDraftInPanning", true);
    inline const LC_Setting<bool> o_DrawTextsAsDraftInPreview(&Group, "DrawTextsAsDraftInPreview", true);
    inline const LC_Setting<bool> o_ArcRenderInterpolate(&Group, "ArcRenderInterpolate", false);
    inline const LC_Setting<bool> o_ArcRenderInterpolateSegmentFixed(&Group, "ArcRenderInterpolateSegmentFixed", true);
    inline const LC_Setting<int> o_ArcRenderInterpolateSegmentAngle(&Group, "ArcRenderInterpolateSegmentAngle", 500);
    inline const LC_Setting<int> o_ArcRenderInterpolateSegmentSagitta(&Group, "ArcRenderInterpolateSegmentSagitta", 90);
    inline const LC_Setting<bool> o_CircleRenderAsArcs(&Group, "CircleRenderAsArcs", false);
    inline const LC_Setting<int> o_FontLettersColumnsCount(&Group, "FontLettersColumnsCount", 10);
}

#endif
