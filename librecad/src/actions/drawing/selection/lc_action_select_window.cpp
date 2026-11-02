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
#include "lc_action_select_window.h"

#include <QMouseEvent>

#include "lc_cursoroverlayinfo.h"
#include "lc_graphicviewport.h"
#include "lc_selectwindowoptions.h"
#include "rs_selection.h"

/**
 * Constructor.
 *
 * @param actionContext
 * @param select true: select window. false: invertSelectionOperation window
 */
LC_ActionSelectWindow::LC_ActionSelectWindow(LC_ActionContext* actionContext, const bool select)
    : RS_ActionSelectBase(select ? "ActionSelectWindow" : "ActionDeselectWindow", actionContext,
                          select ? RS2::ActionSelectWindow : RS2::ActionDeselectWindow),
      m_performSelect(select), m_currentSelect(select) {
}

LC_ActionSelectWindow::LC_ActionSelectWindow(RS2::EntityType typeToSelect, LC_ActionContext* actionContext)
    : RS_ActionSelectBase("ActionSelectWindow", actionContext, RS2::ActionSelectWindow),
      m_performSelect(true), m_currentSelect(true) {
    if (typeToSelect == RS2::EntityUnknown) {
        m_catchForSelectionEntityTypes = {};
    }
    else {
        m_catchForSelectionEntityTypes.append(typeToSelect);
    }
}

LC_ActionSelectWindow::LC_ActionSelectWindow(const QString& name, LC_ActionContext* actionContext,
                                             const RS2::ActionType actionType, const RS2::EntityType typeToSelect, const bool select)
    : RS_ActionSelectBase(name, actionContext, actionType),
      m_performSelect(select), m_currentSelect(select) {
    if (typeToSelect == RS2::EntityUnknown) {
        m_catchForSelectionEntityTypes = {};
    }
    else {
        m_catchForSelectionEntityTypes.append(typeToSelect);
    }
}

LC_ActionSelectWindow::~LC_ActionSelectWindow() = default;

void LC_ActionSelectWindow::reset() {
    m_corner1.valid = false;
    m_currentSelect = m_performSelect;
    m_lmbPressed = false;
    setMassSelectionMode(MassSelectionMode::NONE);
    setMassSelectionStyle(MassSelectionStyle::Auto);
}

void LC_ActionSelectWindow::init(const int status) {
    reset();
    RS_ActionSelectBase::init(status);
}
void LC_ActionSelectWindow::doSaveOptions() {
    RS_ActionSelectBase::doSaveOptions(); // fixme - complete
}

void LC_ActionSelectWindow::doLoadOptions() {
    RS_ActionSelectBase::doLoadOptions(); // fixme - complete
}
void LC_ActionSelectWindow::onMouseMoveEvent([[maybe_unused]] const int status, const LC_MouseEvent* e) {
    if (e == nullptr) {
        return;
    }

    const RS_Vector mouse = e->graphPoint;
    updateCoordinateWidgetByRelZero(mouse);

    if (status == SetCorner1) {
        // Check press-and-drag threshold to start mass selection gesture
        if (m_lmbPressed && m_corner1.valid && (toGuiDX(m_corner1.distanceTo(mouse)) > 10.0)) {
            if (getMassSelectionMode() == MassSelectionMode::NONE) {
                if (m_allowsLassoSelectionByPressedMouseButton) {
                    setMassSelectionMode(MassSelectionMode::LASSO);
                    m_lassoRegion.clear();
                    m_lassoRegion.addPoint(m_corner1);
                    updateActionPrompt();
                }
                else {
                    setMassSelectionMode(MassSelectionMode::BOX);
                    setStatus(SetCorner2);
                    updateActionPrompt();
                }
            }
        }

        if (getMassSelectionMode() == MassSelectionMode::LASSO) {
            processLassoSelectionMove(mouse, e);
            return;
        }
    }

    if (status == SetCorner2 || getMassSelectionMode() == MassSelectionMode::BOX) {
        if (m_corner1.valid) {
            processBoxSelectionMove(m_corner1, mouse, e);
        }
    }
}

void LC_ActionSelectWindow::onMouseLeftButtonPress(const int status, const LC_MouseEvent* e) {
    switch (status) {
        case SetCorner1: {
            if (e != nullptr && e->isControl) {
                finish();
                return;
            }
            if (e != nullptr) {
                m_corner1 = e->graphPoint;
                m_lmbPressed = true;
            }
            break;
        }
        default: {
            break;
        }
    }
}

void LC_ActionSelectWindow::onMouseLeftButtonRelease(const int status, const LC_MouseEvent* e) {
    if (getMassSelectionMode() == MassSelectionMode::LASSO) {
        finishLassoSelection(e);
        reset();
        setStatus(SetCorner1);
        updateActionPrompt();
        m_lmbPressed = false;
        return;
    }

    if (status == SetCorner1) {
        if (m_corner1.valid) {
            // Click without drag: switch to waiting for second corner
            setStatus(SetCorner2);
            setMassSelectionMode(MassSelectionMode::BOX);
            updateActionPrompt();
        }
    }
    else if (status == SetCorner2) {
        finishBoxSelection(e);
        reset();
        setStatus(SetCorner1);
        updateActionPrompt();
    }

    m_lmbPressed = false;
}

void LC_ActionSelectWindow::onMouseRightButtonRelease(const int status, [[maybe_unused]] const LC_MouseEvent* e) {
    if (isMassSelecting() || status == SetCorner2) {
        deletePreviewAndHighlights();
        reset();
        setStatus(SetCorner1);
        updateActionPrompt();
        if (m_graphicView != nullptr) {
            m_graphicView->redraw(RS2::RedrawOverlay);
        }
        return;
    }

    finish();
}

void LC_ActionSelectWindow::updateActionPrompt() {
    const auto mode = getMassSelectionMode();
    if (mode == MassSelectionMode::LASSO) {
        const QString actionText = m_performSelect ? tr("select") : tr("deselect");
        const QString shiftHint = m_performSelect ? tr("Deselect") : tr("Select");
        updatePromptTRBack(tr("Draw lasso and release to %1 | Space: Switch selection style").arg(actionText),
                           MOD_SHIFT_AND_CTRL(shiftHint, tr("Select Intersecting")));
        return;
    }

    if (mode == MassSelectionMode::BOX || getStatus() == SetCorner2) {
        const QString shiftHint = m_performSelect ? tr("Deselect entities") : tr("Select entities");
        updatePromptTRBack(tr("Choose second edge | Space: Switch selection style"),
                           MOD_SHIFT_AND_CTRL(shiftHint, tr("Select Intersecting")));
        return;
    }

    doUpdateActionPromptForSelectionStart();
}

void LC_ActionSelectWindow::doUpdateActionPromptForSelectionStart() {
    const QString actionText = m_performSelect ? tr("select entities") : tr("deselect entities");
    const QString shiftHint = m_performSelect ? tr("Deselect entities") : tr("Select entities");
    const QString promptText = tr("Click corner or drag for lasso to %1").arg(actionText);

    updatePromptTRCancel(promptText + " " + getSelectionCompletionHintMsg(),
                         MOD_SHIFT_LC(shiftHint));
}

RS_Vector LC_ActionSelectWindow::getSelectionCorner1() {
    return m_corner1;
}

void LC_ActionSelectWindow::invalidateSelectionCorner1() {
    m_corner1.valid = false;
}

LC_ActionOptionsWidget* LC_ActionSelectWindow::createOptionsWidget() {
    return new LC_SelectWindowOptionsWidget();
}

LC_ActionOptionsPropertiesFiller* LC_ActionSelectWindow::createOptionsFiller() {
    return RS_ActionSelectBase::createOptionsFiller(); // fixme - complete
}

void LC_ActionSelectWindow::selectionFinishedByKey([[maybe_unused]] QKeyEvent* e, [[maybe_unused]] bool escape) {
    finish();
}

bool LC_ActionSelectWindow::isSelectAllEntityTypes() const {
    return m_catchForSelectionEntityTypes.isEmpty();
}

void LC_ActionSelectWindow::setSelectAllEntityTypes() {
    m_catchForSelectionEntityTypes.clear();
}

void LC_ActionSelectWindow::setEntityTypesToSelect(const QList<RS2::EntityType>& types) {
    m_catchForSelectionEntityTypes = types;
}
