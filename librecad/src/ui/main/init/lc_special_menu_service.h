
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

#ifndef LC_SPECIAL_MENU_SERVICE_H
#define LC_SPECIAL_MENU_SERVICE_H

#include <QMenu>
#include <QString>
#include <QToolBar>

#include "lc_special_menu_service_interface.h"
#include "qc_applicationwindow.h"

class QC_ApplicationWindow;

class LC_SpecialMenuService: public LC_SpecialMenuServiceInterface {
public:
    explicit LC_SpecialMenuService(QC_ApplicationWindow * appWin);

    bool bindMenu(const QString& specialMenuName, QMenu* menu) override;
    bool embedToolbarControl(const QString& widgetToken, QToolBar* targetToolbar) override;
    QAction* getSpecialAction(const QString& actionToken) const override;

protected:
    // Helpers
    QMenu* createDynamicSubMenu(QMenu* parentMenu, const QString& title, const QString& iconPath = QString(),
                                const std::function<void(QMenu*)>& populateFunc = nullptr) const;
    void populateDockWidgets(QMenu* menu, bool cadWidgetsOnly) const;
    void populateToolbars(QMenu* menu, bool cadToolbarsOnly) const;
    void populateDrawings(QMenu* menu) const;
    QAction* getDockWidgetToggleAction(const QString& actionToken) const;

    // Sub-menu entry points
    void bindRecentFilesMenu(QMenu* parentMenu) const;
    void bindPluginsMenu(QMenu* menu);
    void bindDockWidgetsMenu(QMenu* parentMenu);
    void bindCadDockWidgetsMenu(QMenu* parentMenu);
    void bindToolbarsMenu(QMenu* parentMenu);
    void bindCadToolbarsMenu(QMenu* parentMenu);
    void bindWorkspacesRescueMenu(QMenu* parentMenu);
    void bindWorkspacesListMenu(QMenu* parentMenu);
    void bindDrawingsMenu(QMenu* parentMenu);
    void bindNamedViewsListMenu(QMenu* parentMenu);
    void bindUCSListMenu(QMenu* parentMenu);

    QAction* getAction(const QString& name) const;

private:
    QC_ApplicationWindow *m_appWindow{nullptr};
};

#endif
