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
#ifndef LC_PENPALETTEOPTIONS_H
#define LC_PENPALETTEOPTIONS_H

#include "lc_peninforegistry.h"
#include "lc_settings_pen_palette_widget.h"

using namespace CFG_WidgetPenPallette;

class LC_PenPaletteOptions{
public:
    /**
     * color used to highlight items that matched to regexp
     */
    QColor matchedItemColor = QColor("blue");
    /**
     * defines whether grid is shown
     */
    bool showGrid{true};
    /**
     * mode for displaying color name
     */
    LC_PenInfoRegistry::ColorNameDisplayMode colorNameDisplayMode {LC_PenInfoRegistry::RGB};
    /**
     * should we show all fields for active pen in the table or only for name
     */
    bool showEntireRowBold = o_showEntireActiveRowBold.defaultValue();
    /**
     *  controls whether items matched by regexp should be highlighted or filtered
     */
    bool filterIsInHighlightMode = o_filterInHighlightsMode.defaultValue();
    /**
     * controls whether items regexp matching should ignore case
     */
    bool ignoreCaseOnMatch = o_ignoreCaseOnMatch.defaultValue();

    // fixme - well, probably we should use some reasonable defaults with paths related to the app there...
    /**
     * name of file where pens are stored
     */
    QString pensFileName{""};

    /**
     * columns visibility flags
     */
    bool showColorIcon = o_showColorIconCol.defaultValue();
    bool showColorName  = o_showColorNameCol.defaultValue();
    bool showTypeIcon = o_showLineTypeIconCol.defaultValue();
    bool showTypeName = o_showLineTypeNameCol.defaultValue();
    bool showWidthIcon = o_showLineWidthIconCol.defaultValue();
    bool showWidthName = o_showLineWidthNameCol.defaultValue();
    bool showNoSelectionMessage = o_showNoSelectionMessage.defaultValue();

    /**
     * flag that defines how to process double click on table item
     */
    PenPaletteDoubleClickMode doubleClickOnTableMode = DOUBLE_CLICK_SELECT_ENTITIES_BY_ATTRIBUTES_PEN;

    /**
     * should we show tooltip in table or not
     */
    bool showToolTip  = o_showToolTip.defaultValue();

    /**
     * Saves options in settings
     */
    void saveToSettings() const;
    /**
     * loads options from settings
     */
    void loadFromSettings();
};

#endif
