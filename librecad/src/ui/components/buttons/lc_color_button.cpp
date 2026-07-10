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

#include "lc_color_button.h"

#include <QColorDialog>
#include <QPainter>

LC_ColorButton::LC_ColorButton(const QColor& color, QWidget* parent)
    : QPushButton(parent), m_color(color) {
    connect(this, &QPushButton::clicked, this, &LC_ColorButton::chooseColor);
    updateSwatch();

}LC_ColorButton::LC_ColorButton(QWidget* parent)
    : QPushButton(parent), m_color(QColor()) {
    connect(this, &QPushButton::clicked, this, &LC_ColorButton::chooseColor);
    updateSwatch();
}

QColor LC_ColorButton::color() const {
    return m_color;
}

void LC_ColorButton::setColor(const QColor& color) {
    m_color = color;
    updateSwatch();
}


void LC_ColorButton::setLocked(bool locked) {
    m_locked = locked;

    // Symmetrical cursor feedback
    setCursor(locked ? Qt::ArrowCursor : Qt::PointingHandCursor);

    updateToolTip();
    updateSwatch();
}

void LC_ColorButton::setToolTip(const QString& text) {
    m_normalToolTip = text;
    updateToolTip();
}

void LC_ColorButton::setLockedToolTip(const QString& toolTip) {
    m_lockedToolTip = toolTip;
    updateToolTip();
}

void LC_ColorButton::setDialogTitle(const QString& title) {
    m_dialogTitle = title;
}

// Context-aware tooltip multiplexer
void LC_ColorButton::updateToolTip() {
    if (m_locked && !m_lockedToolTip.isEmpty()) {
        QPushButton::setToolTip(m_lockedToolTip);
    } else {
        QPushButton::setToolTip(m_normalToolTip);
    }
}


void LC_ColorButton::updateSwatch() {
    update();
}

void LC_ColorButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF rF = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // 1. Draw the background
    if (m_color.isValid()) {
        // Draw the solid colored swatch background
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_color);
        painter.drawRoundedRect(rF, 4.0, 4.0);
    } else {
        // Draw a neutral off-white background with a soft red diagonal slash representing unset/inherited
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#f4f4f4"));
        painter.drawRoundedRect(rF, 4.0, 4.0);

        painter.setPen(QPen(QColor("#d9534f"), 1.5, Qt::SolidLine, Qt::RoundCap)); // Soft red slash
        painter.drawLine(rF.topLeft() + QPointF(3, 3), rF.bottomRight() - QPointF(3, 3));
    }

    // 2. Draw the border outline based on the lock status
    if (m_locked) {
        // Dashed outline indicating the swatch is currently auto-calculated
        QPen borderPen(QColor("#aaaaaa"), 1, Qt::DashLine);
        painter.setPen(borderPen);
    } else {
        // Solid outline for manually specified colors
        QPen borderPen(QColor("#777777"), 1, Qt::SolidLine);
        painter.setPen(borderPen);
    }

    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rF, 4.0, 4.0);
}

void LC_ColorButton::chooseColor() {
    if (m_locked) {
        return; // Guard clause against clicks when locked
    }

    QColor initial = m_color.isValid() ? m_color : Qt::white;

    // Use the dynamic dialog title property if set, otherwise fallback to "Select Color"
    QString title = m_dialogTitle.isEmpty() ? tr("Select Color") : m_dialogTitle;

    const QColor col = QColorDialog::getColor(initial, this, title, QColorDialog::ShowAlphaChannel);
    if (col.isValid()) {
        setColor(col);
        emit colorChanged(col);
    }
}
