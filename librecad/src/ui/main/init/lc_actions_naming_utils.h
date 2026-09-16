
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

#ifndef LC_ACTIONSNAMINGUTILS_H
#define LC_ACTIONSNAMINGUTILS_H

#include <qcoreapplication.h>
#include <QString>


#include "lc_action_naming_service_interface.h"

class LC_ActionGroupManager;

class LC_ActionNamingService: public LC_ActionNamingServiceInterface {
    Q_DECLARE_TR_FUNCTIONS(LC_ActionsNamingUtils)
public:
    explicit LC_ActionNamingService(LC_ActionGroupManager* agm);
    ~LC_ActionNamingService() override = default;

    bool isSystemToken(const QString& token) const override;
    QString displayName(const QString& token, bool stripAmpersand = false) const override;
    QString iconPath(const QString& token) const override;
    QString toolbarTitle(const QString& toolbarKey) const override;

    QString canonicalToken(const LC_ActionGroup* group) const override;
    QString canonicalToken(const QString& groupName) const override;

    QList<QPair<QString, QString>> predefinedCategories() const override;

private:
    QString normalizeKeyToGroupName(const QString& key) const;

    LC_ActionGroupManager* m_groupManager{nullptr};
};

#endif
