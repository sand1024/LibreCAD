/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2024 sand1024
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/
#include "lc_penpaletteoptions.h"

#include <QApplication>
#include <QPalette>

#include "lc_palette_color_utils.h"
#include "lc_settings_paths.h"
#include "lc_settings_pen_palette_widget.h"
#include "rs_settings.h"
#include "rs_system.h"

/**
 * Straightforwards storing options to settings
 */
void LC_PenPaletteOptions::loadFromSettings() {
    matchedItemColor = o_matchedItemBgColor;
    showGrid = o_showGrid;

    showToolTip = o_showToolTip;

    showColorName = o_showColorNameCol;
    showColorIcon = o_showColorIconCol;

    showTypeName = o_showLineTypeNameCol;
    showTypeIcon = o_showLineTypeIconCol;

    showWidthName = o_showLineWidthNameCol;
    showWidthIcon = o_showLineWidthIconCol;

    showEntireRowBold = o_showEntireActiveRowBold;;
    filterIsInHighlightMode = o_filterInHighlightsMode;
    ignoreCaseOnMatch = o_ignoreCaseOnMatch;
    showNoSelectionMessage = o_showNoSelectionMessage;

    colorNameDisplayMode = o_colorDisplayMode;
    doubleClickOnTableMode = doubleClickOnTableMode;

    const QString settingsDir = CFG_Paths::o_OtherSettingsDir;
    pensFileName = settingsDir + "/penpalette.lcpp";
}

/**
 * Straightforward loading from settings
 */
void LC_PenPaletteOptions::saveToSettings() const {
    using namespace CFG_WidgetPenPallette;
    o_matchedItemBgColor = matchedItemColor.name();
    o_showGrid = showGrid;

    o_showToolTip = showToolTip;

    o_showColorNameCol = showColorName;
    o_showColorIconCol = showColorIcon;

    o_showLineTypeNameCol = showTypeName;
    o_showLineTypeIconCol = showTypeIcon;

    o_showLineWidthNameCol = showWidthName;
    o_showLineWidthIconCol = showWidthIcon;

    o_showEntireActiveRowBold = showEntireRowBold;
    o_filterInHighlightsMode = filterIsInHighlightMode;
    o_ignoreCaseOnMatch = ignoreCaseOnMatch;
    o_showNoSelectionMessage = showNoSelectionMessage;

    o_colorDisplayMode = colorNameDisplayMode;
    o_doubleClickOnTableMode = doubleClickOnTableMode;
}
