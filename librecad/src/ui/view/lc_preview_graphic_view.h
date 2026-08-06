
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

#ifndef LC_PreviewGraphicView_H
#define LC_PreviewGraphicView_H

#include <QMouseEvent>
#include "qg_graphicview.h"

class LC_GraphicViewRenderer;

class LC_PreviewGraphicView: public QG_GraphicView{
public:
   virtual void refresh(){};
   void zoomPan() const;
   LC_GraphicViewRenderer* getGraphicViewRenderer() const {return static_cast<LC_GraphicViewRenderer*>(getRenderer());}
   void addOverlayDrawable(LC_OverlayDrawable* drawable, const RS2::OverlayGraphics position) const;
   void clearOverlayDrawables(const RS2::OverlayGraphics position) const;
   void clearOverlayEntities(RS2::OverlayGraphics position) const;
   void setHighlightEntitiesOnHover(bool value);
   RS_Entity* getNearestEntity(const RS_Vector& pos) const;


protected:
    LC_PreviewGraphicView(QWidget* parent,LC_ActionContext* actionContext);
    ~LC_PreviewGraphicView() override;
    bool proceedEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
private:
    bool m_highlightOnHover = false;
    RS_Entity* m_highlightedEntity = nullptr;
};

#endif
