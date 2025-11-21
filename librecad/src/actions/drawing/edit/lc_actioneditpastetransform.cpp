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
#include "lc_actioneditpastetransform.h"

#include "lc_copyutils.h"
#include "lc_pastetransformoptions.h"
#include "rs_clipboard.h"
#include "rs_graphic.h"
#include "rs_preview.h"
#include "rs_units.h"

// fixme - sand - ucs - Check for support of UCS!

LC_ActionEditPasteTransform::LC_ActionEditPasteTransform(LC_ActionContext *actionContext)
    :LC_UndoableDocumentModificationAction("PasteTransform", actionContext,  RS2::ActionEditPasteTransform),
    m_referencePoint{new RS_Vector(false)},
    m_pasteData{new PasteData()}{
}

void LC_ActionEditPasteTransform::init(int status) {
    RS_PreviewActionInterface::init(status);
    if (RS_CLIPBOARD->count() == 0){
        commandMessage(tr("Clipboard is empty"));
        finish(false);
    }
}

bool LC_ActionEditPasteTransform::doTriggerModificationsPrepare(LC_DocumentModificationBatch& ctx) {
    int numX = m_pasteData->arrayXCount;
    int numY = m_pasteData->arrayYCount;

    RS_Vector xArrayVector;
    RS_Vector yArrayVector;
    if (m_pasteData->arrayCreated){
        double arrayAngle = m_pasteData->arrayAngle;
        xArrayVector = RS_Vector::polar(m_pasteData->arraySpacing.x, arrayAngle);
        yArrayVector = RS_Vector::polar(m_pasteData->arraySpacing.y, arrayAngle + M_PI_2);
    }
    else{
        numX = 1;
        numY = 1;
    }
    for (int x = 0; x < numX; x++){
        for (int y = 0; y < numY; y++){
            RS_Vector currentPoint  = *m_referencePoint + xArrayVector*x + yArrayVector * y;
            const auto pasteData = LC_CopyUtils::RS_PasteData(currentPoint, m_pasteData->factor , m_pasteData->angle);
            LC_CopyUtils::paste(pasteData, m_graphic, ctx);
            // fixme - some progress is needed there, ++++ speed improvement for paste operation!!
//            LC_ERR << "Paste: " << x+y;
        }
    }
    ctx.dontSetActiveLayerAndPen();
    return true;
}

void LC_ActionEditPasteTransform::doTriggerCompletion(bool success) {
    if (!m_invokedWithControl) {
        finish(false);
    }
}

void LC_ActionEditPasteTransform::onMouseMoveEvent(int status, LC_MouseEvent *e) {
    if (status==SetReferencePoint) {
        *m_referencePoint = e->snapPoint;
        auto clipboardGraphics   = RS_CLIPBOARD->getGraphic();
        m_preview->addAllFrom(*clipboardGraphics,m_viewport);
        m_preview->move(*m_referencePoint);

        if (m_graphic) {
            RS_Vector scaleFactor = LC_CopyUtils::getInterGraphicsScaleFactor(m_pasteData->factor, clipboardGraphics, m_graphic);
            m_preview->scale(*m_referencePoint, scaleFactor);
            m_preview->rotate(*m_referencePoint, m_pasteData->angle);

            if (m_showRefEntitiesOnPreview) {
                previewMultipleReferencePoints();
            }
        }
    }
    else {
        deleteSnapper();
    }
}

void LC_ActionEditPasteTransform::onMouseLeftButtonRelease([[maybe_unused]]int status, LC_MouseEvent *e) {
    m_invokedWithControl = e->isControl;
    fireCoordinateEventForSnap(e);
}

void LC_ActionEditPasteTransform::onMouseRightButtonRelease(int status,[[maybe_unused]] LC_MouseEvent *e) {
    initPrevious(status);
}

void LC_ActionEditPasteTransform::onCoordinateEvent([[maybe_unused]]int status, [[maybe_unused]]bool isZero, const RS_Vector &pos) {
    *m_referencePoint = pos;
    trigger();
}

RS2::CursorType LC_ActionEditPasteTransform::doGetMouseCursor([[maybe_unused]]int status) {
    return RS2::CadCursor;
}

void LC_ActionEditPasteTransform::updateMouseButtonHints() {
    switch (getStatus()) {
        case SetReferencePoint:
            updateMouseWidgetTRCancel(tr("Set paste reference point"), MOD_CTRL(tr("Paste Multiple")));
            break;

        default:
            updateMouseWidget();
            break;
    }
}

bool LC_ActionEditPasteTransform::doUpdateAngleByInteractiveInput(const QString& tag, double angle) {
    if (tag == "angle") {
        setAngle(angle);
        return true;
    }
    else if (tag == "arrayAngle") {
        setArrayAngle(angle);
        return true;
    }
    return false;
}

bool LC_ActionEditPasteTransform::doUpdateDistanceByInteractiveInput(const QString& tag, double distance) {
    if (tag == "spacingX") {
        setArraySpacingX(distance);
        return true;
    }
    else if (tag == "spacingY") {
        setArraySpacingY(distance);
        return true;
    }
    return false;
}

double LC_ActionEditPasteTransform::getAngle() const {return m_pasteData-> angle;}
void LC_ActionEditPasteTransform::setAngle(double angle) const {m_pasteData->angle = angle;}
double LC_ActionEditPasteTransform::getFactor() const {return m_pasteData->factor;}
void LC_ActionEditPasteTransform::setFactor(double factor) const {m_pasteData->factor = factor;}
bool LC_ActionEditPasteTransform::isArrayCreated() const {return m_pasteData->arrayCreated;}
void LC_ActionEditPasteTransform::setArrayCreated(bool arrayCreated) const {m_pasteData->arrayCreated = arrayCreated;}
int LC_ActionEditPasteTransform::getArrayXCount() const {return m_pasteData->arrayXCount;}
void LC_ActionEditPasteTransform::setArrayXCount(int arrayXCount) const {m_pasteData->arrayXCount = arrayXCount;}
int LC_ActionEditPasteTransform::getArrayYCount() const {return m_pasteData->arrayYCount;}
void LC_ActionEditPasteTransform::setArrayYCount(int arrayYCount) const {m_pasteData->arrayYCount = arrayYCount;}
double LC_ActionEditPasteTransform::getArraySpacingX() const {return m_pasteData->arraySpacing.x;}
void LC_ActionEditPasteTransform::setArraySpacingX(double arraySpacing) const {m_pasteData->arraySpacing.x = arraySpacing;}
double LC_ActionEditPasteTransform::getArraySpacingY() const {return m_pasteData->arraySpacing.y;}
void LC_ActionEditPasteTransform::setArraySpacingY(double arraySpacing) const {m_pasteData->arraySpacing.y = arraySpacing;}
double LC_ActionEditPasteTransform::getArrayAngle() const {return m_pasteData->arrayAngle;}
void LC_ActionEditPasteTransform::setArrayAngle(double arrayAngle) const {m_pasteData->arrayAngle = arrayAngle;}
LC_ActionOptionsWidget *LC_ActionEditPasteTransform::createOptionsWidget() {return new LC_PasteTransformOptions();}

void LC_ActionEditPasteTransform::previewMultipleReferencePoints() const {
    int numX = m_pasteData->arrayXCount;
    int numY = m_pasteData->arrayYCount;

    RS_Vector xArrayVector;
    RS_Vector yArrayVector;
    if (m_pasteData->arrayCreated) {
        double arrayAngle = m_pasteData->arrayAngle;
        xArrayVector = RS_Vector::polar(m_pasteData->arraySpacing.x, arrayAngle);
        yArrayVector = RS_Vector::polar(m_pasteData->arraySpacing.y, arrayAngle + M_PI_2);
    }
    else{
        xArrayVector = RS_Vector(0,0,0);
        yArrayVector = RS_Vector(0,0,0);
        numX = 1;
        numY = 1;
    }

    for (int x = 0; x < numX; x++){
        for (int y = 0; y < numY; y++){
            RS_Vector currentPoint = *m_referencePoint + xArrayVector*x + yArrayVector * y;
         /*   const RS_PasteData &pasteData = RS_PasteData(currentPoint, data->factor , data->angle,
                                                         false, "");*/
//            m.paste(pasteData);
            previewRefPoint(currentPoint);
        }
    }
}
