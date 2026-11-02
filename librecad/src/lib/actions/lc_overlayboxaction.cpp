/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

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

#include "lc_overlayboxaction.h"

#include <QKeyEvent>

#include "lc_cursoroverlayinfo.h"
#include "lc_settings_cad_preferences.h"
#include "lc_overlay_selection_box.h"
#include "lc_overlay_selection_lasso.h"
#include "lc_polygon_selection.h"
#include "rs_debug.h"
#include "rs_graphicview.h"
#include "rs_selection.h"
#include "rs_settings.h"

LC_OverlayBoxAction::LC_OverlayBoxAction(const QString& name, LC_ActionContext *actionContext, const RS2::ActionType actionType)
    : LC_UndoableDocumentModificationAction(name, actionContext, actionType),
      m_overlayBoxOptions{std::make_unique<LC_SelectionBoxOptions>()} {
}

LC_OverlayBoxAction::~LC_OverlayBoxAction() = default;

void LC_OverlayBoxAction::init(const int status) {
    LC_UndoableDocumentModificationAction::init(status);
    m_selectIntersecting = false;
    m_massSelectionStyle = MassSelectionStyle::Auto;
}

void LC_OverlayBoxAction::drawOverlaySelectionBox(const RS_Vector &corner1, const RS_Vector &corner2, bool replace) const {
    const bool isFence = (m_massSelectionStyle == MassSelectionStyle::Fence);
    auto* ob = new LC_OverlaySelectionBox(corner1, corner2, m_overlayBoxOptions.get(),m_selectIntersecting, isFence);
    if (replace) {
        replaceOverlay(ob, RS2::OverlayGraphics::OverlayEffects, [](LC_OverlayDrawable* e)->bool {
            auto candidate = dynamic_cast<LC_OverlaySelectionBox*>(e);
            if (candidate == nullptr) {
                return false;
            }
            delete candidate;
            return true;
        });
    }
    else {
        addOverlay(ob, RS2::OverlayGraphics::OverlayEffects);
    }
}

void LC_OverlayBoxAction::drawOverlaySelectionLasso(bool replace) const {
    const bool isFence = (m_massSelectionStyle == MassSelectionStyle::Fence);
    auto* ob = new LC_OverlaySelectionLasso(m_overlayBoxOptions.get());
    ob->update(m_lassoRegion.getPoints(), m_selectIntersecting, isFence);

    if (replace) {
        replaceOverlay(ob, RS2::OverlayGraphics::OverlayEffects, [](LC_OverlayDrawable* e)->bool {
          auto* candidate = dynamic_cast<LC_OverlaySelectionLasso*>(e);
          if (candidate == nullptr) {
              return false;
          }
          delete candidate;
          return true;
      });
    }
    else {
        addOverlay(ob, RS2::OverlayGraphics::OverlayEffects);
    }
}

void LC_OverlayBoxAction::addPointToSelectionLasso(const RS_Vector& wcsPosition) {
    m_lassoRegion.addPoint(wcsPosition);
}

void LC_OverlayBoxAction::setMassSelectionStyle(MassSelectionStyle style) {
    m_massSelectionStyle = style;
}

void LC_OverlayBoxAction::setMassSelectionMode(MassSelectionMode mode) {
    m_lassoRegion.clear();
    m_massSelectionMode = mode;
    // LC_ERR << "Mass Selection Mode: " << (int)mode;
}

void LC_OverlayBoxAction::initFromSettings() {
    LC_UndoableDocumentModificationAction::initFromSettings();
    m_overlayBoxOptions->loadSettings();
    m_allowsLassoSelectionByPressedMouseButton = CFG_CADPreferences::o_AllowLassoSelectionByPressedMouseButton;
}

void LC_OverlayBoxAction::cycleMassSelectionStyle() {
    switch (m_massSelectionStyle) {
        case MassSelectionStyle::Auto: {
            m_massSelectionStyle = m_selectIntersecting ? MassSelectionStyle::Window : MassSelectionStyle::Crossing;
            break;
        }
        case MassSelectionStyle::Window: {
            m_massSelectionStyle = MassSelectionStyle::Crossing;
            break;
        }
        case MassSelectionStyle::Crossing: {
            m_massSelectionStyle = MassSelectionStyle::Fence;
            break;
        }
        case MassSelectionStyle::Fence: {
            m_massSelectionStyle = MassSelectionStyle::Auto;
            break;
        }
    }
}

bool LC_OverlayBoxAction::resolveCrossingMode(const bool naturalCrossing, const bool isControl) const {
    switch (m_massSelectionStyle) {
        case MassSelectionStyle::Fence:
        case MassSelectionStyle::Crossing: {
            return true ^ isControl;
        }
        case MassSelectionStyle::Window: {
            return false ^ isControl;
        }
        case MassSelectionStyle::Auto:
        default: {
            return naturalCrossing ^ isControl;
        }
    }
}

bool LC_OverlayBoxAction::handleMassSelectionKeyEvent(QKeyEvent* e, const bool isPress) {
    if (e == nullptr || !isMassSelecting()) {
        return false;
    }

    const int key = e->key();

    // 1. Spacebar cycles style: only on key press and ignoring auto-repeat
    if (isPress && key == Qt::Key_Space && !e->isAutoRepeat()) {
        if (!isAllowCycleMassSelectionStyle()) {
            return false;
        }
        cycleMassSelectionStyle();
        refreshMassSelectionState(e);
        e->accept();
        return true;
    }

    // 2. Modifiers Shift and Ctrl affect selection/crossing: updated on both press and release
    if (key == Qt::Key_Shift || key == Qt::Key_Control) {
        refreshMassSelectionState(e);
        e->accept();
        return true;
    }

    return false;
}

void LC_OverlayBoxAction::keyPressEvent(QKeyEvent* e) {
    if (handleMassSelectionKeyEvent(e, true)) {
        return;
    }
    LC_UndoableDocumentModificationAction::keyPressEvent(e);
}

void LC_OverlayBoxAction::keyReleaseEvent(QKeyEvent* e) {
    if (handleMassSelectionKeyEvent(e, false)) {
        return;
    }
    LC_UndoableDocumentModificationAction::keyReleaseEvent(e);

    LC_UndoableDocumentModificationAction::keyReleaseEvent(e);
}

void LC_OverlayBoxAction::updateSelectionInfoCursor(const RS_Vector& mouse, const bool isLasso, const bool performSelection) {
    if (m_infoCursorOverlayPrefs == nullptr || !m_infoCursorOverlayPrefs->enabled || m_infoCursorOverlayData == nullptr) {
        return;
    }

    QString msg = performSelection ? tr("Selecting") : tr("De-Selecting");
    msg.append(tr(" entities "));

    if (m_massSelectionStyle == MassSelectionStyle::Fence) {
        msg.append(isLasso ? tr("crossed by fence") : tr("crossed by box perimeter"));
    }
    else if (isLasso) {
        msg.append(m_selectIntersecting ? tr("that intersect with (or within) lasso") : tr("that are within lasso"));
    }
    else {
        msg.append(m_selectIntersecting ? tr("that intersect with (or within) box") : tr("that are within box"));
    }

    switch (m_massSelectionStyle) {
        case MassSelectionStyle::Fence: {
            msg.append(tr(" [Fence]"));
            break;
        }
        case MassSelectionStyle::Crossing: {
            msg.append(tr(" [Locked Crossing]"));
            break;
        }
        case MassSelectionStyle::Window: {
            msg.append(tr(" [Locked Window]"));
            break;
        }
        case MassSelectionStyle::Auto: {
            msg.append(tr(" [Auto]"));
            break;
        }
    }

    if (isAllowCycleMassSelectionStyle()) {
    msg.append(tr(" (Space: switch mode)"));
    }

    m_infoCursorOverlayData->setZone2(msg);
    forceUpdateInfoCursor(mouse);
}

void LC_OverlayBoxAction::processBoxSelectionMove(const RS_Vector& corner1, const RS_Vector& mouse, const LC_MouseEvent* e) {
    const RS_Vector ucsP1 = toUCS(corner1);
    const RS_Vector ucsP2 = toUCS(mouse);
    const bool naturalCrossing = (ucsP1.x > ucsP2.x);

    m_selectIntersecting = resolveCrossingMode(naturalCrossing, e != nullptr && e->isControl);
    drawOverlaySelectionBox(corner1, mouse);
    updateSelectionInfoCursor(mouse, false, isPerformSelect(e));
}

void LC_OverlayBoxAction::processLassoSelectionMove(const RS_Vector& mouse, const LC_MouseEvent* e) {
    m_lassoRegion.addPoint(mouse);
    const bool naturalCrossing = m_lassoRegion.isClockwise();

    m_selectIntersecting = resolveCrossingMode(naturalCrossing, e != nullptr && e->isControl);
    drawOverlaySelectionLasso();
    updateSelectionInfoCursor(mouse, true, isPerformSelect(e));
}

void LC_OverlayBoxAction::endBoxSelectionMode(const LC_MouseEvent* e) {
    const RS_Vector mouse = e->graphPoint;
    deletePreviewAndHighlights();

    auto selectionCorner1 = getSelectionCorner1();
    const RS_Vector ucsP1 = toUCS(selectionCorner1);
    const RS_Vector ucsP2 = toUCS(mouse);

    const bool performSelection = isPerformSelect(e);
    const bool isFence = (m_massSelectionStyle == MassSelectionStyle::Fence);

    // Expand selection WCS to ensure selection box in UCS is fully within the WCS bounding rect
    RS_Vector wcsP1;
    RS_Vector wcsP2;
    m_viewport->worldBoundingBox(ucsP1, ucsP2, wcsP1, wcsP2);

    RS_Selection s(m_document, m_viewport);
    if (getEntityTypesToSelect().isEmpty()) {
        s.selectWindow(RS2::EntityUnknown, wcsP1, wcsP2, performSelection, m_selectIntersecting, isFence);
    }
    else {
        s.selectWindow(getEntityTypesToSelect(), wcsP1, wcsP2, performSelection, m_selectIntersecting, isFence);
    }
}

void LC_OverlayBoxAction::endLassoSelectionMode(const LC_MouseEvent* e) {
        deletePreviewAndHighlights();

    const bool performSelection = isPerformSelect(e);
    const bool isFence = (m_massSelectionStyle == MassSelectionStyle::Fence);

    // Fence mode finalizes an open chain (closed = false)
    m_lassoRegion.finalize(!isFence);
    if (m_lassoRegion.isValid()) {
        if (getEntityTypesToSelect().isEmpty()) {
            LC_PolygonSelection::select(m_document, m_viewport, m_lassoRegion, RS2::EntityUnknown, true, performSelection);
        }
        else {
            LC_PolygonSelection::select(m_document, m_viewport, m_lassoRegion, getEntityTypesToSelect(), true, performSelection);
        }
    }
}

void LC_OverlayBoxAction::finishBoxSelection(const LC_MouseEvent* e) {
    endBoxSelectionMode(e);
    setMassSelectionMode(MassSelectionMode::NONE);
    setMassSelectionStyle(MassSelectionStyle::Auto);
    invalidateSelectionCorner1();
}

void LC_OverlayBoxAction::finishLassoSelection(const LC_MouseEvent* e) {
    endLassoSelectionMode(e);
    setMassSelectionMode(MassSelectionMode::NONE);
    setMassSelectionStyle(MassSelectionStyle::Auto);
    invalidateSelectionCorner1();
}

void LC_OverlayBoxAction::updateActionPromptForSelectionMode() {
    const auto massSelectionMode = getMassSelectionMode();
    switch (massSelectionMode) {
        case (MassSelectionMode::BOX): {
            updatePromptTRBack(tr("Choose second edge") + tr(" | Space: Switch selection style"),
                               MOD_SHIFT_AND_CTRL(tr("Select/Deselect entities"), tr("Select Intersecting"))
                              );
            break;
        }
        case (MassSelectionMode::LASSO): {
            updatePromptTRBack(tr("Draw lasso and release") + tr(" | Space: Switch selection style"),
                               MOD_SHIFT_AND_CTRL(tr("Select/Deselect entities"), tr("Select Intersecting")));
            break;
        }
        case MassSelectionMode::NONE: {
            doUpdateActionPromptForSelectionStart();
            break;
        }
        default: {
            break;
        }
    }
}

void LC_OverlayBoxAction::refreshMassSelectionState(const QInputEvent* e) {
    const auto massSelectionMode = getMassSelectionMode();
    if (!isMassSelecting()){
        return;
    }

    if (m_graphicView == nullptr || m_viewport == nullptr) {
        return;
    }

    const RS_Vector currentMouse = m_lastMouseMoveEvent.graphPoint;
    const bool isControl = (e != nullptr) ? (e->modifiers() & Qt::ControlModifier) != 0 : false;
        const bool performSelection = isPerformSelect(e);

    if (massSelectionMode == MassSelectionMode::BOX) {
        const RS_Vector ucsP1 = toUCS(getSelectionCorner1());
        const RS_Vector ucsP2 = toUCS(currentMouse);
        const bool naturalCrossing = (ucsP1.x > ucsP2.x);

        m_selectIntersecting = resolveCrossingMode(naturalCrossing, isControl);
        drawOverlaySelectionBox(getSelectionCorner1(), currentMouse, true);
        updateSelectionInfoCursor(currentMouse, false, performSelection);
    }
    else {
        const bool naturalCrossing = m_lassoRegion.isClockwise();

        m_selectIntersecting = resolveCrossingMode(naturalCrossing, isControl);
        drawOverlaySelectionLasso(true);
        updateSelectionInfoCursor(currentMouse, true, performSelection);
    }

    m_graphicView->redraw(RS2::RedrawOverlay);
}
