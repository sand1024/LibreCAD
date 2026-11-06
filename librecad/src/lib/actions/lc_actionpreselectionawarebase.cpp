/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "lc_actionpreselectionawarebase.h"

#include <QMouseEvent>

#include "lc_cursoroverlayinfo.h"
#include "lc_graphicviewport.h"
#include "lc_polygon_selection.h"
#include "lc_settings_drafting.h"
#include "rs_document.h"
#include "rs_selection.h"


LC_ActionPreSelectionAwareBase::LC_ActionPreSelectionAwareBase(
    const QString& name, LC_ActionContext *actionContext, const RS2::ActionType actionType,
    const QList<RS2::EntityType> &entityTypeList)
    :RS_ActionSelectBase(name, actionContext, actionType, entityTypeList){}

bool LC_ActionPreSelectionAwareBase::isKeepModifiedEntitiesSelected() const {
    const bool keepSelected = CFG_DraftingOperations::o_KeepModifiedEntitiesSelected;
    return keepSelected;
}

LC_ActionPreSelectionAwareBase::~LC_ActionPreSelectionAwareBase() {
    m_selectedEntities.clear();
}

void LC_ActionPreSelectionAwareBase::doInitWithContextEntity(RS_Entity* contextEntity, [[maybe_unused]]const RS_Vector& clickPos) {
    if (isForceSelectContextEntity() && !contextEntity->isSelected()) {
        select(contextEntity);
        redrawDrawing();
    }
    m_selectedEntities.push_back(contextEntity);
    onSelectionCompleted(true, true);
}

void LC_ActionPreSelectionAwareBase::init(const int status) {
    RS_PreviewActionInterface::init(status);
    if (status < 0){
        m_selectedEntities.clear();
    }
    else{
        if (!m_selectionComplete) {
            const unsigned int selectedCount = collectSelectedEntities();
            if (selectedCount > 0) {
                onSelectionCompleted(false, true);
            }
        }
    }
}

unsigned int LC_ActionPreSelectionAwareBase::collectSelectedEntities() {
    m_selectedEntities.clear();
    QList<RS_Entity*> entities;
    m_document->collectSelected(entities, false, m_catchForSelectionEntityTypes);
    for (const auto e: std::as_const(entities)) {
        if (isEntityAllowedToSelect(e)) {
            m_selectedEntities.push_back(e);
        }
    }
    const unsigned int selectedCount = m_selectedEntities.size();
//    LC_ERR << " Selected Count: " << selectedCount;
    return selectedCount;
}

void LC_ActionPreSelectionAwareBase::selectionFinishedByKey([[maybe_unused]]QKeyEvent *e, const bool escape) {
    setMassSelectionStyle(MassSelectionStyle::Auto);
    if (escape){
        m_selectedEntities.clear();
        finish();
    }
    else{
        if (!m_selectionComplete) {
            onSelectionCompleted(false,false);
        }
    }
}

void LC_ActionPreSelectionAwareBase::proceedSelectedEntity(const LC_MouseEvent* e) {
    if (e->isControl) {
        onSelectionCompleted(true, false);
    }
}

void LC_ActionPreSelectionAwareBase::onMouseLeftButtonPress([[maybe_unused]]int status, const LC_MouseEvent* e) {
    if (!m_selectionComplete){
        const auto massSelectionMode = getMassSelectionMode();
        if (massSelectionMode == MassSelectionMode::NONE) {
            m_selectionCorner1 = e->graphPoint;
        }
        m_lmbPressed = true;
    }
}

void LC_ActionPreSelectionAwareBase::onMouseLeftButtonRelease(const int status, const LC_MouseEvent* e) {
    if (m_selectionComplete){
        onMouseLeftButtonReleaseSelected(status, e);
    }
    else{
        auto massSelectionMode = getMassSelectionMode();
        switch (massSelectionMode) {
            case MassSelectionMode::BOX: {
                finishBoxSelection(e);
                break;
            }
            case MassSelectionMode::LASSO: {
                finishLassoSelection(e);
                break;
            }
            case MassSelectionMode::NONE: {
                RS_Entity* entityToSelect = catchEntityByEvent(e, m_catchForSelectionEntityTypes);
                const bool selectContour = e->isShift;
                // if (selectEntity(entityToSelect, selectContour)) {
                //     m_selectionCorner1.valid = false;
                //     proceedSelectedEntity(e);
                // }
                // else if (e->isControl) { // completion of selection by click with CTRL
                //     onSelectionCompleted(false, false);
                // }
                // change from PR LibreCAD#3246
                if (e->isControl && collectSelectedEntities() > 0) {
                    // The prompt offers CTRL+LMB click as the mouse's Enter: complete the
                    // selection that is there. Picking what lies under the cursor would
                    // toggle it off when it is selected (or, without additive selection,
                    // replace the whole selection), so the action would run on other
                    // entities than the ones the user selected (#3084).
                    m_selectionCorner1.valid = false;
                    onSelectionCompleted(false, false);
                }
                else if (selectEntity(entityToSelect, selectContour)) {
                    // nothing selected yet: CTRL picks this entity and completes at once
                    m_selectionCorner1.valid = false;
                    proceedSelectedEntity(e);
                }
                break;
            }
        }
        invalidateSnapSpot();
    }
    m_lmbPressed = false;
}

void LC_ActionPreSelectionAwareBase::onMouseRightButtonRelease(const int status, const LC_MouseEvent* e) {
    if (m_selectionComplete) {
        onMouseRightButtonReleaseSelected(status, e);
    }
    else{
        m_selectedEntities.clear();
        finish();
    }
}

void LC_ActionPreSelectionAwareBase::applyBoxSelectionModeIfNeeded(const RS_Vector mouse) {
    auto massSelectionMode = getMassSelectionMode();
    switch (massSelectionMode) {
        case (MassSelectionMode::NONE): {
            if (m_selectionCorner1.valid && (m_viewport->toGuiDX(m_selectionCorner1.distanceTo(mouse)) > 10.0)) {
                if (!m_lmbPressed) {
                    if (m_allowsLassoSelectionByPressedMouseButton) {
                        setMassSelectionMode(MassSelectionMode::BOX); // box only when we allow lasso
                    }
                    else {
                        setMassSelectionMode(MassSelectionMode::NONE);  // we require classical selection with pressed LMB
                    }
                }
                else {
                    if (m_allowsLassoSelectionByPressedMouseButton) {
                        setMassSelectionMode(MassSelectionMode::LASSO); // lasso with pressed LMB
                    }
                    else {
                        setMassSelectionMode(MassSelectionMode::BOX); // classical lc selection
                    }
                }
            }
            break;
        }
        case MassSelectionMode::LASSO: {
            // fixme - complete lasso
            break;
        }
        case MassSelectionMode::BOX: {
            break;
        }
        default:
            break;
    }
}

void LC_ActionPreSelectionAwareBase::onMouseMoveEvent(const int status, const LC_MouseEvent* e) {
    if (m_selectionComplete){
        onMouseMoveEventSelected(status, e);
    }
    else{
        const RS_Vector mouse = e->graphPoint;
        applyBoxSelectionModeIfNeeded(mouse);
        auto massSelectionMode = getMassSelectionMode();
        switch (massSelectionMode) {
            case (MassSelectionMode::BOX): {
                processBoxSelectionMove(m_selectionCorner1, mouse, e);
                break;
            }
            case (MassSelectionMode::LASSO): {
                processLassoSelectionMove(mouse, e);
                break;
            }
            case MassSelectionMode::NONE: {
                selectionMouseMove(e);
                finishMouseMoveOnSelection(e);
                RS_Snapper::drawSnapperOrPickBox();
                break;
            }
            default:
                break;
        }
    }
}

void LC_ActionPreSelectionAwareBase::doProcessSpaceKey(QKeyEvent* keyEvent, bool isPress) {
    if (m_selectionComplete) {
        m_selectedEntities.clear();
        finish();
    }
    else {
        if (m_document->hasSelection() || isAllowSelectionFinishByEnterForEmptySelection()) {
            selectionFinishedByKey(keyEvent, false);
        }
    }
}

void LC_ActionPreSelectionAwareBase::drawSnapperOrPickBox() {
    // if (m_selectionComplete) {
        RS_Snapper::drawSnapperOrPickBox();
    // }
}

void LC_ActionPreSelectionAwareBase::updateActionPrompt() {
    if (m_selectionComplete) {
        updateActionPromptForSelected(getStatus());
    }
    else {
        updateActionPromptForSelectionMode();
    }
}

void LC_ActionPreSelectionAwareBase::onSelectionCompleted([[maybe_unused]] const bool singleEntity, const bool fromInit) {
    setSelectionComplete(isAllowTriggerOnEmptySelection(), fromInit);
    updateActionPrompt();
    forceUpdateInfoCursor(m_lastMouseMoveEvent.graphPoint);
    if (m_selectionComplete) {
        trigger();
        if (singleEntity) {
            deselectAll();
            m_selectionComplete = false; // continue with selection, don't finish
        } else {
            setStatus(-1);
        }
        m_graphicView->redraw(RS2::RedrawDrawing);
    }
}

void LC_ActionPreSelectionAwareBase::proceedSelectionComplete(const bool allowEmptySelection, [[maybe_unused]]bool fromInit, [[maybe_unused]] const unsigned int selectedCount) {
    const bool proceed = selectedCount > 0 || allowEmptySelection;
    if (proceed) {
        m_selectionComplete = true;
        updateMouseCursor();
        updateActionPromptForSelected(getStatus());
    }
    else{
        commandMessage(tr("No valid entities selected, select them first"));
    }
}

void LC_ActionPreSelectionAwareBase::setSelectionComplete(const bool allowEmptySelection, const bool fromInit) {
    unsigned int selectedCount = 0;
    if (fromInit) {
       selectedCount = m_selectedEntities.size();
    }
    else{
        selectedCount = collectSelectedEntities();
    }
    proceedSelectionComplete(allowEmptySelection, fromInit, selectedCount);
}

void LC_ActionPreSelectionAwareBase::updateActionPromptForSelected([[maybe_unused]]int status) {
    updatePrompt();
}

void LC_ActionPreSelectionAwareBase::onMouseLeftButtonReleaseSelected([[maybe_unused]]int status, [[maybe_unused]] const LC_MouseEvent* event) {}

void LC_ActionPreSelectionAwareBase::onMouseRightButtonReleaseSelected([[maybe_unused]]int status, [[maybe_unused]] const LC_MouseEvent* event) {}

void LC_ActionPreSelectionAwareBase::onMouseMoveEventSelected([[maybe_unused]] int status, [[maybe_unused]] const LC_MouseEvent* event) {}

RS2::CursorType LC_ActionPreSelectionAwareBase::doGetMouseCursor(const int status) {
    if (m_selectionComplete){
        return doGetMouseCursorSelected(status);
    }
    return RS_ActionSelectBase::doGetMouseCursor(status);
}

RS2::CursorType LC_ActionPreSelectionAwareBase::doGetMouseCursorSelected([[maybe_unused]]int status) {
    return RS2::CadCursor;
}

void LC_ActionPreSelectionAwareBase::doTriggerSelections(const LC_DocumentModificationBatch& ctx) {
    const bool keepSelected = isKeepModifiedEntitiesSelected();
    doTriggerSelectionUpdate(keepSelected, ctx);
}

void LC_ActionPreSelectionAwareBase::finishMouseMoveOnSelection([[maybe_unused]] const LC_MouseEvent* event) {

}

void LC_ActionPreSelectionAwareBase::doSelectEntity(RS_Entity *entityToSelect, const bool selectContour) const {
    if (entityToSelect != nullptr){
        const RS_Selection s(m_document, m_viewport);
        // try to minimize selection clicks - and select contour based on selected entity. May be optional, but what for?
        if (entityToSelect->isAtomic() && selectContour) {
            s.selectContour(entityToSelect);
        }
        else{
            s.selectSingle(entityToSelect);
        }
    }
}
