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


#include "lc_matchdescriptor_dimlinear.h"

#include "rs_dimlinear.h"

void LC_MatchDescriptorDimLinear::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
        auto entity = new LC_TypedEntityMatchDescriptor<RS_DimLinear>(tr("Dimension Linear"), RS2::EntityDimLinear);
    initCommonEntityAttributesProperties<RS_DimLinear>(entity);
    initCommonDimensionAttributes(entity);

    entity->addVector("defX", [](RS_DimLinear* e) {
        return e->getDefinitionPoint();
    }, tr("Definition Point X"), tr("X coordinate for definition point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("defY", [](RS_DimLinear* e) {
        return e->getDefinitionPoint();
    }, tr("Definition Point Y"), tr("Y coordinate for definition point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->addVector("ext1X", [](RS_DimLinear* e) {
        return e->getExtensionPoint1();
    }, tr("First Extension Point X"), tr("X coordinate for first extension point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("ext1Y", [](RS_DimLinear* e) {
        return e->getExtensionPoint1();
    }, tr("First Extension Point Y"), tr("Y coordinate for first extension point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->addVector("ext2X", [](RS_DimLinear* e) {
        return e->getExtensionPoint2();
    }, tr("Second Extension Point X"), tr("X coordinate for second extension point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("ext2Y", [](RS_DimLinear* e) {
        return e->getExtensionPoint2();
    }, tr("Second Extension Point Y"), tr("Y coordinate for second extension point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->addVector("textMiddleX", [](RS_DimLinear* e) {
        return e->getMiddleOfText();
    }, tr("Text middle point X"), tr("X coordinate for text middle point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("textMiddleX", [](RS_DimLinear* e) {
        return e->getMiddleOfText();
    }, tr("Text middle point Y"), tr("Y coordinate for text middle point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("angle", [](RS_DimLinear* e) {
        return e->getAngle();
    }, tr("Angle"), tr("Dimension rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("obligue", [](RS_DimLinear* e) {
        return e->getOblique();
    }, tr("Obligue"), tr("Dimension obligue angle"), LC_PropertyMatcherTypes::ANGLE);

    map.insert(RS2::EntityDimLinear, entity);

}
