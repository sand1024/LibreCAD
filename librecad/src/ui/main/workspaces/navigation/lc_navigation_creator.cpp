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

#include "lc_navigation_creator.h"

#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include "lc_action_group_manager.h"
#include "lc_actions_naming_utils.h"
#include "lc_cad_tool_matrix_dock_widget.h"
#include "lc_default_navigation_layout_builder.h"
#include "lc_repository_menu_bar_and_toolbars.h"
#include "lc_settings_app_state.h"
#include "lc_settings_paths.h"
#include "lc_settings_startup.h"
#include "lc_special_menu_service_interface.h"
#include "qc_applicationwindow.h"
#include "qg_graphicview.h"
#include "qg_pentoolbar.h"
#include "qg_snaptoolbar.h"

QToolBar* LC_NavigationControlsCreator::findExistingToolbar(const QString& name, const QString& resolvedTitle) const {
    if (m_appWindow == nullptr) {
        return nullptr;
    }

    const QString lowerName = name.toLower();
    const QString lowerNoSpaces = QString(name).remove(' ').toLower();

    for (auto* tb : m_appWindow->findChildren<QToolBar*>()) {
        if (tb == nullptr) {
            continue;
        }
        const QString objName = tb->objectName().toLower();
        const QString winTitle = tb->windowTitle();

        if (objName == lowerName || objName == lowerNoSpaces || objName == lowerName + "_toolbar" || objName == lowerNoSpaces + "_toolbar"
            || winTitle == name || winTitle == resolvedTitle) {
            return tb;
        }
    }
    return nullptr;
}


LC_NavigationControlsCreator::LC_NavigationControlsCreator(QC_ApplicationWindow* appWin, LC_ActionGroupManager* actionGroupManager,
                                                           LC_SpecialMenuServiceInterface* specialMenuService)
    : LC_MenuBuilderBase(specialMenuService), m_appWindow{appWin}, m_actionGroupManager(actionGroupManager){

    const QString baseFolder = CFG_Paths::o_OtherSettingsDir;
    m_repository = std::make_unique<LC_RepositoryMenuBarAndToolbars>(baseFolder + "/menus_toolbars");
}


void createCustomMenuForFirstRunIfNeeded() {
    const bool firstLoad = CFG_Startup::o_FirstLoad;
    if (firstLoad) {
        QStringList list;
        list << "ZoomAuto";
        QSettings settings;
        const auto menuName = "AutoZoom";
        const auto key = QString("CustomMenus/%1").arg(menuName);
        settings.setValue(key, list);

        LC_MenuActivator zoomActivator("", false, false, false, LC_MenuActivator::MIDDLE, LC_MenuActivator::DBL_CLICK, false,
                                       RS2::EntityUnknown);
        zoomActivator.update();
        const auto shortcut = zoomActivator.getShortcut();
        const auto activatorKey = QString("Activators/%1").arg(shortcut);

        settings.setValue(activatorKey, menuName);
    }
}

// void LC_NavigationControlsCreator::createToolbar(const QString& toolbarName, const QStringList& actionNames, const int areaIndex) const {
//     auto toolbar = m_appWindow->findChild<QToolBar*>(toolbarName);
//
//     if (toolbar != nullptr) {
//         toolbar->clear();
//     }
//     else {
//         toolbar = new QToolBar(toolbarName, m_appWindow);
//         toolbar->setObjectName(toolbarName);
//         if (m_showToolbarTooltips) {
//             toolbar->setToolTip(tr("Toolbar: %1 (Custom)").arg(toolbarName));
//         }
//         Qt::ToolBarArea area;
//         switch (areaIndex) {
//             case 0:
//                 area = Qt::BottomToolBarArea;
//                 break;
//             case 1:
//                 area = Qt::LeftToolBarArea;
//                 break;
//             case 2:
//                 area = Qt::RightToolBarArea;
//                 break;
//             case 3:
//                 area = Qt::TopToolBarArea;
//                 break;
//             default:
//                 area = Qt::BottomToolBarArea;
//                 break;
//         }
//         m_appWindow->addToolBar(area, toolbar);
//     }
//
//     for (const auto& key : actionNames) {
//         if ("" == key) {
//             toolbar->addSeparator();
//         }
//         else {
//             toolbar->addAction(getAction(key));
//         }
//     }
//
//     connect(toolbar, &QToolBar::visibilityChanged, this, &LC_NavigationControlsCreator::onCustomToolbarVisibilityChanged);
// }

void LC_NavigationControlsCreator::destroyToolbar(const QString& toolbarName) const {
    const auto toolbar = m_appWindow->findChild<QToolBar*>(toolbarName);
    toolbar->setVisible(false);
    disconnect(toolbar, &QToolBar::visibilityChanged, this, &LC_NavigationControlsCreator::onCustomToolbarVisibilityChanged);
    delete toolbar;
}

void LC_NavigationControlsCreator::onCustomToolbarVisibilityChanged(const bool visible) {
    const auto toolbar = dynamic_cast<QToolBar*>(sender());
    if (toolbar != nullptr) {
        const QString toolbarName = toolbar->objectName();
        QSettings settings;
        settings.beginGroup("CustomToolbarsVisibility");
        QStringList visibleToolbars = settings.value("VisibleList").toStringList();

        const qsizetype size = visibleToolbars.count();
        int idx = -1;
        for (qsizetype i = 0; i < size; i++) {
            const QString& s = visibleToolbars.at(i);
            if (s == toolbarName) {
                idx = i;
                break;
            }
        }
        if (visible) {
            if (idx == -1) {
                visibleToolbars << toolbarName;
            }
        }
        else {
            if (idx != -1) {
                visibleToolbars.remove(idx);
            }
        }
        settings.setValue("VisibleList", visibleToolbars);
        settings.endGroup();

        // LC_ERR << "TB_Visible " << toolbarName << (visible ? " Yes" : " no");
    }
}



void LC_NavigationControlsCreator::applyActiveLayoutScheme() {
    const QString activeKey = CFG_AppState::o_ActiveNavigationLayoutScheme;
    NavigationLayoutConfig config;

    if (activeKey == DEFAULT_THEME_KEY || activeKey.isEmpty() ||
        m_repository == nullptr || !m_repository->loadByKey(activeKey, config)) {
        auto* actionFactory = (m_appWindow != nullptr) ? m_appWindow->getActionFactory() : nullptr;
        config = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(actionFactory, m_actionGroupManager);
        }

    applyMenusToolbarsScheme(config);
}
QAction* LC_NavigationControlsCreator::getAction(const QString& key) const {
    return m_appWindow->getAction(key);
}


void LC_NavigationControlsCreator::applyMenusToolbarsScheme(const NavigationLayoutConfig& config) {
    if (m_appWindow == nullptr) {
        return;
    }

    applyMenuBar(config);
    applyCadDockWidgets(config);
    applyToolbars(config);
}

void LC_NavigationControlsCreator::applyMenuBar(const NavigationLayoutConfig& config) {
    QMenuBar* menuBar = m_appWindow->menuBar();
    if (menuBar == nullptr) {
        return;
    }

    menuBar->clear();

    const bool expanded = CFG_Startup::o_ExpandedToolsMenu;
    const bool tillEntity = CFG_Startup::o_ExpandedToolsMenuTillEntity;

    const QList<ActionNode>& activeMenuNodes = (!expanded)
        ? config.menuCompact
        : (!tillEntity ? config.menuCompactTools : config.menuExtended);

    const bool showIcons = CFG_Appearance::o_ShowMainMenuIcons;
    const bool allowTearOff = CFG_Appearance::o_AllowMenusTearOff;

    m_pluginsMenu = nullptr;

    const LC_ActionNamingServiceInterface* namingService = m_actionGroupManager->getNamingService();

    for (const auto& topGroup : activeMenuNodes) {
        if (topGroup.type == ActionNodeType::Group) {
            const bool isPluginsMenu = (topGroup.groupTitle == "Menu:Plugins");

            if (!isPluginsMenu && !hasAnyValidActions(topGroup.children)) {
                continue;
            }

            const QString menuTitle = namingService->displayName(topGroup.groupTitle, /*stripAmpersand=*/false);
            auto* topMenu = menuBar->addMenu(menuTitle);
            if (topMenu == nullptr) {
                continue;
            }
            topMenu->setTearOffEnabled(allowTearOff);
            topMenu->setObjectName(topGroup.groupTitle);

            if (isPluginsMenu) {
                topMenu->setToolTipsVisible(true);
                m_pluginsMenu = topMenu;
            }

            if (showIcons && !topGroup.groupIcon.isEmpty()) {
                topMenu->setIcon(QIcon(topGroup.groupIcon));
            }

            populateMenuRecursive(topMenu, topGroup.children, namingService, allowTearOff);
        }
    }

    menuBar->setVisible(CFG_Appearance::o_MainMenuVisible);
}

void LC_NavigationControlsCreator::applyCadDockWidgets(const NavigationLayoutConfig& config) {
    // 1. Update CAD Tools Matrix (find by type or objectName)
    auto* megaDock = m_appWindow->findChild<LC_CADToolMatrixDockWidget*>("dock_cad_mega");
    if (megaDock == nullptr) {
        megaDock = m_appWindow->findChild<LC_CADToolMatrixDockWidget*>();
    }

    if (megaDock != nullptr) {
        for (const auto& tbDef : config.toolbars) {
            if (tbDef.kind == ToolbarKind::CadMatrix) {
                megaDock->updateActionsFromNodes(tbDef.nodes, m_actionGroupManager);
                break;
            }
        }
    }

    // 2. Update individual CAD dock widgets
    for (const auto& tbDef : config.toolbars) {
        if (tbDef.kind == ToolbarKind::CadDockWidget) {
            // Match "dock_<name>" or find by title
            const QString objName = QString(tbDef.name).startsWith("dock_")
                ? tbDef.name
                : "dock_" + QString(tbDef.name).toLower();

            auto* cadDock = m_appWindow->findChild<LC_CADDockWidget*>(objName);
            if (cadDock != nullptr) {
                cadDock->updateActionsFromNodes(tbDef.nodes, m_actionGroupManager);
            }
        }
    }
}


void LC_NavigationControlsCreator::populateToolbarNodes(QToolBar* tb, const QList<ActionNode>& nodes) {
    if (tb == nullptr) {
        return;
    }

    const QSize childButtonIconSize = (m_appWindow != nullptr) ? m_appWindow->iconSize() : QSize(24, 24);
    const LC_ActionNamingServiceInterface* namingService = m_actionGroupManager->getNamingService();

    for (const auto& node : nodes) {
        if (node.type == ActionNodeType::Separator) {
            tb->addSeparator();
        }
        else if (node.type == ActionNodeType::Action) {
            if (node.actionName.startsWith("Widget:")) {
                if (m_specialMenuService != nullptr) {
                    m_specialMenuService->embedToolbarControl(node.actionName, tb);
                }
            }
            else {
                QAction* act = nullptr;
                if (m_specialMenuService != nullptr) {
                    act = m_specialMenuService->getSpecialAction(node.actionName);
                }
                if (act == nullptr) {
                    act = getAction(node.actionName);
                }
                if (act != nullptr) {
                    tb->addAction(act);
                }
            }
        }
        else if (node.type == ActionNodeType::Group) {
            if (!hasAnyValidActions(node.children)) {
                continue;
            }

            auto* toolBtn = new QToolButton(tb);
            toolBtn->setIconSize(childButtonIconSize);

            const QString cleanTitle = namingService->displayName(node.groupTitle, /*stripAmpersand=*/true);
            toolBtn->setText(cleanTitle);
            toolBtn->setToolTip(cleanTitle);

            QString iconPath = node.groupIcon;
            if (iconPath.isEmpty()) {
                iconPath = namingService->iconPath(node.groupTitle);
            }
            if (!iconPath.isEmpty()) {
                toolBtn->setIcon(QIcon(iconPath));
            }

            auto* menu = new QMenu(toolBtn);
            populateMenuRecursive(menu, node.children, namingService, /*allowTearOff=*/false);
            toolBtn->setMenu(menu);

            if (node.popupMode == ToolButtonPopupMode::InstantPopup) {
                toolBtn->setPopupMode(QToolButton::InstantPopup);
            }
            else {
                toolBtn->setPopupMode(QToolButton::MenuButtonPopup);
                for (const auto& child : node.children) {
                    if (child.type == ActionNodeType::Action) {
                        auto* firstAct = getAction(child.actionName);
                        if (firstAct != nullptr) {
                            toolBtn->setDefaultAction(firstAct);
                            break;
                        }
                    }
                }
                if (node.popupMode == ToolButtonPopupMode::SplitLastUsedAction) {
                    connect(menu, &QMenu::triggered, toolBtn, [toolBtn](QAction* executed) {
                        if (executed != nullptr) {
                            toolBtn->setDefaultAction(executed);
                        }
                    });
                }
            }
            tb->addWidget(toolBtn);
        }
    }
}

void LC_NavigationControlsCreator::populateToolbar(QToolBar* tb, const ToolbarDef& tbDef) {
    if (tb == nullptr) {
        return;
    }

    const LC_ActionNamingServiceInterface* namingService = m_actionGroupManager->getNamingService();

    if (m_showToolbarTooltips) {
        tb->setToolTip(tr("Toolbar: %1").arg(namingService->toolbarTitle(tbDef.name)));
    }

    if (!tbDef.icon.isEmpty()) {
        tb->toggleViewAction()->setIcon(QIcon(tbDef.icon));
    }
    else if (!tbDef.nodes.isEmpty()) {
        auto* firstAct = getAction(tbDef.nodes.first().actionName);
        if (firstAct != nullptr) {
            tb->toggleViewAction()->setIcon(firstAct->icon());
        }
    }

    populateToolbarNodes(tb, tbDef.nodes);
}

void LC_NavigationControlsCreator::updatePenToolbar(const ToolbarDef& tbDef) {
    auto* penTb = m_appWindow->getPenToolBar();
    if (penTb == nullptr) {
        return;
    }

    const Qt::ToolBarArea currentArea = m_appWindow->toolBarArea(penTb);
    if (currentArea != tbDef.area && !penTb->isFloating()) {
        m_appWindow->addToolBar(tbDef.area, penTb);
    }

    penTb->setVisible(tbDef.visible);
    penTb->toggleViewAction()->setIcon(QIcon(":/icons/pen_apply.lci"));

    // Preserve the 3 permanent combo boxes (indices 0, 1, 2) and remove trailing items
    const QList<QAction*> currentActions = penTb->actions();
    for (int i = 3; i < currentActions.size(); ++i) {
        penTb->removeAction(currentActions.at(i));
    }

    // Populate all configured nodes (actions, separators, and flyout groups)
    populateToolbarNodes(penTb, tbDef.nodes);
}

void LC_NavigationControlsCreator::applyToolbars(const NavigationLayoutConfig& config) {
    QSet<QToolBar*> activeToolbars;

    const auto namingService = m_actionGroupManager->getNamingService();

    for (const auto& tbDef : config.toolbars) {
        if (tbDef.kind == ToolbarKind::CadMatrix || tbDef.kind == ToolbarKind::CadDockWidget) {
            continue;
        }

        if (!hasAnyValidActions(tbDef.nodes)) {
            continue;
        }

        // Pen toolbar handling
        auto* penTb = m_appWindow->getPenToolBar();
        if (penTb != nullptr && (tbDef.name == tr("Pen") || tbDef.name == "Pen" || tbDef.name == "pen_toolbar")) {
            updatePenToolbar(tbDef);
            activeToolbars.insert(penTb);
            continue;
        }

        bool isNewlyCreated = false;
        auto toolbarTitle = namingService->toolbarTitle(tbDef.name);
        QToolBar* tb = findExistingToolbar(tbDef.name, toolbarTitle);

        if (tb != nullptr) {
            tb->clear();
        }
        else {
            tb = new QToolBar(toolbarTitle, m_appWindow);
            tb->setObjectName(QString(tbDef.name).remove(' ').toLower() + "_toolbar");
            if (tbDef.kind == ToolbarKind::Cad) {
                tb->setProperty("_group", 2);
            }
            isNewlyCreated = true;
        }

        tb->setProperty("LC_ManagedToolbar", true);
        populateToolbar(tb, tbDef);

        // Only call addToolBar if the toolbar is newly created or its dock area changed
        const Qt::ToolBarArea currentArea = m_appWindow->toolBarArea(tb);
        if (isNewlyCreated || (currentArea != tbDef.area && !tb->isFloating())) {
            m_appWindow->addToolBar(tbDef.area, tb);
        }

        tb->setVisible(tbDef.visible);
        activeToolbars.insert(tb);
    }

    for (auto* tb : m_appWindow->findChildren<QToolBar*>()) {
        if (tb != nullptr && tb->property("LC_ManagedToolbar").toBool()) {
            if (!activeToolbars.contains(tb) && tb != m_appWindow->getPenToolBar() &&
                tb != m_appWindow->getSnapToolBar()) {
                m_appWindow->removeToolBar(tb);
                tb->deleteLater();
            }
        }
    }

    if (m_appWindow != nullptr) {
        m_appWindow->updateToolbarsIconSize();
    }
}

QMenu* LC_NavigationControlsCreator::createMainWindowPopupMenu() const {
    if (m_appWindow == nullptr) {
        return nullptr;
    }

    auto* menu = new QMenu(tr("Context"), m_appWindow);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto addAct = [this, menu](const QString& name) {
        auto* act = getAction(name);
        if (act != nullptr) {
            menu->addAction(act);
        }
    };

    addAct("Fullscreen");
    addAct("MainMenu");
    addAct("ViewStatusBar");
    menu->addSeparator();

    if (m_specialMenuService != nullptr) {
        m_specialMenuService->bindMenu(LC_ActionNames::MenuToolbars, menu);
        m_specialMenuService->bindMenu(LC_ActionNames::MenuDockWidgets, menu);
        m_specialMenuService->bindMenu(LC_ActionNames::MenuCadToolbars, menu);
        m_specialMenuService->bindMenu(LC_ActionNames::MenuCadDockWidgets, menu);
    }

    return menu;
}
