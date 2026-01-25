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

#include "lc_propertysheetwidget.h"

#include <QTimer>

#include "lc_actioncontext.h"
#include "lc_entity_property_containerprovider.h"
#include "lc_entitymetauiutils.h"
#include "lc_graphicviewport.h"
#include "lc_properties_sheet.h"
#include "lc_property_container.h"
#include "lc_property_double.h"
#include "lc_property_layer.h"
#include "lc_property_multi.h"
#include "lc_property_rsvector.h"
#include "lc_rectregion.h"
#include "rs_debug.h"
#include "rs_document.h"
#include "rs_graphicview.h"
#include "rs_selection.h"
#include "ui_lc_propertysheetwidget.h"

LC_PropertySheetWidget::LC_PropertySheetWidget(QWidget* parent,LC_ActionContext* actionContext, QAction* selectQuick,
                                               QAction* toggleSelectModeAction, QAction* selectEntitiesAction)
    : LC_GraphicViewAwareWidget(parent), ui(new Ui::LC_PropertySheetWidget), m_actionContext{actionContext} {
    ui->setupUi(this);

    connect(ui->propertySheet->propertiesSheet(), &LC_PropertiesSheet::propertyEdited, this, &LC_PropertySheetWidget::onPropertyEdited);
    connect(ui->propertySheet->propertiesSheet(), &LC_PropertiesSheet::beforePropertyEdited, this, &LC_PropertySheetWidget::onBeforePropertyEdited);
    connect(ui->propertySheet->propertiesSheet(), &LC_PropertiesSheet::activePropertyChanged, this, &LC_PropertySheetWidget::onActivePropertyChanged);
    connect(ui->cbSelection, &QComboBox::currentIndexChanged, this, &LC_PropertySheetWidget::onSelectionIndexChanged);

    ui->tbSelectQuick->setDefaultAction(selectQuick);
    ui->tbPickAddSwitch->setDefaultAction(toggleSelectModeAction);
    ui->tbSelectObjects->setDefaultAction(selectEntitiesAction);

    m_entityContainerProvider = std::make_unique<LC_EntityPropertyContainerProvider>();
    m_entityContainerProvider->init(this, actionContext);

    updateWidgetSettings();
}

LC_PropertySheetWidget::~LC_PropertySheetWidget() {
    delete ui;
}

void LC_PropertySheetWidget::setGraphicView(RS_GraphicView* gv) {
    RS_Document* doc = nullptr;
    LC_GraphicViewport* viewport = nullptr;
    // remove tracking of relative point from old view
    if (m_graphicView != nullptr) {
        disconnect(m_graphicView, &RS_GraphicView::relativeZeroChanged, this, &LC_PropertySheetWidget::onRelativeZeroChanged);
        disconnect(m_graphicView, &RS_GraphicView::ucsChanged, this, &LC_PropertySheetWidget::onUcsChanged);
        disconnect(m_graphicView, &RS_GraphicView::defaultActionActivated, this, &LC_PropertySheetWidget::onViewDefaultActionActivated);
        if (m_document != nullptr) {
            m_document->getSelection()->removeListener(this);
        }
    }

    m_graphicView = gv;

    // add tracking of relative point for new view
    if (gv != nullptr) {
        connect(m_graphicView, &RS_GraphicView::relativeZeroChanged, this, &LC_PropertySheetWidget::onRelativeZeroChanged);
        connect(m_graphicView, &RS_GraphicView::ucsChanged, this, &LC_PropertySheetWidget::onUcsChanged);
        connect(m_graphicView, &RS_GraphicView::defaultActionActivated, this, &LC_PropertySheetWidget::onViewDefaultActionActivated);
        viewport = gv->getViewPort();
        doc = gv->getDocument();
        doc->getSelection()->addListener(this);
    }

    m_document = doc;
    m_viewport = viewport;

    m_entityContainerProvider->setGraphicView(gv);

    initPropertySheet();
    setupSelectionTypeCombobox(RS2::EntityContainer,"");
}

void LC_PropertySheetWidget::selectionChanged() {
    // LC_ERR << "On Selection Changed!";

    if (m_handleSelectionChange) {
        const int selectionIndex = ui->cbSelection->currentIndex();
        if (selectionIndex >= 0) {
            // save current state
            int itemEntityType = getCurrentlySelectedEntityType(selectionIndex);
            const auto entityType = static_cast<RS2::EntityType>(itemEntityType);
            const auto activeProperty = ui->propertySheet->propertiesSheet()->activeProperty();
            const QString propertyName = activeProperty  != nullptr ? activeProperty->getName() : "";

            setupSelectionTypeCombobox(entityType, propertyName);
        }
        else {
            // clean init
            setupSelectionTypeCombobox(RS2::EntityContainer, "");
        }
    }
}

void LC_PropertySheetWidget::setShouldHandleSelectionChange(const bool value) {
    m_handleSelectionChange = value;
}

void LC_PropertySheetWidget::updateFormats() {
}

void LC_PropertySheetWidget::onLateRequestCompleted(const bool shouldBeSkipped) {
    if (shouldBeSkipped) {
        const auto interactiveInput = m_actionContext->getInteractiveInputInfo();
        interactiveInput->requestor = nullptr;
        interactiveInput->state = LC_ActionContext::InteractiveInputInfo::NONE;
        // fixme - cancel editing or defreese state?
    }
    else {
        const auto interactiveInputInfo = m_actionContext->getInteractiveInputInfo();
        const bool updateInteractiveInputValues = interactiveInputInfo->state == LC_ActionContext::InteractiveInputInfo::REQUESTED;

        auto inputType = interactiveInputInfo->inputType;

        if (updateInteractiveInputValues) {
            switch (inputType) {
                case LC_ActionContext::InteractiveInputInfo::DISTANCE: {
                    setPickedPropertyValue(interactiveInputInfo->requestorTag, interactiveInputInfo->distance, inputType);
                    break;
                }
                case LC_ActionContext::InteractiveInputInfo::ANGLE: {
                    setPickedPropertyValue(interactiveInputInfo->requestorTag, interactiveInputInfo->angleRad, inputType);
                    break;
                }
                case LC_ActionContext::InteractiveInputInfo::POINT: {
                    const RS_Vector ucsVector = m_viewport->toUCS(interactiveInputInfo->wcsPoint);
                    setPickedPointPropertyValue(interactiveInputInfo->requestorTag, ucsVector);
                    break;
                }
                case LC_ActionContext::InteractiveInputInfo::POINT_X: {
                    const RS_Vector ucsVector = m_viewport->toUCS(interactiveInputInfo->wcsPoint);
                    setPickedPropertyCoordinateValue(interactiveInputInfo->requestorTag, ucsVector.x, true);
                    break;
                }
                case LC_ActionContext::InteractiveInputInfo::POINT_Y: {
                    const RS_Vector ucsVector = m_viewport->toUCS(interactiveInputInfo->wcsPoint);
                    setPickedPropertyCoordinateValue(interactiveInputInfo->requestorTag, ucsVector.y, false);
                    break;
                }
                default:
                    break;
            }
        }
        else {
           inputType = LC_ActionContext::InteractiveInputInfo::NOTNEEDED;
        }
    }
}

void LC_PropertySheetWidget::onUcsChanged([[maybe_unused]] LC_UCS* ucs) {
    m_handleSelectionChange = true;
    selectionChanged();
}

void LC_PropertySheetWidget::onViewDefaultActionActivated(const bool defaultActionActivated, const RS2::ActionType prevActionRtti) {
    setShouldHandleSelectionChange(defaultActionActivated);
    if (defaultActionActivated) {
        // prevent refresh of the widget if activation is called after interactive input  - it might be that update
        // is already performed as result of setting value of property
        if (!RS2::isInteractiveInputAction(prevActionRtti) && prevActionRtti != RS2::ActionDefault) {
            selectionChanged();
        }
    }
}

void LC_PropertySheetWidget::onRelativeZeroChanged(const RS_Vector&) {
}

void LC_PropertySheetWidget::setupSelectionTypeCombobox(const RS2::EntityType entityTypeTryToSet,  [[maybe_unused]]QString propertyTryToSet)  {
    ui->cbSelection->blockSignals(true);
    ui->cbSelection->clear();
    int entityTypesCount = 0;
    if (m_document != nullptr) {
        const auto selection = RS_Selection(m_document, m_viewport);
        QMap<RS2::EntityType, int> selectedEntityTypesMap;
        selection.countSelectedEntities(selectedEntityTypesMap);
        LC_EntityMetaUIUtils::setupSelectionEntityTypesCombobox(ui->cbSelection, selectedEntityTypesMap);
        entityTypesCount = selectedEntityTypesMap.count();
    }
    ui->cbSelection->blockSignals(false);

    int indexToSet = 0;

    if (entityTypesCount == 1) {
        indexToSet = 1;
    }
    else {
        if (entityTypeTryToSet != RS2::EntityContainer) {
            const int indexForEntityType = ui->cbSelection->findData(entityTypeTryToSet, Qt::UserRole);
            if (indexForEntityType != -1) {
                indexToSet = indexForEntityType;
            }
        }
    }

    const int currentIndex = ui->cbSelection->currentIndex();
    if (currentIndex == indexToSet) {
        onSelectionIndexChanged(currentIndex);
    }
    else {
        ui->cbSelection->setCurrentIndex(indexToSet);
    }
}

void LC_PropertySheetWidget::entityModified([[maybe_unused]] RS_Entity* originalEntity, [[maybe_unused]] RS_Entity* entityClone) {
    LC_ERR << "entity Modified!";
    m_orginalEntities.push_back(originalEntity);
    m_modifiedEntities.push_back(entityClone);
}

void LC_PropertySheetWidget::clearContextEntities() {
    for (const auto e: m_orginalEntities) {
        if (RS2::isDimensionalEntity(e->rtti())) {
            auto *d  = static_cast<RS_Dimension*>(e);
            d->clearCachedDimStyle();
        }
    }
    for (const auto e: m_modifiedEntities) {
        if (RS2::isDimensionalEntity(e->rtti())) {
            const auto d  = static_cast<RS_Dimension*>(e);
            d->clearCachedDimStyle();
        }
    }
    m_orginalEntities.clear();
    m_modifiedEntities.clear();
}

void LC_PropertySheetWidget::initPropertySheet() {
}

void LC_PropertySheetWidget::onBeforePropertyEdited([[maybe_unused]]LC_Property* property, [[maybe_unused]] LC_Property::PropertyValuePtr newValue,
                                                    [[maybe_unused]] int typeId) {
    // LC_ERR << "On Before Edit " << property->getName() << " " << property->getRootProperty()->getName();
}

void LC_PropertySheetWidget::onPropertyEdited(LC_Property* property) {
    const bool propertyIsVirtual = isVirtualProperty(property);
    if (propertyIsVirtual) {
        return;
    }
    // special processing for layer property - hidden layer may be selected, so such entity should not be selected
    QString propertyName = property->getName();
    bool layerHidden = false;
    if (propertyName == "layer") {
        LC_PropertyAtomic* p = property->asAtomic();
        const auto* multiProperty = dynamic_cast<LC_PropertyMulti*>(p);
        if (multiProperty != nullptr) {
            const auto layerAtomic = multiProperty->getFirstProperty();
            const auto layerProperty = dynamic_cast<LC_PropertyLayer*>(layerAtomic);
            if (layerProperty != nullptr) {
                const RS_Layer* layer = layerProperty->value();
                if (layer != nullptr) {
                    layerHidden = layer->isFrozen();
                }
            }
        }
    }

    // fixme - add delayed modification method
    QTimer::singleShot(30,  [this, propertyName, layerHidden]()-> void {
        LC_ERR << "On Edited - " << propertyName;
        m_document->undoableModify(m_viewport, [this](LC_DocumentModificationBatch& ctx)-> bool {
                                       ctx.entitiesToAdd.append(m_modifiedEntities);
                                       ctx.entitiesToDelete.append(m_orginalEntities);
                                       ctx.dontSetActiveLayerAndPen();
                                       clearContextEntities();
                                       return true;
                                   }, [layerHidden](const LC_DocumentModificationBatch& ctx, RS_Document* doc)-> void {
                                       if (!layerHidden){
                                           doc->select(ctx.entitiesToAdd);
                                       }
                                   });
    });
}

int LC_PropertySheetWidget::getCurrentlySelectedEntityType(const int index) const {
    const int itemEntityType = ui->cbSelection->itemData(index, Qt::UserRole).toInt();
    return itemEntityType;
}

void LC_PropertySheetWidget::onSelectionIndexChanged(const int index) {
    int itemEntityType = getCurrentlySelectedEntityType(index);
    const auto entityType = static_cast<RS2::EntityType>(itemEntityType);
    const auto newContainer = preparePropertiesContainer(entityType);
    const auto previousContainer = ui->propertySheet->propertyContainer();
    ui->propertySheet->setPropertyContainer(newContainer);
    if (previousContainer != nullptr) {
        destroyContainer(previousContainer);
    }
}

LC_PropertyContainer* LC_PropertySheetWidget::preparePropertiesContainer(const RS2::EntityType entityType)  {
    QList<RS_Entity*> entitiesToModify;
    collectEntitiesToModify(entityType, entitiesToModify);
    LC_PropertyContainer* result = createPropertiesContainer(entityType, entitiesToModify);
    return result;
}

void LC_PropertySheetWidget::collectEntitiesToModify(RS2::EntityType entityType, QList<RS_Entity*>& entitiesToModify) const{
    if (entityType == RS2::EntityUnknown || entityType == RS2::EntityContainer) {
        m_document->collectSelected(entitiesToModify);
    }
    else {
        m_document->collectSelected(entitiesToModify, false, {entityType});
    }
}

LC_PropertyContainer* LC_PropertySheetWidget::createPropertiesContainer(const RS2::EntityType entityType, const QList<RS_Entity*>& list) {
    LC_PropertyContainer* result = nullptr;
    if (!list.isEmpty()) {
        if (isVisible()) {
            result = new LC_PropertyContainer(this);
        }
        m_entityContainerProvider->fillPropertyContainer(m_document, result, entityType, list);
    }
    else {
        result = new LC_PropertyContainer(this);
    }
    return result;
}

void LC_PropertySheetWidget::destroyContainer(LC_PropertyContainer* previousContainer) const {
    m_entityContainerProvider->cleanup();
    m_viewport->clearLocationsHighlight();
    previousContainer->deleteLater();
}

void LC_PropertySheetWidget::setPickedPropertyValue(const QString& propertyName, const double interactiveInputValue,
    [[maybe_unused]] LC_ActionContext::InteractiveInputInfo::InputType input) const {
    const auto propertyContainer = ui->propertySheet->propertyContainer();
    auto propertiesByName = propertyContainer->findChildProperties(propertyName);
    const auto propertiesSheet = ui->propertySheet->propertiesSheet();
    if (propertiesByName.isEmpty()) {
        LC_ERR << "Unable to find property by name " << propertyName;
    }
    else {
        const auto sheetModel = propertiesSheet->getModel();
        for (const auto p : propertiesByName) {
            auto* valueMultiProperty = dynamic_cast<LC_PropertyMulti*>(p);
            if (valueMultiProperty != nullptr) {
                const auto propertyItem = sheetModel->findItem(sheetModel->getRootItem(), valueMultiProperty);
                const auto valuePropertyAtomic = valueMultiProperty->getFirstProperty();
                const auto valuePropertyDouble = dynamic_cast<LC_PropertyDouble*>(valuePropertyAtomic);
                if (valuePropertyDouble != nullptr) {
                    const auto coordinatePropertyView = propertyItem->view.get();
                    valueMultiProperty->markEdited(true);
                    propertiesSheet->connectOnPropertyChange(valueMultiProperty, true);
                    valuePropertyDouble->setValue(interactiveInputValue, coordinatePropertyView->editReason());
                    propertiesSheet->connectOnPropertyChange(valueMultiProperty, false);
                }
            }
        }
    }
}

void LC_PropertySheetWidget::setPickedPropertyCoordinateValue(const QString& propertyName, const double interactiveInputValue, const bool forX) const {
    const auto propertyContainer = ui->propertySheet->propertyContainer();
    auto propertiesByName = propertyContainer->findChildProperties(propertyName);
    const auto propertiesSheet = ui->propertySheet->propertiesSheet();

    if (propertiesByName.isEmpty()) {
        LC_ERR << "Unable to find property by name " << propertyName;
    }
    else {
        const auto sheetModel = propertiesSheet->getModel();
        for (const auto p : propertiesByName) {
            const auto* vectorMultiProperty = dynamic_cast<LC_PropertyMulti*>(p);
            if (vectorMultiProperty == nullptr) {
                const auto vectorProperty = dynamic_cast<LC_PropertyRSVector*>(p);
                if (vectorProperty != nullptr) { // this is vertext of polyline, spline etc.
                    const int subPropertyIdx = forX ? 0 : 1; // x should be first component, y - second one
                    const auto propertyItem = sheetModel->findItem(sheetModel->getRootItem(), vectorProperty);
                    const auto coordinatePropertyItem = propertyItem->children.at(subPropertyIdx).get();
                    const auto coordinatePropertyView = coordinatePropertyItem->view.get();

                    const auto coordinateProperty = static_cast<LC_PropertyDouble*>(coordinatePropertyView->getProperty());

                    propertiesSheet->connectOnPropertyChange(vectorProperty, true);
                    coordinateProperty->setValue(interactiveInputValue, propertyItem->view->editReason());
                    propertiesSheet->connectOnPropertyChange(vectorProperty, false);
                }
            }
            else {  // this is normal case - we're in multi property
                const auto propertyItem = sheetModel->findItem(sheetModel->getRootItem(), vectorMultiProperty);
                if (propertyItem != nullptr) {
                    const auto vectorMultiPropertyView = propertyItem->view.get();
                    const int subPropertyIdx = forX ? 0 : 1; // x should be first component, y - second one]
                    const auto coordinateSubProperty = vectorMultiPropertyView->getSubProperty(subPropertyIdx);
                    auto* coordinateMultiProperty = dynamic_cast<LC_PropertyMulti*>(coordinateSubProperty);
                    if (coordinateMultiProperty != nullptr) {
                        const auto coordinateFirstAtomic = coordinateMultiProperty->getFirstProperty();
                        const auto coordinateDoubleProperty = dynamic_cast<LC_PropertyDouble*>(coordinateFirstAtomic);
                        if (coordinateDoubleProperty != nullptr) {
                            const auto coordinatePropertyItem = propertyItem->children.at(subPropertyIdx).get();
                            const auto coordinatePropertyView = coordinatePropertyItem->view.get();
                            coordinateMultiProperty->markEdited(true);
                            propertiesSheet->connectOnPropertyChange(vectorMultiProperty, true);
                            coordinateDoubleProperty->setValue(interactiveInputValue, coordinatePropertyView->editReason());
                            propertiesSheet->connectOnPropertyChange(vectorMultiProperty, false);
                        }
                    }
                }
            }
        }
    }
}

void LC_PropertySheetWidget::setPickedPointPropertyValue(const QString& propertyName, const RS_Vector &ucsVector) const {
    const auto propertyContainer = ui->propertySheet->propertyContainer();

    auto lcProperties = propertyContainer->findChildProperties(propertyName);
    if (lcProperties.isEmpty()) {
        LC_ERR << "Unable to find property by name " << propertyName;
    }
    else {
        const auto propertiesSheet = ui->propertySheet->propertiesSheet();
        const auto sheetModel = propertiesSheet->getModel();
        for (const auto p : lcProperties) {
            const auto vectorPropertyMulti = dynamic_cast<LC_PropertyMulti*>(p);
            if (vectorPropertyMulti == nullptr) {
                const auto vectorProperty = dynamic_cast<LC_PropertyRSVector*>(p);
                if (vectorProperty != nullptr) { // this is vertex of polyline, spline etc.
                    propertiesSheet->connectOnPropertyChange(vectorProperty, true);
                    const auto propertyItem = sheetModel->findItem(sheetModel->getRootItem(), vectorProperty);
                    vectorProperty->setValue(ucsVector, propertyItem->view->editReason());
                    propertiesSheet->connectOnPropertyChange(vectorProperty, false);
                }
            }
            else { // normal multi property
                const auto propertyItem = sheetModel->findItem(sheetModel->getRootItem(), vectorPropertyMulti);
                if (propertyItem != nullptr) {
                    const auto vectorAtomic = vectorPropertyMulti->getFirstProperty();
                    const auto vectorProperty = dynamic_cast<LC_PropertyRSVector*>(vectorAtomic);
                    if (vectorProperty != nullptr) {
                        vectorPropertyMulti->markEdited(true);
                        propertiesSheet->connectOnPropertyChange(vectorPropertyMulti, true);
                        vectorProperty->setValue(ucsVector, propertyItem->view->editReason());
                        propertiesSheet->connectOnPropertyChange(vectorPropertyMulti, false);
                    }
                }
            }
        }
    }
}

bool LC_PropertySheetWidget::isVirtualProperty(const LC_Property* property) {
    const auto descriptor = property->getViewDescriptor();
    bool result = false;
    if (descriptor != nullptr) {
        descriptor->load(LC_PropertyView::ATTR_VIRTUAL, result);
    }
    return result;
}

bool LC_PropertySheetWidget::isCollapsedSection(const QString& name) const {
    return m_collapsedContainerNames.contains(name);
}

void LC_PropertySheetWidget::markContainerCollapsed(const QString& name, const bool collapse) {
    if (collapse) {
        m_collapsedContainerNames << name;
    }
    else {
        m_collapsedContainerNames.remove(name);
    }
}

void LC_PropertySheetWidget::checkSectionCollapsed(LC_PropertyContainer* result) {
    const QString name = result->getName();
    const bool containerCollapsed = isCollapsedSection(name);
    if (containerCollapsed) {
        result->collapse();
    }
    connect(result, &LC_Property::afterPropertyChange, this, &LC_PropertySheetWidget::onSectionPropertyChanged);
}

void LC_PropertySheetWidget::onSectionPropertyChanged(const LC_PropertyChangeReason reason) {
    if (reason.testFlag(PropertyChangeReasonStateLocal)) {
        const auto property = qobject_cast<LC_Property *>(sender());
        const QString name = property->getName();
        const bool collapsed = property->isCollapsed();
        markContainerCollapsed(name, collapsed);
    }
}

void LC_PropertySheetWidget::onActivePropertyChanged(LC_Property* activeProperty) {
    m_viewport->clearLocationsHighlight();
    if (activeProperty == nullptr) {
        return;
    }
    const auto masterProperty = activeProperty->getMasterProperty();
    if (masterProperty != nullptr) {
        checkIfVectorAndGetLocation(masterProperty);
    }
    else {
        checkIfVectorAndGetLocation(activeProperty);
    }
}

void LC_PropertySheetWidget::highlightVectorPropertyPosition(const LC_PropertyRSVector* vectorProperty) const {
    LC_ERR << "is vector";
    if (!vectorProperty->isMultiValue()) {
        const RS_Vector ucsPosition = vectorProperty->value();
        const auto wcsPosition = m_viewport->toWorld(ucsPosition);
        m_viewport->highlightLocation(wcsPosition);
    }
}

void LC_PropertySheetWidget::checkIfVectorAndGetLocation(LC_Property* property) {
    auto vectorProperty = dynamic_cast<LC_PropertyRSVector*>(property);
    if (vectorProperty == nullptr) {
        const auto multiCandidate = dynamic_cast<LC_PropertyMulti*>(property);
        if (multiCandidate != nullptr) {
            const auto firstProperty = multiCandidate->getFirstProperty();
            if (firstProperty != nullptr) {
                vectorProperty = dynamic_cast<LC_PropertyRSVector*>(firstProperty);
                if (vectorProperty != nullptr) {
                    highlightVectorPropertyPosition(vectorProperty);
                }
            }
        }
    }
    else {
        highlightVectorPropertyPosition(vectorProperty);
    }
}

void LC_PropertySheetWidget::invalidateCached() const {
    const auto propertiesSheet = ui->propertySheet->propertiesSheet();
    const auto sheetModel = propertiesSheet->getModel();
    sheetModel->invalidateCached();
}

void LC_PropertySheetWidget::onDockVisibilityChanged(const bool visible) {
    LC_ERR << "Dock visibility changed - " << visible;
    if (visible) {
        m_handleSelectionChange = true;
        selectionChanged();
    }
    else {
        m_handleSelectionChange = false;
    }
}
