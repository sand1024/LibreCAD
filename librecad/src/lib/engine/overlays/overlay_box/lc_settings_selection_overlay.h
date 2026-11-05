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

#ifndef LC_SETTINGS_SELECTION_OVERLAY_H
#define LC_SETTINGS_SELECTION_OVERLAY_H
#include "lc_setting.h"

namespace  CFG_OverlaySelection {
    inline const LC_SettingsGroupBase Group("OverlayEntitiesSelection");

    // Selection Overlay
    inline const LC_Setting<QColor> o_ColorSelectionOverlayLine(&Group, "ColorNormalLine", QColor("#3232ff"));
    inline const LC_Setting<QColor> o_ColorSelectionOverlayFill(&Group, "ColorNormalBoxFill", QColor("#0909ff"));
    inline const LC_Setting<QColor> o_ColorSelectionOverlayInvertedLine(&Group, "ColorInvertedLine", QColor("#32ff32"));
    inline const LC_Setting<QColor> o_ColorSelectionOverlayInvertedFill(&Group, "ColorInvertedFill", QColor("#09ff09"));
    inline const LC_Setting<int>    o_SelectionOverlayTransparency(&Group, "OverlayTransparency", 90);

    inline const LC_Setting<RS2::LineType> o_SelectionOverlayLineType(&Group, "NormalLineType", RS2::SolidLine);
    inline const LC_Setting<RS2::LineType> o_SelectionOverlayInvertedLineType(&Group, "InvertedLineType", RS2::DashLine);

}

#endif
