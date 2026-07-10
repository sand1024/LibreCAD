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


#include "lc_cad_tool_matrix_dock_widget.h"

#include <QApplication>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QPainter>
#include <QToolButton>
#include <QProxyStyle>

#include "lc_proxy_style.h"

class LC_ProxyStyle;

LC_CADToolMatrixDockWidget::LC_CADToolMatrixDockWidget(QWidget* parent, bool scrollContent)
    : LC_CADDockWidget(parent, scrollContent) {

    if (m_frame) {
        m_frame->installEventFilter(this); // Intercept m_frame paint events
    }
    if (m_gridLayout) {
        m_gridLayout->setContentsMargins(1, 1, 1, 1);
    }
    m_addHorizontalSpacer = true;
}

bool LC_CADToolMatrixDockWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_frame && event->type() == QEvent::Paint) {
        auto *style = qobject_cast<const LC_ProxyStyle*>(QApplication::style());
        if (style && style->useSegmentedToolButtons()) { // <--- Safe Outer Check
            QPainter painter(m_frame);
            style->drawSegmentedGroupBackdrops(&painter, m_frame);
        }
    }
    return LC_CADDockWidget::eventFilter(watched, event);
}

void LC_CADToolMatrixDockWidget::changeEvent(QEvent* event) {
    LC_CADDockWidget::changeEvent(event);

    // Trigger dynamic color precomputations as soon as a new theme is applied
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange) {
        updateSegmentedButtonsMask();
    }
}

void LC_CADToolMatrixDockWidget::onBeforeAddActions() {
    m_currentGroupId = 1;
}

bool LC_CADToolMatrixDockWidget::shouldCreateButtonForAction(QAction* action) const {
    if (!action) return true;
    return !action->isSeparator(); // Prevent physical toolbutton creation for separators
}

void LC_CADToolMatrixDockWidget::handleIgnoredAction(QAction* action) {
    if (action && action->isSeparator()) {
        m_currentGroupId++; // Boundary encountered: increment local group partition
    }
}

void LC_CADToolMatrixDockWidget::configureButton(QToolButton* toolButton, QAction* action) {
    Q_UNUSED(action);
    if (toolButton) {
        toolButton->setProperty("buttonGroup", m_currentGroupId); // Assign group ID
    }
}

void LC_CADToolMatrixDockWidget::onLayoutUpdated() {
    updateSegmentedButtonsMask();
}

void LC_CADToolMatrixDockWidget::doSetupGridLayout(QGridLayout* newGridLayout) {
    newGridLayout->setSpacing(0);
    newGridLayout->setContentsMargins(1, 1, 1, 1);

}

void LC_CADToolMatrixDockWidget::updateSegmentedButtonsMask() const {
    if (!m_gridLayout) return;

    QMap<QPair<int, int>, QToolButton*> gridMap;
    // 1. Scan and map all grid layout toolbuttons to row/column coordinate keys
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        QLayoutItem *item = m_gridLayout->itemAt(i);
        if (!item) continue;
        if (auto *btn = qobject_cast<QToolButton*>(item->widget())) {
            int r, c, rSpan, cSpan;
            m_gridLayout->getItemPosition(i, &r, &c, &rSpan, &cSpan);
            gridMap.insert(qMakePair(r, c), btn);
        }
    }

    QSet<int> uniqueGroups;
    for (auto *btn : gridMap) {
        QVariant g = btn->property("buttonGroup");
        if (g.isValid()) {
            uniqueGroups.insert(g.toInt());
        }
    }
    int totalGroups = uniqueGroups.size();

    auto *style = qobject_cast<const LC_ProxyStyle*>(QApplication::style());
    if (style) {
        style->precomputeSegmentedGroupColors(m_frame, totalGroups);
    }

    // 2. Loop through mapped toolbuttons and compute neighboring adjacency
    for (auto it = gridMap.begin(); it != gridMap.end(); ++it) {
        int r = it.key().first;
        int c = it.key().second;
        QToolButton *btn = it.value();

        QVariant groupVal = btn->property("buttonGroup");
        if (!groupVal.isValid()) {
            btn->setProperty("groupNeighbors", QVariant());
            continue;
        }

        // Base checks (Any physical button neighbor present)
        bool hasT = gridMap.contains(qMakePair(r - 1, c));
        bool hasB = gridMap.contains(qMakePair(r + 1, c));
        bool hasL = gridMap.contains(qMakePair(r, c - 1));
        bool hasR = gridMap.contains(qMakePair(r, c + 1));

        // Group-matching checks
        bool sameT = hasT && (gridMap.value(qMakePair(r - 1, c))->property("buttonGroup") == groupVal);
        bool sameB = hasB && (gridMap.value(qMakePair(r + 1, c))->property("buttonGroup") == groupVal);
        bool sameL = hasL && (gridMap.value(qMakePair(r, c - 1))->property("buttonGroup") == groupVal);
        bool sameR = hasR && (gridMap.value(qMakePair(r, c + 1))->property("buttonGroup") == groupVal);

        int mask = 0;

        // Bits 0-3: Same group adjacency
        if (sameT) mask |= 0x01;
        if (sameB) mask |= 0x02;
        if (sameL) mask |= 0x04;
        if (sameR) mask |= 0x08;

        // Bits 4-7: Physical adjacency (any group)
        if (hasT) mask |= 0x10;
        if (hasB) mask |= 0x20;
        if (hasL) mask |= 0x40;
        if (hasR) mask |= 0x80;

        btn->setProperty("groupNeighbors", mask);
    }
}
