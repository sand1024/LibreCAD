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

#include "lc_repository_widgets.h"

QJsonObject LC_RepositoryWidgets::configToJson(const LC_WidgetsConfig& config) const {
    QJsonObject root;
    root["name"] = config.name;

    QJsonObject toolbars;
    toolbars["allow_icon_size"] = config.toolbarAllowIconSize;
    toolbars["icon_size"] = config.toolbarIconSize;
    toolbars["flat_pick_buttons"] = config.pickValueButtonsFlatIcons;
    root["toolbars"] = toolbars;

    QJsonObject dockWidgets;
    dockWidgets["flat_buttons"] = config.dockWidgetsFlatButtons;
    dockWidgets["icon_size"] = config.dockWidgetsIconSize;
    dockWidgets["tab_display_mode"] = config.dockTabDisplayMode;
    dockWidgets["tab_override_icon_size"] = config.dockTabOverrideIconSize;
    dockWidgets["tab_icon_size"] = config.dockTabIconSize;
    dockWidgets["titlebar_vertical"] = config.dockTitleBarVertical;
    dockWidgets["vertical_tabs"] = config.dockTabVertical;
    root["dock_widgets"] = dockWidgets;

    QJsonObject cadWidgets;
    cadWidgets["flat_buttons"] = config.cadDockWidgetFlatButtons;
    cadWidgets["icon_size"] = config.cadDockWidgetIconSize;
    cadWidgets["columns_count"] = config.cadDockWidgetColumnsCount;
    cadWidgets["tab_display_mode"] = config.cadDockTabDisplayMode;
    cadWidgets["tab_override_icon_size"] = config.cadDockTabOverrideIconSize;
    cadWidgets["tab_icon_size"] = config.cadDockTabIconSize;
    cadWidgets["titlebar_vertical"] = config.cadDockTitleBarVertical;
    cadWidgets["vertical_tabs"] = config.cadDockVerticalTabs;
    root["cad_widgets"] = cadWidgets;

    QJsonObject cadMatrix;
    cadMatrix["flat_buttons"] = config.cadToolsMatrixFlatButtons;
    cadMatrix["icon_size"] = config.cadToolsMatrixIconSize;
    cadMatrix["columns_count"] = config.cadToolsMatrixColumnsCount;
    root["cad_matrix"] = cadMatrix;

    QJsonObject docking;
    docking["allow_nested"] = config.dockAllowNested;
    root["docking"] = docking;

    QJsonObject statusbar;
    statusbar["allow_height"] = config.allowStatusbarHeight;
    statusbar["height"] = config.statusbarHeight;
    statusbar["allow_font_size"] = config.allowStatusbarFontSize;
    statusbar["font_size"] = config.statusbarFontSize;
    root["statusbar"] = statusbar;

    return root;
}

bool LC_RepositoryWidgets::configFromJson(const QJsonObject& json, LC_WidgetsConfig& config) const {
    LC_WidgetsConfigUtils::initializeDefaultConfig(config);

    if (json.contains("name")) {
        config.name = json["name"].toString();
    }

    const QJsonObject toolbars = json["toolbars"].toObject();
    config.toolbarAllowIconSize = toolbars["allow_icon_size"].toBool(config.toolbarAllowIconSize);
    config.toolbarIconSize = toolbars["icon_size"].toInt(config.toolbarIconSize);
    config.pickValueButtonsFlatIcons = toolbars["flat_pick_buttons"].toBool(config.pickValueButtonsFlatIcons);

    const QJsonObject dockWidgets = json["dock_widgets"].toObject();
    config.dockWidgetsFlatButtons = dockWidgets["flat_buttons"].toBool(config.dockWidgetsFlatButtons);
    config.dockWidgetsIconSize = dockWidgets["icon_size"].toInt(config.dockWidgetsIconSize);
    config.dockTabDisplayMode = dockWidgets["tab_display_mode"].toInt(config.dockTabDisplayMode);
    config.dockTabOverrideIconSize = dockWidgets["tab_override_icon_size"].toBool(config.dockTabOverrideIconSize);
    config.dockTabIconSize = dockWidgets["tab_icon_size"].toInt(config.dockTabIconSize);
    config.dockTitleBarVertical = dockWidgets["titlebar_vertical"].toBool(config.dockTitleBarVertical);
    config.dockTabVertical = dockWidgets["vertical_tabs"].toBool(config.dockTabVertical);

    const QJsonObject cadWidgets = json["cad_widgets"].toObject();
    config.cadDockWidgetFlatButtons = cadWidgets["flat_buttons"].toBool(config.cadDockWidgetFlatButtons);
    config.cadDockWidgetIconSize = cadWidgets["icon_size"].toInt(config.cadDockWidgetIconSize);
    config.cadDockWidgetColumnsCount = cadWidgets["columns_count"].toInt(config.cadDockWidgetColumnsCount);
    config.cadDockTabDisplayMode = cadWidgets["tab_display_mode"].toInt(config.cadDockTabDisplayMode);
    config.cadDockTabOverrideIconSize = cadWidgets["tab_override_icon_size"].toBool(config.cadDockTabOverrideIconSize);
    config.cadDockTabIconSize = cadWidgets["tab_icon_size"].toInt(config.cadDockTabIconSize);
    config.cadDockTitleBarVertical = cadWidgets["titlebar_vertical"].toBool(config.cadDockTitleBarVertical);
    config.cadDockVerticalTabs = cadWidgets["vertical_tabs"].toBool(config.cadDockVerticalTabs);

    const QJsonObject cadMatrix = json["cad_matrix"].toObject();
    config.cadToolsMatrixFlatButtons = cadMatrix["flat_buttons"].toBool(config.cadToolsMatrixFlatButtons);
    config.cadToolsMatrixIconSize = cadMatrix["icon_size"].toInt(config.cadToolsMatrixIconSize);
    config.cadToolsMatrixColumnsCount = cadMatrix["columns_count"].toInt(config.cadToolsMatrixColumnsCount);

    const QJsonObject docking = json["docking"].toObject();
    config.dockAllowNested = docking["allow_nested"].toBool(config.dockAllowNested);

    const QJsonObject statusbar = json["statusbar"].toObject();
    config.allowStatusbarHeight = statusbar["allow_height"].toBool(config.allowStatusbarHeight);
    config.statusbarHeight = statusbar["height"].toInt(config.statusbarHeight);
    config.allowStatusbarFontSize = statusbar["allow_font_size"].toBool(config.allowStatusbarFontSize);
    config.statusbarFontSize = statusbar["font_size"].toInt(config.statusbarFontSize);

    return true;
}
