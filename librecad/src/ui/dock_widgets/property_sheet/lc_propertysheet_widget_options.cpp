/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_propertysheet_widget_options.h"

#include "lc_settings_property_sheet_widget.h"
#include "rs_settings.h"

void LC_PropertySheetWidgetOptions::save() const {
    using namespace CFG_WidgetPropertySheet;
    o_NoSelectionActivePen = noSelectionActivePen;
    o_NoSelectionActiveLayer = noSelectionActiveLayer;
    o_NoSelectionNamedView = noSelectionNamedView;
    o_noSelectionUCS = noSelectionUCS;
    o_noSelectionGrid = noSelectionGrid;
    o_noSelectionDrawingUnits = noSelectionDrawingUnits;
    o_noSelectionPrintPaper = noSelectionPrintPaper;
    o_noSelectionWorkspace = noSelectionWorkspace;
    o_noSelectionGraphicView = noSelectionGraphicView;
    o_showLinks = showLinks;
    o_showSingleEntityCommands = showSingleEntityCommands;
    o_showMultiEntityCommands = showMultiEntityCommands;
    o_showComputed = showComputed;
    o_duplicateSelectionAction = duplicateSelectionAction;
    o_showToolOptions = showToolOptions;
    o_fontSize = fontSize;
}

void LC_PropertySheetWidgetOptions::load() {
    using namespace CFG_WidgetPropertySheet;
    noSelectionActivePen = o_NoSelectionActivePen;
    noSelectionActiveLayer = o_NoSelectionActiveLayer;
    noSelectionNamedView = o_NoSelectionNamedView;
    noSelectionUCS = o_noSelectionUCS;
    noSelectionGrid = o_noSelectionGrid;
    noSelectionDrawingUnits = o_noSelectionDrawingUnits;
    noSelectionPrintPaper = o_noSelectionPrintPaper;
    noSelectionWorkspace = o_noSelectionWorkspace;
    noSelectionGraphicView = o_noSelectionGraphicView;
    showLinks = o_showLinks;
    showSingleEntityCommands = o_showSingleEntityCommands;
    showMultiEntityCommands = o_showMultiEntityCommands;
    showComputed = o_showComputed;
    duplicateSelectionAction = o_duplicateSelectionAction;
    showToolOptions = o_showToolOptions;
    fontSize = o_fontSize;
}
