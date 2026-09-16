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

#include "lc_special_menu_service.h"

#include <QAction>
#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QToolBar>

#include "lc_action_factory.h"
#include "lc_action_group_manager.h"
#include "lc_mdiapplicationwindow.h"
#include "lc_namedviewslistwidget.h"
#include "lc_settings_appearance.h"
#include "lc_settings_startup.h"
#include "lc_ucslistwidget.h"
#include "lc_view.h"
#include "lc_ucs.h"
#include "lc_workspacelistbutton.h"
#include "qc_applicationwindow.h"

LC_SpecialMenuService::LC_SpecialMenuService(QC_ApplicationWindow* appWin) : m_appWindow(appWin) {
}

QAction* LC_SpecialMenuService::getAction(const QString& name) const {
    if (m_appWindow != nullptr) {
        return m_appWindow->getAction(name);
    }
    return nullptr;
}

QMenu* LC_SpecialMenuService::createDynamicSubMenu(QMenu* parentMenu, const QString& title, const QString& iconPath,
                                                   const std::function<void(QMenu*)>& populateFunc) const {
    if (m_appWindow == nullptr || parentMenu == nullptr) {
        return nullptr;
    }

    // Only inherit tear-off if parent menu is also tear-off enabled (main menu bar only)
    const bool allowTearOff = CFG_Appearance::o_AllowMenusTearOff && parentMenu->isTearOffEnabled();

    auto* subMenu = parentMenu->addMenu(title);
    if (subMenu == nullptr) {
        return nullptr;
    }

    if (!iconPath.isEmpty()) {
        subMenu->setIcon(QIcon(iconPath));
    }
    subMenu->setTearOffEnabled(allowTearOff);

    if (populateFunc != nullptr) {
        QObject::connect(subMenu, &QMenu::aboutToShow, subMenu, [subMenu, populateFunc]() {
            subMenu->clear();
            populateFunc(subMenu);
        });
    }

    return subMenu;
}

QAction* LC_SpecialMenuService::getDockWidgetToggleAction(const QString& actionToken) const {
    if (m_appWindow == nullptr) {
        return nullptr;
    }

    static const QMap<QString, const char*> s_dockMap = {
        {LC_ActionNames::ToggleDockCommandLine, "command_dockwidget"},
        {LC_ActionNames::ToggleDockLayers, "layer_dockwidget"},
        {LC_ActionNames::ToggleDockLayerTree, "layer_tree_dockwidget"},
        {LC_ActionNames::ToggleDockBlocks, "block_dockwidget"},
        {LC_ActionNames::ToggleDockProperties, "property_sheet"},
        {LC_ActionNames::ToggleDockLibrary, "library_dockwidget"},
        {LC_ActionNames::ToggleDockQuickInfo, "quick_entity_info"},
        {LC_ActionNames::ToggleDockPenPalette, "pen_palette_dockwidget"},
        {LC_ActionNames::ToggleDockPenWizard, "pen_wiz_dockwidget"},
        {LC_ActionNames::ToggleDockNamedViews, "view_dockwidget"},
        {LC_ActionNames::ToggleDockUCS, "ucs_dockwidget"}
    };

    const auto it = s_dockMap.find(actionToken);
    if (it != s_dockMap.end()) {
        auto* dw = m_appWindow->findChild<QDockWidget*>(it.value());
        if (dw != nullptr) {
            return dw->toggleViewAction();
        }
    }
    return nullptr;
}

QAction* LC_SpecialMenuService::getSpecialAction(const QString& actionToken) const {
    return getDockWidgetToggleAction(actionToken);
}

bool LC_SpecialMenuService::bindMenu(const QString& specialMenuName, QMenu* parentMenu) {
    if (specialMenuName == LC_ActionNames::MenuRecentFiles) {
        bindRecentFilesMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuDockWidgets) {
        bindDockWidgetsMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuCadDockWidgets) {
        bindCadDockWidgetsMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuToolbars) {
        bindToolbarsMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuCadToolbars) {
        bindCadToolbarsMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuWorkspacesList) {
        bindWorkspacesListMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuDrawings) {
        bindDrawingsMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuNamedViewsList) {
        bindNamedViewsListMenu(parentMenu);
        return true;
    }
    if (specialMenuName == LC_ActionNames::MenuUCSList) {
        bindUCSListMenu(parentMenu);
        return true;
    }

    // Individual dock widget toggles
    auto* toggleAct = getDockWidgetToggleAction(specialMenuName);
    if (toggleAct != nullptr) {
        parentMenu->addAction(toggleAct);
        return true;
    }

    QAction* act = getAction(specialMenuName);
    if (act != nullptr) {
        parentMenu->addAction(act);
        return true;
    }
    return false;
}

void LC_SpecialMenuService::bindRecentFilesMenu(QMenu* parentMenu) const {
    if (m_appWindow != nullptr && parentMenu != nullptr) {
        QMenu* recent = m_appWindow->getRecentFilesMenu();
        if (recent != nullptr) {
            const bool allowTearOff = CFG_Appearance::o_AllowMenusTearOff && parentMenu->isTearOffEnabled();
            recent->setTearOffEnabled(allowTearOff);
            parentMenu->addMenu(recent);
        }
    }
}

void LC_SpecialMenuService::populateDockWidgets(QMenu* menu, bool cadWidgetsOnly) const {
    if (m_appWindow == nullptr || menu == nullptr) {
        return;
    }

    QList<QDockWidget*> dockwidgetsList = m_appWindow->findChildren<QDockWidget*>();
    m_appWindow->sortWidgetsByTitle(dockwidgetsList);

    for (auto* dw : dockwidgetsList) {
        if (dw != nullptr) {
            const bool isCadWidget = (m_appWindow->dockWidgetArea(dw) == Qt::LeftDockWidgetArea);
            if (cadWidgetsOnly == isCadWidget) {
                menu->addAction(dw->toggleViewAction());
            }
        }
    }
}

void LC_SpecialMenuService::bindDockWidgetsMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("Wid&gets"), QString(), [this](QMenu* menu) {
        populateDockWidgets(menu, false);
    });
}

void LC_SpecialMenuService::bindCadDockWidgetsMenu(QMenu* parentMenu) {
    if (!CFG_Startup::o_EnableLeftSidebar) {
        return;
    }
    createDynamicSubMenu(parentMenu, QObject::tr("CAD Wid&gets"), QString(), [this](QMenu* menu) {
        populateDockWidgets(menu, true);
    });
}

void LC_SpecialMenuService::populateToolbars(QMenu* menu, bool cadToolbarsOnly) const {
    if (m_appWindow == nullptr || menu == nullptr) {
        return;
    }

    QList<QToolBar*> toolbarsList = m_appWindow->findChildren<QToolBar*>();
    QList<QToolBar*> filtered;

    for (auto* tb : toolbarsList) {
        if (tb != nullptr) {
            const bool isCad = (tb->property("_group").toInt() == 2);
            if (cadToolbarsOnly == isCad) {
                filtered.append(tb);
            }
        }
    }

    m_appWindow->sortWidgetsByGroupAndTitle(filtered);
    for (auto* tb : filtered) {
        if (tb != nullptr) {
            menu->addAction(tb->toggleViewAction());
        }
    }
}

void LC_SpecialMenuService::bindToolbarsMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("&Toolbars"), QString(), [this](QMenu* menu) {
        populateToolbars(menu, false);
    });
}

void LC_SpecialMenuService::bindCadToolbarsMenu(QMenu* parentMenu) {
    if (!CFG_Startup::o_EnableCADToolbars) {
        return;
    }
    createDynamicSubMenu(parentMenu, QObject::tr("&CAD Toolbars"), QString(), [this](QMenu* menu) {
        populateToolbars(menu, true);
    });
}

void LC_SpecialMenuService::bindWorkspacesListMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("Saved Workspaces"), ":/icons/workspace.lci", [this](QMenu* menu) {
        QList<QPair<int, QString>> wsList;
        m_appWindow->fillWorkspacesList(wsList);

        for (const auto& ws : wsList) {
            auto* act = menu->addAction(ws.second);
            const int id = ws.first;
            QObject::connect(act, &QAction::triggered, m_appWindow, [this, id]() {
                m_appWindow->applyWorkspaceById(id);
            });
        }
    });
}

void LC_SpecialMenuService::populateDrawings(QMenu* menu) const {
    if (m_appWindow == nullptr || menu == nullptr) {
        return;
    }

    QMdiArea* mdiArea = m_appWindow->getMdiArea();
    if (mdiArea == nullptr) {
        return;
    }

    const bool tabbed = (mdiArea->viewMode() == QMdiArea::TabbedView);

    auto* actTab = menu->addAction(QObject::tr("Ta&b mode"), m_appWindow, &LC_MDIApplicationWindow::slotToggleTab);
    actTab->setCheckable(true);
    actTab->setChecked(tabbed);

    auto* actWin = menu->addAction(QObject::tr("&Window mode"), m_appWindow, &LC_MDIApplicationWindow::slotToggleTab);
    actWin->setCheckable(true);
    actWin->setChecked(!tabbed);

    menu->addSeparator();
    menu->addAction(QObject::tr("&Cascade"), m_appWindow, &LC_MDIApplicationWindow::slotCascade);
    menu->addAction(QObject::tr("&Tile"), m_appWindow, &LC_MDIApplicationWindow::slotTile);
    menu->addAction(QObject::tr("Tile &Vertically"), m_appWindow, &LC_MDIApplicationWindow::slotTileVertical);
    menu->addAction(QObject::tr("Tile &Horizontally"), m_appWindow, &LC_MDIApplicationWindow::slotTileHorizontal);

    const auto windowList = mdiArea->subWindowList();
    if (!windowList.isEmpty()) {
        menu->addSeparator();
        const QMdiSubWindow* active = mdiArea->activeSubWindow();
        for (int i = 0; i < windowList.size(); ++i) {
            auto* sub = windowList.at(i);
            if (sub != nullptr) {
                auto* act = menu->addAction(sub->windowTitle(), m_appWindow, &QC_ApplicationWindow::slotWindowsMenuActivated);
                act->setCheckable(true);
                act->setData(i);
                act->setChecked(sub == active);
            }
        }
    }
}

void LC_SpecialMenuService::bindDrawingsMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("&Drawings"), QString(), [this](QMenu* menu) {
        populateDrawings(menu);
    });
}

void LC_SpecialMenuService::bindNamedViewsListMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("Saved Views"), ":/icons/nview_visible.lci", [this](QMenu* menu) {
        auto* viewsWidget = m_appWindow->getNamedViewsListWidget();
        if (viewsWidget != nullptr) {
            QList<LC_View*> viewsList;
            viewsWidget->fillViewsList(viewsList);

            for (const auto* v : viewsList) {
                if (v != nullptr) {
                    const QString name = v->getName();
                    auto* act = menu->addAction(viewsWidget->getViewTypeIcon(v), name);
                    QObject::connect(act, &QAction::triggered, m_appWindow, [this, name]() {
                        m_appWindow->restoreNamedView(name);
                    });
                }
            }
        }
    });
}

void LC_SpecialMenuService::bindUCSListMenu(QMenu* parentMenu) {
    createDynamicSubMenu(parentMenu, QObject::tr("User Coordinate Systems"), ":/icons/ucs_ucs.lci", [this](QMenu* menu) {
        auto* ucsWidget = m_appWindow->getUCSListWidget();
        if (ucsWidget != nullptr) {
            QList<LC_UCS*> ucsList;
            ucsWidget->fillUCSList(ucsList);
            const auto* activeUCS = ucsWidget->getActiveUCS();

            for (const auto* u : ucsList) {
                if (u != nullptr) {
                    const QString name = u->getName();
                    auto* act = menu->addAction(ucsWidget->getUCSTypeIcon(u), name);
                    act->setCheckable(true);
                    act->setChecked(u == activeUCS);
                    const QModelIndex idx = ucsWidget->getIndexForUCS(u);

                    QObject::connect(act, &QAction::triggered, ucsWidget, [ucsWidget, idx]() {
                        ucsWidget->applyUCSByIndex(idx);
                    });
                }
            }
        }
    });
}

bool LC_SpecialMenuService::embedToolbarControl(const QString& widgetToken, QToolBar* targetToolbar) {
    if (m_appWindow == nullptr || targetToolbar == nullptr) {
        return false;
    }


    if (widgetToken == LC_ActionNames::WidgetUCSSelector) {
        auto ucsListWidget = m_appWindow->getUCSListWidget();
        if (ucsListWidget != nullptr) {
            auto* ucsCreate = m_appWindow->getAction("UCSCreate");
            auto* ucsSetWcs = m_appWindow->getAction("UCSSetWCS");
            ucsSetWcs->setCheckable(false);
            QObject::connect(ucsSetWcs, &QAction::triggered, ucsListWidget, &LC_UCSListWidget::setWCS);
            const auto ucsSelectionWidget = ucsListWidget->createSelectionWidget(ucsCreate, ucsSetWcs);
            ucsListWidget->setStateWidget(m_appWindow->getUcsStateWidget());
            if (ucsSelectionWidget != nullptr) {
                targetToolbar->addWidget(ucsSelectionWidget);
                return true;
            }
        }
    }
    else if (widgetToken == LC_ActionNames::WidgetNamedViewsSelector) {
        auto namedViewsListWidget = m_appWindow->getNamedViewsListWidget();
        if (namedViewsListWidget != nullptr) {
            auto* saveView = m_appWindow->getAction("ZoomViewSave");
            auto* restoreView = m_appWindow->getAction("ZoomViewRestore");
            auto* w = namedViewsListWidget->createSelectionWidget(saveView, restoreView);
            if (w != nullptr) {
                targetToolbar->addWidget(w);
                return true;
            }
        }
    }
    else if (widgetToken == LC_ActionNames::WidgetWorkspaceSelector) {
        auto* w = new LC_WorkspaceListButton(m_appWindow);
        auto* restoreAction = m_appWindow->getAction("WorkspaceRestore");
        if (restoreAction != nullptr) {
            w->setDefaultAction(restoreAction);
            QObject::connect(m_appWindow, &QC_ApplicationWindow::workspacesChanged, w, &LC_WorkspaceListButton::enableSubActions);
            QObject::connect(m_appWindow, &QC_ApplicationWindow::workspacesChanged, restoreAction, &QAction::setEnabled);
        }
        targetToolbar->addWidget(w);
        return true;
    }

    return false;
}
