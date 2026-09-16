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
#include "lc_action_naming_service_interface.h"
#include "lc_special_menu_service_interface.h"

LC_MenuBuilderBase::LC_MenuBuilderBase(LC_SpecialMenuServiceInterface* specialMenuService)
    : m_specialMenuService(specialMenuService) {
}

void LC_MenuBuilderBase::setSpecialMenuService(LC_SpecialMenuServiceInterface* service) {
    m_specialMenuService = service;
}

LC_SpecialMenuServiceInterface* LC_MenuBuilderBase::getSpecialMenuService() const {
    return m_specialMenuService;
}

void LC_MenuBuilderBase::populateMenuRecursive(QMenu* parentMenu, const QList<ActionNode>& nodes,const LC_ActionNamingServiceInterface* naming, bool allowTearOff) {
    if (parentMenu == nullptr) {
        return;
    }

    for (const auto& node : nodes) {
        if (node.type == ActionNodeType::Separator) {
            parentMenu->addSeparator();
        } else if (node.type == ActionNodeType::Action) {
            if (!shouldIncludeNode(node)) {
                continue;
            }

            bool handled = false;
            if (m_specialMenuService != nullptr) {
                handled = m_specialMenuService->bindMenu(node.actionName, parentMenu);
            }
            if (!handled) {
                appendActionItem(parentMenu, node);
                }
            }
        else if (node.type == ActionNodeType::Group) {
            const QString subMenuTitle = (naming != nullptr)
                ? naming->displayName(node.groupTitle, /*stripAmpersand=*/false)
                : node.groupTitle;

            auto* subMenu = parentMenu->addMenu(subMenuTitle);
            if (subMenu != nullptr) {
                subMenu->setTearOffEnabled(allowTearOff);

                QString iconPath = node.groupIcon;
                if (iconPath.isEmpty() && naming != nullptr) {
                    iconPath = naming->iconPath(node.groupTitle);
                }
                if (!iconPath.isEmpty()) {
                    subMenu->setIcon(QIcon(iconPath));
                }

                populateMenuRecursive(subMenu, node.children, naming, allowTearOff);
                if (subMenu->isEmpty()) {
                    delete subMenu;
                }
            }
        }
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
    QAction* act = nullptr;
    if (m_specialMenuService != nullptr) {
        act = m_specialMenuService->getSpecialAction(node.actionName);
    }
    if (act == nullptr) {
        act = getAction(node.actionName);
    }
    if (act != nullptr && parentMenu != nullptr) {
        parentMenu->addAction(act);
    }
}
