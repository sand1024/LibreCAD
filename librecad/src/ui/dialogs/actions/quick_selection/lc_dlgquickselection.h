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

#ifndef LC_DLGQUICKSELECTION_H
#define LC_DLGQUICKSELECTION_H

#include <qcombobox.h>
#include <qlistwidget.h>

#include "lc_actioncontext.h"
#include "lc_dialog.h"
#include "lc_entitymatchdescriptorsregistry.h"
#include "lc_entitymatchdescriptor.h"
#include "lc_graphicviewport.h"
#include "rs_selection.h"

namespace Ui {
    class LC_DlgQuickSelection;
}


struct LC_QuickSearchConditions {
    bool m_applyToSelection = false;
    RS2::EntityType m_entityType = RS2::EntityUnknown;
    QString m_selectedPropertyName;
    LC_PropertyMatchOperation m_matchOperation;
    bool m_includeIntoNewSet  = false;
    bool m_appendToCurrentSet = false;
};

struct LC_QuickSearchSelectionDialogState;

class LC_DlgQuickSelection : public LC_Dialog {
    Q_OBJECT
public:
    LC_DlgQuickSelection(QWidget* parent, RS_Document* doc, LC_GraphicViewport* viewport, LC_ActionContext::InteractiveInputInfo::InputType inputType,
                         LC_QuickSearchSelectionDialogState* savedState, double interactiveInputValue1,
                         double interactiveInputValue2);
    ~LC_DlgQuickSelection() override;
    LC_ActionContext::InteractiveInputInfo::InputType isInteractiveInputRequested() const {return m_interactiveInputRequested;}
    LC_QuickSearchSelectionDialogState* getSavedState();
    bool isAdditionalSelectionRequested() const {return m_selectionRequested;}
    RS_Selection::ConditionalSelectionOptions getSelectionOptions();
    void accept() override;
protected slots:
    void onApplyToCurrentIndexChanged(int index);
    void onOperatorChanged(int index);
    void onEntityTypeIndexChanged(int index);
    void onPropertyListRowChanged(int currentRow);
    void onAppendToCurrentSetClicked();
    void onPickLengthClicked();
    void onPickCoord();
    void onPickAngle();
    void onManualSelection();
    void onPrecisionTextEditingFinished();
    void onUpdatePrecisionByDocumentSettings();
private:
    Ui::LC_DlgQuickSelection *ui;
    RS_Document *m_document {nullptr};
    LC_GraphicViewport* m_viewport {nullptr};
    RS_Selection::CurrentSelectionState m_selectionState;
    RS2::EntityType m_entityType = RS2::EntityUnknown;
    LC_EntityMatchDescriptor* m_entityMatchDescriptor {nullptr};
    LC_PropertyMatchDescriptor* m_propertyDescriptor {nullptr};
    LC_PropertyMatchOperation m_operationType = LC_PropertyMatchOperation::MATCH_OPERATION_EQUALS;

    bool m_selectionRequested{false};
    LC_ActionContext::InteractiveInputInfo::InputType  m_interactiveInputRequested{LC_ActionContext::InteractiveInputInfo::NOTNEEDED};
    QString m_inputTag{""};

    QString m_precisionLength;
    QString m_precisionAngle;

    LC_EntityMatcher *obtainEntityMatcher();
    RS2::EntityType getEntityTypeToMatch();

    void saveState();
    void setupEntitiesTypesList(const QMap<RS2::EntityType, int>& map) const;
    void addOperationItem(LC_PropertyMatchDescriptor* propertyDescriptor, LC_PropertyMatchOperation type, QString label);
    void setupOperationsCombobox(LC_PropertyMatchDescriptor* propertyDescriptor);
    void enablePrecisionLength();
    void enablePrecisionAngle();
    void disablePrecision();
    void setPropertyValueInput(LC_PropertyMatchDescriptor* propertyDescriptor);
    void restoreFromSavedState(LC_QuickSearchSelectionDialogState* savedState,
        LC_ActionContext::InteractiveInputInfo::InputType inputType, double interactiveInputValue1, double interactiveInputValue2);
    QString obtainPropertyName(int currentRow) const;
    QString obtainCurrentPropertyName() const;
    RS2::EntityType obtainEntityType(int index) const;
    RS2::EntityType obtainCurrentEntityType() const;
    LC_PropertyMatchOperation obtainCurrentOperation() const;
    LC_PropertyMatchOperation obtainOperation(int index) const;
    void updateWidgetSettings() const;
    bool setCurrentEntityType(int entityType);
    void setCurrentPropertyName(const QString& propertyName);
    void setCurrentOperation(int operation);
    void tryToRestorePreviousState();
    void updatePrecisionLength();
    void updatePrecisionAngle();
};

#endif
