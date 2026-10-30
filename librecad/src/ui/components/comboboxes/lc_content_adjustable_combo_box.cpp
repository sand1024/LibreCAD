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

#include "lc_content_adjustable_combo_box.h"

#include <QStyle>
#include <QStyleOptionComboBox>

LC_ContentAdjustableComboBox::LC_ContentAdjustableComboBox(QWidget* parent)
    : QComboBox(parent) {
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QSize LC_ContentAdjustableComboBox::sizeHint() const {
    QSize hint = QComboBox::sizeHint();
    if (count() == 0) {
        return hint;
    }

    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    // Get the actual rect of the drop-down button
    QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);

    // Add extra margin for the delegate's internal margins + text spacing
    const int extraPadding = arrowRect.isValid() ? 8 : 20;

    hint.rwidth() += extraPadding;
    return hint;
}

QSize LC_ContentAdjustableComboBox::minimumSizeHint() const {
    // Prevent the layout manager from compressing the widget below the required sizeHint
    return sizeHint();
}
