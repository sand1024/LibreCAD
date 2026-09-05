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

#include "lc_highlight_overlay.h"

#include "lc_palette_color_utils.h"
#include "lc_settings_colors_semantics.h"

LC_HighlightOverlay::LC_HighlightOverlay(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    if (parent) {
        resize(parent->size());
        parent->installEventFilter(this);
    }
    show();
}

bool LC_HighlightOverlay::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::Resize) {
        if (const auto* parentWidget = qobject_cast<QWidget*>(obj)) {
            resize(parentWidget->size());
        }
    }
    return QWidget::eventFilter(obj, event);
}

void LC_HighlightOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // auto highlightColor = parentWidget()->palette().color(QPalette::Accent);
    // Amber Focus boundary border matching the visual IDE schema
    // QPen pen(QColor(189, 99, 19, 180), 1.5); // fixme - sand - customize color?? or at least use constant
    constexpr qreal highlightWidth = 1.5;// fixme - sand - customize color?? or at least use constant
    const QColor highlightColor = LC_PaletteColorUtils::getSemanticColor(
        LC_SemanticColors::SearchResultItem,
        (parentWidget() != nullptr) ? parentWidget()->palette() : QApplication::palette()
    );


    const QPen pen(QColor(highlightColor), highlightWidth);
    painter.setPen(pen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
}
