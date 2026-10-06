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

#ifndef LC_APP_WINDOW_TOOLBAR_MANAGER_H
#define LC_APP_WINDOW_TOOLBAR_MANAGER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QToolBar>

class QC_ApplicationWindow;
class QAction;

struct ToolBarAreasToggleActions {
    QAction* left{nullptr};
    QAction* right{nullptr};
    QAction* top{nullptr};
    QAction* bottom{nullptr};
};

class LC_AppWindowToolBarManager : public QObject {
    Q_OBJECT
public:
    explicit LC_AppWindowToolBarManager(QC_ApplicationWindow* appWin);
    ~LC_AppWindowToolBarManager() override = default;

    void initializeToolBarAreas();
    void setToolBarAreaRequested(Qt::ToolBarArea area, bool enable);
    bool isToolBarAreaRequested(Qt::ToolBarArea area) const;

    void toggleLeftToolBarArea(bool state);
    void toggleRightToolBarArea(bool state);
    void toggleTopToolBarArea(bool state);
    void toggleBottomToolBarArea(bool state);

    void reflowToolBars(int availableWidth, int availableHeight);
    void reflowToolBarArea(Qt::ToolBarArea area, int budget);

    void updateToolbarsIconSize();
    void updateToolbarsIconSize(bool allowCustom, int customSize);
    void updateToolbarsTooltips(bool show);
    void updateToolbarsTooltips();
    void resetLayoutToDefault();

    void updateToolBarAreaActions();
    const ToolBarAreasToggleActions& getToolBarAreasToggleActions() const;
private:
    QC_ApplicationWindow* m_appWin{nullptr};
    ToolBarAreasToggleActions m_toolbarAreasToggleActions;
    QHash<int, bool> m_requestedToolBarAreas;
    QHash<QString, bool> m_preCloseToolBarVisibility;
    QSet<QString> m_autoToolbarBreaks;
    bool m_toolBarLayoutInitialized{false};
};

#endif // LC_APP_WINDOW_TOOLBAR_MANAGER_H
