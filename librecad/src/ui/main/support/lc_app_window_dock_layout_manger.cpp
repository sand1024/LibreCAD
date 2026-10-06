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

#include "lc_app_window_dock_layout_manger.h"

#include <QDockWidget>
#include <QMdiArea>
#include <QMenuBar>
#include <QScopedValueRollback>
#include <QScreen>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
#include <QToolBar>
#include <QWindow>

#include "lc_dock_names.h"
#include "lc_dock_tab_bar_manager.h"
#include "lc_namedviewslistwidget.h"
#include "lc_widget_factory.h"
#include "qc_applicationwindow.h"

LC_AppWindowDockLayoutManger::LC_AppWindowDockLayoutManger(QC_ApplicationWindow* appWindow) : m_appWin(appWindow) {}

// code is base on the original implementation that was added to QC_ApplicationWindow.
// however, that implementation has two serious defects:
// 1) widgets that are floated at the moment of closing the application, were restored unproperly - just as a fully transparent
// windows (at least under Win). That was fixed by shifting restoring them to later stage when main window is created and visible.
// 2) trial-collapse layout reflow under specific conditions leads to non-finishing infinite cycles (trying to arrange widgets
// when unable to satisfy conditions. Layouting was simplified.

namespace LayoutConstants {
    constexpr int COLLAPSE_LEFT_THRESHOLD = 900;
    constexpr int RESTORE_LEFT_THRESHOLD  = 1050;
    constexpr int COLLAPSE_RIGHT_THRESHOLD = 600;
    constexpr int RESTORE_RIGHT_THRESHOLD  = 750;
    constexpr int MINIMUM_DOCK_FRAME_SIZE = 32;
    constexpr int TOOLBAR_BUDGET_PADDING  = 48;
}

void LC_AppWindowDockLayoutManger::initializeDockAreas() {
    m_dockAreasToggleActions.left = m_appWin->getAction(QStringLiteral("LeftDockAreaToggle"));
    m_dockAreasToggleActions.right = m_appWin->getAction(QStringLiteral("RightDockAreaToggle"));
    m_dockAreasToggleActions.top = m_appWin->getAction(QStringLiteral("TopDockAreaToggle"));
    m_dockAreasToggleActions.bottom = m_appWin->getAction(QStringLiteral("BottomDockAreaToggle"));
    m_dockAreasToggleActions.floating = m_appWin->getAction(QStringLiteral("FloatingDockwidgetsToggle"));

}

void LC_AppWindowDockLayoutManger::initializeDockLayout() {
    if (m_dockLayoutInitialized) {
        return;
    }

    m_dockLayoutInitialized = true;

    for (Qt::DockWidgetArea area : {Qt::LeftDockWidgetArea, Qt::RightDockWidgetArea, Qt::TopDockWidgetArea, Qt::BottomDockWidgetArea}) {
        m_requestedDockAreas.insert(int(area), true);
    }
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        const QString name = dock->objectName();
        if (name.isEmpty()) {
            continue;
        }
        m_factoryDockVisibility.insert(name, !dock->isHidden());
        m_requestedDockVisibility.insert(name, !dock->isHidden());
        dock->installEventFilter(this);
        connect(dock->toggleViewAction(), &QAction::triggered, this, [this, dock](bool checked) {
            if (m_dockLayoutApplying) {
                return;
            }
            m_requestedDockVisibility[dock->objectName()] = checked;
            if (checked) {
                requestDockVisible(dock);
            }
            else {
                if (m_priorityDockName == dock->objectName()) {
                    m_priorityDockName.clear();
                }
                scheduleDockFit();
            }
        });
        connect(dock, &QDockWidget::visibilityChanged, this, [this](bool) {
             if (!m_dockLayoutApplying && !m_dockFitRunning) {
                 scheduleDockFit();
             }
         });
        connect(dock, &QDockWidget::dockLocationChanged, this, [this, dock](Qt::DockWidgetArea area) {
            if (!m_dockLayoutApplying && !m_dockFitRunning && !dock->isHidden() && area != Qt::NoDockWidgetArea) {
                m_requestedDockVisibility[dock->objectName()] = true;

                if (!dockAreaRequested(area)) {
                    // Behavior 2: If docked into a closed area, re-open the area and restore its previous contents
                    setDockAreaRequested(area, true);
                    dock->raise();
                }
                else {
                    scheduleDockFit();
                }
            }
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this, dock](bool floating) {
            if (!m_dockLayoutApplying && !m_dockFitRunning) {
                if (floating) {
                    m_floatingDocksRequested = true;
                    QTimer::singleShot(0, dock, [this, dock] {
                        if (QWindow* handle = dock->windowHandle(); handle != nullptr && !handle->property("_lc_screen_watched").toBool()) {
                            handle->setProperty("_lc_screen_watched", true);
                            connect(handle, &QWindow::screenChanged, this, [this](QScreen*) {
                                scheduleDockFit();
                            });
                        }
                        scheduleDockFit();
                    });
                }
                scheduleDockFit();
            }
        });
    }

    auto statusBar = m_appWin->statusBar();
    if (statusBar != nullptr) {
        statusBar->installEventFilter(this);
    }
    auto menuBar = m_appWin->menuBar();
    if (menuBar) {
        menuBar->installEventFilter(this);
    }
    m_appWin->mdiAreaCAD()->installEventFilter(this);

    connect(m_appWin, &QMainWindow::tabifiedDockWidgetActivated, this, [this](QDockWidget* dock) {
        if (dock && !m_dockLayoutApplying && !m_dockFitRunning) {
            m_selectedTabs.insert(dockGroupKey(dock), dock->objectName());
        }
    });
    updateDockAreaActions();
}

void LC_AppWindowDockLayoutManger::prepareWindowForShow() {
    auto layout = m_appWin->layout();
    if (layout != nullptr) {
        layout->setSizeConstraint(QLayout::SetNoConstraint);
    }
    m_appWin->setMinimumSize(0, 0);

    auto mdiAreaCad = m_appWin->mdiAreaCAD();
    if (mdiAreaCad != nullptr) {
        mdiAreaCad->setMinimumSize(0, 0);
    }

    QScreen* screen = QGuiApplication::screenAt(m_appWin->frameGeometry().center());
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen != nullptr) {
        const QSize avail = screen->availableGeometry().size();
        m_appWin->getToolBarManager()->reflowToolBars(avail.width(), avail.height());
    }
    clampWindowToScreen(m_appWin);
}


QMap<QString, bool> LC_AppWindowDockLayoutManger::requestedDockVisibility() const {
    return m_requestedDockVisibility;
}

bool LC_AppWindowDockLayoutManger::dockAreaRequested(Qt::DockWidgetArea area) const {
    return m_requestedDockAreas.value(int(area), true);
}

bool LC_AppWindowDockLayoutManger::floatingDocksRequested() const {
    return m_floatingDocksRequested;
}

void LC_AppWindowDockLayoutManger::restoreDockLayout(const QMap<QString, bool>& requested, bool hasRequested, const QHash<int, bool>& areas,
                                                     const QByteArray& state) {
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);
    m_fittedDockState.clear();
    m_selectedTabs.clear();
    m_priorityDockName.clear();
    m_requestedDockVisibility.clear();
    m_pendingFloatingDocks.clear();
    m_pendingFloatingPos.clear();
    m_pendingFloatingSize.clear();
    m_autoCollapsedLeftArea = false;
    m_autoCollapsedRightArea = false;

    const bool stateRestored = !state.isEmpty() && m_appWin->restoreState(state);

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock == nullptr) {
            continue;
        }
        const QString name = dock->objectName();
        if (name.isEmpty()) {
            continue;
        }

        // If restored as floating while the main window is still hidden, defer floating until showEvent
        if (dock->isFloating()) {
            m_pendingFloatingDocks.insert(name);
            m_pendingFloatingPos.insert(name, dock->pos());
            m_pendingFloatingSize.insert(name, dock->size());
            dock->setFloating(false);
        }

        const bool factoryDefault = m_factoryDockVisibility.value(name, !dock->isHidden());
        const auto area = m_appWin->dockWidgetArea(dock);
        const bool visible = hasRequested
                                 ? requested.value(name, factoryDefault)
                                 : stateRestored && areas.value(int(area), true)
                                 ? !dock->isHidden()
                                 : factoryDefault;
        m_requestedDockVisibility.insert(name, visible);
    }
    for (auto it = areas.cbegin(); it != areas.cend(); ++it) {
        m_requestedDockAreas.insert(it.key(), it.value());
    }
    m_floatingDocksRequested = areas.value(int(Qt::NoDockWidgetArea), true);
    applyRequestedDockVisibility();
    updateDockAreaActions();

    // If the main window is already visible (e.g. workspace switch during active session),
    // float pending docks immediately since showEvent will not fire again.
    if (m_appWin != nullptr && m_appWin->isVisible()) {
        applyPendingFloatingDocks();
    }

    scheduleDockFit();
}

void LC_AppWindowDockLayoutManger::setDockAreaRequested(Qt::DockWidgetArea area, bool state) {
    if (m_appWin == nullptr) {
        return;
    }

    // Reset automatic collapse flags if the user explicitly interacted with area toggles
    if (area == Qt::LeftDockWidgetArea) {
        m_autoCollapsedLeftArea = false;
    }
    else if (area == Qt::RightDockWidgetArea) {
        m_autoCollapsedRightArea = false;
    }

    if (!state) {
        // 1. Snapshot open docks (tabified or standalone) and currently active tab before disabling
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (dock != nullptr && !dock->isFloating() && m_appWin->dockWidgetArea(dock) == area) {
                const QString name = dock->objectName();
                // A dock in this area is open if it is either the active tab (!isHidden) or an open tab in the stack (!tabifiedDockWidgets.isEmpty)
                const bool isDockOpen = !dock->isHidden() || !m_appWin->tabifiedDockWidgets(dock).isEmpty();
                m_preCloseDockVisibility[name] = isDockOpen;

                if (!dock->isHidden()) {
                    m_preCloseActiveDock[int(area)] = name;
                }
            }
        }

        if (!m_priorityDockName.isEmpty()) {
            if (QDockWidget* priority = m_appWin->findChild<QDockWidget*>(m_priorityDockName); priority != nullptr && m_appWin->dockWidgetArea(priority) == area) {
                m_priorityDockName.clear();
            }
        }
    }
    else {
        // 2. When enabling the area, restore only the docks that were open in the snapshot
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (dock != nullptr && !dock->isFloating() && m_appWin->dockWidgetArea(dock) == area) {
                const QString name = dock->objectName();
                if (m_preCloseDockVisibility.contains(name)) {
                    m_requestedDockVisibility[name] = m_preCloseDockVisibility.value(name);
                }
            }
        }
    }

    m_requestedDockAreas[int(area)] = state;
    applyRequestedDockVisibility();

    // 3. Re-raise the tab that was active before the area was closed
    if (state && m_preCloseActiveDock.contains(int(area))) {
        const QString activeName = m_preCloseActiveDock.value(int(area));
        QDockWidget* activeDock = m_appWin->findChild<QDockWidget*>(activeName);
        if (activeDock != nullptr && !activeDock->isHidden()) {
            activeDock->raise();
        }
    }

    updateDockAreaActions();
    scheduleDockFit();
}

void LC_AppWindowDockLayoutManger::toggleLeftDockArea(bool state) {
    setDockAreaRequested(Qt::LeftDockWidgetArea, state);
}

void LC_AppWindowDockLayoutManger::toggleRightDockArea(bool state) {
    setDockAreaRequested(Qt::RightDockWidgetArea, state);
}

void LC_AppWindowDockLayoutManger::toggleTopDockArea(bool state) {
    setDockAreaRequested(Qt::TopDockWidgetArea, state);
}

void LC_AppWindowDockLayoutManger::toggleBottomDockArea(bool state) {
    setDockAreaRequested(Qt::BottomDockWidgetArea, state);
}

void LC_AppWindowDockLayoutManger::toggleFloatingDockwidgets(bool state) {
    m_floatingDocksRequested = state;
    applyRequestedDockVisibility();
    updateDockAreaActions();
    scheduleDockFit();
}

void LC_AppWindowDockLayoutManger::requestDockVisible(QDockWidget* dock) {
    if (dock == nullptr) {
        return;
    }

    const QString dockName = dock->objectName();
    m_requestedDockVisibility[dockName] = true;
    m_priorityDockName = dockName;

    QString cadMatrixName = LC_DockNames::cadDockName(LC_DockNames::CAD_MEGA);

    // Enforce Option 3: Mutual exclusivity between CAD Tools Matrix and individual CAD category docks
    if (dockName == cadMatrixName) {
        for (QDockWidget* d : m_appWin->findChildren<QDockWidget*>()) {
            if (d != nullptr && d != dock && !d->isFloating() && m_appWin->dockWidgetArea(d) == Qt::LeftDockWidgetArea && d->objectName().
                startsWith(QLatin1String(LC_DockNames::PREFIX_DOCK_CAD))) {
                m_requestedDockVisibility[d->objectName()] = false;
            }
        }
    }
    else if (dockName.startsWith(QLatin1String(LC_DockNames::PREFIX_DOCK_CAD)) && !dock->isFloating() && m_appWin->dockWidgetArea(dock) ==
        Qt::LeftDockWidgetArea) {
        m_requestedDockVisibility[cadMatrixName] = false;
    }

    if (dock->isFloating()) {
        m_floatingDocksRequested = true;
    }
    else {
        const auto area = m_appWin->dockWidgetArea(dock);
        m_requestedDockAreas[int(area)] = true;
        if (area == Qt::LeftDockWidgetArea) {
            m_autoCollapsedLeftArea = false;
        }
        else if (area == Qt::RightDockWidgetArea) {
            m_autoCollapsedRightArea = false;
        }
    }
    applyRequestedDockVisibility();
    dock->raise();
    updateDockAreaActions();
    scheduleDockFit();
}

QString LC_AppWindowDockLayoutManger::dockGroupKey(QDockWidget* dock) const {
    if (dock == nullptr) {
        return QString();
    }
    QStringList names{dock->objectName()};
    if (!dock->isFloating()) {
        for (QDockWidget* peer : m_appWin->tabifiedDockWidgets(dock)) {
            if (peer != nullptr) {
                names.append(peer->objectName());
            }
        }
    }
    names.sort();
    return names.join(QLatin1Char('|'));
}



QByteArray LC_AppWindowDockLayoutManger::dockLayoutStateForSaving() {
    if (m_appWin == nullptr) {
        return QByteArray();
    }
    return m_appWin->saveState();
}

bool LC_AppWindowDockLayoutManger::eventFilter(QObject* watched, QEvent* event) {
    if (processEvent(watched, event)) {
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void LC_AppWindowDockLayoutManger::applyRequestedDockVisibility() {
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock == nullptr) {
            continue;
        }
        const QString dockObjectName = dock->objectName();
        if (dockObjectName.isEmpty()) {
            continue;
        }
        const bool areaEnabled = dock->isFloating() ? m_floatingDocksRequested : dockAreaRequested(m_appWin->dockWidgetArea(dock));
        const bool visible = m_requestedDockVisibility.value(dockObjectName, m_factoryDockVisibility.value(dockObjectName, true)) && areaEnabled;

        dock->setVisible(visible);
    }
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && !dock->isHidden() && m_selectedTabs.value(dockGroupKey(dock)) == dock->objectName()) {
            dock->raise();
        }
    }
}

void LC_AppWindowDockLayoutManger::updateDockAreaActions() {
    if (m_appWin == nullptr) {
        return;
    }

 const auto setChecked = [](QAction* action, bool checked) {
        if (action != nullptr) {
        const QSignalBlocker blocker(action);
        action->setChecked(checked);
        }
    };
    const auto anyShown = [this](Qt::DockWidgetArea area) {
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (dock != nullptr && !dock->isFloating() && m_appWin->dockWidgetArea(dock) == area && !dock->isHidden()) {
                return true;
            }
        }
        return false;
    };

    setChecked(m_dockAreasToggleActions.left, anyShown(Qt::LeftDockWidgetArea));
    setChecked(m_dockAreasToggleActions.right, anyShown(Qt::RightDockWidgetArea));
    setChecked(m_dockAreasToggleActions.top, anyShown(Qt::TopDockWidgetArea));
    setChecked(m_dockAreasToggleActions.bottom, anyShown(Qt::BottomDockWidgetArea));

    bool floatingShown = false;
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        floatingShown |= (dock != nullptr && dock->isFloating() && !dock->isHidden());
    }

    setChecked(m_dockAreasToggleActions.floating, floatingShown);
}

void LC_AppWindowDockLayoutManger::clampWindowToScreen(QWidget* window) {
    if (window == nullptr || window->isMaximized() || window->isFullScreen()) {
        return;
    }
    const auto screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return;
    }
    const QRect frame = window->frameGeometry();
    if (frame.width() < LayoutConstants::MINIMUM_DOCK_FRAME_SIZE || frame.height() < LayoutConstants::MINIMUM_DOCK_FRAME_SIZE) {
        return;
    }
    QScreen* target = nullptr;
    if (window->isVisible() && window->windowHandle() != nullptr) {
        target = window->windowHandle()->screen();
    }
    if (target == nullptr) {
        target = QGuiApplication::screenAt(frame.center());
    }
    if (target == nullptr) {
        int bestIntersection = 0;
        for (QScreen* screen : screens) {
            const QRect intersection = frame.intersected(screen->geometry());
            const int area = intersection.width() * intersection.height();
            if (area > bestIntersection) {
                bestIntersection = area;
                target = screen;
            }
        }
    }
    if (target == nullptr) {
        target = QGuiApplication::primaryScreen();
    }
    if (target == nullptr) {
        return;
    }

    const QRect available = target->availableGeometry();
    if (available.contains(frame)) {
        return;
    }
    const QSize margins = frame.size() - window->size();
    const QPoint inset = window->geometry().topLeft() - frame.topLeft();
    QRect bounded(frame.topLeft(), frame.size().boundedTo(available.size()));
    if (bounded.right() > available.right()) {
        bounded.moveRight(available.right());
    }
    if (bounded.bottom() > available.bottom()) {
        bounded.moveBottom(available.bottom());
    }
    if (bounded.left() < available.left()) {
        bounded.moveLeft(available.left());
    }
    if (bounded.top() < available.top()) {
        bounded.moveTop(available.top());
    }
    const QSize client(qMax(1, bounded.width() - margins.width()), qMax(1, bounded.height() - margins.height()));
    window->setGeometry(QRect(bounded.topLeft() + inset, client));
}


void LC_AppWindowDockLayoutManger::scheduleDockFit() {
    if (m_dockFitPending || m_dockFitRunning || !m_appWin->isVisible()) {
        return;
    }
    m_dockFitPending = true;
    QTimer::singleShot(0, this, [this] {
        m_dockFitPending = false;
        fitDocksToWindow();
    });
}

void LC_AppWindowDockLayoutManger::fitDocksToWindow() {
    if (m_dockFitRunning || !m_appWin->isVisible() || m_appWin->isMinimized() || m_appWin->mdiAreaCAD() == nullptr) {
        return;
    }
    QScopedValueRollback<bool> running(m_dockFitRunning, true);
    QScreen* screen = m_appWin->windowHandle() ? m_appWin->windowHandle()->screen() : nullptr;
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return;
    }

    const auto options = m_appWin->dockOptions();
    m_appWin->setDockOptions(options & ~QMainWindow::AnimatedDocks);

    const auto activateLayout = [this] {
        auto layout = m_appWin->layout();
        if (layout != nullptr) {
            layout->invalidate();
            layout->activate();
        }
        if (m_appWin->centralWidget() != nullptr && m_appWin->centralWidget()->layout() != nullptr) {
            m_appWin->centralWidget()->layout()->activate();
        }
    };

    const QSize available = (m_appWin->isFullScreen() ? screen->geometry() : screen->availableGeometry()).size();
    if (available != m_fittedDockScreen) {
        m_priorityDockName.clear();
    }

    // 1. Reflow toolbars across all 4 areas
    if (m_appWin->getToolBarManager() != nullptr) {
        m_appWin->getToolBarManager()->reflowToolBars(qMin(m_appWin->width(), available.width()),
                                                      qMin(m_appWin->height(), available.height()));
    }
    clampWindowToScreen(m_appWin);
    activateLayout();

    // 2. Staged small-screen dock collapse with hysteresis
    const int winWidth = m_appWin->width();

    // Tier 1 & 2: Left CAD Dock Area collapse / restore
    if (m_autoCollapsedLeftArea) {
        if (winWidth > LayoutConstants::RESTORE_LEFT_THRESHOLD) {
            m_autoCollapsedLeftArea = false;
            setDockAreaRequested(Qt::LeftDockWidgetArea, true);
        }
    }
    else {
        if (winWidth < LayoutConstants::COLLAPSE_LEFT_THRESHOLD && dockAreaRequested(Qt::LeftDockWidgetArea)) {
            m_autoCollapsedLeftArea = true;
            setDockAreaRequested(Qt::LeftDockWidgetArea, false);
        }
    }

    // Tier 3: Right Dock Area collapse / restore as last resort
    if (m_autoCollapsedRightArea) {
        if (winWidth > LayoutConstants::RESTORE_RIGHT_THRESHOLD) {
            m_autoCollapsedRightArea = false;
            setDockAreaRequested(Qt::RightDockWidgetArea, true);
        }
    }
    else {
        if (winWidth < LayoutConstants::COLLAPSE_RIGHT_THRESHOLD && dockAreaRequested(Qt::RightDockWidgetArea)) {
            m_autoCollapsedRightArea = true;
            setDockAreaRequested(Qt::RightDockWidgetArea, false);
        }
    }

    // 3. Bounded right dock width adjustment if still constrained
    auto layout = m_appWin->layout();
    const int minWidth = layout ? layout->minimumSize().width() : 0;
    const int excessWidth = minWidth - m_appWin->width();
    if (excessWidth > 0) {
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (dock != nullptr && !dock->isFloating() && !dock->isHidden() && m_appWin->dockWidgetArea(dock) == Qt::RightDockWidgetArea) {
                const int target = qMax(dock->minimumSizeHint().width(), dock->width() - excessWidth);
                m_appWin->resizeDocks({dock}, {target}, Qt::Horizontal);
                activateLayout();
                break;
            }
        }
    }

    clampWindowToScreen(m_appWin);
    updateDockAreaActions();
    m_appWin->setDockOptions(options);
    m_fittedDockState = m_appWin->saveState();
    m_fittedDockCanvas = m_appWin->mdiAreaCAD()->size();
    m_fittedDockScreen = available;
}

bool LC_AppWindowDockLayoutManger::processEvent(QObject* obj, QEvent* event) {
    if (m_dockLayoutApplying || obj == nullptr || event == nullptr) {
        return false;
    }

    switch (event->type()) {
        case QEvent::Close: {
            auto* dock = qobject_cast<QDockWidget*>(obj);
            if (dock != nullptr) {
                m_requestedDockVisibility[dock->objectName()] = false;
                if (m_priorityDockName == dock->objectName()) {
                    m_priorityDockName.clear();
                }
                scheduleDockFit();
            }
            break;
        }

        case QEvent::Resize: {
            if (m_dockFitRunning) {
                break;
            }
            auto* dock = qobject_cast<QDockWidget*>(obj);
            if (dock != nullptr && dock->isFloating()) {
                scheduleDockFit();
            }
            break;
        }

        case QEvent::Show:
        case QEvent::Hide: {
            if (m_dockLayoutApplying || m_dockFitRunning) {
                break;
            }

            // If a toolbar was shown or hidden by the user, synchronize area actions and adapt layout
            if (qobject_cast<QToolBar*>(obj) != nullptr) {
                if (m_appWin != nullptr && m_appWin->getToolBarManager() != nullptr) {
                    m_appWin->getToolBarManager()->updateToolBarAreaActions();
                }
                scheduleDockFit();
            }
            break;
        }

        default: {
            break;
        }
    }

    return false;
}


void LC_AppWindowDockLayoutManger::clearPriorityDockName() {
    if (!m_dockFitRunning && !m_dockLayoutApplying) {
        m_priorityDockName.clear();
    }
}

void LC_AppWindowDockLayoutManger::redockAllWidgets() {
    if (m_appWin == nullptr) {
        return;
    }

    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);

    // 1. Clear fitter cache, priority dock, and pending float queues
    m_fittedDockState.clear();
    m_priorityDockName.clear();
    m_pendingFloatingDocks.clear();
    m_pendingFloatingPos.clear();
    m_pendingFloatingSize.clear();
    m_autoCollapsedLeftArea = false;
    m_autoCollapsedRightArea = false;

    // 2. Unfloat all floating docks
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->isFloating()) {
            dock->setFloating(false);
        }
    }

    // 3. Tabify and layout docks into standard default groups
    LC_WidgetFactory::redockAllDockWidgets(m_appWin);

    // 4. Ensure inner widgets are visible and correctly parented
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->widget() != nullptr && !dock->isHidden()) {
            dock->widget()->show();
        }
    }

    // 5. Ensure docked areas are enabled, floating is disabled
    m_requestedDockAreas[int(Qt::LeftDockWidgetArea)] = true;
    m_requestedDockAreas[int(Qt::RightDockWidgetArea)] = true;
    m_floatingDocksRequested = false;

    // 6. Synchronize tab bars (shapes, icons, tooltips)
    auto* dockTabBarManager = m_appWin->getDockTabBarManager();
    if (dockTabBarManager != nullptr) {
        dockTabBarManager->synchronizeAll();
    }

    // 7. Re-apply visibility and update action checkmarks
    applyRequestedDockVisibility();
    updateDockAreaActions();
    scheduleDockFit();
}

void LC_AppWindowDockLayoutManger::applyPendingFloatingDocks() {
    if (m_appWin == nullptr || m_pendingFloatingDocks.isEmpty()) {
        return;
    }

    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);

    for (const QString& name : std::as_const(m_pendingFloatingDocks)) {
        auto* dock = m_appWin->findChild<QDockWidget*>(name);
        if (dock != nullptr && m_requestedDockVisibility.value(name, true) && m_floatingDocksRequested) {
            const QPoint targetPos = m_pendingFloatingPos.value(name);
            const QSize targetSize = m_pendingFloatingSize.value(name);

            dock->setFloating(true);
            if (!targetPos.isNull()) {
                dock->move(targetPos);
            }
            if (targetSize.isValid()) {
                dock->resize(targetSize);
            }

            if (dock->titleBarWidget() != nullptr) {
                dock->titleBarWidget()->show();
            }
            if (dock->widget() != nullptr) {
                dock->widget()->show();
            }
            dock->show();
            dock->raise();
            dock->activateWindow();
        }
    }
    m_pendingFloatingDocks.clear();
    m_pendingFloatingPos.clear();
    m_pendingFloatingSize.clear();
    updateDockAreaActions();
}

void LC_AppWindowDockLayoutManger::resetLayoutToDefault() {
    if (m_appWin == nullptr) {
        return;
    }

    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);

    m_fittedDockState.clear();
    m_selectedTabs.clear();
    m_priorityDockName.clear();
    m_pendingFloatingDocks.clear();
    m_pendingFloatingPos.clear();
    m_pendingFloatingSize.clear();
    m_preCloseDockVisibility.clear();
    m_preCloseActiveDock.clear();
    m_autoCollapsedLeftArea = false;
    m_autoCollapsedRightArea = false;

    // Reset requested areas: Left is FALSE by default (per default scheme specification), Right is TRUE
    for (Qt::DockWidgetArea area : {Qt::LeftDockWidgetArea, Qt::RightDockWidgetArea, Qt::TopDockWidgetArea, Qt::BottomDockWidgetArea}) {
        m_requestedDockAreas[int(area)] = (area == Qt::RightDockWidgetArea);
    }
    m_floatingDocksRequested = false;

    // Reset visibility to factory defaults
    m_requestedDockVisibility = m_factoryDockVisibility;

    // Unfloat all docks
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->isFloating()) {
            dock->setFloating(false);
        }
    }

    // Re-dock into standard configuration
    LC_WidgetFactory::redockAllDockWidgets(m_appWin);

    // Ensure inner widgets are visible
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->widget() != nullptr && !dock->isHidden()) {
            dock->widget()->show();
        }
    }

    auto* dockTabBarManager = m_appWin->getDockTabBarManager();
    if (dockTabBarManager != nullptr) {
        dockTabBarManager->synchronizeAll();
    }

    applyRequestedDockVisibility();
    updateDockAreaActions();
    scheduleDockFit();
}
