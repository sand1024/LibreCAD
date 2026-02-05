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

#include "rs_actiondrawcircle3p.h"

#include "lc_creation_circle.h"
#include "rs_circle.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"

struct RS_ActionDrawCircle3P::Points {
    RS_CircleData circleData;
    /**
     * 1st point.
     */
    RS_Vector point1 = RS_Vector(false);
    /**
     * 2nd point.
     */
    RS_Vector point2 = RS_Vector(false);
    /**
     * 3rd point.
     */
    RS_Vector point3 = RS_Vector(false);
};

RS_ActionDrawCircle3P::RS_ActionDrawCircle3P(LC_ActionContext* actionContext)
    : LC_ActionDrawCircleBase("Draw circles", actionContext, RS2::ActionDrawCircle3P), m_actionData(std::make_unique<Points>()) {
}

RS_ActionDrawCircle3P::~RS_ActionDrawCircle3P() = default;

void RS_ActionDrawCircle3P::reset() {
    m_actionData.reset(new Points{});
}

RS_Entity* RS_ActionDrawCircle3P::doTriggerCreateEntity() {
    preparePreview();
    if (m_actionData->circleData.isValid()) {
        auto* circle = new RS_Circle{m_document, m_actionData->circleData};
        if (m_moveRelPointAtCenterAfterTrigger) {
            moveRelativeZero(m_actionData->circleData.center);
        }
        return circle;
    }
    RS_DIALOGFACTORY->requestWarningDialog(tr("Invalid circle data.")); // fixme - sand - check whether this is possible?
    return nullptr;
}

void RS_ActionDrawCircle3P::doTriggerCompletion([[maybe_unused]] bool success) {
    setStatus(SetPoint1);
    reset();
}

void RS_ActionDrawCircle3P::preparePreview() const {
    m_actionData->circleData = RS_CircleData{};
    if (m_actionData->point1.valid && m_actionData->point2.valid && m_actionData->point3.valid) {
        const bool success = LC_CreationCircle::createFrom3P(m_actionData->point1, m_actionData->point2, m_actionData->point3,m_actionData->circleData);
        if (!success) {
            m_actionData->circleData = RS_CircleData{};
        }
    }
}

// todo - think about preview improving to give it more geometric meaning
void RS_ActionDrawCircle3P::onMouseMoveEvent(const int status, const LC_MouseEvent* e) {
    RS_Vector mouse = e->snapPoint;
    switch (status) {
        case SetPoint1:
            m_actionData->point1 = mouse;
            trySnapToRelZeroCoordinateEvent(e);
            break;

        case SetPoint2: {
            mouse = getSnapAngleAwarePoint(e, m_actionData->point1, mouse, true);
            m_actionData->point2 = mouse;
            const RS_Vector center = (mouse + m_actionData->point1) / 2;
            const double radius = m_actionData->point1.distanceTo(center);
            previewCircle(RS_CircleData(center, radius));

            if (m_showRefEntitiesOnPreview) {
                previewRefPoint(m_actionData->point1);
                previewRefLine(m_actionData->point1, mouse);
                previewRefSelectablePoint(mouse);
            }
            break;
        }
        case SetPoint3: {
            m_actionData->point3 = mouse;
            preparePreview();
            const auto& circleData = m_actionData->circleData;
            if (circleData.isValid()) {
                previewToCreateCircle(circleData);
                if (m_showRefEntitiesOnPreview) {
                    previewRefPoint(circleData.center);
                    previewRefPoint(m_actionData->point1);
                    previewRefPoint(m_actionData->point2);
                    previewRefSelectablePoint(mouse);
                    previewRefLine(m_actionData->point1, circleData.center);
                    previewRefLine(m_actionData->point2, circleData.center);
                    previewRefLine(mouse, circleData.center);
                }
            }
            break;
        }
        default:
            break;
    }
}

void RS_ActionDrawCircle3P::onMouseLeftButtonRelease(const int status, [[maybe_unused]] const LC_MouseEvent* e) {
    RS_Vector coord = e->snapPoint;
    if (status == SetPoint2) {
        coord = getSnapAngleAwarePoint(e, m_actionData->point1, coord);
    }
    fireCoordinateEvent(coord);
}

void RS_ActionDrawCircle3P::onMouseRightButtonRelease(const int status, [[maybe_unused]] const LC_MouseEvent* e) {
    deletePreview();
    initPrevious(status);
}

void RS_ActionDrawCircle3P::onCoordinateEvent(const int status, [[maybe_unused]] bool isZero, const RS_Vector& coord) {
    switch (status) {
        case SetPoint1: {
            m_actionData->point1 = coord;
            moveRelativeZero(coord);
            setStatus(SetPoint2);
            break;
        }
        case SetPoint2: {
            m_actionData->point2 = coord;
            moveRelativeZero(coord);
            setStatus(SetPoint3);
            break;
        }
        case SetPoint3: {
            m_actionData->point3 = coord;
            trigger();
            break;
        }
        default:
            break;
    }
}

void RS_ActionDrawCircle3P::updateMouseButtonHints() {
    switch (getStatus()) {
        case SetPoint1:
            updateMouseWidgetTRCancel(tr("Specify first point"), MOD_SHIFT_RELATIVE_ZERO);
            break;
        case SetPoint2:
            updateMouseWidgetTRBack(tr("Specify second point"), MOD_SHIFT_ANGLE_SNAP);
            break;
        case SetPoint3:
            updateMouseWidgetTRBack(tr("Specify third point"));
            break;
        default:
            updateMouseWidget();
            break;
    }
}
