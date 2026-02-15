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

#include "lc_entity_type_propertiesprovider.h"

#include "lc_property_double_interactivepick_view.h"
#include "lc_property_layer.h"
#include "lc_property_linetype.h"
#include "lc_property_linewidth.h"
#include "lc_property_multi.h"
#include "lc_property_rscolor.h"
#include "lc_property_rsvector_view.h"
#include "rs_document.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_settings.h"

const QString LC_EntityTypePropertiesProvider::SECTION_GENERAL = "_secGeneral";
const QString LC_EntityTypePropertiesProvider::SECTION_GEOMETRY = "_secGeometry";
const QString LC_EntityTypePropertiesProvider::SECTION_CALCULATED_INFO = "_secCalcInfo";
const QString LC_EntityTypePropertiesProvider::SECTION_TEXT = "_secText";

LC_PropertyRSVector* LC_EntityTypePropertiesProvider::createVectorProperty(const LC_Property::Names& names,
                                                                           QList<LC_PropertyAtomic*>* props, LC_PropertyContainer* cont,
                                                                           LC_ActionContext* actionContext,
                                                                           LC_LateCompletionRequestor* requestor) {
    auto* property = new LC_PropertyRSVector(cont, false);
    property->setNames(names);
    if (requestor != nullptr) {
        property->setInteractiveInputType(LC_ActionContext::InteractiveInputInfo::POINT);
    }
    property->setViewDescriptorProvider([]() -> LC_PropertyViewDescriptor {
        return {{{LC_PropertyRSVectorView::ATTR_X_DISPLAY_NAME, tr("X")}, {LC_PropertyRSVectorView::ATTR_Y_DISPLAY_NAME, tr("Y")}}};
    });
    property->setActionContextAndLaterRequestor(actionContext, requestor);
    props->push_back(property);
    return property;
}

void LC_EntityTypePropertiesProvider::addMultipleProperties(LC_PropertyContainer* cont, QList<LC_PropertyAtomic*> props) {
    const auto firstProperty = props.first();
    const auto propertyMulti = new LC_PropertyMulti(firstProperty->metaObject(), cont);
    for (const auto prop : props) {
        propertyMulti->addProperty(prop);
    }
    propertyMulti->setName(firstProperty->getName());
    propertyMulti->setDisplayName(firstProperty->getDisplayName());
    propertyMulti->setDescription(firstProperty->getDescription());
    cont->addChildProperty(propertyMulti);
}

LC_PropertyQString* LC_EntityTypePropertiesProvider::createReadonlyStringProperty(const LC_Property::Names& names,
                                                                                  QList<LC_PropertyAtomic*>* props,
                                                                                  LC_PropertyContainer* cont, const QString& value) {
    auto* property = new LC_PropertyQString(cont, true);
    property->setNames(names);
    property->setValue(value);
    property->setReadOnly();
    props->push_back(property);
    return property;
}

LC_PropertyDouble* LC_EntityTypePropertiesProvider::createDoubleProperty(const LC_Property::Names& names, QList<LC_PropertyAtomic*>* props,
                                                                         LC_PropertyContainer* cont,
                                                                         const LC_ActionContext::InteractiveInputInfo::InputType inputType,
                                                                         LC_ActionContext* actionContext,
                                                                         LC_LateCompletionRequestor* requestor) {
    auto* property = new LC_PropertyDouble(cont, false);
    property->setNames(names);
    property->setInteractiveInputType(inputType);
    if (inputType != LC_ActionContext::InteractiveInputInfo::NOTNEEDED) {
        LC_PropertyViewDescriptor attrs;
        attrs.viewName = LC_PropertyDoubleInteractivePickView::VIEW_NAME;
        property->setViewDescriptor(attrs);
    }
    property->setActionContextAndLaterRequestor(actionContext, requestor);
    props->push_back(property);
    return property;
}

QString LC_EntityTypePropertiesProvider::formatLinear(const double length) const {
    const LC_Formatter* formatter = getFormatter();
    return formatter->formatLinear(length);
}

QString LC_EntityTypePropertiesProvider::formatDouble(const double length) const {
    const LC_Formatter* formatter = getFormatter();
    return formatter->formatDouble(length);
}

QString LC_EntityTypePropertiesProvider::formatInt(const int length) const {
    const LC_Formatter* formatter = getFormatter();
    return formatter->formatInt(length);
}

QString LC_EntityTypePropertiesProvider::formatInt(const double length) const {
    const LC_Formatter* formatter = getFormatter();
    const int len = static_cast<int>(length);
    return formatter->formatInt(len);
}

QString LC_EntityTypePropertiesProvider::formatInt(const qsizetype length) const {
    const LC_Formatter* formatter = getFormatter();
    const int len = static_cast<int>(length);
    return formatter->formatInt(len);
}

QString LC_EntityTypePropertiesProvider::formatWCSAngleDegrees(const double wcsAngle) const {
    const LC_Formatter* formatter = getFormatter();
    auto result = formatter->formatWCSAngleDegrees(wcsAngle);
    return result;
}

QString LC_EntityTypePropertiesProvider::formatRawAngle(const double length) const {
    const LC_Formatter* formatter = getFormatter();
    auto result = formatter->formatRawAngle(length);
    return result;
}

QString LC_EntityTypePropertiesProvider::formatRawAngle(const double length, const RS2::AngleFormat format) const {
    const LC_Formatter* formatter = getFormatter();
    auto result = formatter->formatRawAngle(length, format);
    return result;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createGeometrySection(LC_PropertyContainer* container) const {
    const auto result = createSection(container, {SECTION_GEOMETRY, tr("Geometry"), tr("Geometrical properties")});
    return result;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createTextContainer(LC_PropertyContainer* container) const {
    const auto contText = createSection(container, {SECTION_TEXT, tr("Text"), tr("Text properties")});
    return contText;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createCalculatedInfoSection(LC_PropertyContainer* container) const {
    const bool createCalculatedSection = m_widget->getOptions()->showComputed;
    if (createCalculatedSection) {
        const auto result = createSection(container, {SECTION_CALCULATED_INFO, tr("Other"), tr("Other properties")});
        return result;
    }
    return nullptr;
}

LC_Formatter* LC_EntityTypePropertiesProvider::getFormatter() const {
    const auto graphicView = m_actionContext->getGraphicView();
    const auto viewport = graphicView->getViewPort();
    LC_Formatter* result = viewport->getFormatter();
    return result;
}

RS_Vector LC_EntityTypePropertiesProvider::toUCS(const RS_Vector& wcs) const {
    const auto graphicView = m_actionContext->getGraphicView();
    const auto viewport = graphicView->getViewPort();
    const auto ucs = viewport->toUCS(wcs);
    return ucs;
}

double LC_EntityTypePropertiesProvider::toUCSBasisAngle(const double wcsAngle) const {
    const auto graphicView = m_actionContext->getGraphicView();
    const auto viewport = graphicView->getViewPort();
    const double ucsAngle = viewport->toUCSAngle(wcsAngle);
    const double ucsBasisAngle = viewport->toBasisUCSAngle(ucsAngle);
    return ucsBasisAngle;
}

double LC_EntityTypePropertiesProvider::toWCSAngle(const double ucsBasicAngle) const {
    const auto graphicView = m_actionContext->getGraphicView();
    const auto viewport = graphicView->getViewPort();
    const double ucsAngle = viewport->toAbsUCSAngle(ucsBasicAngle);
    const double wcsAngle = viewport->toWorldAngle(ucsAngle);
    return wcsAngle;
}

RS_Vector LC_EntityTypePropertiesProvider::toWCS(const RS_Vector& ucs) const {
    const auto viewport = m_actionContext->getGraphicView()->getViewPort();
    const auto wcs = viewport->toWorld(ucs);
    return wcs;
}

void LC_EntityTypePropertiesProvider::fillEntityProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& entitiesList) {
    fillGenericAttributes(container, entitiesList);
    doFillEntitySpecificProperties(container, entitiesList);
}

void LC_EntityTypePropertiesProvider::addCommon(const LC_Property::Names& names, const FunCreateGenericProperty& propertyInit,
                                                const QList<RS_Entity*>& list, LC_PropertyContainer* cont) {
    QList<LC_PropertyAtomic*> props;
    props.reserve(list.size());
    for (const auto entity : list) {
        propertyInit(names, entity, cont, &props);
    }
    addMultipleProperties(cont, props);
}

LC_PropertyContainer*
LC_EntityTypePropertiesProvider::createSection(LC_PropertyContainer* container, const LC_Property::Names& names) const {
    const auto result = new LC_PropertyContainer(container);
    result->setNames(names);
    container->addChildProperty(result);
    m_widget->checkSectionCollapsed(result);
    return result;
}

void LC_EntityTypePropertiesProvider::fillGenericAttributes(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    const auto containerGeneric = createSection(container, {SECTION_GENERAL, tr("General"), tr("General properties")});

    const RS_Document* doc = getDocument();
    RS_Graphic* graphic = doc->getGraphic();
    RS_LayerList* layerList = graphic->getLayerList();
    bool allowByBlock = doc->getBlock() != nullptr;

    addCommon({"layer", tr("Layer"), tr("Layer of entity")},
              [this, layerList, allowByBlock](const LC_Property::Names& names, RS_Entity* entity, LC_PropertyContainer* cont,
                                              QList<LC_PropertyAtomic*>* props) -> void {
                  auto* property = new LC_PropertyLayer(cont, false);
                  property->setNames(names);
                  property->setLayerList(layerList);
                  property->setAllowByBlockValues(allowByBlock);
                  createDelegatedStorage<RS_Layer*, RS_Entity>([](const RS_Entity* e) -> RS_Layer* {
                                                                   return e->getLayer();
                                                               }, [](RS_Layer* l, RS_Entity* e) -> void {
                                                                   e->setLayer(l);
                                                               }, [](const RS_Layer* l, const RS_Entity* e) -> bool {
                                                                   return l == e->getLayer();
                                                               }, entity, property);
                  props->push_back(property);
              }, list, containerGeneric);

    addCommon({"color", tr("Color"), tr("Color of entity")}, [this](const LC_Property::Names& names, RS_Entity* entity,
                                                                    LC_PropertyContainer* cont, QList<LC_PropertyAtomic*>* props) -> void {
        auto* property = new LC_PropertyRSColor(cont, false);
        property->setNames(names);
        createDelegatedStorage<RS_Color, RS_Entity>([](const RS_Entity* e) -> RS_Color {
                                                        return e->getPen(false).getColor();
                                                    }, [](const RS_Color& color,  const RS_Entity* e) -> void {
                                                        RS_Pen pen = e->getPen(false);
                                                        pen.setColor(color);
                                                        e->setPen(pen);
                                                    }, [](const RS_Color& l, const RS_Entity* e) -> bool {
                                                        const bool equals = l == e->getPen(false).getColor();
                                                        return equals;
                                                    }, entity, property);
        props->push_back(property);
    }, list, containerGeneric);

    addCommon({"linetype", tr("Line Type"), tr("Type of entity pen line")},
              [this](const LC_Property::Names& names, RS_Entity* entity, LC_PropertyContainer* cont,
                     QList<LC_PropertyAtomic*>* props) -> void {
                  auto* property = new LC_PropertyLineType(cont, false);
                  property->setNames(names);
                  createDelegatedStorage([](const RS_Entity* e) -> RS2::LineType {
                                             return e->getPen(false).getLineType();
                                         }, [](const RS2::LineType& linetype, const RS_Entity* e) -> void {
                                             RS_Pen pen = e->getPen(false);
                                             pen.setLineType(linetype);
                                             e->setPen(pen);
                                         }, [](const RS2::LineType& l, const RS_Entity* e) -> bool {
                                             const bool equals = l == e->getPen(false).getLineType();
                                             return equals;
                                         }, entity, property);
                  props->push_back(property);
              }, list, containerGeneric);

    addCommon({"linewidth", tr("Line Width"), tr("Width of entity pen line")},
              [this](const LC_Property::Names& names, RS_Entity* entity, LC_PropertyContainer* cont,
                     QList<LC_PropertyAtomic*>* props) -> void {
                  auto* property = new LC_PropertyLineWidth(cont, false);
                  property->setNames(names);
                  createDelegatedStorage([](const RS_Entity* e) -> RS2::LineWidth {
                                             return e->getPen(false).getWidth();
                                         }, [](const RS2::LineWidth& width, const RS_Entity* e) -> void {
                                             RS_Pen pen = e->getPen(false);
                                             pen.setWidth(width);
                                             e->setPen(pen);
                                         }, [](const RS2::LineWidth& l, const RS_Entity* e) -> bool {
                                             const bool equals = l == e->getPen(false).getWidth();
                                             return equals;
                                         }, entity, property);
                  props->push_back(property);
              }, list, containerGeneric);
}
