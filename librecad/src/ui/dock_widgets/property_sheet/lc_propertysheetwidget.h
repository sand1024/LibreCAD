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

#ifndef LC_PROPERTYSHEETWIDGET_H
#define LC_PROPERTYSHEETWIDGET_H

#include "lc_actioncontext.h"
#include "lc_entitypropertyvaluedelegate.h"
#include "lc_graphicviewawarewidget.h"
#include "lc_graphicviewport.h"
#include "lc_latecompletionrequestor.h"
#include "lc_property.h"
#include "lc_property_rsvector.h"
#include "lc_selectedsetlistener.h"
#include "rs_entity.h"

class LC_ActionContext;
class LC_EntityPropertyContainerProvider;

namespace Ui {
    class LC_PropertySheetWidget;
}

class LC_PropertySheetWidget : public LC_GraphicViewAwareWidget, public LC_SelectedSetListener, public LC_EntitiesModificationContext,
                               public LC_LateCompletionRequestor {
    Q_OBJECT

public:
    explicit LC_PropertySheetWidget(QWidget* parent, LC_ActionContext* actionContext, QAction* selectQuick, QAction* toggleSelectModeAction,
                                    QAction* selectEntitiesAction);
    ~LC_PropertySheetWidget() override;
    void loadCollapsedSections();
    void saveCollapsedSections();
    void setGraphicView(RS_GraphicView* gv) override;
    void selectionChanged() override;
    void setShouldHandleSelectionChange(bool value);
    void updateFormats();
    void onLateRequestCompleted(bool shouldBeSkipped) override;
    bool isCollapsedSection(const QString& name) const;
    void markContainerCollapsed(const QString& name, bool collapse);
    void checkSectionCollapsed(LC_PropertyContainer* result);
    void entityModified(RS_Entity* originalEntity, RS_Entity* entityClone) override;

public slots :
    void onUcsChanged(LC_UCS* ucs);
    void onViewDefaultActionActivated(bool defaultActionActivated, RS2::ActionType prevActionRtti);
    void onRelativeZeroChanged(const RS_Vector&);
    void onBeforePropertyEdited(LC_Property* property, LC_Property::PropertyValuePtr newValue, int typeId);
    void onPropertyEdited(LC_Property* property);
    void onSelectionIndexChanged(int index);
    void onSectionPropertyChanged(LC_PropertyChangeReason reason);
    void onActivePropertyChanged(LC_Property* activeProperty);
    void highlightVectorPropertyPosition(const LC_PropertyRSVector* vectorProperty) const;
    void checkIfVectorAndGetLocation(LC_Property* property);
    void invalidateCached() const;
    void onDockVisibilityChanged(bool visible);

protected:
    void setupSelectionTypeCombobox(RS2::EntityType entityTypeTryToSet, QString propertyTryToSet);
    void clearContextEntities();
    void collectEntitiesToModify(RS2::EntityType entityType, QList<RS_Entity*>& entitiesToModify) const;
    LC_PropertyContainer* createPropertiesContainer(RS2::EntityType entityType, const QList<RS_Entity*>& list);
    LC_PropertyContainer* preparePropertiesContainer(RS2::EntityType entityType);
    void destroyContainer(LC_PropertyContainer* previousContainer) const;
    void setPickedPointPropertyValue(const QString& propertyName, const RS_Vector& ucsVector) const;
    void setPickedPropertyValue(const QString& propertyName, double interactiveInputValue,
                                LC_ActionContext::InteractiveInputInfo::InputType input) const;
    void setPickedPropertyCoordinateValue(const QString& propertyName, double interactiveInputValue, bool forX) const;
    bool isVirtualProperty(const LC_Property* property);
    int getCurrentlySelectedEntityType(int index) const;
    QLayout* getTopLevelLayout() const override;
private:
    Ui::LC_PropertySheetWidget* ui;
    void initPropertySheet();
    RS_GraphicView* m_graphicView = nullptr; // fixme - sand - review dependency
    RS_Document* m_document = nullptr;
    LC_GraphicViewport* m_viewport = nullptr;
    bool m_handleSelectionChange = true;
    LC_ActionContext* m_actionContext{nullptr};
    std::unique_ptr<LC_EntityPropertyContainerProvider> m_entityContainerProvider;
    QSet<QString> m_collapsedContainerNames;
    QList<RS_Entity*> m_orginalEntities;
    QList<RS_Entity*> m_modifiedEntities;
};

#endif
