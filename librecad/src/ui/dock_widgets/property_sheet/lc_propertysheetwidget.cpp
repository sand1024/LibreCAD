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


#include "lc_entitymetauiutils.h"
#include "lc_entitypropertyvaluedelegate.h"
#include "lc_graphicviewport.h"
#include "lc_parabola.h"
#include "lc_propertiessheet.h"
#include "lc_propertycontainer.h"
#include "lc_propertyrsvector.h"
#include "lc_propertyrsvectorview.h"
#include "rs_debug.h"
#include "rs_document.h"
#include "rs_graphicview.h"
#include "rs_line.h"
#include "rs_selection.h"
#include "ui_lc_propertysheetwidget.h"

LC_PropertySheetWidget::LC_PropertySheetWidget(QWidget *parent)
    : LC_GraphicViewAwareWidget(parent)
    , ui(new Ui::LC_PropertySheetWidget)
{
    ui->setupUi(this);

    connect(ui->propertySheet->propertiesSheet(), &LC_PropertiesSheet::propertyEdited, this, &LC_PropertySheetWidget::onPropertyEdited);
    connect(ui->propertySheet->propertiesSheet(), &LC_PropertiesSheet::beforePropertyEdited, this, &LC_PropertySheetWidget::onBeforePropertyEdited);
}

LC_PropertySheetWidget::~LC_PropertySheetWidget(){
    delete ui;
}

void LC_PropertySheetWidget::onBeforePropertyEdited(LC_Property* property, [[maybe_unused]]LC_Property::PropertyValuePtr newValue, [[maybe_unused]]int typeId) {
    LC_ERR << "On Before Edit " << property->getName() << " " << property->getRootProperty()->getName();
}

void LC_PropertySheetWidget::onPropertyEdited(LC_Property* property) {
    LC_ERR << "On Edited " << property->getName();
}

void LC_PropertySheetWidget::setGraphicView(RS_GraphicView* gv) {
    RS_Document *doc = nullptr;
    LC_GraphicViewport* viewport = nullptr;
    // remove tracking of relative point from old view
    if (m_graphicView != nullptr){
        disconnect(m_graphicView, &RS_GraphicView::relativeZeroChanged, this, &LC_PropertySheetWidget::onRelativeZeroChanged);
        disconnect(m_graphicView, &RS_GraphicView::ucsChanged, this, &LC_PropertySheetWidget::onUcsChanged);
        if (m_document != nullptr) {
            m_document->getSelection()->removeListener(this);
        }
    }

    m_graphicView = gv;

    // add tracking of relative point for new view
    if (gv != nullptr){
        connect(m_graphicView, &RS_GraphicView::relativeZeroChanged, this, &LC_PropertySheetWidget::onRelativeZeroChanged);
        connect(m_graphicView, &RS_GraphicView::ucsChanged, this, &LC_PropertySheetWidget::onUcsChanged);
        viewport = gv->getViewPort();
        doc = gv->getDocument();
        doc->getSelection()->addListener(this);
    }

    m_document = doc;
    m_viewport = viewport;
    initPropertySheet();
    // if (gv != nullptr) {
        setupSelectionCombobox();
    // }
    /*m_entityData->setDocumentAndView(doc, viewport);
    m_pointsData->setDocumentAndView(doc, viewport);
    showNoDataMessage();
    m_hasOwnPreview = false;*/
}

void LC_PropertySheetWidget::selectionChanged() {
    LC_ERR << "On Selection Changed!";
    setupSelectionCombobox();
}

void LC_PropertySheetWidget::updateFormats() {
}

void LC_PropertySheetWidget::onUcsChanged([[maybe_unused]]LC_UCS* ucs) {
}

void LC_PropertySheetWidget::onRelativeZeroChanged(const RS_Vector&) {
}

void LC_PropertySheetWidget::setupSelectionCombobox() const {
    ui->cbSelection->blockSignals(true);
    ui->cbSelection->clear();
    if (m_document != nullptr) {
        auto selection = RS_Selection(m_document, m_viewport);
        QMap<RS2::EntityType, int> selectedEntityTypesMap;
        selection.countSelectedEntities(selectedEntityTypesMap);
        LC_EntityMetaUIUtils::setupSelectionEntityTypesCombobox(ui->cbSelection, selectedEntityTypesMap);
    }
    ui->cbSelection->blockSignals(false);
}

class ModificationContext: public LC_EntitiesModificationContext {
public:
    ~ModificationContext() override = default;

    void entityModified([[maybe_unused]]RS_Entity* originalEntity, [[maybe_unused]]RS_Entity* entityClone) override {
        LC_ERR << "entity Modified!";
    };
};


void LC_PropertySheetWidget::updateWidgetSettings()  { // fixme- temporary test code
    auto* container = new LC_PropertyContainer(this);
    auto cont = new LC_PropertyContainer(container);
    cont->setDisplayName("Geometry");
    cont->setName("geometry");

    container->addChildProperty(cont);

    ui->propertySheet->setPropertyContainer(container);

    RS_Line* line = new RS_Line({11,12},{523.3,342.35234});

    LC_PropertyRSVector* start = new LC_PropertyRSVector(cont, false);
    start->setName("startEndpoint");
    start->setDisplayName("Start");
    start->setViewDescriptorProvider([]() -> LC_PropertyViewDescriptor {
        LC_PropertyViewDescriptor info;
        info[LC_PropertyRSVectorView::ATTR_X_DISPLAY_NAME] = "Start X";
        info[LC_PropertyRSVectorView::ATTR_Y_DISPLAY_NAME] = "Start Y";
        return info;
    });



    auto startValueStorage = new LC_EntityPropertyValueDelegate<RS_Vector, RS_Line>();
    startValueStorage->setup(line, new ModificationContext());

    startValueStorage->m_getValue = [](RS_Line* line) -> RS_Vector {
        return line->getStartpoint();
    };
    startValueStorage->m_setValue = []( RS_Vector& v, [[maybe_unused]]LC_PropertyChangeReason reason, RS_Line* l) -> void {
         l->setStartpoint(v);
        LC_ERR << "Start point " << l->getStartpoint();
    };
    startValueStorage->m_isValueEqual = [] (RS_Vector& v, RS_Line* l) -> bool {
        return v == l->getStartpoint();
    };

    start->setValueStorage(startValueStorage, true);
    /*RS_Vector startV(12.23, 34.23);
    start->setValue(startV);
    */

    cont->addChildProperty(start);

    LC_PropertyRSVector* end = new LC_PropertyRSVector(cont);
    end->setName("startEndpoint");
    end->setDisplayName("End");
    // LC_PropertyViewDescriptor descriptor;
    // start->setViewDescriptor(descriptor);
    RS_Vector endV(12.23, 34.23);
    end->setValue(endV);

    cont->addChildProperty(end);
}

void LC_PropertySheetWidget::initPropertySheet() {
}
