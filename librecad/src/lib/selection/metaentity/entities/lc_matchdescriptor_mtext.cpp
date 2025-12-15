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


#include "lc_matchdescriptor_mtext.h"

#include "rs_mtext.h"

void LC_MatchDescriptorMText::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
        auto entity = new LC_TypedEntityMatchDescriptor<RS_MText>(tr("MText"), RS2::EntityMText);
    initCommonEntityAttributesProperties<RS_MText>(entity);

    entity->addVector("insertX", [](RS_MText* e) {
        return e->getInsertionPoint();
    }, tr("Insert X"), tr("X coordinate for text's insertion point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("insertY", [](RS_MText* e) {
        return e->getInsertionPoint();
    }, tr("Insert Y"), tr("Y coordinate for text's insertion point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("angle", [](RS_MText* e) {
        return e->getAngle();
    }, tr("Angle"), tr("Text rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<QString>("style", [](RS_MText* e) {
        return e->getStyle();
    }, tr("Style"), tr("Name of the text style"), LC_PropertyMatcherTypes::STRING); // fixme - choice combox for available text style names?

    entity->add<double>("height", [](RS_MText* e) {
        return e->getHeight();
    }, tr("Height"), tr("Height of the text"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("width", [](RS_MText* e) {
        return e->getWidth();
    }, tr("Width"), tr("Width of the text"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<int>("linesCount", [](RS_MText* e) {
        return e->getNumberOfLines();
    }, tr("Lines Count"), tr("Number of lines in multiline text"), LC_PropertyMatcherTypes::INT);

    entity->add<int>("halign", [](RS_MText* e) {
        return e->getHAlign();
    }, tr("Horizontal Align"), tr("Horizontal align for text"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("Left"), QVariant(RS_MTextData::HAlign::HALeft)},
        {tr("Center"), QVariant(RS_MTextData::HAlign::HACenter)},
        {tr("Right"), QVariant(RS_MTextData::HAlign::HARight)}
    });

    entity->add<int>("valign", [](RS_MText* e) {
        return e->getVAlign();
    }, tr("Vertical Align"), tr("Vertical align for text"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("Bottom"), QVariant(RS_MTextData::VAlign::VABottom)},
        {tr("Middle"), QVariant(RS_MTextData::VAlign::VAMiddle)},
        {tr("Top"), QVariant(RS_MTextData::VAlign::VATop)}
    });

    entity->add<int>("drawDirection", [](RS_MText* e) {
        return e->getVAlign();
    }, tr("Direction"), tr("Drawing direction for the text"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("Left to right"), QVariant(RS_MTextData::MTextDrawingDirection::LeftToRight)},
        {tr("Right to left"), QVariant(RS_MTextData::MTextDrawingDirection::RightToLeft)},
        {tr("Top to bottom"), QVariant(RS_MTextData::MTextDrawingDirection::TopToBottom)},
        {tr("By Style"), QVariant(RS_MTextData::MTextDrawingDirection::ByStyle)},
    });

    entity->add<int>("lspacingStyle", [](RS_MText* e) {
        return e->getVAlign();
    }, tr("Line spacing style"), tr("Style of linespacing"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("At Least"), QVariant(RS_MTextData::MTextLineSpacingStyle::AtLeast)},
        {tr("Exact"), QVariant(RS_MTextData::MTextLineSpacingStyle::Exact)}
    });

    entity->add<double>("linespacingFactor", [](RS_MText* e) {
        return e->getLineSpacingFactor();
    }, tr("Linespacing"), tr("Linespacing factor for the text"), LC_PropertyMatcherTypes::DOUBLE);

    map.insert(RS2::EntityMText, entity);

}
