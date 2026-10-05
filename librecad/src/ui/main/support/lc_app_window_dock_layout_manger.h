
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

#ifndef LC_AppWindowDockLayoutManger_H
#define LC_AppWindowDockLayoutManger_H

#include <qdockwidget.h>
#include <QString>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QSize>

class QC_ApplicationWindow;

class LC_AppWindowDockLayoutManger: public QObject {
    Q_OBJECT
public:
    LC_AppWindowDockLayoutManger(QC_ApplicationWindow* appWindow);
    void initializeDockLayout();
    void prepareWindowForShow();
    void toggleLeftDockArea(bool state);
    void toggleRightDockArea(bool state);
    void toggleTopDockArea(bool state);
    void toggleBottomDockArea(bool state);
    void toggleFloatingDockwidgets(bool state);
    void scheduleDockFit();
    QMap<QString, bool> requestedDockVisibility() const;
    bool dockAreaRequested(Qt::DockWidgetArea area) const;
    bool floatingDocksRequested() const;
    void restoreDockLayout(const QMap<QString, bool>& requested, bool hasRequested, const QHash<int, bool>& areas, const QByteArray& state);
    void setDockAreaRequested(Qt::DockWidgetArea area, bool state);
    void requestDockVisible(QDockWidget* dock);
    QByteArray dockLayoutStateForSaving();
    bool processEvent(QObject* obj, QEvent* event);
    void clearPriorityDockName();
    void redockAllWidgets();
    void applyPendingFloatingDocks();
private:
    QMap<QString, bool> m_requestedDockVisibility;
    QMap<QString, bool> m_factoryDockVisibility;
    QHash<int, bool> m_requestedDockAreas;
    QStringList m_autoCollapsedGroups;
    QHash<QString, QString> m_collapsedSelectedTabs;
    QHash<QString, QStringList> m_collapsedGroupMembers;
    QHash<QString, QString> m_collapsedMemberKey;
    QHash<QString, Qt::Orientation> m_collapsedPressure;
    QSet<QString> m_autoToolbarBreaks;
    QHash<QString, QString> m_selectedTabs;
    QString m_priorityDockName;
    bool m_floatingDocksRequested{true};
    bool m_dockLayoutApplying{false};
    bool m_dockFitPending{false};
    bool m_dockFitRunning{false};
    bool m_dockLayoutInitialized{false};
    bool m_screenSignalsConnected{false};
    QSet<QString> m_pendingFloatingDocks;
    QHash<QString, QPoint> m_pendingFloatingPos;
    QHash<QString, QSize> m_pendingFloatingSize;
    QByteArray m_fittedDockState;
    QSize m_fittedDockCanvas;
    QSize m_fittedDockScreen;
    QC_ApplicationWindow* m_appWin {nullptr};


    QString dockGroupKey(QDockWidget* dock) const;
    void collapseDockGroup(const QList<QDockWidget*>& docks, const QString& key, QDockWidget* selected, Qt::Orientation pressure);
    void restoreCollapsedGroup(const QString& key);

    void applyRequestedDockVisibility();
    void updateDockAreaActions();
    void clampWindowToScreen(QWidget* window);
    void reflowBottomToolbars(int availableWidth);
    void fitDocksToWindow();
};

#endif
