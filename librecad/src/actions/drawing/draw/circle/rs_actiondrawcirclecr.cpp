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

#include "rs_actiondrawcirclecr.h"

#include "qg_circleoptions.h"
#include "rs_circle.h"
#include "rs_document.h"
#include "rs_entitycontainer.h"

/**
 * Constructor.
 */
RS_ActionDrawCircleCR::RS_ActionDrawCircleCR(LC_ActionContext* actionContext)
    : LC_ActionDrawCircleBase("Draw circles CR", actionContext, RS2::ActionDrawCircleCR), m_circleData(std::make_unique<RS_CircleData>()) {
    RS_ActionDrawCircleCR::reset();
}

RS_ActionDrawCircleCR::~RS_ActionDrawCircleCR() = default;

void RS_ActionDrawCircleCR::reset() {
    m_circleData = std::make_unique<RS_CircleData>();
}

void RS_ActionDrawCircleCR::init(const int status) {
    LC_ActionDrawCircleBase::init(status);
}

RS_Entity* RS_ActionDrawCircleCR::doTriggerCreateEntity() {
    auto* circle = new RS_Circle(m_document, *m_circleData);
    switch (getStatus()) {
        case SetCenter:
            moveRelativeZero(circle->getCenter());
            break;
        case SetRadius:
            break;
        default:
            break;
    }
    return circle;
}

void RS_ActionDrawCircleCR::doTriggerCompletion([[maybe_unused]] bool success) {
    setStatus(SetCenter);
}

void RS_ActionDrawCircleCR::onMouseMoveEvent(const int status, const LC_MouseEvent* e) {
    const RS_Vector mouse = e->snapPoint;
    switch (status) {
        case SetCenter: {
            if (!trySnapToRelZeroCoordinateEvent(e)) {
                m_circleData->center = mouse;
                previewToCreateCircle(*m_circleData);
                previewRefSelectablePoint(m_circleData->center);
            }
            else {
                setStatus(-1);
            }
            break;
        }
        default:
            break;
    }
}

bool RS_ActionDrawCircleCR::doUpdateDistanceByInteractiveInput(const QString& tag, const double distance) {
    if (tag == "radius") {
        setRadius(distance);
        return true;
    }
    return false;
}

void RS_ActionDrawCircleCR::onCoordinateEvent(const int status, [[maybe_unused]] bool isZero, const RS_Vector& pos) {
    switch (status) {
        case SetCenter: {
            m_circleData->center = pos;
            trigger();
            break;
        }
        default:
            break;
    }
}

bool RS_ActionDrawCircleCR::doProcessCommand(const int status, const QString& command) {
    bool accept = false;
    switch (status) {
        case SetCenter: {
            if (checkCommand("radius", command)) {
                deletePreview();
                setStatus(SetRadius);
                accept = true;
            }
            break;
        }
        case SetRadius: {
            bool ok = false;
            // fixme - review processing and add more messages if needed
            const double r = RS_Math::eval(command, &ok);
            if (ok && r > RS_TOLERANCE) {
                m_circleData->radius = r;
                accept = true;
                trigger();
            }
            else {
                commandMessage(tr("Not a valid expression"));
            }
            updateOptions();
            break;
        }
        default:
            break;
    }
    return accept;
}

bool RS_ActionDrawCircleCR::setRadiusStr(const QString& sr) const {
    bool ok = false;
    const double r = RS_Math::eval(sr, &ok);
    if (!ok) {
        // fixme - good candidate for generic utility method, may be useful for setting values via ui
        commandMessage(tr("radius=%1 is invalid (expression)").arg(sr));
    }
    else if (std::signbit(r)) {
        commandMessage(tr("radius=%1 is invalid (negative)").arg(sr));
        ok = false;
    }
    else if (r <= RS_TOLERANCE) {
        commandMessage(tr("radius=%1 is invalid (zero)").arg(sr));
        ok = false;
    }
    else {
        m_circleData->radius = r;
    }
    return ok;
}

QStringList RS_ActionDrawCircleCR::getAvailableCommands() {
    QStringList cmd;
    switch (getStatus()) {
        case SetCenter:
            cmd += command("radius");
            break;
        default:
            break;
    }
    return cmd;
}

void RS_ActionDrawCircleCR::updateMouseButtonHints() {
    switch (getStatus()) {
        case SetCenter:
            updateMouseWidgetTRCancel(tr("Specify circle center"), MOD_SHIFT_RELATIVE_ZERO);
            break;
        case SetRadius:
            updateMouseWidgetTRBack(tr("Specify circle radius"));
            break;
        default:
            updateMouseWidget();
            break;
    }
}

void RS_ActionDrawCircleCR::setRadius(const double val) const {
    m_circleData->radius = val;
}

double RS_ActionDrawCircleCR::getRadius() const {
    return m_circleData->radius;
}

LC_ActionOptionsWidget* RS_ActionDrawCircleCR::createOptionsWidget() {
    return new QG_CircleOptions();
}
