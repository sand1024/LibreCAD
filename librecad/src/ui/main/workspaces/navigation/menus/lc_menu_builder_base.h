
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

#ifndef LC_MENU_BUILDER_BASE_H
#define LC_MENU_BUILDER_BASE_H

#include <QList>
#include <QString>

class LC_ActionGroupManager;
class QAction;
class QMenu;
struct ActionNode;
class LC_SpecialMenuServiceInterface;

class LC_MenuBuilderBase {
public:
    explicit LC_MenuBuilderBase(LC_ActionGroupManager* actionGroupManager, LC_SpecialMenuServiceInterface* specialMenuService = nullptr);
    virtual ~LC_MenuBuilderBase() = default;

    void setSpecialMenuService(LC_SpecialMenuServiceInterface* service);
    LC_SpecialMenuServiceInterface* getSpecialMenuService() const;

    virtual QAction* getAction(const QString& key) const = 0;

protected:
    void populateMenuRecursive(QMenu* parentMenu, const QList<ActionNode>& nodes, bool allowTearOff = false);
    bool hasAnyValidActions(const QList<ActionNode>& nodes) const;

    virtual bool shouldIncludeNode(const ActionNode& node) const;
    virtual void appendActionItem(QMenu* parentMenu, const ActionNode& node);


    LC_SpecialMenuServiceInterface* m_specialMenuService{nullptr};
    LC_ActionGroupManager* m_actionGroupManager {nullptr};
};

#endif
