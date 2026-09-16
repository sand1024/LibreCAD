
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

#ifndef LC_ACTION_TYPE_MAPPER_H
#define LC_ACTION_TYPE_MAPPER_H

#include <QMap>
#include <QString>
#include "rs.h"

class LC_ActionGroupManager;

class LC_ActionTypeMapper {
public:
    explicit LC_ActionTypeMapper(const LC_ActionGroupManager* agm = nullptr);
    ~LC_ActionTypeMapper() = default;

    void setActionGroupManager(const LC_ActionGroupManager* agm);

    RS2::ActionType actionTypeFromName(const QString& actionName) const;
    QString actionNameFromType(RS2::ActionType actionType) const;

    bool hasAction(const QString& actionName) const;
    bool hasActionType(RS2::ActionType actionType) const;

private:
    void rebuildMap();

    const LC_ActionGroupManager* m_agm{nullptr};
    QMap<QString, RS2::ActionType> m_nameToType;
    QMap<RS2::ActionType, QString> m_typeToName;
};

#endif
