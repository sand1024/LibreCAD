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
#include <qwidgetaction.h>

#include "lc_action_group_manager.h"
#include "lc_cad_tool_matrix_dock_widget.h"
#include "lc_default_navigation_layout_builder.h"
#include "lc_repository_menu_bar_and_toolbars.h"
#include "lc_settings_app_state.h"
#include "lc_settings_paths.h"
#include "lc_settings_startup.h"
#include "lc_special_menu_service_interface.h"
#include "lc_wait_cursor_guard.h"
#include "qc_applicationwindow.h"
#include "qg_graphicview.h"
#include "qg_pentoolbar.h"
#include "qg_snaptoolbar.h"

class QWidgetAction;

namespace {
    inline const LC_SettingsGroupBase Group("CustomToolbarsVisibility");
    inline const LC_Setting<QString> o_InvisibleCustomToolbars(&Group, "InvisibleCustomToolbars", "");
    inline const char* TOOLBAR_NAMES_SEPARATOR = "{_lc_names_separator_}";

    QStringList getInvisibleCustomToolbars() {
        QString packed = o_InvisibleCustomToolbars;
        return packed.split(TOOLBAR_NAMES_SEPARATOR, Qt::SkipEmptyParts);
    }

    void updateInvisibleCustomToolbars(const QString& toolbarName, bool add) {
        QStringList list = getInvisibleCustomToolbars();
        if (add) {
            list.push_back(toolbarName);
        }
        else {
            list.removeAll(toolbarName);
        }
        QString value;
        if (!list.isEmpty()) {
            value = list.join(TOOLBAR_NAMES_SEPARATOR);
        }
        o_InvisibleCustomToolbars = value;
    }
}

QToolBar* LC_NavigationControlsCreator::findExistingToolbar(const QString& name, const QString& resolvedTitle) const {
    if (m_appWindow == nullptr || name.isEmpty()) {
        return nullptr;
    }

    const QString lowerName = name.toLower();
    const bool isCadToolbar = lowerName.startsWith("tb_cad_") || lowerName.startsWith("cad_");

    QStringList candidateNames;
    candidateNames << lowerName;
    if (!lowerName.endsWith("_toolbar")) {
        candidateNames << (lowerName + "_toolbar");
    }

    if (lowerName.startsWith("tb_s_")) {
        const QString base = lowerName.mid(5);
        candidateNames << base << (base + "_toolbar");
    }
    else if (lowerName.startsWith("tb_cad_")) {
        const QString base = lowerName.mid(7);
        candidateNames << ("cad_" + base) << ("cad_" + base + "_toolbar");
    }

    const auto toolbars = m_appWindow->findChildren<QToolBar*>();

    // 1. Primary search: exact objectName match
    for (auto* tb : toolbars) {
        if (tb == nullptr) {
            continue;
        }
        // Exclude toolbars hosted inside QDockWidgets to prevent reparenting them
        if (tb->parentWidget() != m_appWindow && tb->parent() != m_appWindow) {
            continue;
        }

        const QString objName = tb->objectName().toLower();
        if (candidateNames.contains(objName)) {
            return tb;
        }
    }

    // 2. Fallback search by title
    if (!resolvedTitle.isEmpty()) {
        for (auto* tb : toolbars) {
            if (tb == nullptr) {
                continue;
            }
            if (tb->parentWidget() != m_appWindow && tb->parent() != m_appWindow) {
                continue;
            }

            const QString winTitle = tb->windowTitle();
            if (winTitle.isEmpty()) {
                continue;
            }

            if (winTitle == name || winTitle == resolvedTitle) {
                const QString objName = tb->objectName().toLower();
                const bool candidateIsCad = objName.startsWith("tb_cad_") || objName.startsWith("cad_");

                if (isCadToolbar == candidateIsCad) {
                    return tb;
                }
            }
        }
    }

    return nullptr;
}

LC_NavigationControlsCreator::LC_NavigationControlsCreator(LC_RepositoryMenuBarAndToolbars* repository, QC_ApplicationWindow* appWin,
                                                           LC_ActionGroupManager* actionGroupManager,
                                                           LC_SpecialMenuServiceInterface* specialMenuService)
    : LC_MenuBuilderBase(actionGroupManager, specialMenuService), m_appWindow{appWin}, m_repository(repository) {
    const QString baseFolder = CFG_Paths::o_OtherSettingsDir;
}

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
        updateInvisibleCustomToolbars(toolbarName, !visible);
        // LC_ERR << "TB_Visible " << toolbarName << (visible ? " Yes" : " no");
    }
}

void LC_NavigationControlsCreator::applyActiveLayoutScheme() {
    LC_WaitCursorGuard guard;
    const QString activeKey = CFG_AppState::o_ActiveNavigationLayoutScheme;
    NavigationLayoutConfig config;

    if (m_repository->isDefaultKey(activeKey) || activeKey.isEmpty() || m_repository == nullptr || !m_repository->loadByKey(
        activeKey, config)) {
        auto* actionFactory = (m_appWindow != nullptr) ? m_appWindow->getActionFactory() : nullptr;
        config = LC_DefaultNavigationLayoutBuilder::createDefaultConfig(actionFactory, m_actionGroupManager);
    }

    // Only apply initial default visibility if there is no saved state to restore
    const bool applyInitialVisibility = !hasSavedWidgetsState();
    applyMenusToolbarsScheme(config, applyInitialVisibility);
}

QAction* LC_NavigationControlsCreator::getAction(const QString& key) const {
    return m_appWindow->getAction(key);
}

void LC_NavigationControlsCreator::applyMenusToolbarsScheme(const NavigationLayoutConfig& config, bool applyInitialVisibility) {
    if (m_appWindow == nullptr) {
        return;
    }

    applyMenuBar(config);
    applyCadDockWidgets(config, applyInitialVisibility);
    applyToolbars(config, applyInitialVisibility);
}

void LC_NavigationControlsCreator::applyMenuBar(const NavigationLayoutConfig& config) {
    QMenuBar* menuBar = m_appWindow->menuBar();
    if (menuBar == nullptr) {
        return;
    }

    menuBar->clear();

    using namespace CFG_Appearance;
    const auto menuType = o_MainMenuType;
    const QList<ActionNode>& activeMenuNodes = (menuType == MENU_Minimal)
                                                   ? config.menuMinimal
                                                   : (menuType == MENU_Compact)
                                                   ? config.menuCompact
                                                   : config.menuExtended;

    const bool showIcons = o_MainMenuTopLevelIconsOnly;
    const bool allowTearOff = o_AllowMenusTearOff;

    m_pluginsMenu = nullptr;

    for (const auto& node : activeMenuNodes) {
        // --- Mount Persistent Special Menus Directly ---
        if (node.actionName == LC_ActionNames::MenuPlugins) {
            QMenu* pluginsMenu = m_appWindow->getPluginsMenu();
            if (pluginsMenu != nullptr) {
                if (!pluginsMenu->icon().isNull() || !showIcons) {
                    // Already configured
                }
                else {
                    pluginsMenu->setIcon(QIcon(":/icons/plugin.lci"));
                }
                menuBar->addMenu(pluginsMenu);
            }
            continue;
        }

        // --- Standard Declarative Menus ---
        const QString rawTitle = !node.groupTitle.isEmpty() ? node.groupTitle : node.actionName;
        const QString title = (m_actionGroupManager != nullptr)
                                  ? m_actionGroupManager->displayName(rawTitle, /*stripAmpersand=*/false)
                                  : rawTitle;

        QString iconPath;
        if (showIcons) {
            iconPath = node.groupIcon;

            // 2. Fallback: Query LC_ActionGroupManager using the canonical token or raw title
            if (iconPath.isEmpty() && m_actionGroupManager != nullptr) {
                iconPath = m_actionGroupManager->iconPath(rawTitle);
            }
        }

        auto* topMenu = new QMenu(title, menuBar);
        if (!iconPath.isEmpty()) {
            topMenu->setIcon(QIcon(iconPath));
            topMenu->menuAction()->setIcon(QIcon(iconPath));
            topMenu->menuAction()->setText(""); // Displays only the icon
            topMenu->menuAction()->setToolTip(title); // Tooltip shows the full localized name
        }

        menuBar->addMenu(topMenu);

        if (topMenu != nullptr) {
            topMenu->setTearOffEnabled(allowTearOff);
            populateMenuRecursive(topMenu, node.children);
        }
    }

    menuBar->setVisible(o_MainMenuVisible);
}

void LC_NavigationControlsCreator::applyCadDockWidgets(const NavigationLayoutConfig& config, bool applyInitialVisibility) {
    // 1. Update CAD Tools Matrix (find by type or objectName)
    auto* megaDock = m_appWindow->findChild<LC_CADToolMatrixDockWidget*>("dock_cad_mega");
    if (megaDock == nullptr) {
        megaDock = m_appWindow->findChild<LC_CADToolMatrixDockWidget*>();
    }

    if (megaDock != nullptr) {
        for (const auto& tbDef : config.toolbars) {
            if (tbDef.kind == ToolbarKind::CadMatrix) {
                megaDock->updateActionsFromNodes(tbDef.nodes, m_actionGroupManager);
                if (applyInitialVisibility) {
                    megaDock->setVisible(tbDef.visible);
                }
                break;
            }
        }
    }

    // 2. Update individual CAD dock widgets
    const auto allCadDocks = m_appWindow->findChildren<LC_CADDockWidget*>();

    for (const auto& tbDef : config.toolbars) {
        if (tbDef.kind != ToolbarKind::CadDockWidget) {
            continue;
        }

        const QString targetName = tbDef.name.toLower(); // "dock_cad_<group>"
        QString shortToken = targetName;
        if (shortToken.startsWith("dock_cad_")) {
            shortToken = shortToken.mid(9);
        }
        else if (shortToken.startsWith("dock_")) {
            shortToken = shortToken.mid(5);
        }

        LC_CADDockWidget* cadDock = nullptr;
        for (auto* dock : allCadDocks) {
            if (dock == nullptr) {
                continue;
            }
            const QString objName = dock->objectName().toLower();
            if (objName == targetName || objName == ("dock_cad_" + shortToken) || objName == ("dock_" + shortToken)) {
                cadDock = dock;
                break;
            }
        }

        if (cadDock != nullptr) {
            cadDock->updateActionsFromNodes(tbDef.nodes, m_actionGroupManager);
            if (applyInitialVisibility) {
                cadDock->setVisible(tbDef.visible);
            }
        }
    }
}

void LC_NavigationControlsCreator::populateToolbarNodes(QToolBar* tb, const QList<ActionNode>& nodes) {
    if (tb == nullptr) {
        return;
    }

    const QSize childButtonIconSize = (m_appWindow != nullptr) ? m_appWindow->iconSize() : QSize(24, 24);
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

            const QString cleanTitle = m_actionGroupManager->displayName(node.groupTitle, /*stripAmpersand=*/true);
            toolBtn->setText(cleanTitle);
            toolBtn->setToolTip(cleanTitle);

            QString iconPath = node.groupIcon;
            if (iconPath.isEmpty()) {
                iconPath = m_actionGroupManager->iconPath(node.groupTitle);
            }
            if (!iconPath.isEmpty()) {
                toolBtn->setIcon(QIcon(iconPath));
            }

            auto* menu = new QMenu(toolBtn);
            populateMenuRecursive(menu, node.children, /*allowTearOff=*/false);
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

void LC_NavigationControlsCreator::clearToolbar(QToolBar* tb) {
    if (tb == nullptr) {
        return;
    }

    const bool isPenTb = (m_appWindow != nullptr && tb == m_appWindow->getPenToolBar());
    const int startIndex = isPenTb ? 3 : 0;

    const QList<QAction*> actions = tb->actions();
    for (int i = actions.size() - 1; i >= startIndex; --i) {
        QAction* act = actions.at(i);
        if (act != nullptr) {
            tb->removeAction(act);
        }
    }
}

bool LC_NavigationControlsCreator::hasSavedWidgetsState() const {
    LC_GROUP("Geometry");
    const QString state = LC_GET_STR("StateOfWidgets", "");
    LC_GROUP_END();
    const bool hasState = !state.trimmed().isEmpty();
    LC_ERR << "[DEBUG_NAV] hasSavedWidgetsState():" << hasState << "state length:" << state.length();
    return hasState;
}

void LC_NavigationControlsCreator::populateToolbar(QToolBar* tb, const ToolbarDef& tbDef) {
    if (tb == nullptr) {
        return;
    }

    if (tbDef.kind == ToolbarKind::Host) {
        return;
    }

    clearToolbar(tb);

    if (m_showToolbarTooltips) {
        const QString toolbarTitle = (m_actionGroupManager != nullptr) ? m_actionGroupManager->displayName(tbDef.name, true) : tbDef.name;
        tb->setToolTip(tr("Toolbar: %1").arg(toolbarTitle));
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
    auto* penTb = (m_appWindow != nullptr) ? m_appWindow->getPenToolBar() : nullptr;
    if (penTb == nullptr) {
        return;
    }

    const Qt::ToolBarArea currentArea = m_appWindow->toolBarArea(penTb);
    if (currentArea != tbDef.area && !penTb->isFloating()) {
        m_appWindow->addToolBar(tbDef.area, penTb);
    }

    // penTb->setVisible(tbDef.visible);
    penTb->toggleViewAction()->setIcon(QIcon(":/icons/pen_apply.lci"));

    populateToolbar(penTb, tbDef);
}

void LC_NavigationControlsCreator::applyToolbars(const NavigationLayoutConfig& config, bool applyInitialVisibility) {
    if (m_appWindow == nullptr) {
        return;
    }

    QStringList invisibleToolbars = getInvisibleCustomToolbars();

    // Collect existing custom toolbars to clean up any removed by scheme modifications
    QSet<QString> activeToolbarNames;

    for (const auto& tbDef : config.toolbars) {
        // Skip dock widgets and CAD matrix (handled by applyCadDockWidgets)
        if (tbDef.kind == ToolbarKind::CadDockWidget || tbDef.kind == ToolbarKind::CadMatrix) {
            continue;
        }

        if (tbDef.isActionDriven() && !hasAnyValidActions(tbDef.nodes)) {
            continue;
        }

       activeToolbarNames.insert(tbDef.name);

        QString toolbarTitle;
        if (tbDef.kind == ToolbarKind::Cad) {
            QString groupKey = tbDef.name;
            if (groupKey.startsWith("tb_cad_")) {
                groupKey = groupKey.mid(7);
            }
            else if (groupKey.startsWith("cad_")) {
                groupKey = groupKey.mid(4);
            }
            const auto* group = (m_actionGroupManager != nullptr) ? m_actionGroupManager->getActionGroup(groupKey) : nullptr;
            toolbarTitle = (group != nullptr) ? group->cleanTitle() : tbDef.name;
        }
        else {
            QString displayNameKey = tbDef.name;
            if (displayNameKey.startsWith("tb_s_")) {
                displayNameKey = displayNameKey.mid(5);
            }
            else if (displayNameKey.startsWith("tb_c_")) {
                displayNameKey = displayNameKey.mid(5);
            }
            toolbarTitle = (m_actionGroupManager != nullptr)
                               ? m_actionGroupManager->displayName(displayNameKey, /*stripAmpersand=*/true)
                               : tbDef.name;
        }

        QToolBar* tb = findExistingToolbar(tbDef.name, toolbarTitle);
        const bool isNew = (tb == nullptr);

        if (isNew) {
            tb = new QToolBar(toolbarTitle, m_appWindow);
            const QString objectName = tbDef.name.endsWith("_toolbar") ? tbDef.name : (tbDef.name + "_toolbar");
            tb->setObjectName(objectName);

            m_appWindow->addToolBar(tbDef.area, tb);
            if (tbDef.lineBreak) {
                m_appWindow->insertToolBarBreak(tb);
            }

            if (tbDef.kind == ToolbarKind::Custom) {
                const bool visible = !invisibleToolbars.contains(objectName);
                tb->setVisible(visible);
                connect(tb, &QToolBar::visibilityChanged, this, &LC_NavigationControlsCreator::onCustomToolbarVisibilityChanged);
            }
            else /*if (applyInitialVisibility)*/ {
                LC_ERR << "[DEBUG_NAV] applyInitialVisibility TRUE -> setting" << objectName << "visible:" << tbDef.visible;
                tb->setVisible(tbDef.visible);
            }
            // else {
            //     LC_ERR << "[DEBUG_NAV] applyInitialVisibility FALSE -> leaving" << objectName << "untouched";
            // }
        }
        else {
            tb->setWindowTitle(toolbarTitle);

            // Dock pre-instantiated special toolbars (pen, snap, tool_options) into their designated area and row
            if (m_appWindow->toolBarArea(tb) == Qt::NoToolBarArea) {
                m_appWindow->addToolBar(tbDef.area, tb);
                if (tbDef.lineBreak) {
                    m_appWindow->insertToolBarBreak(tb);
                }
                if (applyInitialVisibility) {
                    tb->setVisible(tbDef.visible);
                }
            }
        }

        const int groupProp = (tbDef.kind == ToolbarKind::Cad) ? 2 : 1;
        tb->setProperty("_group", groupProp);
        tb->setProperty("_lc_toolbar_name", tbDef.name);

        if (tbDef.kind == ToolbarKind::Custom) {
            tb->setProperty("_is_custom_toolbar", true);
        }

        populateToolbar(tb, tbDef);
    }

    const auto allToolbars = m_appWindow->findChildren<QToolBar*>();
    for (auto* tb : allToolbars) {
        if (tb != nullptr && tb->property("_is_custom_toolbar").toBool()) {
            const QString name = tb->property("_lc_toolbar_name").toString();
            if (!activeToolbarNames.contains(name)) {
                m_appWindow->removeToolBar(tb);
                tb->deleteLater();
            }
        }
    }

    if (m_appWindow != nullptr) {
        m_appWindow->updateToolbarsIconSize();
        updateToolbarsTooltips();
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

void LC_NavigationControlsCreator::updateToolbarsTooltips(const bool showTooltips) {
    if (m_appWindow == nullptr) {
        return;
    }

    const auto toolbars = m_appWindow->findChildren<QToolBar*>();
    for (auto* tb : toolbars) {
        if (tb == nullptr) {
            continue;
        }

        if (showTooltips) {
            tb->setToolTip(QObject::tr("Toolbar: %1").arg(tb->windowTitle()));
        }
        else {
            tb->setToolTip(QString());
        }
    }
}

void LC_NavigationControlsCreator::updateToolbarsTooltips() {
    const bool show = CFG_Startup::o_ShowToolbarsTooltip;
    updateToolbarsTooltips(show);
}

void LC_NavigationControlsCreator::resetToolbarsLayout(const NavigationLayoutConfig& config) {
    if (m_appWindow == nullptr) {
        return;
    }

    for (const auto& tbDef : config.toolbars) {
        if (tbDef.kind == ToolbarKind::CadDockWidget || tbDef.kind == ToolbarKind::CadMatrix) {
            continue;
        }

        QString toolbarTitle;
        if (tbDef.kind == ToolbarKind::Cad) {
            QString groupKey = tbDef.name;
            if (groupKey.startsWith("tb_cad_")) {
                groupKey = groupKey.mid(7);
            }
            else if (groupKey.startsWith("cad_")) {
                groupKey = groupKey.mid(4);
            }
            const auto* group = (m_actionGroupManager != nullptr) ? m_actionGroupManager->getActionGroup(groupKey) : nullptr;
            toolbarTitle = (group != nullptr) ? group->cleanTitle() : tbDef.name;
        }
        else {
            QString displayNameKey = tbDef.name;
            if (displayNameKey.startsWith("tb_s_")) {
                displayNameKey = displayNameKey.mid(5);
            }
            else if (displayNameKey.startsWith("tb_c_")) {
                displayNameKey = displayNameKey.mid(5);
            }
            toolbarTitle = (m_actionGroupManager != nullptr)
                               ? m_actionGroupManager->displayName(displayNameKey, /*stripAmpersand=*/true)
                               : tbDef.name;
        }

        QToolBar* tb = findExistingToolbar(tbDef.name, toolbarTitle);
        if (tb != nullptr) {
            m_appWindow->removeToolBar(tb);
        }
    }
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
