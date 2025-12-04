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
#include "rs_actionpolylinesegment.h"

#include "rs_arc.h"
#include "rs_debug.h"
#include "rs_document.h"
#include "rs_entitycontainer.h"
#include "rs_polyline.h"
#include "rs_preview.h"

namespace {
QList<RS2::EntityType>
entityType{RS2::EntityLine, RS2::EntityPolyline, RS2::EntityArc};
}

RS_ActionPolylineSegment::RS_ActionPolylineSegment(LC_ActionContext *actionContext)
        :LC_UndoableDocumentModificationAction("Create Polyline Existing from Segments",actionContext,RS2::ActionPolylineSegment) {
}

RS_ActionPolylineSegment::RS_ActionPolylineSegment(LC_ActionContext *actionContext,RS_Entity* target)
    :LC_UndoableDocumentModificationAction("Create Polyline Existing from Segments",actionContext, RS2::ActionPolylineSegment) {
    m_targetEntity = target;
    m_initWithTarget = true;
}

void RS_ActionPolylineSegment::drawSnapper() {
    // disable snapper for this action
}

void RS_ActionPolylineSegment::doInitWithContextEntity(RS_Entity* contextEntity,[[maybe_unused]] const RS_Vector& clickPos) {
    m_targetEntity = contextEntity;
    m_initWithTarget = true;
}

void RS_ActionPolylineSegment::init(int status){
    RS_PreviewActionInterface::init(status);
    if (m_initWithTarget){
        m_initWithTarget = false;
        trigger();
        commandMessage(tr("Polyline created"));
        finish(false);
        return;
    }
    else {
        m_targetEntity  = nullptr;
        auto selection = m_document->getSelection();
        //trigger action, if already has selected entities
        if (!selection->isEmpty()){
            //find a selected entity
            //TODO, find a better starting point
            QList<RS_Entity*> selectedEntities;
            selection->collectSelectedEntities(selectedEntities);
            for (const auto e: selectedEntities) {
                if (std::count(entityType.begin(), entityType.end(), e->rtti())){
                    m_targetEntity = e;
                    break;
                }
            }
            if (m_targetEntity){
                trigger();
                commandMessage(tr("Polyline created"));
                finish(false);
                return;
            }
        }
    }
}

/**
 * Utility function for convertPolyline
 * Appends in current the vertex from toAdd reversing if reversed is true
 * The first vertex is not added and the last is returned instead of added
 *
 * @retval RS_Vector with the last vertex not inserted
 *
 * @author Rallaz
 */
RS_Vector RS_ActionPolylineSegment::appendPol(RS_Polyline *current, RS_Polyline *toAdd, bool reversed) const {
    QList<RS_Entity *> entities;

    for (auto v: *toAdd) {
        if (reversed) {
            entities.prepend(v);
        }
        else {
            entities.append(v);
        }
    }
    //bad polyline without vertex
    if (entities.isEmpty()) {
        return RS_Vector(false);
    }

    double bulge = 0.0;
    RS_Entity *e = entities.takeFirst();

    //First polyline vertex
    if (isArc(e)) {
        if (reversed) {
            current->setNextBulge(static_cast<RS_Arc*>(e)->getBulge() * -1);
        }
        else {
            current->setNextBulge(static_cast<RS_Arc*>(e)->getBulge());
        }
    }

    while (!entities.isEmpty()) {
        e = entities.takeFirst();
        if (isArc(e)) {
            if (reversed) {
                bulge = static_cast<RS_Arc*>(e)->getBulge() * -1;
            }
            else {
                bulge = static_cast<RS_Arc*>(e)->getBulge();
            }
        } else {
            bulge = 0.0;
        }
        if (reversed) {
            current->addVertex(e->getEndpoint(), bulge, false);
        }
        else {
            current->addVertex(e->getStartpoint(), bulge, false);
        }
    }
    if (reversed) {
        return e->getStartpoint();
    }
    else {
        return e->getEndpoint();
    }
}

/**
 * Rearranges the lines, arcs or opened polylines entities
 *  in this container, non-recoursive.
 * document can not be null
 *
 * @retval true contour are closed
 * @retval false if the contour is not closed
 *
 * @author Rallaz
 */
RS_Polyline* RS_ActionPolylineSegment::convertPolyline(RS_Entity *selectedEntity, bool useSelected, LC_DocumentModificationBatch& ctx){
    QList<RS_Entity *> remaining;
    QList<RS_Entity *> completed;
    RS_Vector start = selectedEntity->getStartpoint();
    RS_Vector end = selectedEntity->getEndpoint();
    if (!useSelected || selectedEntity->isSelected()){
        completed.append(selectedEntity);
    }

    //get list with useful entities

    for (RS_Entity *e1: *m_document) {  // fixme - selection - review this cycle, may we rely on already selected entities there ?
        if (useSelected && !e1->isSelected()) {  // fixme - SELECTION - selection collection!
            continue;
        }
        if (e1->isLocked() || !e1->isVisible() || e1 == selectedEntity) {
            continue;
        }
        if (isLine(e1) || isArc(e1) || e1->rtti() == RS2::EntityPolyline){ // fixme - support of polyline
            if (selectedEntity->rtti() == RS2::EntityPolyline && static_cast<RS_Polyline*>(selectedEntity)->isClosed()) {
                continue;
            }
            remaining.append(e1);
        }
    }

    // find all connected entities:
    // fixme - this is search for contour - reuse the same code as for contour selection
    bool done = true;
    do {
        done = true;
        for (int i = (remaining.size() - 1); i >= 0; --i) {
            RS_Entity *e                = remaining.at(i);
            const RS_Vector& endpoint   = e->getEndpoint();
            const RS_Vector& startpoint = e->getStartpoint();
            if (endpoint.distanceTo(start) < 1.0e-4){  // fixme = RS_TOLERANCE??
                completed.prepend(e);
                start = startpoint;
                remaining.removeAt(i);
                done = false;
            } else if (startpoint.distanceTo(start) < 1.0e-4){
                completed.prepend(e);
                start = endpoint;
                remaining.removeAt(i);
                done = false;
            } else if (endpoint.distanceTo(end) < 1.0e-4){
                completed.append(e);
                end = startpoint;
                remaining.removeAt(i);
                done = false;
            } else if (startpoint.distanceTo(end) < 1.0e-4){
                completed.append(e);
                end = endpoint;
                remaining.removeAt(i);
                done = false;
            }
        }
    } while (!done);

    //cleanup for no more needed list
    remaining.clear();

    bool closed = false;
    RS_Polyline *newPolyline = nullptr;

    bool revert  = false;
    double bulge = 0.0;
    if (end.distanceTo(start) < 1.0e-4) {
        closed = true;
    }

    newPolyline = new RS_Polyline(nullptr, RS_PolylineData(RS_Vector(false), RS_Vector(false), closed));

    //complete polyline
    end = start;
    while (!completed.isEmpty()) {
        RS_Entity* e2 = completed.takeFirst();

        ctx -= e2;

        if (e2->getStartpoint().distanceTo(end) < 1.0e-4) {
            revert = false;
            start  = e2->getStartpoint();
            end    = e2->getEndpoint();
        }
        else {
            revert = true;
            start  = e2->getEndpoint();
            end    = e2->getStartpoint();
        }
        if (e2->rtti() == RS2::EntityArc) {
            auto arc = static_cast<RS_Arc*>(e2);
            bulge = arc->getBulge();
            if (revert) {
                bulge = bulge * -1.0;
            }
        }
        else {
            bulge = 0.0;
        }

        if (e2->rtti() == RS2::EntityPolyline) { /// fixme - how it could be?? nested polyline?
            newPolyline->addVertex(start, bulge);
            end = appendPol(newPolyline, static_cast<RS_Polyline*>(e2), revert);
        }
        else {
            newPolyline->addVertex(start, bulge);
        }
    }

    if (closed) {
        newPolyline->setClosed(true);
    }
    else {
        newPolyline->addVertex(end, bulge);
    }

    newPolyline->endPolyline();

    ctx += newPolyline;
    return newPolyline;
}

bool RS_ActionPolylineSegment::doTriggerModifications(LC_DocumentModificationBatch& ctx) {
    if (m_targetEntity != nullptr /*&& selectedSegment && targetPoint.valid */){
        convertPolyline(m_targetEntity, false, ctx);
        select(ctx.entitiesToAdd);
        return true;
    }
    return false;
}

void RS_ActionPolylineSegment::doTriggerCompletion(bool success) {
    if (success) {
        m_targetEntity = nullptr;
        setStatus(ChooseEntity);
    }
}

void RS_ActionPolylineSegment::onMouseMoveEvent([[maybe_unused]]int status, LC_MouseEvent *event) {
    RS_Entity* en = catchAndDescribe(event, entityType, RS2::ResolveNone);
    if (en != nullptr){
        highlightHover(en);
        if (!(en->rtti() == RS2::EntityPolyline && static_cast<RS_Polyline*>(en)->isClosed())){
            LC_DocumentModificationBatch ctx;
            convertPolyline(en, false, ctx);
            m_preview->addAllFromList(ctx.entitiesToAdd);
        }
    }
}

void RS_ActionPolylineSegment::onMouseLeftButtonRelease(int status, LC_MouseEvent *e) {
    switch (status) {
        case ChooseEntity:
            m_targetEntity = catchEntityByEvent(e, entityType);
            if (m_targetEntity == nullptr){
                commandMessage(tr("No Entity found."));
            } else if (m_targetEntity->rtti() == RS2::EntityPolyline && static_cast<RS_Polyline*>(m_targetEntity)->isClosed()){
                commandMessage(tr("Entity can not be a closed polyline."));
            } else {
                redraw();
                trigger();
            }
            break;
        default:
            break;
    }
}

void RS_ActionPolylineSegment::onMouseRightButtonRelease(int status, [[maybe_unused]]  LC_MouseEvent *e) {
    deleteSnapper();
    if (m_targetEntity){
         redraw();
    }
    initPrevious(status);
}

void RS_ActionPolylineSegment::updateMouseButtonHints(){
    switch (getStatus()) {
        case ChooseEntity:
            updateMouseWidgetTRCancel(tr("Choose one of the segments on the original polyline"));
            break;
        default:
            updateMouseWidget();
            break;
    }
}
RS2::CursorType RS_ActionPolylineSegment::doGetMouseCursor([[maybe_unused]] int status){
    return RS2::SelectCursor;
}
