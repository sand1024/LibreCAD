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

#include "lc_icon_label.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEvent>
#include <QScreen>
#include <QWindow>

LC_IconLabel::LC_IconLabel(QWidget *parent)
    : QWidget(parent)
    , m_icon()
    , m_isHovered(false)
    , m_isPressed(false)
    , m_fallbackIconSize(16, 16)
{
    // Enable hover event tracking on the widget to trigger QEvent::HoverEnter/Leave
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
}

LC_IconLabel::LC_IconLabel(const QIcon &icon, QWidget *parent)
    : QWidget(parent)
    , m_icon(icon)
    , m_isHovered(false)
    , m_isPressed(false)
    , m_fallbackIconSize(16, 16)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
}

void LC_IconLabel::setIcon(const QIcon &icon)
{
    m_icon = icon;
    updateGeometry(); // Notify layouts that the widget aspect ratios might have shifted
    update();         // Force a redraw
}

void LC_IconLabel::setFallbackIconSize(const QSize &size)
{
    if (m_fallbackIconSize != size) {
        m_fallbackIconSize = size;
        updateGeometry();
        update();
    }
}

QSize LC_IconLabel::getBaseIconSize() const
{
    if (m_icon.isNull()) {
        return m_fallbackIconSize;
    }

    // Attempt to query the sizes provided by the QIconEngine
    QList<QSize> sizes = m_icon.availableSizes();
    if (!sizes.isEmpty()) {
        return sizes.first();
    }

    return m_fallbackIconSize;
}

bool LC_IconLabel::hasHeightForWidth() const
{
    return !m_icon.isNull();
}

int LC_IconLabel::heightForWidth(int w) const
{
    QSize baseSize = getBaseIconSize();
    if (baseSize.width() <= 0) {
        return w;
    }

    // Proportional height scaling: H = W * (BaseH / BaseW)
    return qRound(qreal(w) * qreal(baseSize.height()) / qreal(baseSize.width()));
}

QSize LC_IconLabel::sizeHint() const
{
    return getBaseIconSize();
}

QSize LC_IconLabel::minimumSizeHint() const
{
    // Minimum 8x8 size boundary to prevent layout locks
    return QSize(8, 8);
}

void LC_IconLabel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_icon.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Sizing handled strictly inside logical coordinate boundaries
    QSize widgetSize = rect().size();
    QSize iconSize = getBaseIconSize();

    // Preserve aspect ratio inside target rendering bounds
    QSize targetSize = iconSize.scaled(widgetSize, Qt::KeepAspectRatio);

    QRect targetRect(QPoint(0, 0), targetSize);
    targetRect.moveCenter(rect().center());

    // Evaluate active interactive states
    QIcon::Mode mode = QIcon::Normal;
    if (!isEnabled()) {
        mode = QIcon::Disabled;
    } else if (m_isPressed && m_isHovered) {
        mode = QIcon::Selected;
    } else if (m_isHovered) {
        mode = QIcon::Active;
    }

    // QIconEngine automatically rasterizes vector paths based on the painter's device scale factor
    m_icon.paint(&painter, targetRect, Qt::AlignCenter, mode, QIcon::Off);
}

bool LC_IconLabel::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::HoverEnter:
            m_isHovered = true;
            update();
            break;
        case QEvent::HoverLeave:
            m_isHovered = false;
            // Preserves standard logical pressed state during drag-out operations
            update();
            break;
        case QEvent::ScreenChangeInternal:
            update(); // Trigger redraw upon shifting monitors with different scale metrics
            break;
        default:
            break;
    }
    return QWidget::event(event);
}

void LC_IconLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void LC_IconLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false;
        update();
        event->accept();

        // Emit click trigger only if the mouse was released inside the widget geometry bounds
        if (rect().contains(event->pos())) {
            emit clicked();
        }
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void LC_IconLabel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    // Refresh rendering upon theme-palette transitions or widget state lock toggles
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::EnabledChange) {
        update();
    }
}
