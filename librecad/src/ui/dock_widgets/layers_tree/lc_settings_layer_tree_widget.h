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

#ifndef LC_SETTINGS_LAYER_TREE_WIDGET_H
#define LC_SETTINGS_LAYER_TREE_WIDGET_H

#include "lc_setting.h"

namespace CFG_WidgetLayersTree {
    inline const LC_SettingsGroupBase Group("Widget.LayersTree");

    inline const LC_Setting<QColor> o_highlightedItemColor(&Group, "highlightedItemColor", QColor("blue"));
    inline const LC_Setting<bool> o_showGrid(&Group, "showGrid", true);
    inline const LC_Setting<QString> o_namingLayerSeparator(&Group, "namingLayerSeparator", "-");
    inline const LC_Setting<QString> o_namingInfoSuffix(&Group, "namingInfoSuffix", "_meta");
    inline const LC_Setting<QString> o_namingDimSuffix(&Group, "namingDimSuffix", "+");
    inline const LC_Setting<QString> o_namingAltSuffix(&Group, "namingAltSuffix", "_pos");
    inline const LC_Setting<QString> o_namingCopyPrefix(&Group, "namingCopyPrefix", "(Copy");
    inline const LC_Setting<QString> o_namingCopySuffix(&Group, "namingCopySuffix", ")");
    inline const LC_Setting<bool> o_hideLayerTypeIcons(&Group, "hideLayerTypeIcons", false);
    inline const LC_Setting<bool> o_dragDropEnabled(&Group, "dragDropEnabled", true);
    inline const LC_Setting<bool> o_showIndentedName(&Group, "showIndentedName", true);
    inline const LC_Setting<bool> o_showToolTips(&Group, "showToolTips", true);
    inline const LC_Setting<bool> o_renameSecondaryOnPrimary(&Group, "renameSecondaryOnPrimary", true);
    inline const LC_Setting<int> o_indentSize(&Group, "indentSize", 4);


    // defaultPenNormal = RS_Settings::readPen("NormalLayer", defaults.defaultPenNormal);
    // defaultPenDimensional = RS_Settings::readPen("DimensionalLayer", defaults.defaultPenDimensional);
    // defaultPenInformational = RS_Settings::readPen("InfoLayer", defaults.defaultPenInformational);
    // defaultPenAlternatePosition = RS_Settings::readPen("AltPosLayer", defaults.defaultPenAlternatePosition);
}

#endif
