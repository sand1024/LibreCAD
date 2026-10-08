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

#include "lc_property_layer.h"
#include "lc_property_linetype.h"
#include "lc_property_linewidth.h"
#include "lc_property_multi.h"
#include "lc_property_rscolor.h"
#include "lc_property_rsvector_view.h"
#include "lc_propertyprovider_utils.h"
#include "rs_document.h"
#include "rs_graphic.h"
#include "rs_selection.h"
#include "rs_settings.h"

const QString LC_EntityTypePropertiesProvider::SECTION_GENERAL = "_secGeneral";
const QString LC_EntityTypePropertiesProvider::SECTION_GEOMETRY = "_secGeometry";
const QString LC_EntityTypePropertiesProvider::SECTION_CALCULATED_INFO = "_secCalcInfo";
const QString LC_EntityTypePropertiesProvider::SECTION_SINGLE_ENTITY_ACTIONS = "_secSingleEntityActions";
const QString LC_EntityTypePropertiesProvider::SECTION_MULTI_ENTITY_ACTIONS = "_secMultiEntityActions";
const QString LC_EntityTypePropertiesProvider::SECTION_TEXT = "_secText";
const QString LC_EntityTypePropertiesProvider::SECTION_TOOL_OPTIONS = "_secToolOptions";
const QString LC_EntityTypePropertiesProvider::SECTION_SNAP_TOOL_OPTIONS = "_secSnapToolOptions";

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

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createGeometrySection(LC_PropertyContainer* container) const {
    const auto result = createSection(container, {SECTION_GEOMETRY, tr("Geometry"), tr("Geometrical properties")});
    return result;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createTextContainer(LC_PropertyContainer* container) const {
    const auto contText = createSection(container, {SECTION_TEXT, tr("Text"), tr("Text properties")});
    return contText;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createCalculatedInfoSection(LC_PropertyContainer* container) const {
    const auto result = createSection(container, {SECTION_CALCULATED_INFO, tr("Calculated"), tr("Calculated properties")});
    return result;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createSingleEntityActionsSection(LC_PropertyContainer* container) const {
    const auto result = createSection(container, {SECTION_SINGLE_ENTITY_ACTIONS, tr("Single Entity Actions"), tr("Action commands applicable to single edited entity")});
    return result;
}

LC_PropertyContainer* LC_EntityTypePropertiesProvider::createMultipleEntityActionsSection(LC_PropertyContainer* container) const {
    const auto result = createSection(container, {SECTION_MULTI_ENTITY_ACTIONS, tr("Selected Set Actions"), tr("Action commands applicable for ALL selected entities")});
    return result;
}


void LC_EntityTypePropertiesProvider::doCreateSelectedSetCommands(LC_PropertyContainer* propertyContainer, [[maybe_unused]]const QList<RS_Entity*>& list) {
    const std::list<CommandLinkInfo> commands = {
        {tr("Moving or rotating operations"), RS2::ActionModifyMove, RS2::ActionModifyRotate},
        {tr("Duplicating or mirroring"), RS2::ActionModifyCopy, RS2::ActionModifyDuplicate,},
        {tr("Scaling and stretching"), RS2::ActionModifyScale, RS2::ActionModifyStretch},
        {tr("Mirroring"), RS2::ActionModifyMirror},
        {tr("Aligning selection"), RS2::ActionModifyAlign, RS2::ActionModifyAlignRef},
        {tr("Aligning selection"), RS2::ActionModifyMoveRotate, RS2::ActionModifyRotateTwice}
    };

    createEntityContextCommands<RS_Document>(commands, propertyContainer, nullptr, "multiEntityCommands", false);
}

void LC_EntityTypePropertiesProvider::doCreateSingleEntityCommands([[maybe_unused]]LC_PropertyContainer* cont, [[maybe_unused]]RS_Entity* entity){}

void LC_EntityTypePropertiesProvider::fillSelectedSetCommands(LC_PropertyContainer* container, const QList<RS_Entity*>& entitiesList) {
    const auto multiCommandsContainer = createMultipleEntityActionsSection(container);
    doCreateSelectedSetCommands(multiCommandsContainer, entitiesList);
}

void LC_EntityTypePropertiesProvider::fillComputedProperites(LC_PropertyContainer* container, const QList<RS_Entity*>& entitiesList) {
    const auto contComputed = createCalculatedInfoSection(container);
    doCreateCalculatedProperties(contComputed, entitiesList);
}

void LC_EntityTypePropertiesProvider::fillSingleEntityCommands(LC_PropertyContainer* container, const QList<RS_Entity*>& entitiesList) {
    const auto cont = createSingleEntityActionsSection(container);
    doCreateSingleEntityCommands(cont, entitiesList.first());
}

void LC_EntityTypePropertiesProvider::fillEntityProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& entitiesList) {
    fillGenericAttributes(container, entitiesList);
    doCreateEntitySpecificProperties(container, entitiesList);
    if (isShowComputedSection()) {
        fillComputedProperites(container, entitiesList);
    }
    const bool singleEntity = entitiesList.size() == 1;
    if (singleEntity && isShowSingleEntitySection()) {
        fillSingleEntityCommands(container, entitiesList);
    }
    if (m_widget->getOptions()->showMultiEntityCommands) {
        fillSelectedSetCommands(container, entitiesList);
    }
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

    if (isShowLinks() && list.size() == 1) {
        auto layerClickHandler = [this]([[maybe_unused]] const RS_Entity* entity, const int linkIndex) {
            bool select = false;
            switch (linkIndex) {
                case 0: {
                    select = true;
                    break;
                }
                case 1: {
                    select = false;
                    break;
                }
                default:
                    select = true;
                    break;
            }
            const RS_Selection s(getDocument(), m_actionContext->getViewport());
            s.selectLayer(entity->getLayer(true), select);
        };

        LC_PropertyProviderUtils::createSingleEntityCommand<RS_Entity>(containerGeneric, "layerSingleSelect", tr("Select all"),
                                                                       tr("Select All\n\nSelect all entities in layer"), tr("Unselect All"),
                                                                       tr("Unselect All\n\nUnselect all entities in layer"), list.first(),
                                                                       layerClickHandler, tr("Selection of layer's entities"));
    }

    addCommon({"color", tr("Color"), tr("Color of entity")}, [this](const LC_Property::Names& names, RS_Entity* entity,
                                                                    LC_PropertyContainer* cont, QList<LC_PropertyAtomic*>* props) -> void {
        auto* property = new LC_PropertyRSColor(cont, false);
        property->setNames(names);
        createDelegatedStorage<RS_Color, RS_Entity>([](const RS_Entity* e) -> RS_Color {
                                                        return e->getPen(false).getColor();
                                                    }, [](const RS_Color& color, const RS_Entity* e) -> void {
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

void LC_EntityTypePropertiesProvider::createActionBasedCommands(LC_PropertyContainer* cont, const char* propertyName,
    QString commonDescription, RS2::ActionType actionType1, RS2::ActionType actionType2) const {
    auto pickClickHandler = [this, actionType1, actionType2]([[maybe_unused]] RS_Document* g, const int linkIndex) {
        switch (linkIndex) {
            case 0: {
                m_actionContext->setCurrentAction(actionType1, nullptr);
                break;
            }
            case 1: {
                m_actionContext->setCurrentAction(actionType2, nullptr);
                break;
            }
            default:
                break;
        }
    };

    const auto left = m_widget->getActionDescriptor(actionType1);
    auto right = m_widget->getActionDescriptor(actionType2);

    LC_PropertyProviderUtils::createSingleEntityCommand<RS_Document>(cont, propertyName, left.title, left.tooltip, right.title,
                                                                     right.tooltip, nullptr, pickClickHandler, commonDescription);
}
