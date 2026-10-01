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

#include "lc_rotated_icon_engine.h"

#include <QPainter>
#include <QPixmap>
#include <QRectF>

LC_RotatedIconEngine::LC_RotatedIconEngine(const QIcon& sourceIcon, const qreal angle)
    : m_sourceIcon(sourceIcon)
    , m_angle(angle) {
}

void LC_RotatedIconEngine::paint(QPainter* painter, const QRect& rect, const QIcon::Mode mode, const QIcon::State state) {
    if (painter == nullptr || rect.isEmpty() || m_sourceIcon.isNull()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QPointF center = QRectF(rect).center();
    painter->translate(center);
    painter->rotate(m_angle);

    const QRect localRect(-rect.width() / 2, -rect.height() / 2, rect.width(), rect.height());
    m_sourceIcon.paint(painter, localRect, Qt::AlignCenter, mode, state);

    painter->restore();
}

QIconEngine* LC_RotatedIconEngine::clone() const {
    return new LC_RotatedIconEngine(*this);
}

QPixmap LC_RotatedIconEngine::pixmap(const QSize& size, const QIcon::Mode mode, const QIcon::State state) {
    if (size.isEmpty()) {
        return QPixmap();
    }

    QPixmap px(size);
    px.fill(Qt::transparent);

    QPainter p(&px);
    paint(&p, QRect(QPoint(0, 0), size), mode, state);
    p.end();

    return px;
}

QSize LC_RotatedIconEngine::actualSize(const QSize& size, const QIcon::Mode mode, const QIcon::State state) {
    return m_sourceIcon.actualSize(size, mode, state);
}
