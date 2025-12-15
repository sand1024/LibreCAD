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

#include "lc_matchdescriptor_arc.h"

#include "rs_arc.h"

void LC_MatchDescriptorArc::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
        auto entity = new LC_TypedEntityMatchDescriptor<RS_Arc>(tr("Arc"), RS2::EntityArc);
    initCommonEntityAttributesProperties<RS_Arc>(entity);
    entity->addVector("centerX", [](RS_Arc* e) {
        return e->getCenter();
    }, tr("Center X"), tr("X coordinate for center point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("centerY", [](RS_Arc* e) {
        return e->getCenter();
    }, tr("Center Y"), tr("Y coordinate for center point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("radius", [](RS_Arc* e) {
        return e->getRadius();
    }, tr("Radius"), tr("Radius of arc"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<bool>("reversed", [](RS_Arc* e) {
        return e->isReversed();
    }, tr("Is Reversed"), tr("Clockwise direction if reversed, counterclockwise otherwise"), LC_PropertyMatcherTypes::BOOL);

    entity->add<double>("diameter", [](RS_Arc* e) {
        return e->getRadius() * 2.0;
    }, tr("Diameter"), tr("Diameter of arc"), LC_PropertyMatcherTypes::LENGTH);

    entity->addVector("startX", [](RS_Arc* e) {
        return e->getStartpoint();
    }, tr("Start X"), tr("X coordinate for start point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("startY", [](RS_Arc* e) {
        return e->getStartpoint();
    }, tr("Start Y"), tr("Y coordinate for start point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->addVector("endX", [](RS_Arc* e) {
        return e->getEndpoint();
    }, tr("End X"), tr("X coordinate for end point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("endY", [](RS_Arc* e) {
        return e->getEndpoint();
    }, tr("End Y"), tr("Y coordinate for end point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("angleLen", [](RS_Arc* e) {
        return e->getAngleLength();
    }, tr("Angle Length"), tr("Angle length for arc"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("circumference", [](RS_Arc* e) {
        return e->getLength();
    }, tr("Circumference"), tr("Circumference of arc"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("chord", [](RS_Arc* e) {
        return e->getStartpoint().distanceTo(e->getEndpoint());
    }, tr("Chord"), tr("Chord length (distance from start to end point)"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("sagitta", [](RS_Arc* e) {
        return e->getSagitta();
    }, tr("Sagitta"), tr("Sagitta of the arc"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("angle1", [](RS_Arc* e) {
        return e->getAngle1();
    }, tr("Start Angle"), tr("Start angle of arc"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("angle2", [](RS_Arc* e) {
        return e->getAngle2();
    }, tr("End Angle"), tr("End angle of arc"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("bulge", [](RS_Arc* e) {
        return e->getBulge();
    }, tr("Bulge"), tr("Bulge of arc "), LC_PropertyMatcherTypes::LENGTH);

    map.insert(RS2::EntityArc, entity);

}
