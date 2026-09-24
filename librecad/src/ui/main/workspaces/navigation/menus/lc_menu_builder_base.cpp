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

#include "lc_menu_builder_base.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include "lc_action_node.h"
// #include "lc_actions_naming_utils.h"
#include "lc_action_factory.h"
#include "lc_action_group_manager.h"
#include "lc_action_naming_service_interface.h"
#include "lc_special_menu_service_interface.h"

LC_MenuBuilderBase::LC_MenuBuilderBase(LC_ActionGroupManager* actionGroupManager, LC_SpecialMenuServiceInterface* specialMenuService)
    : m_actionGroupManager(actionGroupManager), m_specialMenuService(specialMenuService) {
}

void LC_MenuBuilderBase::setSpecialMenuService(LC_SpecialMenuServiceInterface* service) {
    m_specialMenuService = service;
}

LC_SpecialMenuServiceInterface* LC_MenuBuilderBase::getSpecialMenuService() const {
    return m_specialMenuService;
}

void LC_MenuBuilderBase::populateMenuRecursive(QMenu* parentMenu, const QList<ActionNode>& nodes, bool allowTearOff) {
    if (parentMenu == nullptr) {
        return;
    }

    bool effectiveTearOff = allowTearOff && parentMenu->isTearOffEnabled();

    for (const auto& node : nodes) {
        if (node.type == ActionNodeType::Separator) {
            parentMenu->addSeparator();
            continue;
        }
        if (!shouldIncludeNode(node)) {
            continue;
        }
        if (node.actionName.startsWith(LC_ActionNames::PrefixSpecialMenu)) {
            if (m_specialMenuService != nullptr) {
                m_specialMenuService->bindMenu(node.actionName, parentMenu);
            }
            continue;
        }
        if (node.type == ActionNodeType::Group || !node.children.isEmpty()) {
            const QString rawTitle = !node.groupTitle.isEmpty() ? node.groupTitle : node.actionName;
            const QString title = (m_actionGroupManager != nullptr)
                ? m_actionGroupManager->displayName(rawTitle)
                : rawTitle;

            auto* subMenu = parentMenu->addMenu(title);
            if (subMenu != nullptr) {
                QString iconPath = node.groupIcon;
                if (iconPath.isEmpty() && m_actionGroupManager != nullptr) {
                    iconPath = m_actionGroupManager->iconPath(rawTitle);
                }
                if (!iconPath.isEmpty()) {
                    subMenu->setIcon(QIcon(iconPath));
                }

                subMenu->setTearOffEnabled(effectiveTearOff);
                populateMenuRecursive(subMenu, node.children);
            }
            continue;
        }

        appendActionItem(parentMenu, node);
    }
}

bool LC_MenuBuilderBase::hasAnyValidActions(const QList<ActionNode>& nodes) const {
    for (const auto& node : nodes) {
        if (node.type == ActionNodeType::Action && getAction(node.actionName) != nullptr) {
            return true;
        }
        if (node.type == ActionNodeType::Group && hasAnyValidActions(node.children)) {
            return true;
        }
    }
    return false;
}

bool LC_MenuBuilderBase::shouldIncludeNode(const ActionNode& node) const {
    Q_UNUSED(node);
    return true;
}

void LC_MenuBuilderBase::appendActionItem(QMenu* parentMenu, const ActionNode& node) {
    if (parentMenu == nullptr) {
        return;
    }

    QAction* act = nullptr;
    if (node.actionName.startsWith(LC_ActionNames::PrefixSpecialAction)) {
        if (m_specialMenuService != nullptr) {
            act = m_specialMenuService->getSpecialAction(node.actionName);
        }
    }
    else {
        act = getAction(node.actionName);
    }

    if (act == nullptr) {
        return;
    }

    // 1. Declarative Local Caption Override via Synchronized Proxy
    if (!node.customLabel.isEmpty()) {
        auto* proxy = parentMenu->addAction(node.customLabel);
        proxy->setIcon(act->icon());
        proxy->setCheckable(act->isCheckable());
        proxy->setChecked(act->isChecked());
        proxy->setEnabled(act->isEnabled());
        proxy->setVisible(act->isVisible());

        // Keep proxy state synchronized with source action
        QObject::connect(act, &QAction::toggled, proxy, &QAction::setChecked);
        QObject::connect(act, &QAction::changed, proxy, [act, proxy]() {
            proxy->setEnabled(act->isEnabled());
            proxy->setVisible(act->isVisible());
            proxy->setIcon(act->icon());
        });
        QObject::connect(proxy, &QAction::triggered, act, &QAction::trigger);
    }
    else {
        // 2. Standard Action Insertion
        parentMenu->addAction(act);
    }
}
