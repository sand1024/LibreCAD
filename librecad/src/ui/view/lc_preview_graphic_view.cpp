/*******************************************************************************
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
 ******************************************************************************/


#include "lc_preview_graphic_view.h"

#include "lc_defaultactioncontext.h"
#include "lc_eventhandler.h"
#include "lc_overlayentitiescontainer.h"

// class LC_PreviewActionContext: public LC_DefaultActionContext {
// public:
//     explicit LC_PreviewActionContext(QG_ActionHandler* actionHandler)
//         : LC_DefaultActionContext{actionHandler} {
//     }
//
//     ~LC_PreviewActionContext() override {deleteActionHandler();}
// };

LC_PreviewGraphicView::LC_PreviewGraphicView(QWidget* parent, LC_ActionContext* actionContext):
  QG_GraphicView(parent, nullptr, actionContext){
}

LC_PreviewGraphicView::~LC_PreviewGraphicView() {
    deleteActionContext();
}

bool LC_PreviewGraphicView::proceedEvent(QEvent* event) {
    return QWidget::event(event);
}

void LC_PreviewGraphicView::zoomPan() const {
    switchToAction(RS2::ActionZoomPan);
}

void LC_PreviewGraphicView::mousePressEvent(QMouseEvent* event){
    // pan zoom with middle mouse button
    if (event->button()==Qt::MiddleButton){
        switchToAction(RS2::ActionZoomPan);
        getCurrentAction()->mousePressEvent(event);
    }
    else {
        getEventHandler()->mousePressEvent(event);
    }
}

void LC_PreviewGraphicView::mouseDoubleClickEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::MiddleButton:
            switchToAction(RS2::ActionZoomAuto);
            break;
        default:
            break;
    }
    e->accept();
}

void LC_PreviewGraphicView::mouseReleaseEvent(QMouseEvent* event){
    event->accept();

    switch (event->button()) {
        case Qt::RightButton: {
            back(Qt::KeyboardModifier::NoModifier);
            break;
        }
        case Qt::XButton1:
            processEnterKey();
            emit xbutton1_released();
            break;
        default:
            getEventHandler()->mouseReleaseEvent(event);
            break;
    }
}

void LC_PreviewGraphicView::mouseMoveEvent(QMouseEvent* event) {
    if (m_highlightOnHover) {
        if (event->button() == Qt::NoButton && event->modifiers() == Qt::NoModifier) {
            double dist;
            const QPointF& pointF = event->position();
            const RS_Vector graphPoint = getViewPort()->toWorldFromUi(pointF.x(), pointF.y());
            const auto e = getDocument()->getNearestEntity(graphPoint, &dist, RS2::ResolveNone);
            if (m_highlightedEntity != nullptr) {
                if (e->getUserDefVar("_fixedHighlight").isEmpty()) {
                    m_highlightedEntity->setHighlighted(false);
                }
                m_highlightedEntity = nullptr;
            }
            if (e != nullptr) {
                if (dist < getViewPort()->toUcsDX(10)) { // rough 10px range
                    m_highlightedEntity = e;
                    e->setHighlighted(true);
                };
            }
            redraw(RS2::RedrawDrawing);
            return;
        }
    }

    QG_GraphicView::mouseMoveEvent(event);
}

void LC_PreviewGraphicView::addOverlayDrawable(LC_OverlayDrawable* drawable, const RS2::OverlayGraphics position) const {
    const auto drawablesContainer = getViewPort()->getOverlaysDrawablesContainer(position);
    drawablesContainer->add(drawable);
}

void LC_PreviewGraphicView::clearOverlayDrawables(const RS2::OverlayGraphics position) const {
    const auto drawablesContainer = getViewPort()->getOverlaysDrawablesContainer(position);
    drawablesContainer->clear();
}

void LC_PreviewGraphicView::clearOverlayEntities(const RS2::OverlayGraphics position) const {
    auto* drawablesContainer = getViewPort()->getOverlayEntitiesContainer(position);
    drawablesContainer->clear();
}

void LC_PreviewGraphicView::setHighlightEntitiesOnHover(bool value) {
    if (!value) {
        m_highlightedEntity = nullptr;
    }
    m_highlightOnHover = value;
}

RS_Entity* LC_PreviewGraphicView::getNearestEntity(const RS_Vector& pos) const {
    return getDocument()->getNearestEntity(pos);
}
