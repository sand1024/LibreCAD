/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include "lc_action_select_intersected.h"

#include "lc_cursoroverlayinfo.h"
#include "rs_debug.h"
#include "rs_entity.h"
#include "rs_selection.h"

struct LC_ActionSelectIntersected::ActionData {
    RS_Vector v1;
    RS_Vector v2;
    RS_Entity* entityToIntersect{nullptr};
};

/**
 * Constructor.
 *
 * @param actionContext
 * @param select true: select window. false: deselect window
 */
LC_ActionSelectIntersected::LC_ActionSelectIntersected(LC_ActionContext* actionContext, const bool select)
    : RS_ActionSelectBase(select ? "ActionSelectIntersected" : "ActionDeselectIntersected", actionContext,
                          select ? RS2::ActionSelectIntersected : RS2::ActionDeselectIntersected),
      m_actionData(std::make_unique<ActionData>()), m_performSelect(select), m_currentSelect(select) {
}

LC_ActionSelectIntersected::~LC_ActionSelectIntersected() = default;

void LC_ActionSelectIntersected::reset() {
    m_actionData->entityToIntersect = nullptr;
    m_actionData->v1.valid = false;
    m_actionData->v2.valid = false;
    m_currentSelect = m_performSelect;
    m_snapMode.clear();
    m_snapMode.restriction = RS2::RestrictNothing;
}

void LC_ActionSelectIntersected::init(const int status) {
    reset();
    RS_PreviewActionInterface::init(status);
}

void LC_ActionSelectIntersected::doInitWithContextEntity(RS_Entity* contextEntity, [[maybe_unused]] const RS_Vector& clickPos) {
    if (isValidEntityForSelection(contextEntity)) {
        m_actionData->entityToIntersect = contextEntity;
        trigger();
    }
}

void LC_ActionSelectIntersected::doTrigger() {
    if (m_actionData->entityToIntersect != nullptr) {
        // Intersect with picked entity or child segment
        const RS_Selection s(m_document, m_viewport);
        s.selectIntersected(m_actionData->entityToIntersect, m_currentSelect);
        reset();
        setStatus(SetPoint1);
    }
    else if (m_actionData->v1.valid && m_actionData->v2.valid) {
        // Intersect with 2-point line cut
        if (toGuiDX(m_actionData->v1.distanceTo(m_actionData->v2)) > 10.0) {
            const RS_Selection s(m_document, m_viewport);
            s.selectIntersected(m_actionData->v1, m_actionData->v2, m_currentSelect);
            reset();
            setStatus(SetPoint1);
        }
    }
}

void LC_ActionSelectIntersected::selectionFinishedByKey([[maybe_unused]] QKeyEvent* e, [[maybe_unused]] bool escape) {
    finish();
}

void LC_ActionSelectIntersected::onMouseMoveEvent(const int status, const LC_MouseEvent* e) {
    if (status == SetPoint1) {
        const RS_Vector mouse = e->graphPoint;

        // 1. Press-and-drag threshold detection for Lasso Fence mode
        if (m_lmbPressed && m_allowsLassoSelectionByPressedMouseButton) {
            if (getMassSelectionMode() == MassSelectionMode::NONE) {
                if (m_actionData->v1.valid && (toGuiDX(m_actionData->v1.distanceTo(mouse)) > 10.0)) {
                    setMassSelectionMode(MassSelectionMode::LASSO);
                    setMassSelectionStyle(MassSelectionStyle::Fence);
                    m_lassoRegion.clear();
                    m_lassoRegion.addPoint(m_actionData->v1);
                    updateActionPrompt();
                }
            }
            if (getMassSelectionMode() == MassSelectionMode::LASSO) {
                processLassoSelectionMove(mouse, e);
                return;
            }
        }

        // 2. Context-aware entity hover (Ctrl toggles child entity resolution)
        const RS2::ResolveLevel resolveLevel = (e != nullptr && e->isControl) ? RS2::ResolveAll : RS2::ResolveNone;
        const RS_Entity* entity = catchAndDescribe(e, resolveLevel);

        if (entity != nullptr && isValidEntityForSelection(entity)) {
            highlightHover(entity);

            if (m_infoCursorOverlayPrefs != nullptr && m_infoCursorOverlayPrefs->enabled && m_infoCursorOverlayData != nullptr) {
                const bool performingSelect = isPerformSelect(e != nullptr && e->isShift);
                QString msg = performingSelect ? tr("Select") : tr("De-Select");
                if (e != nullptr && e->isControl && entity->getParent() != nullptr) {
                    msg.append(tr(" entities intersecting with child segment"));
                }
                else {
                    msg.append(tr(" entities intersecting with clicked entity"));
                    msg.append(tr(" (Ctrl: pick child segment)"));
                }
                m_infoCursorOverlayData->setZone2(msg);
                forceUpdateInfoCursor(e->graphPoint);
            }
        }
        else {
            deleteHighlights();
            if (m_infoCursorOverlayPrefs != nullptr && m_infoCursorOverlayPrefs->enabled && m_infoCursorOverlayData != nullptr) {
                const bool performingSelect = isPerformSelect(e != nullptr && e->isShift);
                QString msg = performingSelect ? tr("Selecting") : tr("De-Selecting");
                msg.append(tr(" by line cut (Click-Click) or fence (Drag)"));
                m_infoCursorOverlayData->setZone2(msg);
                forceUpdateInfoCursor(e->graphPoint);
            }
        }
    }
    else if (status == SetPoint2 && m_actionData->v1.valid) {
        m_actionData->v2 = e->snapPoint;
        previewLine(m_actionData->v1, m_actionData->v2);

        if (m_infoCursorOverlayPrefs != nullptr && m_infoCursorOverlayPrefs->enabled && m_infoCursorOverlayData != nullptr) {
            const bool performingSelect = isPerformSelect(e != nullptr && e->isShift);
            QString msg = performingSelect ? tr("Selecting") : tr("De-Selecting");
            msg.append(tr(" entities crossed by line cut"));
            m_infoCursorOverlayData->setZone2(msg);
            forceUpdateInfoCursor(e->graphPoint);
        }
    }
}

void LC_ActionSelectIntersected::onMouseLeftButtonPress(const int status, const LC_MouseEvent* e) {
    switch (status) {
        case SetPoint1: {
                m_actionData->v1 = e->snapPoint;
                m_lmbPressed = true;
            break;
        }
        default: {
            break;
        }
    }
}

void LC_ActionSelectIntersected::onMouseLeftButtonRelease(const int status, const LC_MouseEvent* e) {
    RS_DEBUG->print("RS_ActionSelectIntersected::mouseReleaseEvent()");

    // 1. Commit Lasso Fence if dragging occurred
    if (getMassSelectionMode() == MassSelectionMode::LASSO) {
        finishLassoSelection(e);
        reset();
        setStatus(SetPoint1);
        m_lmbPressed = false;
        return;
    }

    if (status == SetPoint1) {
        const RS2::ResolveLevel resolveLevel = e->isControl ? RS2::ResolveAll : RS2::ResolveNone;
        RS_Entity* entity = catchAndDescribe(e, resolveLevel);

        // Feature snap check: if snapped to an endpoint, center, or intersection, prioritize starting the cutting line
        const bool hasFeatureSnap = e->snapPoint.valid && (toGuiDX(e->snapPoint.distanceTo(e->graphPoint)) > 4.0);

        if (entity != nullptr && isValidEntityForSelection(entity) && (!hasFeatureSnap || (e != nullptr && e->isControl))) {
            // Picked entity or child segment as cutting knife
            m_actionData->entityToIntersect = entity;
            m_currentSelect = isPerformSelect(e != nullptr && e->isShift);
            trigger();
        }
        else {
            // Clicked to set Point 1 of 2-point line cut
            m_actionData->v1 = e->snapPoint;
            setStatus(SetPoint2);
        }
    }
    else if (status == SetPoint2) {
        // Set Point 2 and execute 2-point line cut
        m_actionData->v2 = e->snapPoint;
        m_currentSelect = isPerformSelect(e != nullptr && e->isShift);
        trigger();
        if (e != nullptr && e->isControl) {
            finish();
        }
    }
    m_lmbPressed = false;
}

bool LC_ActionSelectIntersected::isValidEntityForSelection(const RS_Entity* entity) {
    const RS2::EntityType entityType = entity->rtti();
    const bool entityTypeValidForSelection = !entity->isParentIgnoredOnModifications() && entityType != RS2::EntitySpline;
    return entityTypeValidForSelection;
}

void LC_ActionSelectIntersected::onMouseRightButtonRelease(const int status, [[maybe_unused]] const LC_MouseEvent* e) {
    RS_DEBUG->print("RS_ActionSelectIntersected::mouseReleaseEvent()");
    if (getMassSelectionMode() == MassSelectionMode::LASSO) {
        deletePreviewAndHighlights();
        setMassSelectionMode(MassSelectionMode::NONE);
        setMassSelectionStyle(MassSelectionStyle::Auto);
        reset();
        setStatus(SetPoint1);
        m_lmbPressed = false;
        return;
    }
    if (getStatus() == SetPoint2) {
        deletePreview();
    }
    initPrevious(status);
}

void LC_ActionSelectIntersected::updateActionPrompt() {
    // 1. Dedicated prompt while actively dragging a freehand Lasso Fence
    if (getMassSelectionMode() == MassSelectionMode::LASSO) {
        const QString actionText = m_performSelect ? tr("select") : tr("deselect");
        const QString shiftHint = m_performSelect ? tr("Deselect") : tr("Select");
        updatePromptTRBack(tr("Draw fence path across entities and release to %1").arg(actionText),
                           MOD_SHIFT_LC(shiftHint));
        return;
    }

    // 2. Standard point-based status prompts
    switch (getStatus()) {
        case SetPoint1: {
            const QString promptText = m_performSelect
                ? tr("Click first point of line cut or drag for fence to select")
                : tr("Click first point of line cut or drag for fence to deselect");

            const QString shiftActionText = m_performSelect ? tr("Deselect intersecting") : tr("Select intersecting");
            updatePromptTRCancel(promptText + " " + getSelectionCompletionHintMsg(),
                                 MOD_SHIFT_AND_CTRL(shiftActionText, tr("Pick child segment of entity")));
            break;
        }
        case SetPoint2: {
            // Clean, uncluttered prompt for placing the second line point
            const QString promptText = m_performSelect
                ? tr("Click second point of line cut to select")
                : tr("Click second point of line cut to deselect");

            updatePromptTRBack(promptText);
            break;
        }
        default: {
            RS_ActionSelectBase::updateActionPrompt();
            break;
        }
    }
}

void LC_ActionSelectIntersected::doUpdateActionPromptForSelectionStart() {
    updateActionPrompt();
}

RS2::CursorType LC_ActionSelectIntersected::doGetMouseCursor([[maybe_unused]] int status) {
    return RS2::SelectCursor;
}

RS_Vector LC_ActionSelectIntersected::getSelectionCorner1() {
    return m_actionData != nullptr ? m_actionData->v1 : RS_Vector(false);
}

void LC_ActionSelectIntersected::invalidateSelectionCorner1() {
    if (m_actionData != nullptr) {
        m_actionData->v1.valid = false;
    }
}
