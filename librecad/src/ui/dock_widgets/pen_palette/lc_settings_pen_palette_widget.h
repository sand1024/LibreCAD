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

#ifndef LC_SETTINGS_PEN_PALETTE_WIDGET_H
#define LC_SETTINGS_PEN_PALETTE_WIDGET_H

#include "lc_peninforegistry.h"
#include "lc_setting.h"

enum PenPaletteDoubleClickMode{
    DOUBLE_CLICK_DOES_NOTHING,
    DOUBLE_CLICK_SELECT_ENTITIES_BY_ATTRIBUTES_PEN,
    DOUBLE_CLICK_SELECT_ENTITIES_BY_DRAWING_PEN
};


namespace CFG_WidgetPenPallette {
    inline const LC_SettingsGroupBase Group("Widget.PenPalette");

    inline const LC_Setting<QColor> o_matchedItemBgColor(&Group, "matchedItemBgColor", QColor("blue"));

    inline const LC_Setting<bool> o_showGrid(&Group, "showGrid", true);
    inline const LC_Setting<bool> o_showToolTip(&Group, "showToolTip", true);
    inline const LC_Setting<bool> o_showColorNameCol(&Group, "showColorNameCol", false);
    inline const LC_Setting<bool> o_showColorIconCol(&Group, "showColorIconCol", true);
    inline const LC_Setting<bool> o_showLineTypeNameCol(&Group, "showLineTypeNameCol", false);
    inline const LC_Setting<bool> o_showLineTypeIconCol(&Group, "showLineTypeIconCol", true);
    inline const LC_Setting<bool> o_showLineWidthNameCol(&Group, "showLineWidthNameCol", false);
    inline const LC_Setting<bool> o_showLineWidthIconCol(&Group, "showLineWidthIconCol", true);
    inline const LC_Setting<bool> o_showEntireActiveRowBold(&Group, "showEntireActiveRowBold", true);
    inline const LC_Setting<bool> o_filterInHighlightsMode(&Group, "filterInHighlightsMode", false);
    inline const LC_Setting<bool> o_ignoreCaseOnMatch(&Group, "ignoreCaseOnMatch", true);
    inline const LC_Setting<bool> o_showNoSelectionMessage(&Group, "showNoSelectionMessage", false);
    inline const LC_Setting<LC_PenInfoRegistry::ColorNameDisplayMode> o_colorDisplayMode(&Group, "colorDisplayMode", LC_PenInfoRegistry::ColorNameDisplayMode::RGB);
    inline const LC_Setting<PenPaletteDoubleClickMode> o_doubleClickOnTableMode(&Group, "doubleClickOnTableMode", DOUBLE_CLICK_SELECT_ENTITIES_BY_ATTRIBUTES_PEN);
}

#endif
