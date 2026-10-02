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


#ifndef LC_SETTINGS_WIDGET_H
#define LC_SETTINGS_WIDGET_H
#include "lc_setting.h"

namespace CFG_Widgets {
    inline const LC_SettingsGroupBase Group("Widgets");

    enum DockTabDisplayMode {
        TabDisplay_IconAndText = 0,
        TabDisplay_IconOnly    = 1,
        TabDisplay_TextOnly    = 2
    };


    inline const LC_Setting<bool> o_ToolbarAllowIconSize(&Group, "AllowToolbarIconSize", false);
    inline const LC_Setting<int>  o_ToolbarIconSize(&Group, "ToolbarIconSize", 25);
    inline const LC_Setting<bool> o_PickValueButtonsFlatIcons(&Group, "PickValueButtonsFlatIcons", true);


    inline const LC_Setting<bool> o_DockAllowNested(&Group, "DockAllowNested", true);

    inline const LC_Setting<bool> o_StatusBarAllowFontSize(&Group, "AllowStatusbarFontSize", false);
    inline const LC_Setting<bool> o_StatusBarAllowHeight(&Group, "AllowStatusbarHeight", false);
    inline const LC_Setting<int>  o_StatusbarFontSize(&Group, "StatusbarFontSize", 12);
    inline const LC_Setting<int>  o_StatusbarHeight(&Group, "StatusbarHeight", 64);

    // Standard/Ordinary Dock Widgets (right panel)

    inline const LC_Setting<bool> o_DockWidgetsFlatButtons(&Group, "DockWidgetsFlatIcons", true);
    inline const LC_Setting<int>  o_DockWidgetsIconSize(&Group, "DockWidgetsIconSize", 16);
    inline const LC_Setting<int>  o_DockTabDisplayMode(&Group, "DockTabDisplayMode", TabDisplay_IconAndText);
    inline const LC_Setting<bool> o_DockTabOverrideIconSize(&Group, "DockTabOverrideIconSize", false);
    inline const LC_Setting<int>  o_DockTabIconSize(&Group, "DockTabIconSize", 16);
    inline const LC_Setting<bool> o_DockTabVertical(&Group, "DockVerticalTabs", true);
    inline const LC_Setting<bool> o_DockWidgetTitleBarVertical(&Group, "DockTitleBarVertical", false);

    // CAD Tools Matrix

    inline const LC_Setting<bool> o_CADToolsMatrixFlatButtons(&Group, "LeftToolbarAllFlatIcons", true);
    inline const LC_Setting<int>  o_CADToolsMatrixIconSize(&Group, "LeftToolbarAllIconSize", 24);
    inline const LC_Setting<int>  o_CADToolsMatrixColumnsCount(&Group, "LeftToolbarAllColumnsCount", 5);

    // CAD Widgets (left dock panel)

    inline const LC_Setting<bool> o_CADDockWidgetFlatButtons(&Group, "LeftToolbarFlatIcons", true);
    inline const LC_Setting<int>  o_CADDockWidgetIconSize(&Group, "LeftToolbarIconSize", 24);
    inline const LC_Setting<int>  o_CADDockWidgetColumnsCount(&Group, "LeftToolbarColumnsCount", 5);
    inline const LC_Setting<int>  o_CADDockTabDisplayMode(&Group, "CadDockTabDisplayMode", TabDisplay_IconOnly);
    inline const LC_Setting<bool> o_CADDockTabOverrideIconSize(&Group, "CadDockTabOverrideIconSize", false);
    inline const LC_Setting<int>  o_CADDockTabIconSize(&Group, "CadDockTabIconSize", 20);
    inline const LC_Setting<bool> o_CADDockTabVertical(&Group, "CadDockVerticalTabs", false);
    inline const LC_Setting<bool> o_CADDockWidgetTitleBarVertical(&Group, "CadDockTitleBarVertical", false);

}
#endif
