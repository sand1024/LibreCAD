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

#include "lc_matchdescriptor_hatch.h"

#include "rs_hatch.h"

void LC_MatchDescriptorHatch::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
    auto entity = new LC_TypedEntityMatchDescriptor<RS_Hatch>(tr("Hatch"), RS2::EntityHatch);
    initCommonEntityAttributesProperties<RS_Hatch>(entity);

    entity->add<bool>("solid", [](RS_Hatch* e) {
        return e->isSolid();
    }, tr("Is Solid"), tr("Determines whether hatch is solid or not"), LC_PropertyMatcherTypes::BOOL);

    entity->add<QString>("pattern", [](RS_Hatch* e) {
        return e->getPattern();
    }, tr("Pattern"), tr("Hatch pattern name"), LC_PropertyMatcherTypes::STRING); // fixme - choice combox for available patterns?

    entity->add<double>("scale", [](RS_Hatch* e) {
        return e->getScale();
    }, tr("Scale"), tr("Hatch scale"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<double>("angle", [](RS_Hatch* e) {
        return e->getAngle();
    }, tr("Angle"), tr("Hatch rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    map.insert(RS2::EntityHatch, entity);

}
