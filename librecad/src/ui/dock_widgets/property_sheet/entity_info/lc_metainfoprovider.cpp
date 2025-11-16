/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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

#include "lc_metainfoprovider.h"

#include "rs_entity.h"

LC_MetaInfoProvider::LC_MetaInfoProvider() {}

void LC_MetaInfoProvider::fillPropertyDescriptorsForSelection(RS2::EntityType entityType, QList<LC_PropertyDescriptor>& propertyDescriptors) {
}



void LC_MetaInfoProvider::initPropertiesDescriptors(RS2::EntityType entityType, const std::list<ShortPropertyDescriptor> &descriptorsList) {
    QList<LC_PropertyDescriptor> propertyDescriptors;
    int property_id = 0;
    for (auto d: descriptorsList) {
        LC_PropertyDescriptor desc;
        desc.m_name = d.name;
        desc.m_hasLessGreaterOperators = d.hasLessGreater;
        desc.m_hasPatternMatch = d.hasPattern;
        desc.id = property_id;
        property_id++;
    }
    m_entityPropertyDescriptors[entityType] = propertyDescriptors;
}

void LC_MetaInfoProvider::initPropertyDescriptors() {
   initGenericProperties();
   initLineProperties();
   initCircleProperties();
   initArcProperties();
}

using FunAcceptEntity = std::function<bool(RS_Entity*, RS_Layer*, bool)>;

FunAcceptEntity matchLayer = [](RS_Entity* entity, RS_Layer* layer, bool equals) {
    auto l = entity->getLayer();
    if (equals) {
        return l == layer;
    }
    else {
        return l != layer;
    }
};

void LC_MetaInfoProvider::initGenericProperties() {
    initPropertiesDescriptors(RS2::EntityUnknown,{
        {tr("Color")},
        {tr("Layer")},
        {tr("Line type")},
        {tr("Line width")}
    });
}

void LC_MetaInfoProvider::initLineProperties() {
    initPropertiesDescriptors(RS2::EntityLine, {
        {tr("Color"), false},
        {tr("Layer"), false},
        {tr("Line type"), false},
        {tr("Line width"), false},
        {tr("Start X"), true},
        {tr("Start Y"), true},
        {tr("End X"), true},
        {tr("End Y"), true},
        {tr("Delta X"), true},
        {tr("Delta Y"), true},
        {tr("Length"), true},
        {tr("Angle"), true},
    });
}

void LC_MetaInfoProvider::initCircleProperties() {
    initPropertiesDescriptors(RS2::EntityCircle, {
        {tr("Color")},
        {tr("Layer")},
        {tr("Line type")},
        {tr("Line width")},
        {tr("Center X"), true},
        {tr("Center Y"), true},
        {tr("Radius"), true},
        {tr("Diameter"), true},
        {tr("Circumference"), true},
        {tr("Area"), true},
        {tr("Length"), true},
        {tr("Angle"), true}
    });
}

void LC_MetaInfoProvider::initArcProperties() {
    initPropertiesDescriptors(RS2::EntityArc, {
        {tr("Color")},
        {tr("Layer")},
        {tr("Line type")},
        {tr("Line width")},
        {tr("Start X"), true},
        {tr("Start Y"), true},
        {tr("Center X"), true},
        {tr("Center Y"), true},
        {tr("End X"), true},
        {tr("End Y"), true},
        {tr("Radius"), true},
        {tr("Start Angle"), true},
        {tr("End Angle"), true},
        {tr("Total Angle"), true},
        {tr("Circumference"), true},
        {tr("Chord"), true},
        {tr("Arc Length"), true},
        {tr("Area"), true},
        {tr("Reversed"), true}
    });
}

void LC_MetaInfoProvider::initPolylineProperties() {
    initPropertiesDescriptors(RS2::EntityArc, {
        {tr("Color")},
        {tr("Layer")},
        {tr("Line type")},
        {tr("Line width")},
        {tr("Vertex X"), true},
        {tr("Vertex Y"), true},
        {tr("Length"), true},
        {tr("Closed"), true},
        });
}
