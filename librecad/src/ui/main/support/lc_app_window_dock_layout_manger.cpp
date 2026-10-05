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

#include "lc_dock_tab_bar_manager.h"
#include "lc_widget_factory.h"
#include "qc_applicationwindow.h"

LC_AppWindowDockLayoutManger::LC_AppWindowDockLayoutManger(QC_ApplicationWindow* appWindow) : m_appWin(appWindow) {}

// code is base on the original implementation that was added to QC_ApplicationWindow.
// however, that implementation has two serious defects:
// 1) widgets that are floated at the moment of closing the application, were restored unproperly - just as a fully transparent
// windows (at least under Win). That was fixed by shifting restoring them to later stage when main window is created and visible.
// 2) trial-collapse layout reflow under specific conditions leads to non-finishing infinite cycles (trying to arrange widgets
// when unable to satisfy conditions. Layouting was simplified.

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
                m_requestedDockAreas[int(area)] = true;
            }
            scheduleDockFit();
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this, dock](bool floating) {
            if (floating) {
                if (!m_dockLayoutApplying && !m_dockFitRunning) {
                    m_floatingDocksRequested = true;
                }
                QTimer::singleShot(0, dock, [this, dock] {
                    if (QWindow* handle = dock->windowHandle(); handle && !handle->property("_lc_screen_watched").toBool()) {
                        handle->setProperty("_lc_screen_watched", true);
                        connect(handle, &QWindow::screenChanged, this, [this](QScreen*) {
                            scheduleDockFit();
                        });
                    }
                    scheduleDockFit();
                });
            }
            scheduleDockFit();
        });
    }
    for (QToolBar* toolbar : m_appWin->findChildren<QToolBar*>()) {
        toolbar->installEventFilter(this);
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
    // Let the fitter resolve child minimums instead of preventing native resizing.
    if (layout != nullptr) {
        layout->setSizeConstraint(QLayout::SetNoConstraint);
    }
    m_appWin->setMinimumSize(0, 0);
    auto mdiAreaCad = m_appWin->mdiAreaCAD();
    if (mdiAreaCad != nullptr) {
        mdiAreaCad->setMinimumSize(0, 0);
    }
    QScreen* screen = QGuiApplication::screenAt(m_appWin->frameGeometry().center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen) {
        reflowBottomToolbars(screen->availableGeometry().width() - 48);
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
    m_autoCollapsedGroups.clear();
    m_collapsedSelectedTabs.clear();
    m_collapsedGroupMembers.clear();
    m_collapsedMemberKey.clear();
    m_collapsedPressure.clear();
    m_fittedDockState.clear();
    m_autoToolbarBreaks.clear();
    m_selectedTabs.clear();
    m_priorityDockName.clear();
    m_requestedDockVisibility.clear();
    m_pendingFloatingDocks.clear();
    m_pendingFloatingPos.clear();
    m_pendingFloatingSize.clear();

    const bool stateRestored = !state.isEmpty() && m_appWin->restoreState(state);

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock == nullptr) {
            continue;
        }
        const QString name = dock->objectName();
        if (name.isEmpty()) {
            continue;
        }

        // If restored as floating while the main window is still hidden, remember it and defer floating
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
    scheduleDockFit();
}

void LC_AppWindowDockLayoutManger::setDockAreaRequested(Qt::DockWidgetArea area, bool state) {
    m_requestedDockAreas[int(area)] = state;
    if (!state && !m_priorityDockName.isEmpty()) {
        if (QDockWidget* priority = m_appWin->findChild<QDockWidget*>(m_priorityDockName); priority && m_appWin->dockWidgetArea(priority) == area) {
            m_priorityDockName.clear();
        }
    }
    if (state) {
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (!dock->isFloating() && m_appWin->dockWidgetArea(dock) == area) {
                restoreCollapsedGroup(dockGroupKey(dock));
                if (m_requestedDockVisibility.value(dock->objectName(), false)) {
                    m_priorityDockName = dock->objectName();
                }
            }
        }
    }
    applyRequestedDockVisibility();
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
    if (!dock) {
        return;
    }
    m_requestedDockVisibility[dock->objectName()] = true;
    m_priorityDockName = dock->objectName();
    if (dock->isFloating()) {
        m_floatingDocksRequested = true;
    }
    else {
        m_requestedDockAreas[int(m_appWin->dockWidgetArea(dock))] = true;
    }
    restoreCollapsedGroup(dockGroupKey(dock));
    applyRequestedDockVisibility();
    dock->raise();
    updateDockAreaActions();
    scheduleDockFit();
}

QString LC_AppWindowDockLayoutManger::dockGroupKey(QDockWidget* dock) const {
    const QString collapsed = m_collapsedMemberKey.value(dock->objectName());
    if (!collapsed.isEmpty()) {
        return collapsed;
    }
    QStringList names{dock->objectName()};
    if (!dock->isFloating()) {
        for (QDockWidget* peer : m_appWin->tabifiedDockWidgets(dock)) {
            names.append(peer->objectName());
        }
    }
    names.sort();
    return names.join(QLatin1Char('|'));
}

void LC_AppWindowDockLayoutManger::collapseDockGroup(const QList<QDockWidget*>& docks, const QString& key, QDockWidget* selected,
                                                     Qt::Orientation pressure) {
    QStringList names;
    for (QTabBar* bar : m_appWin->findChildren<QTabBar*>()) {
        QStringList order;
        for (int tab = 0; tab < bar->count(); ++tab) {
            for (QDockWidget* dock : docks) {
                if (bar->tabText(tab) == dock->windowTitle()) {
                    order.append(dock->objectName());
                }
            }
        }
        if (order.size() == docks.size() && docks.size() > 1) {
            names = order;
            break;
        }
    }
    for (QDockWidget* dock : docks) {
        if (!names.contains(dock->objectName())) {
            names.append(dock->objectName());
        }
        m_collapsedMemberKey.insert(dock->objectName(), key);
    }
    m_collapsedGroupMembers.insert(key, names);
    m_collapsedPressure.insert(key, pressure);
    if (QDockWidget* remembered = m_appWin->findChild<QDockWidget*>(m_selectedTabs.value(key))) {
        selected = remembered;
    }
    if (selected) {
        m_collapsedSelectedTabs.insert(key, selected->objectName());
        m_selectedTabs.insert(key, selected->objectName());
    }
    m_autoCollapsedGroups.append(key);
    applyRequestedDockVisibility();
}

void LC_AppWindowDockLayoutManger::restoreCollapsedGroup(const QString& key) {
    if (!m_autoCollapsedGroups.contains(key)) {
        return;
    }
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);
    const QStringList members = m_collapsedGroupMembers.take(key);
    const QString selected = m_collapsedSelectedTabs.take(key);
    m_autoCollapsedGroups.removeAll(key);
    m_collapsedPressure.remove(key);
    for (const QString& name : members) {
        m_collapsedMemberKey.remove(name);
    }
    applyRequestedDockVisibility();
    QDockWidget* first = nullptr;
    for (const QString& name : members) {
        QDockWidget* dock = m_appWin->findChild<QDockWidget*>(name);
        if (!dock || dock->isFloating()) {
            continue;
        }
        if (!first) {
            first = dock;
        }
        else {
            m_appWin->tabifyDockWidget(first, dock);
        }
    }
    applyRequestedDockVisibility();
    if (QDockWidget* dock = m_appWin->findChild<QDockWidget*>(selected))
        dock->raise();
}

QByteArray LC_AppWindowDockLayoutManger::dockLayoutStateForSaving() {
    if (m_autoCollapsedGroups.isEmpty() && m_autoToolbarBreaks.isEmpty()) {
        return m_appWin->saveState();
    }
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);
    QScopedValueRollback<bool> fitting(m_dockFitRunning, true);
    const QRect effectiveGeometry = m_appWin->geometry();
    const auto options = m_appWin->dockOptions();
    m_appWin->setDockOptions(options & ~QMainWindow::AnimatedDocks);
    const bool updates = m_appWin->updatesEnabled();
    m_appWin->setUpdatesEnabled(false);
    const QByteArray effective = m_appWin->saveState();
    const QStringList collapsed = m_autoCollapsedGroups;
    const auto members = m_collapsedGroupMembers;
    const auto memberKeys = m_collapsedMemberKey;
    const auto selected = m_collapsedSelectedTabs;
    const auto selectedTabs = m_selectedTabs;
    const auto pressure = m_collapsedPressure;
    const auto toolbarBreaks = m_autoToolbarBreaks;
    for (const QString& key : collapsed) {
        restoreCollapsedGroup(key);
    }
    for (const QString& name : toolbarBreaks) {
        if (QToolBar* toolbar = m_appWin->findChild<QToolBar*>(name)) {
            m_appWin->removeToolBarBreak(toolbar);
        }
    }
    m_autoToolbarBreaks.clear();
    const QByteArray requested = m_appWin->saveState();
    m_appWin->restoreState(effective);
    m_autoCollapsedGroups = collapsed;
    m_collapsedGroupMembers = members;
    m_collapsedMemberKey = memberKeys;
    m_collapsedSelectedTabs = selected;
    m_selectedTabs = selectedTabs;
    m_collapsedPressure = pressure;
    m_autoToolbarBreaks = toolbarBreaks;
    applyRequestedDockVisibility();
    if (m_appWin->layout()) {
        m_appWin->layout()->activate();
    }
    if (!m_appWin->isMaximized() && !m_appWin->isFullScreen()) {
        m_appWin->setGeometry(effectiveGeometry);
    }
    updateDockAreaActions();
    m_appWin->setDockOptions(options);
    m_appWin->setUpdatesEnabled(updates);
    return requested;
}

void LC_AppWindowDockLayoutManger::applyRequestedDockVisibility() {
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        auto dockObjectName = dock->objectName();
        // LC_ERR << "applyRequestedDockVisibility - Dock Name: " << dockObjectName;
        if (dockObjectName.isEmpty()) {
            continue;
        }
        const bool areaEnabled = dock->isFloating() ? m_floatingDocksRequested : dockAreaRequested(m_appWin->dockWidgetArea(dock));
        const bool visible = m_requestedDockVisibility.value(dockObjectName, m_factoryDockVisibility.value(dockObjectName, true)) &&
            areaEnabled && !m_autoCollapsedGroups.contains(dockGroupKey(dock));

        dock->setVisible(visible);
        // LC_ERR << "applyRequestedDockVisibility - Dock Name: " << dockObjectName << (visible? " Visible" : "Not visible") << (areaEnabled? " Area + " : " area -") << (dock->isFloating()? " Floating + " : " floating -");
    }
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (!dock->isHidden() && m_selectedTabs.value(dockGroupKey(dock)) == dock->objectName()) {
            dock->raise();
        }
    }
}

void LC_AppWindowDockLayoutManger::updateDockAreaActions() {
    const auto setChecked = [](QAction* action, bool checked) {
        if (!action) {
            return;
        }
        const QSignalBlocker blocker(action);
        action->setChecked(checked);
    };
    const auto anyShown = [this](Qt::DockWidgetArea area) {
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (!dock->isFloating() && m_appWin->dockWidgetArea(dock) == area && !dock->isHidden()) {
                return true;
            }
        }
        return false;
    };
    auto dockAreasToggleActions = m_appWin->getDockAreasToggleActions();
    setChecked(dockAreasToggleActions.left, anyShown(Qt::LeftDockWidgetArea));
    setChecked(dockAreasToggleActions.right, anyShown(Qt::RightDockWidgetArea));
    setChecked(dockAreasToggleActions.top, anyShown(Qt::TopDockWidgetArea));
    setChecked(dockAreasToggleActions.bottom, anyShown(Qt::BottomDockWidgetArea));
    bool floatingShown = false;
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        floatingShown |= dock->isFloating() && !dock->isHidden();
    }

    setChecked(dockAreasToggleActions.floating, floatingShown);
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
    if (frame.width() < 32 || frame.height() < 32) {
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

void LC_AppWindowDockLayoutManger::reflowBottomToolbars(int availableWidth) {
    for (const QString& name : std::as_const(m_autoToolbarBreaks)) {
        if (QToolBar* toolbar = m_appWin->findChild<QToolBar*>(name); toolbar && m_appWin->toolBarArea(toolbar) == Qt::BottomToolBarArea) {
            m_appWin->removeToolBarBreak(toolbar);
        }
    }
    m_autoToolbarBreaks.clear();
    if (!m_appWin->layout()) {
        return;
    }
    QList<QToolBar*> toolbars;
    // Layout items follow the user order even before widget geometry updates.
    const int itemCount = m_appWin->layout()->count();
    for (int i = 0; i < itemCount; ++i) {
        auto* item = m_appWin->layout()->itemAt(i);
        auto* toolbar = item ? qobject_cast<QToolBar*>(item->widget()) : nullptr;
        if (toolbar && !toolbar->isHidden() && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == Qt::BottomToolBarArea) {
            toolbars.append(toolbar);
        }
    }
    int rowWidth = 0;
    const int budget = qMax(1, availableWidth);
    for (QToolBar* toolbar : std::as_const(toolbars)) {
        if (m_appWin->toolBarBreak(toolbar)) {
            rowWidth = 0;
        }
        const int width = toolbar->minimumSizeHint().width();
        if (rowWidth > 0 && rowWidth + width > budget) {
            m_appWin->insertToolBarBreak(toolbar);
            m_autoToolbarBreaks.insert(toolbar->objectName());
            rowWidth = 0;
        }
        rowWidth += width;
    }
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
    if (m_dockFitRunning || !m_appWin->isVisible() || m_appWin->isMinimized() || !m_appWin->mdiAreaCAD()) {
        return;
    }
    QScopedValueRollback<bool> running(m_dockFitRunning, true);
    QScreen* screen = m_appWin->windowHandle() ? m_appWin->windowHandle()->screen() : nullptr;
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
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

    reflowBottomToolbars(qMin(m_appWin->width(), available.width()) - 48);
    clampWindowToScreen(m_appWin);
    activateLayout();

    // Bounded dock width adjustment without trial-collapsing or saveState thrashing
        auto layout = m_appWin->layout();
    const int minWidth = layout ? layout->minimumSize().width() : 0;
    const int excessWidth = minWidth - m_appWin->width();
    if (excessWidth > 0) {
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (dock != nullptr && !dock->isFloating() && !dock->isHidden()
                && m_appWin->dockWidgetArea(dock) == Qt::RightDockWidgetArea) {
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

    // 1. Guard against event filter triggers and re-entrant dock fitting
    QScopedValueRollback<bool> guard(m_dockLayoutApplying, true);

    // 2. Clear all auto-collapsed groups and fitter caches
    m_autoCollapsedGroups.clear();
    m_collapsedSelectedTabs.clear();
    m_collapsedGroupMembers.clear();
    m_collapsedMemberKey.clear();
    m_collapsedPressure.clear();
    m_fittedDockState.clear();
    m_priorityDockName.clear();

    // 3. Unfloat all floating docks
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->isFloating()) {
            dock->setFloating(false);
        }
    }

    // 4. Tabify and layout docks into standard default groups
    LC_WidgetFactory::redockAllDockWidgets(m_appWin);

    // 5. Ensure inner widgets are visible and correctly parented
    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock != nullptr && dock->widget() != nullptr && !dock->isHidden()) {
            dock->widget()->show();
        }
    }

    // 6. Synchronize requested area states
    m_requestedDockAreas[int(Qt::LeftDockWidgetArea)] = true;
    m_requestedDockAreas[int(Qt::RightDockWidgetArea)] = true;
    m_floatingDocksRequested = false;

    auto dockTabBarManager = m_appWin->getDockTabBarManager();
    // 7. Synchronize tab bars (shapes, icons, tooltips)
    if (dockTabBarManager != nullptr) {
        dockTabBarManager->synchronizeAll();
    }

    // 8. Re-apply visibility and update action checkmarks
    applyRequestedDockVisibility();
    updateDockAreaActions();
    scheduleDockFit();

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        QWidget* inner = dock->widget();
        auto geometry1 = inner ? inner->geometry() : QRect();
        LC_ERR << " [DOCK_REDOCK_AUDIT]" << dock->objectName()
               << " isFloating: " << dock->isFloating()
               << " dockArea: " << (int)m_appWin->dockWidgetArea(dock)
               << " isVisible: " << dock->isVisible()
               << " isHidden: " << dock->isHidden()
               << " innerPtr: " << (inner != nullptr)
               << " innerVisible: " << (inner ? inner->isVisible() : false)
               << " innerGeom: " << geometry1.x() << ", " << geometry1.y() << ", " << geometry1.width() << ", " << geometry1.height();
    }
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

    const bool stateRestored = !state.isEmpty() && m_appWin->restoreState(state);

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
        if (dock == nullptr) {
            continue;
        }
        const QString name = dock->objectName();
        if (name.isEmpty()) {
            continue;
        }

        // If restored as floating while main window is still hidden, defer floating to showEvent
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
