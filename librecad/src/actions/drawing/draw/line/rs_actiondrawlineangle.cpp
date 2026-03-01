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

#include "rs_actiondrawlineangle.h"

#include "lc_action_draw_line_radiant.h"
#include "qg_lineangleoptions.h"
#include "rs_document.h"
#include "rs_line.h"

struct RS_ActionDrawLineAngle::Points {
    /**
 * Line data defined so far.
 */
    RS_LineData data;
    /**
 * Position.
 */
    RS_Vector pos;
    RS_Vector secondPoint;
    /**
 * Line angle. Stored in radians and in UCS basis coordinate system - to ensure that change of the UCS or Angle Basis when actions's is active
 * is reflected properly
 */
    double ucsBasisAngleRad{0.0};
    /**
 * Line length.
 */
    double length{1.};
    /**
 * Is the angle fixed?
 */
    bool fixedAngle{false};
    /**
 * Snap point (start, middle, end).
 */
    int snpPoint{SNAP_START};
};

RS_ActionDrawLineAngle::RS_ActionDrawLineAngle(LC_ActionContext* actionContext, const bool fixedAngle, const RS2::ActionType actionType)
    : LC_SingleEntityCreationAction("Draw lines with given angle", actionContext, actionType), m_actionData(std::make_unique<Points>()) {
    m_actionData->fixedAngle = fixedAngle;
    reset();
}

RS_ActionDrawLineAngle::~RS_ActionDrawLineAngle() = default;

void RS_ActionDrawLineAngle::reset() const {
    m_actionData->data = {{}, {}};
}

void RS_ActionDrawLineAngle::init(const int status) {
    reset();
    RS_PreviewActionInterface::init(status);
}

void RS_ActionDrawLineAngle::initFromSettings() {
    RS_PreviewActionInterface::initFromSettings();
    if (m_actionType == RS2::ActionDrawLineVertical) {
        m_actionData->ucsBasisAngleRad = M_PI_2;
    }
    else if (m_actionType == RS2::ActionDrawLineHorizontal) {
        m_actionData->ucsBasisAngleRad = 0;
    }
}

RS_Entity* RS_ActionDrawLineAngle::doTriggerCreateEntity() {
    preparePreview();
    auto* line = new RS_Line{m_document, m_actionData->data};
    return line;
}

void RS_ActionDrawLineAngle::doTriggerCompletion([[maybe_unused]] bool success) {
    if (!m_persistRelativeZero) {
        RS_Vector& newRelZero = m_actionData->data.startpoint;
        if (m_actionData->snpPoint == SNAP_MIDDLE) {
            // snap to middle
            newRelZero = (m_actionData->data.startpoint + m_actionData->data.endpoint) * 0.5;
        }
        moveRelativeZero(newRelZero);
    }
    m_persistRelativeZero = false;
    setMainStatus(SetPos);
}

void RS_ActionDrawLineAngle::onMouseMoveEvent(const int status, const LC_MouseEvent* e) {
    RS_Vector position = e->snapPoint;
    if (status == SetPos) {
        position = getRelZeroAwarePoint(e, position);
        m_actionData->pos = position;
        m_alternateDirection = e->isControl;
        if (isFreeLineMode()) {
            if (m_showRefEntitiesOnPreview) {
                previewRefSelectablePoint(position);
            }
        }
        else {
            preparePreview();
            previewToCreateLine(m_actionData->data.startpoint, m_actionData->data.endpoint);
            if (m_showRefEntitiesOnPreview) {
                previewRefSelectablePoint(position);
            }
        }
    }
    else if (status == SetPoint2) {
        m_actionData->secondPoint = position;
        preparePreview();
        previewToCreateLine(m_actionData->data.startpoint, m_actionData->data.endpoint);
        if (m_showRefEntitiesOnPreview) {
            previewRefSelectablePoint(position);
            previewRefSelectablePoint(m_actionData->data.endpoint);
            previewRefPoint(m_actionData->data.startpoint);
            if (isFreeLineMode() && m_actionData->snpPoint == SNAP_MIDDLE) {
                previewRefPoint(m_actionData->pos);
            }
        }
    }
}

void RS_ActionDrawLineAngle::setLengthType(LengthType type, bool doUpdateOptions) {
    m_lengthType = type;
    setMainStatus(SetPos);
    if (doUpdateOptions) {
        updateOptions();
    }
}

bool RS_ActionDrawLineAngle::isFreeLineMode() const {
    return m_lengthType == FREE;
}

void RS_ActionDrawLineAngle::onMouseLeftButtonRelease(const int status, const LC_MouseEvent* e) {
    RS_Vector position = e->snapPoint;
    if (status == SetPos) {
        const bool shiftPressed = e->isShift;
        // potentially, we could eliminate this and set line position on mouse move and complete action there. however,
        // it seems explicit set of position on click is more consistent with default behavior of the action?
        m_alternateDirection = e->isControl;
        if (shiftPressed) {
            const RS_Vector relZero = getRelativeZero();
            if (relZero.valid) {
                position = relZero;
                m_persistRelativeZero = true;
            }
        }
        else {
            m_persistRelativeZero = false;
        }
        fireCoordinateEvent(position);
    }
    else if (status == SetPoint2){
        fireCoordinateEvent(position);
    }
}

void RS_ActionDrawLineAngle::onMouseRightButtonRelease(const int status, [[maybe_unused]] const LC_MouseEvent* e) {
    deletePreview();
    switch (status) {
        case SetPoint2: {
            setStatus(SetPos);
            break;
        }
        case SetPos: {
            setStatus(-1);
            break;
        }
        default:
            setStatus(SetPos);
            break;
    }
}
/*
RS_Vector RS_ActionDrawLineAngle::defineLineSecondPointFree(const RS_Vector& snapped) const {
    const double angle = m_startPoint.angleTo(getActiveRadiant());
    const RS_Vector result = LC_LineMath::calculateEndpointForAngleDirection(angle, m_startPoint, snapped);
    return result;
}

RS_Vector RS_ActionDrawLineAngle::defineLineSecondPointAuto(const RS_Vector& snapped) const {
    const auto activePoint = getActiveRadiant();
    switch (m_lengthType) {
        case LC_ActionDrawLineRadiant::TO_POINT: {
            return activePoint;
        }
        case LINE: {
            const double angle = snapped.angleTo(activePoint);
            const RS_Vector result = snapped.relative(m_length, angle);
            return result;
        }
        case BY_X: {
            const double len = m_length;
            const double angle = snapped.angleTo(activePoint);
            // here we perform all calculations in user coordinate system
            const RS_Vector snappedUCS = toUCS(snapped);
            const RS_Vector activeUCS = toUCS(activePoint);

            double ucsX;
            double ucsY;

            if (snappedUCS.x > activeUCS.x) {
                ucsX = snappedUCS.x - len;
            }
            else {
                ucsX = snappedUCS.x + len;
            }

            const double ucsAngle = toUCSAngle(angle);
            const double correctedAngle = RS_Math::correctAngle0ToPi(ucsAngle);
            if (RS_Math::getAngleDifferenceU(correctedAngle, M_PI_2) < RS_TOLERANCE_ANGLE) {
                ucsY = activePoint.y;
                ucsX = activePoint.x;
            }
            else {
                const double tanAlpha = std::tan(correctedAngle);
                const double leg = len * tanAlpha;
                if (snappedUCS.y > activeUCS.y) {
                    ucsY = snappedUCS.y - leg;
                }
                else {
                    ucsY = snappedUCS.y + leg;
                }
            }
            const RS_Vector ucsPoint(ucsX, ucsY);
            return toWorld(ucsPoint);
        }
        case BY_Y: {
            const double len = m_length;
            const double angle = snapped.angleTo(activePoint);

            const RS_Vector snappedUCS = toUCS(snapped);
            const RS_Vector activeUCS = toUCS(activePoint);
            double ucsY;
            if (snappedUCS.y > activeUCS.y) {
                ucsY = snappedUCS.y - len;
            }
            else {
                ucsY = snappedUCS.y + len;
            }
            double ucsX;
            const double ucsAngle = toUCSAngle(angle);
            const double correctedAngle = RS_Math::correctAngle0ToPi(ucsAngle);
            if (RS_Math::getAngleDifferenceU(correctedAngle, 0) < RS_TOLERANCE_ANGLE ||
                RS_Math::getAngleDifferenceU(correctedAngle, M_PI) < RS_TOLERANCE_ANGLE) {
                ucsY = activePoint.y;
                ucsX = activePoint.x;
            }
            else {
                const double tanAlpha = std::tan(correctedAngle);
                const double leg = len / tanAlpha;
                if (snappedUCS.x > activeUCS.x) {
                    ucsX = snappedUCS.x - leg;
                }
                else {
                    ucsX = snappedUCS.x + leg;
                }
            }
            const RS_Vector ucsPoint(ucsX, ucsY);
            return toWorld(ucsPoint);
        }
        default:
            return RS_Vector(false);
    }
}
*/
void RS_ActionDrawLineAngle::preparePreview() const {
    RS_Vector p1, p2;
    double angle = m_actionData->ucsBasisAngleRad;
    if (m_alternateDirection) {
        if (m_actionType == RS2::ActionDrawLineVertical) {
            angle = 0.0;
        }
        else if (m_actionType == RS2::ActionDrawLineHorizontal) {
            angle = M_PI_2;
        }
        else {
            angle = M_PI - angle;
        }
    }
    double wcsAngleRad = adjustRelativeAngleSignByBasis(angle);
    if (hasFixedAngle()) {
        if (m_orthoToAnglesBasis) {
            wcsAngleRad = toWorldAngleFromUCSBasis(wcsAngleRad);
        }
        else {
            wcsAngleRad = toWorldAngle(wcsAngleRad);
        }
    }
    else {
        wcsAngleRad = toWorldAngleFromUCSBasis(wcsAngleRad);
    }

    double len = m_actionData->length;

    if (isFreeLineMode()) {
       len = m_actionData->pos.distanceTo(m_actionData->secondPoint);
       if (m_actionData->snpPoint == SNAP_MIDDLE) {
         len = len*2;
       };
    }
    else {
        if (!hasFixedAngle()) {
            switch (m_lengthType) {
                case BY_X: {
                    const double ucsAngle = toUCSAngle(wcsAngleRad);
                    const double correctedAngle = RS_Math::correctAngle0ToPi(ucsAngle);
                    const double cosA = std::cos(correctedAngle);
                    if (LC_LineMath::isMeaningful(cosA)) {
                        len = len / cosA;
                    }
                    break;
                }
                case BY_Y: {
                    const double ucsAngle = toUCSAngle(wcsAngleRad);
                    const double correctedAngle = RS_Math::correctAngle0ToPi(ucsAngle);
                    const double sinA = std::sin(correctedAngle);
                    if (LC_LineMath::isMeaningful(sinA)) {
                        len = len / sinA;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (m_actionData->snpPoint == SNAP_END) {
        p2.setPolar(-len, wcsAngleRad);
    }
    else {
        p2.setPolar(len, wcsAngleRad);
    }

    // Middle:
    if (m_actionData->snpPoint == SNAP_MIDDLE) {
        p1 = m_actionData->pos - (p2 / 2);
    }
    else {
        p1 = m_actionData->pos;
    }

    p2 += p1;

    if (isFreeLineMode() && (m_actionData->snpPoint != SNAP_MIDDLE)) {
        RS_Vector projectionPoint = LC_LineMath::getNearestPointOnInfiniteLine(m_actionData->secondPoint, p1,p2);
        if (projectionPoint.distanceTo(p2) > projectionPoint.distanceTo(p1)) {
            p2 = p2.rotate(p1, M_PI);
        }
    }
    m_actionData->data = {p1, p2};
}

void RS_ActionDrawLineAngle::onCoordinateEvent(const int status, [[maybe_unused]] const bool isZero, const RS_Vector& pos) {
    switch (status) {
        case SetPos: {
            m_actionData->pos = pos;
            if (isFreeLineMode()) {
                setMainStatus(SetPoint2);
            }
            else {
                trigger();
            }
            break;
        }
        case SetPoint2: {
            m_actionData->secondPoint = pos;
            trigger();
            break;
        }
        case SetAngle: {
            if (isZero) {
                m_actionData->ucsBasisAngleRad = 0.0;
                updateOptions();
                restoreMainStatus();
            }
            break;
        }

        default:
            break;
    }
}

bool RS_ActionDrawLineAngle::doProcessCommand(const int status, const QString& command) {
    bool accept = true;
    if (!m_actionData->fixedAngle && checkCommand("angle", command)) {
        setStatus(SetAngle);
    }
    else if (checkCommand("length", command)) {
        setStatus(SetLength);
    }
    else if (checkCommand("lentype", command)) {
        setStatus(SetLengthType);
    }
    if (checkCommand("snap", command)) {
        setStatus(SetSnapPoint);
    }
    else {
        switch (status) {
            case SetAngle: {
                double ucsBasisAngleRad;
                const bool ok = parseToUCSBasisAngle(command, ucsBasisAngleRad);
                if (ok) {
                    accept = true;
                    m_actionData->ucsBasisAngleRad = ucsBasisAngleRad;
                }
                else {
                    commandMessage(tr("Not a valid expression for angle"));
                }
                updateOptions();
                restoreMainStatus();
                break;
            }
            case SetLength: {
                bool ok;
                const double l = RS_Math::eval(command, &ok);
                if (ok) {
                    accept = true;
                    m_actionData->length = l;
                }
                else {
                    commandMessage(tr("Not a valid expression for length"));
                }
                updateOptions();
                restoreMainStatus();
                break;
            }
            case SetSnapPoint: {
                if (tr("s") == command) {
                    m_actionData->snpPoint = SNAP_START;
                    updateOptions();
                    restoreMainStatus();
                }
                else if (tr("m") == command) {
                    m_actionData->snpPoint = SNAP_MIDDLE;
                    updateOptions();
                    restoreMainStatus();
                }
                else if (tr("e") == command) {
                    m_actionData->snpPoint = SNAP_END;
                    updateOptions();
                    restoreMainStatus();
                }
                else {
                    accept = false;
                }
                break;
            }
            case SetLengthType: {
                if (tr("line") == command) {
                    setLengthType(LINE);
                    restoreMainStatus();
                    updateOptions();
                }
                else if (tr("x") == command) {
                    setLengthType(BY_X);
                    restoreMainStatus();
                    updateOptions();
                }
                else if (tr("y") == command) {
                    setLengthType(BY_Y);
                    restoreMainStatus();
                    updateOptions();
                }
                else if (tr("free") == command) {
                    setLengthType(FREE);
                    restoreMainStatus();
                    updateOptions();
                }
                else {
                    accept = false;
                }
                break;
            }
            default:
                accept = false;
        }
    }
    return accept;
}

void RS_ActionDrawLineAngle::setSnapPoint(const int sp) const {
    m_actionData->snpPoint = sp;
}

int RS_ActionDrawLineAngle::getSnapPoint() const {
    return m_actionData->snpPoint;
}

void RS_ActionDrawLineAngle::setUcsAngleDegrees(const double ucsRelAngleDegrees) const {
    m_actionData->ucsBasisAngleRad = RS_Math::deg2rad(ucsRelAngleDegrees);
}

double RS_ActionDrawLineAngle::getUcsAngleDegrees() const {
    return RS_Math::rad2deg(m_actionData->ucsBasisAngleRad);
}

void RS_ActionDrawLineAngle::setLength(const double l) const {
    m_actionData->length = l;
}

double RS_ActionDrawLineAngle::getLength() const {
    return m_actionData->length;
}

bool RS_ActionDrawLineAngle::hasFixedAngle() const {
    switch (rtti()) {
        case RS2::ActionDrawLineHorizontal:
        case RS2::ActionDrawLineVertical:
            return true;
        default:
            return false;
    }
}

QStringList RS_ActionDrawLineAngle::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
        case SetPos:
            if (!m_actionData->fixedAngle) {
                cmd += command("angle");
            }
            cmd += command("length");
            cmd += command("lentype");
            cmd += command("snap");
            break;
        default:
            break;
    }
    return cmd;
}

void RS_ActionDrawLineAngle::updateMouseButtonHints() {
    switch (getStatus()) {
        case SetPos: {
            updateMouseWidgetTRCancel(tr("Specify position"), MOD_SHIFT_AND_CTRL(MSG_REL_ZERO, tr("Alternate Direction")));
            break;
        }
        case SetAngle: {
            updateMouseWidgetTRBack(tr("Enter angle:"));
            break;
        }
        case SetLength: {
            updateMouseWidgetTRBack(tr("Enter length:"));
            break;
        }
        case SetLengthType: {
            QString typesStr;
            if (m_actionData->fixedAngle) {
                typesStr = QString("[%1|%2]:").arg(tr("line"), tr("free"));
            }
            else {
                typesStr = QString("[%1|%2|%3|%4]:").arg(tr("line"), tr("x"), tr("y"), tr("free"));
            }
            updateMouseWidgetTRBack(tr("Enter length type ") + typesStr);
            break;
        }
        case SetPoint2: {
            updateMouseWidgetTRBack(tr("Specify second point"));
            break;
        }
        case SetSnapPoint: {
            updateMouseWidgetTRBack(tr("Enter snap type ") + QString("[%1|%2|%3]:").arg(tr("s"), tr("m"),tr("e")));
            break;
        }
        default: {
            updateMouseWidget();
            break;
        }
    }
}

RS2::CursorType RS_ActionDrawLineAngle::doGetMouseCursor([[maybe_unused]] int status) {
    return RS2::CadCursor;
}

LC_ActionOptionsWidget* RS_ActionDrawLineAngle::createOptionsWidget() {
    return new QG_LineAngleOptions();
}

void RS_ActionDrawLineAngle::setInAngleBasis(const bool b) {
    m_orthoToAnglesBasis = b;
}

bool RS_ActionDrawLineAngle::doUpdateAngleByInteractiveInput(const QString& tag, const double angleRad) {
    if (tag == "angle") {
        m_actionData->ucsBasisAngleRad = angleRad;
        return true;
    }
    return false;
}

bool RS_ActionDrawLineAngle::doUpdateDistanceByInteractiveInput(const QString& tag, const double distance) {
    if (tag == "length") {
        setLength(distance);
        return true;
    }
    return false;
}
