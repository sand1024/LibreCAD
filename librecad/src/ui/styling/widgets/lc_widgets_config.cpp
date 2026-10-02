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

#include "lc_widgets_config.h"

#include "lc_settings_widget.h"

void LC_WidgetsConfigUtils::initializeDefaultConfig(LC_WidgetsConfig& config) {
   config.name = "Default";

    using namespace CFG_Widgets;

    // General Toolbars
    config.toolbarAllowIconSize        = o_ToolbarAllowIconSize.defaultValue();
    config.toolbarIconSize             = o_ToolbarIconSize.defaultValue();
    config.pickValueButtonsFlatIcons   = o_PickValueButtonsFlatIcons.defaultValue();

    // Ordinary Dock Widgets
    config.dockWidgetsFlatButtons      = o_DockWidgetsFlatButtons.defaultValue();
    config.dockWidgetsIconSize         = o_DockWidgetsIconSize.defaultValue();
    config.dockTabDisplayMode          = o_DockTabDisplayMode.defaultValue();
    config.dockTabOverrideIconSize     = o_DockTabOverrideIconSize.defaultValue();
    config.dockTabIconSize             = o_DockTabIconSize.defaultValue();
    config.dockTitleBarVertical        = o_DockWidgetTitleBarVertical.defaultValue();
    config.dockTabVertical            = o_DockTabVertical.defaultValue();

    // CAD Tools Widgets (Groups)
    config.cadDockWidgetFlatButtons        = o_CADDockWidgetFlatButtons.defaultValue();
    config.cadDockWidgetIconSize         = o_CADDockWidgetIconSize.defaultValue();
    config.cadDockWidgetColumnsCount     = o_CADDockWidgetColumnsCount.defaultValue();
    config.cadDockTabDisplayMode       = o_CADDockTabDisplayMode.defaultValue();
    config.cadDockTabOverrideIconSize  = o_CADDockTabOverrideIconSize.defaultValue();
    config.cadDockTabIconSize          = o_CADDockTabIconSize.defaultValue();
    config.cadDockTitleBarVertical     = o_CADDockWidgetTitleBarVertical.defaultValue();
    config.cadDockVerticalTabs         = o_CADDockTabVertical.defaultValue();

    // CAD Tools Matrix (Ungrouped)
    config.cadToolsMatrixFlatButtons     = o_CADToolsMatrixFlatButtons.defaultValue();
    config.cadToolsMatrixIconSize      = o_CADToolsMatrixIconSize.defaultValue();
    config.cadToolsMatrixColumnsCount  = o_CADToolsMatrixColumnsCount.defaultValue();

    // General Docking
    config.dockAllowNested             = o_DockAllowNested.defaultValue();

    // Classic Statusbar
    config.allowStatusbarHeight        = o_StatusBarAllowHeight.defaultValue();
    config.statusbarHeight             = o_StatusbarHeight.defaultValue();
    config.allowStatusbarFontSize      = o_StatusBarAllowFontSize.defaultValue();
    config.statusbarFontSize           = o_StatusbarFontSize.defaultValue();
}
