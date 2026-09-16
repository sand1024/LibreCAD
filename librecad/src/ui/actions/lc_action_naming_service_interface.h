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

#ifndef LC_ACTION_NAMING_SERVICE_INTERFACE_H
#define LC_ACTION_NAMING_SERVICE_INTERFACE_H

#include <QList>
#include <lc_action_group.h>

class LC_ActionNamingServiceInterface {
public:
    virtual ~LC_ActionNamingServiceInterface() = default;
    virtual bool isSystemToken(const QString& token) const = 0;
    virtual QString displayName(const QString& token, bool stripAmpersand = false) const = 0;
    virtual QString iconPath(const QString& token) const = 0;
    virtual QString toolbarTitle(const QString& toolbarKey) const = 0;
    virtual QString canonicalToken(const LC_ActionGroup* group) const = 0;
    virtual QString canonicalToken(const QString& groupName) const = 0;
    virtual QList<QPair<QString, QString>> predefinedCategories() const = 0;
};

#endif
