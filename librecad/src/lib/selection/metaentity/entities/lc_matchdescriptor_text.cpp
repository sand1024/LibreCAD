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

#include "lc_matchdescriptor_text.h"

#include "rs_text.h"

void LC_MatchDescriptorText::init(QMap<RS2::EntityType, LC_EntityMatchDescriptor*>& map) {
        auto entity = new LC_TypedEntityMatchDescriptor<RS_Text>(tr("Text"), RS2::EntityText);
    initCommonEntityAttributesProperties<RS_Text>(entity);

    entity->addVector("insertX", [](RS_Text* e) {
        return e->getInsertionPoint();
    }, tr("Insert X"), tr("X coordinate for text's insertion point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("insertY", [](RS_Text* e) {
        return e->getInsertionPoint();
    }, tr("Insert Y"), tr("Y coordinate for text's insertion point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->addVector("secondX", [](RS_Text* e) {
        return e->getInsertionPoint();
    }, tr("Second Point X"), tr("X coordinate for text second point"), LC_PropertyMatcherTypes::COORD_X);

    entity->addVector("secondY", [](RS_Text* e) {
        return e->getInsertionPoint();
    }, tr("Second Point Y"), tr("Y coordinate for text second point"), LC_PropertyMatcherTypes::COORD_Y);

    entity->add<double>("angle", [](RS_Text* e) {
        return e->getAngle();
    }, tr("Angle"), tr("Text rotation angle"), LC_PropertyMatcherTypes::ANGLE);

    entity->add<double>("width", [](RS_Text* e) {
        return e->getWidthRel();
    }, tr("Width"), tr("Width of the text"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<double>("height", [](RS_Text* e) {
        return e->getHeight();
    }, tr("Height"), tr("Height of the text"), LC_PropertyMatcherTypes::LENGTH);

    entity->add<QString>("style", [](RS_Text* e) {
        return e->getStyle();
    }, tr("Style"), tr("Name of the text style"), LC_PropertyMatcherTypes::STRING); // fixme - choice combox for available text style names?

    entity->add<int>("halign", [](RS_Text* e) {
        return e->getHAlign();
    }, tr("Horizontal Align"), tr("Horizontal align for text"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("Left"), QVariant(RS_TextData::HAlign::HALeft)},
        {tr("Center"), QVariant(RS_TextData::HAlign::HACenter)},
        {tr("Right"), QVariant(RS_TextData::HAlign::HARight)},
        {tr("Aligned"), QVariant(RS_TextData::HAlign::HAAligned)},
        {tr("Middle"), QVariant(RS_TextData::HAlign::HAMiddle)},
        {tr("Fit"), QVariant(RS_TextData::HAlign::HAFit)},
    });

    entity->add<int>("valign", [](RS_Text* e) {
        return e->getVAlign();
    }, tr("Vertical Align"), tr("Vertical align for text"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("Baseline"), QVariant(RS_TextData::VAlign::VABaseline)},
        {tr("Bottom"), QVariant(RS_TextData::VAlign::VABottom)},
        {tr("Middle"), QVariant(RS_TextData::VAlign::VAMiddle)},
        {tr("Top"), QVariant(RS_TextData::VAlign::VATop)}
    });

    entity->add<int>("generation", [](RS_Text* e) {
        return e->getTextGeneration();
    }, tr("Generation"), tr("Text generation style"), LC_PropertyMatcherTypes::INT_CHOICE, {
        {tr("None"), QVariant(RS_TextData::TextGeneration::None)},
        {tr("Backward"), QVariant(RS_TextData::TextGeneration::Backward)},
        {tr("Upside-down"), QVariant(RS_TextData::TextGeneration::UpsideDown)}
    });

    map.insert(RS2::EntityText, entity);

}
