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
#include "lc_placeholder_tree_view.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>

LC_PlaceholderTreeView::LC_PlaceholderTreeView(QWidget* parent)
    : QTreeView(parent)
    , m_placeholderText(tr("No items to show")) {
}

void LC_PlaceholderTreeView::setPlaceholderText(const QString& text) {
    if (m_placeholderText != text) {
        m_placeholderText = text;
        viewport()->update();
    }
}

void LC_PlaceholderTreeView::paintEvent(QPaintEvent* event) {
    // Let the base QTreeView paint rows if they exist
    QTreeView::paintEvent(event);

    // If the model has 0 rows, draw the custom placeholder
    if (model() && model()->rowCount() == 0 && !m_placeholderText.isEmpty()) {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        // Theme-Safe Color extraction:
        // Automatically adapts to both Dark and Light desktop themes
        QColor placeholderColor = palette().color(QPalette::PlaceholderText);
        if (!placeholderColor.isValid()) {
            placeholderColor = palette().color(QPalette::Text);
            placeholderColor.setAlpha(128); // 50% opacity fallback
        }
        painter.setPen(placeholderColor);

        // Center-align the fully localized, custom placeholder text
        painter.drawText(viewport()->rect(), Qt::AlignCenter, m_placeholderText);
    }
}
