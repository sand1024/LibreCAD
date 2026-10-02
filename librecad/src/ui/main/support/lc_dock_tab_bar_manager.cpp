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

#include "lc_dock_tab_bar_manager.h"

#include <QDockWidget>
#include <QEvent>
#include <QSet>
#include <QStyle>
#include <QTabBar>
#include <QToolTip>

#include "lc_caddockwidget.h"
#include "lc_rotated_icon_engine.h"
#include "lc_settings_widget.h"
#include "qc_applicationwindow.h"

namespace {
    QString cleanTitle(QString text) {
        text.remove(QLatin1Char('&'));
        return text.trimmed();
    }
}

LC_DockTabBarManager::LC_DockTabBarManager(QC_ApplicationWindow* mainWin)
    : QObject(mainWin), m_mainWindow(mainWin) {
    if (m_mainWindow != nullptr) {
        for (auto* dock : m_mainWindow->findChildren<QDockWidget*>()) {
            hookDockWidget(dock);
        }
    }
}

LC_DockTabBarManager::~LC_DockTabBarManager() {
    for (auto* bar : m_monitoredBars) {
        if (bar != nullptr) {
            bar->removeEventFilter(this);
        }
    }
    m_monitoredBars.clear();
}

void LC_DockTabBarManager::hookTabBar(QTabBar* bar) {
    if (bar == nullptr || m_monitoredBars.contains(bar)) {
        return;
    }

    m_monitoredBars.insert(bar);
    bar->installEventFilter(this);

    connect(bar, &QTabBar::currentChanged, this, [this, bar](int) {
        synchronizeTabBar(bar);
    });

    connect(bar, &QObject::destroyed, this, [this](QObject* obj) {
        m_monitoredBars.remove(static_cast<QTabBar*>(obj));
    });

    synchronizeTabBar(bar);
}

void LC_DockTabBarManager::hookDockWidget(QDockWidget* dock) {
    if (dock == nullptr || m_hookedDocks.contains(dock)) {
        return;
    }

    m_hookedDocks.insert(dock);

    connect(dock, &QDockWidget::topLevelChanged, this, [this](bool floating) {
        if (!floating) {
            QMetaObject::invokeMethod(this, &LC_DockTabBarManager::synchronizeAll, Qt::QueuedConnection);
        }
    });

    connect(dock, &QDockWidget::dockLocationChanged, this, [this](Qt::DockWidgetArea) {
        QMetaObject::invokeMethod(this, &LC_DockTabBarManager::synchronizeAll, Qt::QueuedConnection);
    });

    connect(dock, &QObject::destroyed, this, [this](QObject* obj) {
        m_hookedDocks.remove(static_cast<QDockWidget*>(obj));
    });
}

int LC_DockTabBarManager::resolveTargetIconSize(QTabBar* bar, bool cadDominant) const {
    using namespace CFG_Widgets;

    if (cadDominant) {
        if (o_CADDockTabOverrideIconSize) {
            return qMax(8, static_cast<int>(o_CADDockTabIconSize));
        }
    }
    else {
        if (o_DockTabOverrideIconSize) {
            return qMax(8, static_cast<int>(o_DockTabIconSize));
        }
    }

    if (bar != nullptr && bar->style() != nullptr) {
        const int styleMetric = bar->style()->pixelMetric(QStyle::PM_TabBarIconSize, nullptr, bar);
        if (styleMetric > 0) {
            return styleMetric;
        }
    }

    return 16;
}

void LC_DockTabBarManager::synchronizeTabBar(QTabBar* bar) {
    if (m_isUpdating || bar == nullptr || m_mainWindow == nullptr) {
        return;
    }

    const int tabCount = bar->count();
    if (tabCount == 0) {
        return;
    }

  m_isUpdating = true;

    using namespace CFG_Widgets;

    const QTabBar::Shape currentShape = bar->shape();
    const bool shapeChanged = (!bar->property("_lc_applied_shape").isValid()
                               || bar->property("_lc_applied_shape").toInt() != static_cast<int>(currentShape));
    if (shapeChanged) {
        bar->setProperty("_lc_applied_shape", static_cast<int>(currentShape));
    }

    int cadTabsCount = 0;
    int normalTabsCount = 0;

    for (int i = 0; i < tabCount; ++i) {
        LC_DockWidgetBase* dockBase = resolveDockForTab(bar, i);
        if (dockBase == nullptr) {
            continue;
        }

        const bool isCad = dockBase->isCadDock();
        if (isCad) {
            ++cadTabsCount;
        }
        else {
            ++normalTabsCount;
        }

        const int mode = isCad ? static_cast<int>(o_CADDockTabDisplayMode)
                               : static_cast<int>(o_DockTabDisplayMode);

        const QIcon rawDockIcon = dockBase->windowIcon();
        const QIcon dockIcon = resolveTabIcon(rawDockIcon, currentShape);
        const QString realTitle = dockBase->realTitle();

        // Resolve rich action tooltip (with description and shortcut) or real title fallback
        QAction* toggleAct = dockBase->toggleViewAction();
        if (toggleAct != nullptr) {
            const QString actionTip = (toggleAct != nullptr && !toggleAct->toolTip().isEmpty()) ? toggleAct->toolTip() : realTitle;

            if (bar->tabToolTip(i) != actionTip) {
                bar->setTabToolTip(i, actionTip);
            }

            const bool isDockOpen = dockBase->isVisible();
            if (toggleAct->isChecked() != isDockOpen) {
                toggleAct->setChecked(isDockOpen);
            }
        }

        switch (mode) {
            case TabDisplay_IconOnly: {
                dockBase->setIconOnlyTabMode(true);
                if (!dockIcon.isNull() && (bar->tabIcon(i).isNull() || shapeChanged)) {
                    bar->setTabIcon(i, dockIcon);
                }
                break;
            }
            case TabDisplay_TextOnly: {
                dockBase->setIconOnlyTabMode(false);
                if (!bar->tabIcon(i).isNull()) {
                    bar->setTabIcon(i, QIcon());
                }
                break;
            }
            case TabDisplay_IconAndText:
            default: {
                dockBase->setIconOnlyTabMode(false);
                if (!dockIcon.isNull() && (bar->tabIcon(i).isNull() || shapeChanged)) {
                    bar->setTabIcon(i, dockIcon);
                }
                break;
            }
        }
    }

    const bool cadDominant = (cadTabsCount > normalTabsCount);
    const int iconSizeVal = resolveTargetIconSize(bar, cadDominant);
    const QSize targetIconSize(iconSizeVal, iconSizeVal);

    if (bar->iconSize() != targetIconSize) {
        bar->setIconSize(targetIconSize);
    }

    m_isUpdating = false;
}

void LC_DockTabBarManager::synchronizeAll() {
    if (m_mainWindow == nullptr) {
        return;
    }

    for (auto* dock : m_mainWindow->findChildren<QDockWidget*>()) {
        hookDockWidget(dock);
    }

    const auto tabBars = m_mainWindow->findChildren<QTabBar*>();
    for (auto* bar : tabBars) {
        if (bar != nullptr) {
            hookTabBar(bar);
            synchronizeTabBar(bar);
        }
    }
}

bool LC_DockTabBarManager::eventFilter(QObject* watched, QEvent* event) {
    if (event == nullptr) {
        return false;
    }

    auto* bar = qobject_cast<QTabBar*>(watched);
    if (bar != nullptr) {
        if (event->type() == QEvent::ToolTip) {
            auto* helpEvent = static_cast<QHelpEvent*>(event);
            if (handleToolTipEvent(bar, helpEvent)) {
                return true;
            }
        }
        else if (event->type() == QEvent::Show) {
            synchronizeTabBar(bar);
        }
    }

    return QObject::eventFilter(watched, event);
}


bool LC_DockTabBarManager::handleToolTipEvent(QTabBar* bar, QHelpEvent* helpEvent) {
    if (bar == nullptr || helpEvent == nullptr || m_mainWindow == nullptr) {
        return false;
    }

    const int index = bar->tabAt(helpEvent->pos());
    if (index < 0 || index >= bar->count()) {
        QToolTip::hideText();
        return false;
    }

    LC_DockWidgetBase* dockBase = resolveDockForTab(bar, index);
    if (dockBase == nullptr) {
        return false;
    }

    // 1. Prefer rich tooltip from toggle action (contains Title, Description, and Shortcut)
    QString tip;
    QAction* toggleAct = dockBase->toggleViewAction();
    if (toggleAct != nullptr && !toggleAct->toolTip().isEmpty()) {
        tip = toggleAct->toolTip();
    }

    // 2. Fallback to dock real title
    if (tip.isEmpty()) {
        tip = dockBase->realTitle();
    }

    if (tip.isEmpty()) {
        return false;
    }

    const QRect rect = bar->tabRect(index);
    QToolTip::showText(helpEvent->globalPos(), tip, bar, rect);
    return true;
}

LC_DockWidgetBase* LC_DockTabBarManager::resolveDockForTab(QTabBar* bar, int index) const {
    if (bar == nullptr || index < 0 || index >= bar->count() || m_mainWindow == nullptr) {
        return nullptr;
    }

    const quintptr dockPtr = bar->tabData(index).toULongLong();
    if (dockPtr != 0) {
        auto* candidate = qobject_cast<LC_DockWidgetBase*>(reinterpret_cast<QDockWidget*>(dockPtr));
        if (candidate != nullptr) {
            return candidate;
        }
    }

    // Self-healing fallback: If Qt omitted tabData (e.g. Tab 0), resolve via tabified siblings
    const int tabCount = bar->count();
    if (tabCount > 1) {
        for (int j = 0; j < tabCount; ++j) {
            if (j == index) {
                continue;
            }
            const quintptr otherPtr = bar->tabData(j).toULongLong();
            if (otherPtr != 0) {
                auto* otherDock = qobject_cast<LC_DockWidgetBase*>(reinterpret_cast<QDockWidget*>(otherPtr));
                if (otherDock != nullptr) {
                    const auto siblings = m_mainWindow->tabifiedDockWidgets(otherDock);
                    for (auto* sibling : siblings) {
                        auto* candidate = qobject_cast<LC_DockWidgetBase*>(sibling);
                        if (candidate != nullptr && candidate != otherDock) {
                            bar->setTabData(index, reinterpret_cast<quintptr>(candidate));
                            return candidate;
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

QIcon LC_DockTabBarManager::resolveTabIcon(const QIcon& baseIcon, const QTabBar::Shape shape) const {
    if (baseIcon.isNull()) {
        return baseIcon;
    }

    qreal angle = 0.0;
    switch (shape) {
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            // Qt QCommonStyle rotates West by -90°; counter-rotate by +90° to keep upright
            angle = 90.0;
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            // Qt QCommonStyle rotates East by +90°; counter-rotate by -90° to keep upright
            angle = -90.0;
            break;
        default:
            angle = 0.0;
            break;
    }

    if (qFuzzyIsNull(angle)) {
        return baseIcon;
    }

    return QIcon(new LC_RotatedIconEngine(baseIcon, angle));
}
