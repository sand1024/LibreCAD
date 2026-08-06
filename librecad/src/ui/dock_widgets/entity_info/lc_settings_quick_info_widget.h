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

#ifndef LC_SETTINGS_QUICK_INFO_WIDGET_H
#define LC_SETTINGS_QUICK_INFO_WIDGET_H

#include "lc_quickinfobasedata.h"
#include "lc_setting.h"
#include "rs.h"

namespace CFG_WidgetQuickInfo {
    inline const LC_SettingsGroupBase Group("Widget.QuickInfo");
    inline const LC_Setting<bool> o_ShowDistanceAndAngle(&Group, "ShowDistanceAndAngle", true);
    inline const LC_Setting<bool> o_ShowEntityBoundaries(&Group, "ShowEntityBoundaries", true);
    inline const LC_Setting<bool> o_ShowPointsPathOnPreview(&Group, "ShowPointsPathOnPreview", true);
    inline const LC_Setting<bool> o_ShowPolylineDetails(&Group, "ShowPolylineDetails", true);
    inline const LC_Setting<bool> o_SelectEntityInDefaultAction(&Group, "SelectEntityInDefaultAction", true);
    inline const LC_Setting<bool> o_AutoSelectEntityInDefaultAction(&Group, "AutoSelectEntityInDefaultAction", true);
    inline const LC_Setting<QColor> o_penHighlightColor(&Group, "penHighlightColor", QColor("red"));
    inline const LC_Setting<RS2::LineType> o_penHighlightLineType(&Group, "penHighlightLineType", RS2::LineType::SolidLine);
    inline const LC_Setting<RS2::LineWidth> o_penHighlightLineWidth(&Group, "penHighlightLineWidth", RS2::LineWidth::WidthDefault);
    inline const LC_Setting<bool> o_EntityCoordinatesMode(&Group, "EntityCoordinatesMode", LC_QuickInfoBaseData::COORD_ABSOLUTE);
    inline const LC_Setting<bool> o_PointsCoordinatesMode(&Group, "PointsCoordinatesMode", LC_QuickInfoBaseData::COORD_ABSOLUTE);
}

#endif
