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

#include "lc_layertreemodel_options.h"

#include <QApplication>
#include <QPalette>

#include "lc_layertreeitem.h"
#include "lc_palette_color_utils.h"
#include "lc_settings_layer_tree_widget.h"
#include "rs_settings.h"

/**
 * Returns pen that will be used as default for creation of the layer with given type
 * @param layerType type of layer
 * @return default pen based on settings for the layer type
 */
RS_Pen LC_LayerTreeModelOptions::getDefaultPen(const int layerType) const{
    RS_Pen result;
    switch (layerType){
        case RS_Layer::LayerType::NORMAL:
            return defaultPenNormal;
        case RS_Layer::LayerType::DIMENSIONAL:
            return defaultPenDimensional;
        case RS_Layer::LayerType::INFORMATIONAL:
            return defaultPenInformational;
        case RS_Layer::LayerType::ALTERNATE_POSITION:
            return defaultPenAlternatePosition;
        default:
            // in general, this function should not be called for other types of layers...
            // TODO - throw error there?
            break;
    }
    return result;
}

void LC_LayerTreeModelOptions::save() const{
    {
        using namespace CFG_WidgetLayersTree;

        o_showGrid = showGrid;

        o_namingLayerSeparator = layerLevelSeparator;
        o_namingInfoSuffix = informationalLayerNameSuffix;
        o_namingDimSuffix = dimensionalLayerNameSuffix;
        o_namingAltSuffix = alternatePositionLayerNameSuffix;
        o_namingCopyPrefix = copiedNamePathPrefix;
        o_namingCopySuffix = copiedNamePathSuffix;

        o_hideLayerTypeIcons = hideLayerTypeIcons;
        o_dragDropEnabled = dragDropEnabled;
        o_showIndentedName = showIndentedName;
        o_showToolTips = showToolTips;
        o_renameSecondaryOnPrimary = renameSecondaryLayersOnPrimaryRename;
        o_indentSize = identSize;

        RS_Settings::writePen("NormalLayer", defaultPenNormal);
        RS_Settings::writePen("DimensionalLayer", defaultPenDimensional);
        RS_Settings::writePen("InfoLayer", defaultPenInformational);
        RS_Settings::writePen("AltPosLayer", defaultPenAlternatePosition);
    }
}

void LC_LayerTreeModelOptions::load(){
    using namespace CFG_WidgetLayersTree;
    {
        showGrid = o_showGrid;

        layerLevelSeparator = o_namingLayerSeparator;
        informationalLayerNameSuffix = o_namingInfoSuffix;
        dimensionalLayerNameSuffix = o_namingDimSuffix;
        alternatePositionLayerNameSuffix = o_namingAltSuffix;
        copiedNamePathPrefix = o_namingCopyPrefix;
        copiedNamePathSuffix = o_namingCopySuffix;

        hideLayerTypeIcons = o_hideLayerTypeIcons;
        dragDropEnabled = o_dragDropEnabled;
        showIndentedName = o_showIndentedName;
        showToolTips = o_showToolTips;
        renameSecondaryLayersOnPrimaryRename = o_renameSecondaryOnPrimary;

        identSize = o_indentSize;

        // fixme - sand - settings - rework!
        LC_GROUP(CFG_WidgetLayersTree::Group.groupName());
        defaultPenNormal = RS_Settings::readPen("NormalLayer", RS_Pen(Qt::black, RS2::Width00,RS2::SolidLine));
        defaultPenDimensional = RS_Settings::readPen("DimensionalLayer", RS_Pen(Qt::blue, RS2::Width02,RS2::SolidLine));
        defaultPenInformational = RS_Settings::readPen("InfoLayer", RS_Pen(Qt::magenta, RS2::Width02,RS2::SolidLine));
        defaultPenAlternatePosition = RS_Settings::readPen("AltPosLayer", RS_Pen(Qt::cyan, RS2::Width00,RS2::SolidLine));
    }
}
