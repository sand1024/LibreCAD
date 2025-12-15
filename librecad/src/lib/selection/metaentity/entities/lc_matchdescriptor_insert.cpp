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

#include "lc_matchdescriptor_insert.h"
#include "rs_insert.h"

void LC_MatchDescriptorInsert::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
    auto entity = new LC_TypedEntityMatchDescriptor<RS_Insert>(tr("Insert"), RS2::EntityInsert);
    initCommonEntityAttributesProperties<RS_Insert>(entity);

    entity->add<QString>("name", [](RS_Insert* e) {
        return e->getName();
    }, tr("Name"), tr("Name of the inserted block"), LC_PropertyMatcherTypes::STRING); // fixme - choice combox for available block names?

    entity->addVector("insertX", [](RS_Insert* e) {
        return e->getInsertionPoint();
    }, tr("Insert X"), tr("X coordinate for block's insertion point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("insertY", [](RS_Insert* e) {
        return e->getInsertionPoint();
    }, tr("Insert Y"), tr("Y coordinate for block's insertion point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("angle", [](RS_Insert* e) {
        return e->getAngle();
    }, tr("Angle"), tr("Block rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("scaleX", [](RS_Insert* e) {
        return e->getScale().getX();
    }, tr("Scale X"), tr("Block X scale"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<double>("scaleY", [](RS_Insert* e) {
        return e->getScale().getY();
    }, tr("Scale Y"), tr("Block Y scale"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<int>("cols", [](RS_Insert* e) {
        return e->getCols();
    }, tr("Columns"), tr("Columns number"), LC_PropertyMatcherTypes::INT);

    entity->add<double>("spacingX", [](RS_Insert* e) {
        return e->getSpacing().getX();
    }, tr("Spacing X"), tr("Block columns spacing (by  X)"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<int>("rows", [](RS_Insert* e) {
        return e->getRows();
    }, tr("Rows"), tr("Rows number"), LC_PropertyMatcherTypes::INT);

    entity->add<double>("spacingY", [](RS_Insert* e) {
        return e->getSpacing().getY();
    }, tr("Spacing Y"), tr("Block rows spacing (by  Y)"), LC_PropertyMatcherTypes::DOUBLE);

    map.insert(RS2::EntityInsert, entity);

}
