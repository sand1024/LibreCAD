/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 * Copyright (C) 2026 sand1024
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

#include "lc_action_draw_line_radiant.h"

#include "lc_action_options_editor_typed.h"
#include "lc_line_radiant_options_filler.h"
#include "lc_line_radiant_options_widget.h"
#include "rs_line.h"
#include "rs_settings.h"

void LC_ActionDrawLineRadiantOptions::doSaveSettings() {
    save("Radiant1" ,m_radiantPoints[ONE]);
    save("Radiant2" ,m_radiantPoints[TWO]);
    save("Radiant3", m_radiantPoints[THREE]);
    save("Radiant4", m_radiantPoints[FOUR]);
    save("ActiveRadiantIdx", m_activeRadiantIndex);
    save("Length", m_length);
    save("LengthType", m_lengthType);
}

void LC_ActionDrawLineRadiantOptions::doLoadSettings() {
    m_radiantPoints[ONE] = loadVector("Radiant1" ,RS_Vector(-1000, 0));
    m_radiantPoints[TWO] = loadVector("Radiant2" ,RS_Vector(1000, 0));
    m_radiantPoints[THREE] = loadVector("Radiant3", RS_Vector(0, -1000));
    m_radiantPoints[FOUR] = loadVector("Radiant4", RS_Vector(0, 1000));
    m_activeRadiantIndex = static_cast<RadiantIdx>(loadInt("ActiveRadiantIdx", ONE));
    m_length = loadDouble("Length", 1.0);
    m_lengthType = static_cast<LenghtType>(loadInt("LengthType", LenghtType::LINE));
}

LC_ActionDrawLineRadiant::LC_ActionDrawLineRadiant(LC_ActionContext* actionContext) :
    LC_UndoableDocumentModificationAction("DrawLineRadiant", actionContext, RS2::ActionDrawLineRadiant),
    m_actionOptions{std::make_unique<LC_ActionDrawLineRadiantOptions>("ActionDrawLineRadiant", "")}{
    m_actionOptions->load();
    m_optionsEditor.reset(new LC_ActionOptionsEditorTyped(this, [] {
        return new LC_OptionsWidgetRadiantLine();
    }, [] {
        return new LC_RadiantLineOptionsFiller();
    }));
}

void LC_ActionDrawLineRadiant::init(const int status) {
    LC_UndoableDocumentModificationAction::init(status);
}

bool LC_ActionDrawLineRadiant::doTriggerModifications(LC_DocumentModificationBatch& ctx) {
    RS_Vector secondPoint;
    if (isFreeLength()) {
        secondPoint = defineLineSecondPointFree(m_secondSnapPoint);
    }
    else {
        secondPoint = defineLineSecondPointAuto(m_startPoint);
    }
    if (secondPoint.valid) {
        if (LC_LineMath::isMeaningfulDistance(m_startPoint, secondPoint)) {
            const auto line = new RS_Line(nullptr, {m_startPoint, secondPoint});
            ctx += line;
            return true;
        }
        commandMessage(tr("Resulting line is too short"));
    }
    return false;
}

void LC_ActionDrawLineRadiant::doTriggerCompletion(const bool success) {
    if (success) {
        moveRelativeZero(m_startPoint);
        setStatus(SetPoint);
    }
}

void LC_ActionDrawLineRadiant::doTriggerSelections(const LC_DocumentModificationBatch& ctx) {
    if (ctx.success) {
        const bool keepSelected = LC_GET_ONE_BOOL("Modify", "KeepModifiedSelected", true);
        if (keepSelected) {
            select(ctx.entitiesToAdd);
        }
    }
}

void LC_ActionDrawLineRadiant::previewRadiantLine(const RS_Vector& snapped, const RS_Vector& secondPoint) const {
    if (secondPoint.valid) {
        if (LC_LineMath::isMeaningfulDistance(snapped, secondPoint)) {
            previewToCreateLine(snapped, secondPoint);
            if (m_showRefEntitiesOnPreview) {
                // previewRefLine(snapped, secondPoint);
                previewRefPoint(secondPoint);
                previewRefPoint(getActiveRadiant());
                previewRefSelectablePoint(snapped);
            }
        }
    }
}

bool LC_ActionDrawLineRadiant::isFreeLength() const {
    return m_actionOptions->isFreeLength();
}

void LC_ActionDrawLineRadiant::onMouseMoveEvent([[maybe_unused]] const int status, const LC_MouseEvent* e) {
    const RS_Vector snapped = e->snapPoint;
    switch (status) {
        case SetPoint: {
            if (!trySnapToRelZeroCoordinateEvent(e)) {
                if (isFreeLength()) {
                    if (m_showRefEntitiesOnPreview) {
                        previewRefPoint(getActiveRadiant());
                        previewRefSelectablePoint(snapped);
                    }
                }
                else {
                    const RS_Vector secondPoint = defineLineSecondPointAuto(snapped);
                    previewRadiantLine(snapped, secondPoint);
                }
            }
            break;
        }
        case SetPoint2: {
            const RS_Vector secondPoint = defineLineSecondPointFree(snapped);
            previewRadiantLine(m_startPoint, secondPoint);
            break;
        }
        default:
            break;
    }
}

void LC_ActionDrawLineRadiant::onMouseLeftButtonRelease([[maybe_unused]]int status, const LC_MouseEvent* e) {
    const RS_Vector snapped = getRelZeroAwarePoint(e, e->snapPoint);
    fireCoordinateEvent(snapped);
}

void LC_ActionDrawLineRadiant::onMouseRightButtonRelease(const int status, [[maybe_unused]]const LC_MouseEvent* e) {
    if (status == SetPoint2) {
        setStatus(SetPoint);
    }
    else {
        setStatus(-1);
    }
}

bool LC_ActionDrawLineRadiant::doProcessCommand(const int status, const QString& command) {
    bool result = true;
    if (checkCommand("length", command)) {
        setStatus(SetLength);
    }
    else if (checkCommand("lentype", command)) {
      setStatus(SetLengthType);
    }
    else if (checkCommand("radiant", command)) {
        setStatus(SetRadiant);
    }
    else if (checkCommand("active", command)) {
        setStatus(SetActive);
    }
    else {
        if (status == SetLength) {
            bool ok = false;
            const double l = RS_Math::eval(command, &ok);
            if (ok && l > RS_TOLERANCE) {
                m_actionOptions->setLength(l);
                restoreMainStatus();
                updateOptions();
            }
            else {
                result = false;
            }
        }
        else if (status == SetLengthType) {
           if (tr("line") == command) {
               setLengthType(LC_ActionDrawLineRadiantOptions::LINE);
               updateOptions();
           }
            else if (tr("x") == command) {
               setLengthType(LC_ActionDrawLineRadiantOptions::BY_X);
               updateOptions();
            }
            else if (tr("y") == command) {
               setLengthType(LC_ActionDrawLineRadiantOptions::BY_Y);
               updateOptions();
            }
            else if (tr("point") == command) {
               setLengthType(LC_ActionDrawLineRadiantOptions::TO_POINT);
               updateOptions();
            }
            else if (tr("free") == command) {
               setLengthType(LC_ActionDrawLineRadiantOptions::FREE);
               updateOptions();
            }
            else {
                result = false;
            }
        }
        else if (status == SetActive) {
            bool ok;
            const int idx = command.toInt(&ok);
            if (ok) {
                if (idx < 5 && idx > 0) {
                    const auto type = static_cast<LC_ActionDrawLineRadiantOptions::RadiantIdx>(idx-1);
                    setActiveRadiantIndex(type);
                    updateOptions();
                    RS_Vector activePoint = getActiveRadiant();
                    QString activeStr = formatVector(activePoint);
                    commandMessage(tr("Radiant point to use: ")+ activeStr);
                }
                else {
                    commandMessage(tr("Invalid radiant point number."));
                }
            }
            else {
                result = false;
            }
        }
        else {
            result = false;
        }
    }
    return result;
}

void LC_ActionDrawLineRadiant::onCoordinateEvent(const int status, const bool isZero, const RS_Vector& pos) {
    if (isZero) {
        return;
    }
    switch (status) {
        case SetPoint: {
            m_startPoint = pos;
            if (isFreeLength()) {
                setMainStatus(SetPoint2);
            }
            else {
                trigger();
            }
            break;
        }
        case SetPoint2: {
            m_secondSnapPoint = pos;
            trigger();
            break;
        }
        case SetRadiant: {
            m_actionOptions->setActiveRadiantPoint(pos);
            updateOptions();
            restoreMainStatus();
            break;
        }
        default:
            break;
    }
}

RS_Vector LC_ActionDrawLineRadiant::defineLineSecondPointFree(const RS_Vector& snapped) const {
    const double angle = m_startPoint.angleTo(getActiveRadiant());
    const RS_Vector result = LC_LineMath::calculateEndpointForAngleDirection(angle, m_startPoint, snapped);
    return result;
}

RS_Vector LC_ActionDrawLineRadiant::defineLineSecondPointAuto(const RS_Vector& snapped) const {
    const auto activePoint = getActiveRadiant();
    switch (m_actionOptions->getLenghType()) {
        case LC_ActionDrawLineRadiantOptions::LenghtType::TO_POINT: {
            return activePoint;
        }
        case LC_ActionDrawLineRadiantOptions::LenghtType::LINE: {
            const double angle = snapped.angleTo(activePoint);
            double length = m_actionOptions->getLength();
            const RS_Vector result = snapped.relative(length, angle);
            return result;
        }
        case LC_ActionDrawLineRadiantOptions::LenghtType::BY_X: {
            const double len = m_actionOptions->getLength();
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
        case LC_ActionDrawLineRadiantOptions::LenghtType::BY_Y: {
            const double len = m_actionOptions->getLength();
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

bool LC_ActionDrawLineRadiant::doUpdateDistanceByInteractiveInput(const QString& tag, const double distance) {
    if (tag == "length") {
        setLength(distance);
        return true;
    }
    return false;
}

bool LC_ActionDrawLineRadiant::doUpdatePointByInteractiveInput(const QString& tag, RS_Vector& point) {
    if (tag == "pointX") {
        setActiveX(point.x);
        return true;
    }
    if (tag == "pointY") {
        setActiveY(point.y);
        return true;
    }
    if (tag == "farPoint") {
        setActiveRadiantPoint(point);
        return true;
    }
    return false;
}

RS2::CursorType LC_ActionDrawLineRadiant::doGetMouseCursor([[maybe_unused]]int status) {
    return RS2::CadCursor;
}

void LC_ActionDrawLineRadiant::updateMouseButtonHints() {
    switch (getStatus()) {
        case SetPoint: {
            updateMouseWidgetTRCancel(tr("Specify start point"), MOD_SHIFT_RELATIVE_ZERO);
            break;
        }
        case SetPoint2: {
            updateMouseWidgetTRCancel(tr("Specify second point"), MOD_SHIFT_RELATIVE_ZERO);
            break;
        }
        case SetRadiant: {
            updateMouseWidgetTRCancel(tr("Specify radiant point position"));
            break;
        }
        case SetLength: {
            updateMouseWidgetTRCancel(tr("Specify line length"));
            break;
        }
         case SetLengthType: {
            updateMouseWidgetTRCancel(tr("Specify line length type ") + QString("[%1|%2|%3|%4|%5]").arg(tr("line"), tr("x"), tr("y"), tr("point"), tr("free")));
            break;
        }
        case SetActive: {
            updateMouseWidgetTRCancel(tr("Specify number of radiant point to use [1-4]"));
            break;
        }
        default:
            break;
    }
}

QStringList LC_ActionDrawLineRadiant::getAvailableCommands() {
    QStringList cmd;
    cmd << command("length");
    cmd << command("lentype");
    cmd << command("active");
    cmd << command("radiant");
    return cmd;
}

RS_Vector LC_ActionDrawLineRadiant::getRadiantPoint(const LC_ActionDrawLineRadiantOptions::RadiantIdx idx) const {
    return m_actionOptions->getRadiantPoint(idx);
}

void LC_ActionDrawLineRadiant::setRadiantPoint(const LC_ActionDrawLineRadiantOptions::RadiantIdx idx, const RS_Vector& pos) {
    m_actionOptions->setRadiantPoint(idx, pos);
}

RS_Vector LC_ActionDrawLineRadiant::getActiveRadiant() const {
    return m_actionOptions->getActiveRadiant();
}

void LC_ActionDrawLineRadiant::setActiveRadiantIndex(LC_ActionDrawLineRadiantOptions::RadiantIdx idx) {
    m_actionOptions->setActiveRadiantIndex(idx);
}

LC_ActionDrawLineRadiantOptions::RadiantIdx LC_ActionDrawLineRadiant::getActiveRadiantIndex() const {
    return m_actionOptions->getActiveRadiantIndex();
}

void LC_ActionDrawLineRadiant::setActiveX(const double val) {
    m_actionOptions->setActiveX(val);
}

void LC_ActionDrawLineRadiant::setActiveRadiantPoint(const RS_Vector& v) {
    m_actionOptions->setActiveRadiantPoint(v);
}

void LC_ActionDrawLineRadiant::setActiveY(const double val) {
    m_actionOptions->setActiveY(val);
}

double LC_ActionDrawLineRadiant::getActiveX() const {
    return m_actionOptions->getActiveX();
}

double LC_ActionDrawLineRadiant::getActiveY() const {
    return m_actionOptions->getActiveY();
}

void LC_ActionDrawLineRadiant::setLength(const double len) {
    m_actionOptions->m_length = len;
}

double LC_ActionDrawLineRadiant::getLength() const {
    return m_actionOptions->m_length;
}

void LC_ActionDrawLineRadiant::setLengthType(const LC_ActionDrawLineRadiantOptions::LenghtType type) {
    m_actionOptions->m_lengthType = type;
    setMainStatus(SetPoint);
}

LC_ActionDrawLineRadiantOptions::LenghtType LC_ActionDrawLineRadiant::getLenghType() const {
    return m_actionOptions->m_lengthType;
}
