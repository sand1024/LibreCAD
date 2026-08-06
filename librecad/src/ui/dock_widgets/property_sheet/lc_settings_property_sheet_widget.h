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

#ifndef LC_SETTINGS_PROPERTY_SHEET_WIDGET_H
#define LC_SETTINGS_PROPERTY_SHEET_WIDGET_H

#include "lc_setting.h"

namespace CFG_WidgetPropertySheet {
    inline const LC_SettingsGroupBase Group("Widget.PropertySheet");

    inline const LC_Setting<QString> o_CollapsedSections(&Group, "CollapsedSections", "");
    inline const LC_Setting<bool> o_NoSelectionActivePen(&Group, "NoSelectionActivePen", true);
    inline const LC_Setting<bool> o_NoSelectionActiveLayer(&Group, "NoSelectionActiveLayer", true);
    inline const LC_Setting<bool> o_NoSelectionNamedView(&Group, "NoSelectionNamedView", true);
    inline const LC_Setting<bool> o_noSelectionUCS(&Group, "noSelectionUCS", true);
    inline const LC_Setting<bool> o_noSelectionGrid(&Group, "noSelectionGrid", true);
    inline const LC_Setting<bool> o_noSelectionDrawingUnits(&Group, "noSelectionDrawingUnits", true);
    inline const LC_Setting<bool> o_noSelectionPrintPaper(&Group, "noSelectionPrintPaper", true);
    inline const LC_Setting<bool> o_noSelectionWorkspace(&Group, "noSelectionWorkspace", true);
    inline const LC_Setting<bool> o_noSelectionGraphicView(&Group, "noSelectionGraphicView", true);
    inline const LC_Setting<bool> o_showLinks(&Group, "showLinks", true);
    inline const LC_Setting<bool> o_showSingleEntityCommands(&Group, "showSingleEntityCommands", true);
    inline const LC_Setting<bool> o_showMultiEntityCommands(&Group, "showMultiEntityCommands", true);
    inline const LC_Setting<bool> o_showComputed(&Group, "showComputed", true);
    inline const LC_Setting<bool> o_duplicateSelectionAction(&Group, "duplicateSelectionAction", true);
    inline const LC_Setting<bool> o_showToolOptions(&Group, "showToolOptions", true);
    inline const LC_Setting<int> o_fontSize(&Group, "fontSize", 9);
}

#endif
