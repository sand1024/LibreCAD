/*******************************************************************************
 *
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

#include "lc_overlay_selection_lasso.h"

#include <QPolygonF>

#include "lc_overlayentity.h"
#include "lc_overlay_selection_box.h"
#include "rs_painter.h"
#include "rs_vector.h"

LC_OverlaySelectionLasso::LC_OverlaySelectionLasso(LC_SelectionBoxOptions* options):
    m_options(options) {}

void LC_OverlaySelectionLasso::update(const std::vector<RS_Vector>& points, bool crossing, bool fence) {
    m_inverting = crossing;
    m_fence = fence;
    m_worldPoints = &points;
}

void LC_OverlaySelectionLasso::updateScreenPolygon(const QTransform& transform,
                                                   const std::vector<RS_Vector>* worldPoints) {
    size_t size = worldPoints->size();
    m_screenPoints.reserve(static_cast<qsizetype>(size) +1);

    double tx = 0.0;
    double ty = 0.0;
    for (size_t i = 0; i < size; ++i) {
        auto point = (*worldPoints)[i];
        transform.map(point.x, point.y, &tx, &ty);
        m_screenPoints.push_back(QPointF{tx, ty});

    }
    if (!m_fence) {
        // make polygon closed
        auto point = (*worldPoints)[0];
        transform.map(point.x, point.y, &tx, &ty);
        m_screenPoints.push_back(QPointF{tx, ty});
    }
}

void LC_OverlaySelectionLasso::draw(RS_Painter* painter) {
    if (m_worldPoints->empty()) {
        return;
    }

    auto transform = painter->getToGUITransform();
    updateScreenPolygon(transform, m_worldPoints);

    painter->save();
    if (m_fence) {
        RS_Pen p(m_options->colorLineInverted, RS2::Width00, m_options->invertedLineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        painter->setBrush(Qt::NoBrush);
        painter->QPainter::drawPolyline(m_screenPoints);
    }
    else if (m_inverting) {
        RS_Pen p(m_options->colorLineInverted, RS2::Width00, m_options->invertedLineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        const RS_Color &fillColor = m_options->colorBoxFillInverted;
        painter->setBrush(fillColor.toQColor());
        painter->QPainter::drawPolygon(m_screenPoints, Qt::OddEvenFill);
    }
    else {
        RS_Pen p(m_options->colorBoxLine, RS2::Width00, m_options->lineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        const RS_Color &fillColor = m_options->colorBoxFill;
        painter->setBrush(fillColor.toQColor());
        painter->QPainter::drawPolygon(m_screenPoints, Qt::OddEvenFill);
    }
    painter->restore();
}
