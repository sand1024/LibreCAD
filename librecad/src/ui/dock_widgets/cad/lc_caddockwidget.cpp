/*
 * **************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * *********************************************************************
 *
 */

// fixme - sand - add support of flex layout, with it potentially will be possible to support something ribbon-like
// oh - just have and options (hor/ver  orientation)

#include "lc_caddockwidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolButton>

#include "lc_action_group_manager.h"
#include "lc_action_node.h"
#include "lc_settings_widget.h"

void LC_CADDockWidget::setupUI() {
    setWidget(m_frame);
    doSetupGridLayout(m_gridLayout);
}

LC_CADDockWidget::LC_CADDockWidget(QWidget *parent)
    : LC_DockWidgetBase(parent, QString(), QString(), /*isCadDock=*/true),
      m_frame(new QFrame(this)),
      m_gridLayout(new QGridLayout) {

  m_frame->setContentsMargins(0, 0, 0, 0);
  setupUI();
  m_frame->setLayout(m_gridLayout);
}

void LC_CADDockWidget::addSpacers(QGridLayout *layout, const int columns, bool addHorizontal) {
  const auto verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum,
                                        QSizePolicy::Policy::Expanding);
  const int filledRows = layout->count() / columns;
  layout->addItem(verticalSpacer, filledRows + 1, 0, 1, 1);

    if (addHorizontal) {
        const auto hSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding,
       QSizePolicy::Policy::Minimum); layout->addItem(hSpacer, 0, columns,
       filledRows + 1, 1);
    }
}

void LC_CADDockWidget::addActions(const QList<QAction *> &list, int columns, const int iconSize, const bool flatButton) {
  onBeforeAddActions(); // Hook

  for (const auto&item : list) {
    if (!shouldCreateButtonForAction(item)) { // Hook
       handleIgnoredAction(item);             // Hook
       continue;
    }

    auto *toolButton = new QToolButton(this);
    toolButton->setDefaultAction(item);
    toolButton->setAutoRaise(flatButton);
    toolButton->setIconSize(QSize(iconSize, iconSize));

    toolButton->setFixedSize(QSize(iconSize + 8, iconSize + 8));

    configureButton(toolButton, item); // Hook

    const int count = m_gridLayout->count();
    if (columns == 0) {
       columns = 5;
    }

    m_gridLayout->addWidget(toolButton, count / columns, count % columns);
  }

  addSpacers(m_gridLayout, columns, m_addHorizontalSpacer);

  onLayoutUpdated(); // Hook

  m_frame->setFrameShadow(QFrame::Raised);
  m_frame->setLineWidth(2);
}

void LC_CADDockWidget::doSetupGridLayout(QGridLayout* newGridLayout) {

}

void LC_CADDockWidget::doUpdateWidgetSettings(int columnsCount, const int iconSize, const bool flatIcons) {
  const QSize size(iconSize, iconSize);

  QList<QToolButton *> widgets = m_frame->findChildren<QToolButton *>();

  auto *newGridLayout = new QGridLayout();
  doSetupGridLayout(newGridLayout);

  if (columnsCount == 0) {
    columnsCount = 5;
  }

  foreach (QToolButton *w, widgets) {
    w->setAutoRaise(flatIcons);
    w->setIconSize(size);

    w->setFixedSize(QSize(iconSize + 8, iconSize + 8));

    m_gridLayout->removeWidget(w);
    const int count = newGridLayout->count();
    newGridLayout->addWidget(w, count / columnsCount,
                             count % columnsCount);
  }
  delete m_frame->layout();

  addSpacers(newGridLayout, columnsCount,m_addHorizontalSpacer);
  m_frame->setLayout(newGridLayout);
  m_gridLayout = newGridLayout;

  m_columns = columnsCount;
  m_iconSize = iconSize;

  onLayoutUpdated(); // Hook

  updateMinimumWidth();
}

void LC_CADDockWidget::updateWidgetSettings() {
    int columnsCount = 0;
    int iconSize = 0;
    bool flatIcons = false;

    getMetrics(columnsCount, iconSize, flatIcons);
    doUpdateWidgetSettings(columnsCount, iconSize, flatIcons);
}

void LC_CADDockWidget::updateMinimumWidth() {
    if (!m_scrollArea) return;

    // Mathematically calculate the exact horizontal space required by the grid
    const int contentWidth = m_columns * (m_iconSize + 8);

    // Dynamically check if the vertical scrollbar is currently active (maximum > 0)
    int sbWidth = 0;
    if (m_scrollArea->verticalScrollBar()->maximum() > 0) {
        sbWidth = m_scrollArea->verticalScrollBar()->sizeHint().width();
        if (sbWidth <= 0) {
            sbWidth = 16;
        }
    }

    // Account for m_frame's Raised border shadows
    const int framePadding = 2 * m_frame->lineWidth();

    const int totalMinWidth = contentWidth + sbWidth + framePadding;

    m_scrollArea->setMinimumWidth(totalMinWidth);
    setMinimumWidth(totalMinWidth);

    updateGeometry();
}

QSize LC_CADDockWidget::minimumSizeHint() const {
    QSize baseHint = QDockWidget::minimumSizeHint();
    if (m_scrollArea != nullptr) {
        const int minW = m_scrollArea->minimumWidth();
        if (minW > 0) {
            baseHint.setWidth(minW);
        }
    }
    return baseHint;
}

void LC_CADDockWidget::updateActionsFromNodes(const QList<ActionNode>& nodes, LC_ActionGroupManager* agm) {
    if (agm == nullptr) {
        return;
    }

    QList<QAction*> actions;
    for (const auto& node : nodes) {
        if (node.type == ActionNodeType::Separator) {
            auto* sep = new QAction(this);
            sep->setSeparator(true);
            actions.append(sep);
        } else if (node.type == ActionNodeType::Action) {
            auto* act = agm->getActionByName(node.actionName);
            if (act != nullptr) {
                actions.append(act);
            }
        }
    }

    int cols = 0;
    int sz = 0;
    bool flat = false;
    getMetrics(cols, sz, flat);

    clear();
    addActions(actions, cols, sz, flat);
}

void LC_CADDockWidget::getMetrics(int& cols, int& sz, bool& flat) const {
    using namespace CFG_Widgets;
    cols = o_CADDockWidgetColumnsCount;
    sz = o_CADDockWidgetIconSize;
    flat = o_CADDockWidgetFlatButtons;
}
void LC_CADDockWidget::clear() {
    if (m_frame == nullptr) {
        return;
    }

    // Remove and delete all child tool buttons
    const QList<QToolButton*> buttons = m_frame->findChildren<QToolButton*>();
    for (auto* btn : buttons) {
        if (btn != nullptr) {
            if (m_gridLayout != nullptr) {
                m_gridLayout->removeWidget(btn);
            }
            btn->deleteLater();
        }
    }

    // Delete existing layout and recreate fresh grid
    if (m_frame->layout() != nullptr) {
        const QLayoutItem* item = nullptr;
        while ((item = m_gridLayout->takeAt(0)) != nullptr) {
            delete item;
        }
        delete m_frame->layout();
    }

    m_gridLayout = new QGridLayout();
    doSetupGridLayout(m_gridLayout);
    m_frame->setLayout(m_gridLayout);
}

void LC_CADDockWidget::onBeforeAddActions() {}
bool LC_CADDockWidget::shouldCreateButtonForAction([[maybe_unused]]QAction* action) const { return true; }
void LC_CADDockWidget::handleIgnoredAction([[maybe_unused]]QAction* action) {}
void LC_CADDockWidget::configureButton([[maybe_unused]]QToolButton* toolButton, [[maybe_unused]]QAction* action) {}
void LC_CADDockWidget::onLayoutUpdated() {}


bool LC_CADDockWidget::isCADDockWidget(const QWidget* dw) {
    const bool isCad = dw->property(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET).toBool() || dw->inherits("LC_CADDockWidget");
    return isCad;
}
