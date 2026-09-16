/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "lc_action_factory_base.h"

#include <QActionGroup>

#include "lc_action.h"
#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "shortcuts/lc_shortcut_info.h"

#include "qc_applicationwindow.h"
#include "qg_actionhandler.h"

LC_ActionFactoryBase::LC_ActionFactoryBase(QC_ApplicationWindow* parent, QG_ActionHandler* actionHandler) : QObject(parent),
                                                                                                        LC_AppWindowAware(parent), m_actionHandler(actionHandler) {
}

QAction* LC_ActionFactoryBase::createAction_MW(const char* name, void (QC_ApplicationWindow::*slotPtr)(),
                                               void (QC_ApplicationWindow::*slotBoolPtr)(bool), const QString& text, const char* iconName,
                                               QActionGroup* parent, QMap<QString, QAction*>& actionsMap,
                                               const bool useToggled, const QString& description) const {
    QAction* action = justCreateAction(actionsMap, name, text, iconName, parent, description);
    if (slotPtr != nullptr) {
        if (useToggled) {
            connect(action, &QAction::toggled, m_appWin, slotPtr);
        }
        else {
            connect(action, &QAction::triggered, m_appWin, slotPtr);
        }
    }
    else if (slotBoolPtr != nullptr) {
        if (useToggled) {
            connect(action, &QAction::toggled, m_appWin, slotBoolPtr);
        }
        else {
            connect(action, &QAction::triggered, m_appWin, slotBoolPtr);
        }
    }
    return action;
}

QAction* LC_ActionFactoryBase::createAction_AH(const char* name, RS2::ActionType actionType, const QString& text, const char* iconName,
                                               QActionGroup* parent, QMap<QString, QAction*>& actionsMap,
                                               const QString& description) const {
    QAction* action = justCreateAction(actionsMap, name, text, iconName, parent, description);
    QG_ActionHandler* capturedHandler = m_actionHandler;
    connect(action, &QAction::triggered, capturedHandler, [ capturedHandler, actionType](bool) {
        // fixme - sand - simplify by using data() on QAction and sender()
        // LC_ERR << " ++ captured action handler "<<   capturedHandler;
        capturedHandler->setCurrentAction(actionType);
    });
    LC_ActionGroupManager::associateQActionWithActionType(action, actionType);
    return action;
}

QAction* LC_ActionFactoryBase::justCreateAction(QMap<QString, QAction*>& actionsMap, const char* name, const QString& text,
                                                const char* iconName, QActionGroup* parent,
                                                const QString& description) const {
    auto* action = new LC_Action(text, parent);
    if (iconName != nullptr) {
        action->setIcon(QIcon(iconName));
    }
    if (!description.isEmpty()) {
        action->setDescription(description);
    }
    action->setObjectName(name);
    action->setIconVisibleInMenu(true);
    action->setActionGroup(parent);
    actionsMap.insert(name, action);
    return action;
}

void LC_ActionFactoryBase::createActions(QMap<QString, QAction*>& map, QActionGroup* group,
                                         const std::vector<ActionInfo>& actionList) const {
    for (const ActionInfo& a : actionList) {
        justCreateAction(map, a.key, a.text, a.iconName, group, a.description);
    }
}

void LC_ActionFactoryBase::createActionHandlerActions(QMap<QString, QAction*>& map, QActionGroup* group,
                                                      const std::vector<ActionInfo>& actionList) const {
    for (const ActionInfo& a : actionList) {
        createAction_AH(a.key, a.actionType, a.text, a.iconName, group, map, a.description);
    }
}

void LC_ActionFactoryBase::createMainWindowActions(QMap<QString, QAction*>& map, QActionGroup* group,
                                                   const std::vector<ActionInfo>& actionList, const bool useToggled) const {
    for (const ActionInfo& a : actionList) {
        createAction_MW(a.key, a.slotPtr, a.slotPtrBool, a.text, a.iconName, group, map, useToggled, a.description);
    }
}
void LC_ActionFactoryBase::makeActionsShortcutsNonEditable(const QMap<QString, QAction*>& map,
                                                           const std::vector<const char*>& actionNames) {
    for (const auto name : actionNames) {
        if (map.contains(name)) {
            QAction* action = map[name];
            if (action != nullptr) {
                action->setProperty(LC_ShortcutInfo::PROPERTY_ACTION_SHORTCUT_CONFIGURABLE, false);
            }
        }
    }
}

void LC_ActionFactoryBase::addActionsToMainWindow(const QMap<QString, QAction*>& map) const {
    // add actions to the main window to ensure that shortcuts for them will be invoked - even if the action is not visible.
    // without this, pressing shortcut for the action that is not visible does not activate the action
    for (const auto& a : map) {
        m_appWin->addAction(a);
    }
}

void LC_ActionFactoryBase::createActionGroups(const std::vector<ActionGroupInfo>& actionGroups,
                                              LC_ActionGroupManager* actionGroupManager) const {
    for (const ActionGroupInfo& groupInfo : actionGroups) {
        const auto group = new LC_ActionGroup(actionGroupManager, groupInfo.name, groupInfo.title, groupInfo.description,
                                              groupInfo.iconName);
        group->setActionMappingsMayBeConfigured(groupInfo.isShortcutConfigurable);
        group->setToolbarMenuConfigurable(groupInfo.isToolbarMenuConfigurable);
        actionGroupManager->addActionGroup(groupInfo.name, group, groupInfo.isToolGroup);
    }
}

void LC_ActionFactoryBase::fillActionsList(QList<QAction*>& list, const std::vector<const char*>& actionNames,
                                           const QMap<QString, QAction*>& map) const {
    for (const char* actionName : actionNames) {
        if (map.contains(actionName)) {
            const auto action = map.value(actionName);
            list << action;
        }
    }
}

void LC_ActionFactoryBase::makeActionsExcludedFromRecent(const QMap<QString, QAction*>& map,
                                                         const std::vector<const char*>& actionNames) {
    for (const auto name : actionNames) {
        if (map.contains(name)) {
            QAction* action = map.value(name);
            if (action != nullptr) {
                action->setProperty(LC_ActionNames::PropertyExcludeFromRecent, true);
            }
        }
    }
}

QAction* LC_ActionFactoryBase::createSpecialAction(QMap<QString, QAction*>& map,
                                                   QActionGroup* group,
                                                   const char* key,
                                                   const QString& title,
                                                   const char* icon,
                                                   const QString& description) const {
    auto* act = justCreateAction(map, key, title, icon, group, description);
    if (act != nullptr) {
        act->setProperty(LC_ShortcutInfo::PROPERTY_ACTION_SHORTCUT_CONFIGURABLE, false);
    }
    return act;
}

void LC_ActionFactoryBase::createSpecialActions(QMap<QString, QAction*>& map,
                                                QActionGroup* group,
                                                const std::initializer_list<SpecialActionEntry>& entries) const {
    for (const auto& entry : entries) {
        createSpecialAction(map, group, entry.key, entry.title, entry.icon, entry.description);
    }
}
