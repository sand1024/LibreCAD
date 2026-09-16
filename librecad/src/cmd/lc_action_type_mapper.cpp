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

#include "lc_action_type_mapper.h"

#include <QAction>
#include "lc_action_group_manager.h"
#include "lc_commandItems.h"

LC_ActionTypeMapper::LC_ActionTypeMapper(const LC_ActionGroupManager* agm)
    : m_agm(agm) {
    if (m_agm != nullptr) {
        rebuildMap();
    }
}

void LC_ActionTypeMapper::setActionGroupManager(const LC_ActionGroupManager* agm) {
    m_agm = agm;
    rebuildMap();
}

void LC_ActionTypeMapper::rebuildMap() {
    m_nameToType.clear();
    m_typeToName.clear();

    if (m_agm == nullptr) {
        return;
    }

    // Populate mappings from registered command items
    for (const auto& item : g_commandList) {
        const auto actionType = item.actionType;
        const auto* act = m_agm->getActionByType(actionType);
        if (act != nullptr && !act->objectName().isEmpty()) {
            const QString name = act->objectName();
            m_nameToType.insert(name, actionType);
            m_typeToName.insert(actionType, name);
        }
    }
}

RS2::ActionType LC_ActionTypeMapper::actionTypeFromName(const QString& actionName) const {
    const auto it = m_nameToType.find(actionName);
    if (it != m_nameToType.end()) {
        return it.value();
    }
    return RS2::ActionNone;
}

QString LC_ActionTypeMapper::actionNameFromType(RS2::ActionType actionType) const {
    const auto it = m_typeToName.find(actionType);
    if (it != m_typeToName.end()) {
        return it.value();
    }
    return QString();
}

bool LC_ActionTypeMapper::hasAction(const QString& actionName) const {
    return m_nameToType.contains(actionName);
}

bool LC_ActionTypeMapper::hasActionType(RS2::ActionType actionType) const {
    return m_typeToName.contains(actionType);
}
