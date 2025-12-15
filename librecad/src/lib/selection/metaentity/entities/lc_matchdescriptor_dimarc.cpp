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

#include "lc_matchdescriptor_dimarc.h"

#include "lc_dimarc.h"

void LC_MatchDescriptorDimArc::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
    auto entity = new LC_TypedEntityMatchDescriptor<LC_DimArc>(tr("Dimension Arc"), RS2::EntityDimArc);
    initCommonEntityAttributesProperties<LC_DimArc>(entity);
    initCommonDimensionAttributes(entity);
    entity->add<double>("raidus", [](LC_DimArc* e) {
        return e->getRadius();
    }, tr("Radius"), tr("Dimension radius"), LC_PropertyMatcherTypes::LENGTH);

    entity->addVector("centerX", [](LC_DimArc* e) {
       return e->getCenter();
   }, tr("Center Point X"), tr("X coordinate for dimension center"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("centerY", [](LC_DimArc* e) {
      return e->getCenter();
  }, tr("Center Point Y"), tr("Y coordinate for dimension center"), LC_PropertyMatcherTypes::COORD_Y);

    // addLinearProperty(tr("Arc Length"), data.arcLength);

    entity->add<double>("angleStart", [](LC_DimArc* e) {
          return e->getStartAngle();
    }, tr("Start Angle"), tr("Dimension start angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("angleEnd", [](LC_DimArc* e) {
          return e->getEndAngle();
    }, tr("End Angle"), tr("Dimension end angle"), LC_PropertyMatcherTypes::ANGLE);

    map.insert(RS2::EntityDimArc, entity);
}
