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

#include "lc_dock_widget_base.h"

#include <QAction>
#include <QMainWindow>
#include <QScreen>
#include <QTabBar>
#include <QTimer>

LC_DockWidgetBase::LC_DockWidgetBase(QWidget* parent,
                                     const QString& title,
                                     const QString& verticalTitle,
                                     bool isCadDock,
                                     const Qt::WindowFlags& flags)
    : QDockWidget(title, parent, flags)
    , m_realTitle(title)
    , m_verticalTitle(verticalTitle.isEmpty() ? title : verticalTitle)
    , m_isCadDock(isCadDock) {
    connect(this, &QDockWidget::topLevelChanged, this, &LC_DockWidgetBase::onTopLevelChanged);
    connect(this, &QDockWidget::dockLocationChanged, this, [this](Qt::DockWidgetArea area) {
      if (area != Qt::NoDockWidgetArea) {
          m_lastDockArea = area;
      }
  });
    QAction* toggleAct = toggleViewAction();
    if (toggleAct != nullptr) {
        // 2. Disconnect Qt's default _q_toggleView(bool) slot
        toggleAct->disconnect(this);

        // 3. Connect to our 3-state tab-aware toggle logic
        connect(toggleAct, &QAction::triggered, this, &LC_DockWidgetBase::toggleDockVisibility);
    }
}

void LC_DockWidgetBase::showEvent(QShowEvent* event) {
    QDockWidget::showEvent(event);

    if (widget() != nullptr && widget()->isHidden()) {
        widget()->show();
    }
    if (isFloating() && titleBarWidget() != nullptr && titleBarWidget()->isHidden()) {
        titleBarWidget()->show();
    }
}

QString LC_DockWidgetBase::realTitle() const {
    return m_realTitle;
}

void LC_DockWidgetBase::setRealTitle(const QString& title) {
    m_realTitle = title;
    if (!m_iconOnlyTabMode || isFloating()) {
        QDockWidget::setWindowTitle(title);
    }
}

QString LC_DockWidgetBase::verticalTitle() const {
    return m_verticalTitle;
}

void LC_DockWidgetBase::setVerticalTitle(const QString& title) {
    m_verticalTitle = title;
}

void LC_DockWidgetBase::setWindowTitle(const QString& title) {
    if (!title.isEmpty()) {
        m_realTitle = title;
    }
    QDockWidget::setWindowTitle(title);
}

bool LC_DockWidgetBase::isCadDock() const {
    return m_isCadDock;
}

void LC_DockWidgetBase::setIsCadDock(const bool isCad) {
    m_isCadDock = isCad;
}

bool LC_DockWidgetBase::isIconOnlyTabMode() const {
    return m_iconOnlyTabMode;
}

bool LC_DockWidgetBase::isTabActive() const {
    if (!isVisible() || isFloating()) {
        return isVisible();
    }

    auto* mw = qobject_cast<QMainWindow*>(parentWidget());
    if (mw == nullptr || mw->tabifiedDockWidgets(const_cast<LC_DockWidgetBase*>(this)).isEmpty()) {
        return isVisible(); // Standalone dock: if it's visible, it's active
    }

    // Check if the hosting QTabBar has this dock selected as its active index
    const auto tabBars = mw->findChildren<QTabBar*>();
    for (const auto* bar : tabBars) {
        if (bar == nullptr || !bar->isVisible()) {
            continue;
        }
        const int idx = bar->currentIndex();
        if (idx >= 0 && idx < bar->count()) {
            const quintptr currentDockPtr = bar->tabData(idx).toULongLong();
            if (currentDockPtr == reinterpret_cast<quintptr>(this)) {
                return true;
            }
            if (currentDockPtr == 0 && (bar->tabToolTip(idx) == m_realTitle || bar->tabText(idx) == m_realTitle)) {
                return true;
            }
        }
    }

    return false;
}

bool LC_DockWidgetBase::activateDockTab() {
    auto* mw = qobject_cast<QMainWindow*>(parentWidget());
    if (mw == nullptr) {
        return false;
    }

    const auto tabBars = mw->findChildren<QTabBar*>();
    for (auto* bar : tabBars) {
        if (bar == nullptr) {
            continue;
        }

        const int count = bar->count();
        for (int i = 0; i < count; ++i) {
            const quintptr dockPtr = bar->tabData(i).toULongLong();
            const bool matchesPtr = (dockPtr == reinterpret_cast<quintptr>(this));
            const bool matchesTitle = (dockPtr == 0 && (bar->tabToolTip(i) == m_realTitle || bar->tabText(i) == m_realTitle));

            if (matchesPtr || matchesTitle) {
                // Programmatically switch the active tab on the QTabBar
                bar->setCurrentIndex(i);
                show();
                raise();
                return true;
            }
        }
    }

    return false;
}

void LC_DockWidgetBase::floatWithOffset() {
    if (isFloating()) {
        setFloating(false);
        return;
    }

    auto* mw = qobject_cast<QMainWindow*>(parentWidget());
    if (mw != nullptr) {
        const Qt::DockWidgetArea currentArea = mw->dockWidgetArea(this);
        if (currentArea != Qt::NoDockWidgetArea) {
            m_lastDockArea = currentArea;
        }
    }

    setFloating(true);

    constexpr int delta = 32;
    QPoint offset(0, 0);

    switch (m_lastDockArea) {
        case Qt::LeftDockWidgetArea:
            offset = QPoint(delta, delta / 2); // Shift right & slightly down
            break;
        case Qt::RightDockWidgetArea:
            offset = QPoint(-delta, delta / 2); // Shift left & slightly down
            break;
        case Qt::TopDockWidgetArea:
            offset = QPoint(0, delta); // Shift down into canvas
            break;
        case Qt::BottomDockWidgetArea:
            offset = QPoint(0, -delta); // Shift up into canvas
            break;
        default:
            offset = QPoint(delta, delta);
            break;
    }

    // Apply move asynchronously so the OS window manager finishes detaching
    QTimer::singleShot(0, this, [this, offset]() {
        QPoint targetPos = pos() + offset;

        // Ensure window titlebar stays fully inside available screen boundaries
        if (screen() != nullptr) {
            const QRect availGeo = screen()->availableGeometry();
            const QRect frameGeo = frameGeometry();
            targetPos.setX(std::clamp(targetPos.x(), availGeo.left(), availGeo.right() - frameGeo.width()));
            targetPos.setY(std::clamp(targetPos.y(), availGeo.top(), availGeo.bottom() - frameGeo.height()));
        }

        move(targetPos);
    });
}

void LC_DockWidgetBase::toggleDockVisibility() {
    auto* mw = qobject_cast<QMainWindow*>(parentWidget());
    const bool isTabbed = (mw != nullptr) && !mw->tabifiedDockWidgets(this).isEmpty();

    // 1. If not visible: show, activate its tab, and apply focus target
    if (!isVisible()) {
        show();
        if (isTabbed) {
            activateDockTab();
        }
        else {
            raise();
        }

        if (m_focusTargetWidget != nullptr) {
            m_focusTargetWidget->setFocus();
        }

        if (toggleViewAction() != nullptr) {
            toggleViewAction()->setChecked(true);
        }
        return;
    }

    // 2. If visible but in an inactive tab: activate its tab (keep it open and checked)
    if (isTabbed && !isTabActive()) {
        activateDockTab();
        if (m_focusTargetWidget != nullptr) {
            m_focusTargetWidget->setFocus();
        }

        if (toggleViewAction() != nullptr) {
            toggleViewAction()->setChecked(true);
        }
        return;
    }

    // 3. If visible and currently active tab (or standalone): hide it (close)
    hide();
    if (toggleViewAction() != nullptr) {
        toggleViewAction()->setChecked(false);
    }
}

void LC_DockWidgetBase::setIconOnlyTabMode(const bool iconOnly) {
    m_iconOnlyTabMode = iconOnly;

    if (isFloating()) {
        if (windowTitle() != m_realTitle) {
            QDockWidget::setWindowTitle(m_realTitle);
        }
        return;
    }

    QAction* toggleAct = toggleViewAction();
    QString savedActionText;
    if (toggleAct != nullptr) {
        savedActionText = toggleAct->text();
    }

    if (iconOnly) {
        if (!windowTitle().isEmpty()) {
            QDockWidget::setWindowTitle(QString());
        }
    }
    else {
        if (windowTitle() != m_realTitle) {
            QDockWidget::setWindowTitle(m_realTitle);
        }
    }

    if (toggleAct != nullptr && !savedActionText.isEmpty() && toggleAct->text() != savedActionText) {
        toggleAct->setText(savedActionText);
    }
}

void LC_DockWidgetBase::onTopLevelChanged(const bool floating) {
    if (floating) {
        if (windowTitle() != m_realTitle) {
            QDockWidget::setWindowTitle(m_realTitle);
        }
        if (titleBarWidget() != nullptr && titleBarWidget()->isHidden()) {
            titleBarWidget()->show();
        }
        if (widget() != nullptr && widget()->isHidden()) {
            widget()->show();
        }
    }
    else {
        setIconOnlyTabMode(m_iconOnlyTabMode);
        if (widget() != nullptr && widget()->isHidden()) {
            widget()->show();
        }
    }
}

void LC_DockWidgetBase::setFocusTargetWidget(QWidget* target) {
    m_focusTargetWidget = target;
}

QWidget* LC_DockWidgetBase::focusTargetWidget() const {
    return m_focusTargetWidget.data();
}
