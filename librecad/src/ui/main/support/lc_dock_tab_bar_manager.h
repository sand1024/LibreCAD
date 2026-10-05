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

#ifndef LC_DOCK_TAB_BAR_MANAGER_H
#define LC_DOCK_TAB_BAR_MANAGER_H

#include <QObject>
#include <QSet>
#include <QSize>
#include <QTabBar>

#include "lc_dock_widget_base.h"

class QHelpEvent;
class QTabBar;
class QDockWidget;
class QC_ApplicationWindow;

class LC_DockTabBarManager : public QObject {
    Q_OBJECT

public:
    explicit LC_DockTabBarManager(QC_ApplicationWindow* mainWin);
    ~LC_DockTabBarManager() override;

    void hookDockWidget(QDockWidget* dock);
    void hookTabBar(QTabBar* bar);
    void synchronizeTabBar(QTabBar* bar);
    void synchronizeAll();
    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    bool handleToolTipEvent(QTabBar* bar, QHelpEvent* helpEvent);
    bool handleMiddleMouseClose(QTabBar* bar, QMouseEvent* mouseEvent) const;
    LC_DockWidgetBase* resolveDockForTab(QTabBar* bar, int index) const;
    QIcon resolveTabIcon(const QIcon& baseIcon, QTabBar::Shape shape) const;
    int resolveTargetIconSize(QTabBar* bar, bool cadDominant) const;

    QC_ApplicationWindow* m_mainWindow{nullptr};
    QSet<QTabBar*> m_monitoredBars;
    QSet<QDockWidget*> m_hookedDocks;
    bool m_isUpdating{false};
};

#endif // LC_DOCK_TAB_BAR_MANAGER_H
