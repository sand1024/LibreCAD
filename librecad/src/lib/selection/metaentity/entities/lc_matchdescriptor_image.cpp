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

#include "lc_matchdescriptor_image.h"

#include "rs_image.h"
#include "rs_units.h"

void LC_MatchDescriptorImage::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
        auto entity = new LC_TypedEntityMatchDescriptor<RS_Image>(tr("Image"), RS2::EntityImage);
    initCommonEntityAttributesProperties<RS_Image>(entity);

    entity->add<QString>("file", [](RS_Image* e) {
        return e->getFile();
    }, tr("File"), tr("Name of the image file"), LC_PropertyMatcherTypes::STRING);

    entity->addVector("insertX", [](RS_Image* e) {
        return e->getInsertionPoint();
    }, tr("Insert X"), tr("X coordinate for image insertion point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("insertY", [](RS_Image* e) {
        return e->getInsertionPoint();
    }, tr("Insert Y"), tr("Y coordinate for image insertion point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("scale", [](RS_Image* e) {
        return e->getUVector().magnitude();
    }, tr("Scale"), tr("Scale factor for image"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<double>("angle", [](RS_Image* e) {
        return e->getUVector().angle();
    }, tr("Angle"), tr("Image rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("sizeX", [](RS_Image* e) {
        return e->getData().size.getX();
    }, tr("Width pixels"), tr("Width of image in pixels"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<double>("sizeY", [](RS_Image* e) {
        return e->getData().size.getY();
    }, tr("Height pixels"), tr("Height of image in pixels"), LC_PropertyMatcherTypes::DOUBLE);

    entity->add<double>("width", [](RS_Image* e) {
        return e->getImageWidth();
    }, tr("Width"), tr("Width of image"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("height", [](RS_Image* e) {
        return e->getImageHeight();
    }, tr("Height"), tr("Height of image"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("dpi", [](RS_Image* e) {
        double scale = e->getUVector().magnitude();
        return RS_Units::scaleToDpi(scale, e->getGraphicUnit());
    }, tr("DPI"), tr("Dots per inch for the image"), LC_PropertyMatcherTypes::DOUBLE);

    map.insert(RS2::EntityImage, entity);

}
