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

#ifndef LC_WIDGETS_CONFIG_H
#define LC_WIDGETS_CONFIG_H

#include <QString>

struct LC_WidgetsConfig {
    QString name;

    // General Toolbars
    bool toolbarAllowIconSize{false};
    int  toolbarIconSize{24};
    bool pickValueButtonsFlatIcons{false};

    // Ordinary Dock Widgets
    bool dockWidgetsFlatButtons{false};
    int  dockWidgetsIconSize{24};
    int  dockTabDisplayMode{0};          // TabDisplay_IconAndText
    bool dockTabOverrideIconSize{false};
    int  dockTabIconSize{16};
    bool dockTitleBarVertical{false};
    bool dockTabVertical{true};

    // CAD Tools Widgets (Groups)
    bool cadDockWidgetFlatButtons{false};
    int  cadDockWidgetIconSize{24};
    int  cadDockWidgetColumnsCount{5};
    int  cadDockTabDisplayMode{1};       // TabDisplay_IconOnly
    bool cadDockTabOverrideIconSize{false};
    int  cadDockTabIconSize{20};
    bool cadDockTitleBarVertical{false};
    bool cadDockVerticalTabs{false};

    // CAD Tools Matrix (Ungrouped)
    bool cadToolsMatrixFlatButtons{false};
    int  cadToolsMatrixIconSize{24};
    int  cadToolsMatrixColumnsCount{5};

    // General Docking
    bool dockAllowNested{false};

    // Classic Statusbar
    bool allowStatusbarHeight{false};
    int  statusbarHeight{24};
    bool allowStatusbarFontSize{false};
    int  statusbarFontSize{9};
};

class LC_WidgetsConfigUtils {
public:
    static void initializeDefaultConfig(LC_WidgetsConfig& config);
};

#endif // LC_WIDGETS_CONFIG_H
