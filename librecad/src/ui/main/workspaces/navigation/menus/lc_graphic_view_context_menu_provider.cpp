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

#include "lc_graphic_view_context_menu_provider.h"

#include <QMouseEvent>
#include "lc_actioncontext.h"
// #include "lc_actions_naming_utils.h"
#include "lc_action_group_manager.h"
#include "lc_default_context_menus_builder.h"
#include "lc_menu_activator.h"
#include "lc_preset_manager_menus.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_state.h"
#include "lc_settings_paths.h"
#include "lc_wait_cursor_guard.h"
#include "qc_applicationwindow.h"
#include "qg_graphicview.h"
#include "rs_document.h"
#include "rs_ellipse.h"
#include "rs_graphic.h"
#include "rs_insert.h"
#include "rs_selection.h"

LC_GraphicViewContextMenuProvider::LC_GraphicViewContextMenuProvider(LC_RepositoryGraphicViewContextMenus* repository,
                                                                     LC_ActionFactory* actionFactory,
                                                                     LC_ActionGroupManager* actionGroupManager,
                                                                     LC_SpecialMenuServiceInterface* specialMenuService)
    : LC_MenuBuilderBase(actionGroupManager, specialMenuService), m_actionFactory(actionFactory), m_menusRepository(repository) {
    const QString baseFolder = CFG_Paths::o_OtherSettingsDir;
    m_menusRepository->migrateLegacyMenusIfNeeded();
    loadActiveScheme();
}

LC_GraphicViewContextMenuProvider::~LC_GraphicViewContextMenuProvider() {
    qDeleteAll(m_menuActivators);
    m_menuActivators.clear();
}

void LC_GraphicViewContextMenuProvider::loadActiveScheme() {
    LC_WaitCursorGuard guard;
    const QString activeKey = CFG_AppState::o_ActiveContextMenusScheme;
    ContextMenusConfig config;

    if (activeKey == LC_PresetManagerMenus::THEME_EXTENDED_KEY) {
        config = LC_DefaultContextMenusBuilder::createExtendedConfig(m_actionFactory, m_actionGroupManager);
    }
    else if (activeKey == CFG_AppState::DEFAULT_THEME_KEY || activeKey.isEmpty() || !m_menusRepository->loadByKey(activeKey, config)) {
        config = LC_DefaultContextMenusBuilder::createDefaultConfig(m_actionFactory, m_actionGroupManager);
    }
    applyCustomMenusScheme(config);
}

void LC_GraphicViewContextMenuProvider::applyCustomMenusScheme(const ContextMenusConfig& config) {
    m_activeConfig = config;

    qDeleteAll(m_menuActivators);
    m_menuActivators.clear();

    for (const auto& mDef : m_activeConfig.menus) {
        auto* actCopy = mDef.activator.getCopy();
        if (actCopy != nullptr) {
            actCopy->setMenuName(mDef.name);
            m_menuActivators.push_back(actCopy);
        }
    }
}

const ContextMenuDef* LC_GraphicViewContextMenuProvider::findMatchingMenu(const QMouseEvent* event, const RS_Entity* entity) const {
    const LC_MenuActivator* activatorForEntityType{nullptr};
    const LC_MenuActivator* activatorForAnyEntity{nullptr};
    const LC_MenuActivator* activatorForEitherEntity{nullptr};
    const LC_MenuActivator* activatorForNoEntity{nullptr};
    const bool hasEntity = (entity != nullptr);

    RS2::EntityType entityType = RS2::EntityUnknown;
    if (hasEntity) {
        entityType = entity->rtti();
    }

    for (const auto* a : std::as_const(m_menuActivators)) {
        if (a == nullptr) {
            continue;
        }
        if (a->isEventApplicable(event)) {
            const RS2::EntityType activatorEntityType = a->getEntityType();
            if (a->isEntityRequired()) {
                if (hasEntity) {
                    if (activatorEntityType == entityType) {
                        activatorForEntityType = a;
                    } else if (activatorEntityType == RS2::EntityUnknown) {
                        activatorForAnyEntity = a;
                    } else if (activatorEntityType == RS2::EntityGraphic) {
                        activatorForEitherEntity = a;
                    }
                } else if (activatorEntityType == RS2::EntityGraphic) {
                    activatorForEitherEntity = a;
                }
            } else {
                if (hasEntity) {
                    continue;
                }
                activatorForNoEntity = a;
            }
        }
    }

    const LC_MenuActivator* chosen{nullptr};
    if (hasEntity) {
        chosen = (activatorForEntityType != nullptr) ? activatorForEntityType :
                 (activatorForAnyEntity != nullptr) ? activatorForAnyEntity : activatorForEitherEntity;
    } else {
        chosen = (activatorForNoEntity != nullptr) ? activatorForNoEntity : activatorForEitherEntity;
    }

    if (chosen != nullptr) {
        const QString targetName = chosen->getMenuName();
        for (const auto& mDef : m_activeConfig.menus) {
            if (mDef.name == targetName) {
                return &mDef;
            }
        }
    }

    // Fallback to default right-click if event matches right-click release
    if (isDefaultMenuInvokerEvent(event)) {
        const RS2::EntityType fallbackType = hasEntity ? RS2::EntityUnknown : RS2::EntityUnknown;
        const bool requireEnt = hasEntity;
        for (const auto& mDef : m_activeConfig.menus) {
            if (mDef.activator.getButtonType() == LC_MenuActivator::RIGHT &&
                mDef.activator.getEventType() == LC_MenuActivator::CLICK_RELEASE &&
                mDef.activator.isEntityRequired() == requireEnt &&
                !mDef.activator.hasKeys()) {
                return &mDef;
            }
        }
    }

    return nullptr;
}

QMenu* LC_GraphicViewContextMenuProvider::createContextMenu(QG_GraphicView* graphicView, RS_Entity* entity,
                                                           const RS_Vector& pos, const QMouseEvent* event) {
    if (graphicView == nullptr || event == nullptr) {
        return nullptr;
    }

    const ContextMenuDef* matchedDef = findMatchingMenu(event, entity);
    if (matchedDef == nullptr || matchedDef->nodes.isEmpty()) {
        return nullptr;
    }

    // If single action menu without submenus, trigger directly
    if (matchedDef->nodes.size() == 1 && matchedDef->nodes.first().type == ActionNodeType::Action) {
        QAction* directAction = getAction(matchedDef->nodes.first().actionName);
        if (directAction != nullptr && directAction->isEnabled()) {
            if (auto* ctx = graphicView->getActionContext()) {
                if (entity != nullptr) {
                    ctx->saveContextMenuActionContext(entity, pos, false);
                }
            }
            directAction->trigger();
            return nullptr;
        }
    }

    auto* ctxMenu = new QMenu(graphicView);
    ctxMenu->setAttribute(Qt::WA_DeleteOnClose);

    // Entity highlight while menu is visible
    if (entity != nullptr && !entity->isSelected() && graphicView->getDocument() != nullptr) {
        RS_Selection sel(graphicView->getDocument(), graphicView->getViewPort());
        sel.justSelect(entity, true);
        graphicView->redraw(RS2::RedrawDrawing);

        connect(ctxMenu, &QMenu::aboutToHide, this, [graphicView, entity]() {
            if (graphicView != nullptr && graphicView->getDocument() != nullptr) {
                RS_Selection sel(graphicView->getDocument(), graphicView->getViewPort());
                sel.justSelect(entity, false);
                graphicView->redraw();
            }
        });
    }

    // 1. Establish transient context state
    m_currentGraphicView = graphicView;
    m_currentEntity = entity;
    m_currentPos = pos;

    auto* actionContext = graphicView->getActionContext();
    m_hasSelection = (actionContext != nullptr && actionContext->hasSelection());
    m_undoAvailable = false;
    m_redoAvailable = false;
    if (actionContext != nullptr && actionContext->getDocument() != nullptr) {
        if (auto* doc = actionContext->getDocument()->getDocument()) {
            doc->collectUndoState(m_undoAvailable, m_redoAvailable);
        }
    }
    // 2. Delegate directly to the base class recursive menu builder
    populateMenuRecursive(ctxMenu, matchedDef->nodes, /*allowTearOff=*/false);

    // 3. Clear transient state
    m_currentGraphicView = nullptr;
    m_currentEntity = nullptr;

    return ctxMenu;
}

bool LC_GraphicViewContextMenuProvider::shouldIncludeNode(const ActionNode& node) const {
    const QString& name = node.actionName;

    if (name == LC_ActionNames::EditUndo && !m_undoAvailable) {
        return false;
    }
    if (name == LC_ActionNames::EditRedo && !m_redoAvailable) {
        return false;
    }
    if ((name == LC_ActionNames::EditKillAllActions || name == LC_ActionNames::DeselectAll) && !m_hasSelection) {
        return false;
    }

    if ((name == "DrawArcTangential" || name == "ModifyRevertDirection") &&
        m_currentEntity != nullptr && m_currentEntity->rtti() == RS2::EntityEllipse) {
        const auto* ellipse = static_cast<const RS_Ellipse*>(m_currentEntity);
        if (!ellipse->isEllipticArc()) {
            return false;
        }
    }

    return true;
}

void LC_GraphicViewContextMenuProvider::appendActionItem(QMenu* parentMenu, const ActionNode& node) {
    if (parentMenu == nullptr || m_currentGraphicView == nullptr) {
        return;
    }

    const QString& name = node.actionName;
    auto* actionContext = m_currentGraphicView->getActionContext();
    auto* entity = m_currentEntity;
    const auto pos = m_currentPos;
    auto* graphicView = m_currentGraphicView;

    // 1. Dynamic Recent Actions
    if (name == LC_ActionNames::MenuRecentActions) {
        auto recent = graphicView->getRecentActions();
        if (!recent.isEmpty()) {
            auto* firstAct = recent.first();
            auto* firstProxy = parentMenu->addAction(firstAct->icon(), firstAct->iconText());
            connect(firstProxy, &QAction::triggered, this, [actionContext, entity, pos, firstAct]() {
                if (actionContext != nullptr && entity != nullptr) {
                    actionContext->saveContextMenuActionContext(entity, pos, false);
                }
                firstAct->trigger();
            });

            auto* recentSub = parentMenu->addMenu(tr("Recent"));
            for (auto* a : recent) {
                auto* p = recentSub->addAction(a->icon(), a->iconText());
                connect(p, &QAction::triggered, this, [actionContext, entity, pos, a]() {
                    if (actionContext != nullptr && entity != nullptr) {
                        actionContext->saveContextMenuActionContext(entity, pos, false);
                    }
                    a->trigger();
                });
            }
            parentMenu->addSeparator();
        }
        return;
    }

    // 2. Dynamic Block Name
    if (name == LC_ActionNames::ActionEditBlock) {
        if (entity != nullptr && entity->rtti() == RS2::EntityInsert) {
            const auto* ins = static_cast<const RS_Insert*>(entity);
            const QString title = tr("Edit Block: %1").arg(ins->getName().left(40));
            auto* act = parentMenu->addAction(QIcon(":/icons/properties.lci"), title);
            connect(act, &QAction::triggered, this, [graphicView, entity]() {
                graphicView->launchEditProperty(entity);
            });
        }
        return;
    }

    // 3. Dynamic Layer Activation
    if (name == LC_ActionNames::ActionEntityLayerActivate) {
        if (entity != nullptr) {
            const RS_Graphic* graphic = graphicView->getGraphic(false);
            if (graphic != nullptr && entity->getLayer() != nullptr && graphic->getActiveLayer() != entity->getLayer()) {
                auto* act = getAction(LC_ActionNames::ActionEntityLayerActivate);
                if (act != nullptr) {
                    auto* proxy = parentMenu->addAction(act->icon(), act->iconText());
                    connect(proxy, &QAction::triggered, this, [actionContext, entity, pos, act]() {
                        if (actionContext != nullptr) {
                            actionContext->saveContextMenuActionContext(entity, pos, false);
                        }
                        act->trigger();
                    });
                }
            }
        }
        return;
    }

    // // 4. Fallback Workspaces Menu
    // if (name == LC_ActionNames::MenuWorkspacesRescue) {
    //     if (!CFG_Appearance::o_MainMenuVisible && m_specialMenuService != nullptr) {
    //         auto* ws = parentMenu->addMenu(QIcon(":/icons/workspace.lci"), tr("Workspaces"));
    //         ws->setTearOffEnabled(false);
    //         m_specialMenuService->bindMenu(LC_ActionNames::MenuDockWidgets, ws);
    //         m_specialMenuService->bindMenu(LC_ActionNames::MenuToolbars, ws);
    //         m_specialMenuService->bindMenu(LC_ActionNames::MenuWorkspacesList, ws);
    //     }
    //     return;
    // }

    // 5. Standard action with context saving proxy
    QAction* realAction = nullptr;
    if (m_specialMenuService != nullptr) {
        realAction = m_specialMenuService->getSpecialAction(name);
    }
    if (realAction == nullptr) {
        realAction = getAction(name);
    }

    if (realAction != nullptr && realAction->isEnabled()) {
        auto* proxy = parentMenu->addAction(realAction->icon(), realAction->iconText());
        proxy->setToolTip(realAction->toolTip());
        proxy->setCheckable(realAction->isCheckable());
        proxy->setChecked(realAction->isChecked());

        connect(proxy, &QAction::triggered, this, [actionContext, entity, pos, realAction]() {
            if (actionContext != nullptr && entity != nullptr) {
                actionContext->saveContextMenuActionContext(entity, pos, false);
            }
            realAction->trigger();
        });
    }
}

bool LC_GraphicViewContextMenuProvider::isDefaultMenuInvokerEvent(const QMouseEvent* event) const {
    if (event == nullptr) {
        return false;
    }
    return event->modifiers() == Qt::NoModifier && event->button() == Qt::RightButton && event->type() == QEvent::MouseButtonRelease;
}

QAction* LC_GraphicViewContextMenuProvider::getAction(const QString& key) const {
    auto appWin = QC_ApplicationWindow::getAppWindow();
    if (appWin != nullptr) {
        return appWin->getAction(key);
    }
    return nullptr;
}
