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
#ifndef LC_CREATORINVOKER_H
#define LC_CREATORINVOKER_H

#include "lc_menu_builder_base.h"
#include "lc_repository_graphic_view_context_menus.h"
#include "lc_repository_menu_bar_and_toolbars.h"

class QToolBar;
class LC_SpecialMenuServiceInterface;
class LC_SpecialMenuService;
struct ContextMenusConfig;

class RS_Entity;
class QAction;
class QG_GraphicView;
class QString;
class LC_ActionGroupManager;
class QC_ApplicationWindow;

class LC_NavigationControlsCreator : public QObject, public LC_MenuBuilderBase {
    Q_OBJECT
public:
    QToolBar* findExistingToolbar(const QString& name, const QString& resolvedTitle) const;
    LC_NavigationControlsCreator(LC_RepositoryMenuBarAndToolbars* repository, QC_ApplicationWindow* appWin,
                                 LC_ActionGroupManager* actionGroupManager,
                                 LC_SpecialMenuServiceInterface* specialMenuService);

    LC_RepositoryMenuBarAndToolbars* getRepository() const {
        return m_repository;
    }

    void applyActiveLayoutScheme();
    void applyMenusToolbarsScheme(const NavigationLayoutConfig& config, bool applyInitialVisibility = false);

    LC_ActionGroupManager* getActionGroupManager() const {
        return m_actionGroupManager;
    }
    QMenu* getPluginsMenu() const {
        return m_pluginsMenu;
    }

    QAction* getAction(const QString& key) const override;

    QMenu* createMainWindowPopupMenu() const;
    void updateToolbarsTooltips(bool showTooltips);
    void updateToolbarsTooltips();
    void resetToolbarsLayout(const NavigationLayoutConfig& config);

protected slots:
    // void createToolbar(const QString& toolbarName, const QStringList& actionNames, int areaIndex) const;
    void destroyToolbar(const QString& toolbarName) const;
    void onCustomToolbarVisibilityChanged(bool visible);
protected:
    void applyMenuBar(const NavigationLayoutConfig& config);
    void applyCadDockWidgets(const NavigationLayoutConfig& config, bool applyInitialVisibility);
    void applyToolbars(const NavigationLayoutConfig& config, bool applyInitialVisibility);
    void populateToolbar(QToolBar* tb, const ToolbarDef& tbDef);
    void updatePenToolbar(const ToolbarDef& tbDef);
    void populateToolbarNodes(QToolBar* tb, const QList<ActionNode>& nodes);
    void clearToolbar(QToolBar* tb);
    bool hasSavedWidgetsState() const;
private:
    QC_ApplicationWindow* m_appWindow{nullptr};

    LC_RepositoryMenuBarAndToolbars* m_repository;
    QMenu* m_pluginsMenu{nullptr};
    bool m_showToolbarTooltips {false};
};

#endif
