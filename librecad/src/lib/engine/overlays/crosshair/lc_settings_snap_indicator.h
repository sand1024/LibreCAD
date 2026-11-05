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

#ifndef LC_SETTINGS_SNAP_INDICATOR_H
#define LC_SETTINGS_SNAP_INDICATOR_H

#include "lc_setting.h"

namespace CFG_SnapIndicator {
    inline const LC_SettingsGroupBase Group("SnapIndicator");

    inline const LC_Setting<bool> o_DrawLines(&Group, "IndicatorDrawLines", true);
    inline const LC_Setting<bool> o_DrawShape(&Group, "IndicatorDrawShape", true);

    inline const LC_Setting<int> o_LinesType(&Group, "IndicatorLinesType", 0);
    inline const LC_Setting<int> o_ShapeType(&Group, "IndicatorShapeType", 0);
    inline const LC_Setting<int> o_ShapeSize(&Group, "indicator_shape_size", 4);

    inline const LC_Setting<RS2::LineType> o_LinesLineType(&Group, "LinesLineType", RS2::DashLine);
    inline const LC_Setting<double> o_ScreenLinesLineWidth(&Group, "ScreenLinesLineWidth", 1);
    inline const LC_Setting<int> o_CrosshairLinesSizePercents(&Group, "CrosshairLinesSizePercents", 100);

    inline const LC_Setting<QColor> o_ColorSnapIndicatorLines(&Group, "ColorSnapIndicatorLines", QColor("#FFC200"));
    inline const LC_Setting<QColor> o_ColorSnapIndicatorShape(&Group, "ColorSnapIndicatorShape", QColor("#FFC200"));

    inline const LC_Setting<int> o_AngleSnapMarkerSize(&Group, "AngleSnapMarkerSize", 25);
}

namespace CFG_EntitiesPickBox {
    inline const LC_SettingsGroupBase Group("PickBox");

    // fixme - move to other namespace
    inline const LC_Setting<bool> o_SelectionPickboxInsteadOfCursor(&Group, "SelectionPickBoxShow", true);

    inline const LC_Setting<int> o_SelectionPickBoxLinesType(&Group, "SelectionPickBoxLinesType", 0);
    inline const LC_Setting<int> o_SelectionPickBoxLinesSizeType(&Group, "SelectionPickBoxLinesType", 1);

    // selection pick box
    inline const LC_Setting<QColor> o_ColorSelectionPickBox(&Group, "ColorSelectionPickBox", QColor("#09ff09"));
}

#endif
