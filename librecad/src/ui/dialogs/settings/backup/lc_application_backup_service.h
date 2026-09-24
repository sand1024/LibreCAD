
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

#ifndef LC_APPLICATION_BACKUP_SERVICE_H
#define LC_APPLICATION_BACKUP_SERVICE_H

#include <qcoreapplication.h>
#include <QList>
#include <QString>

#include "lc_dlg_checklist_selection.h"
#include "lc_profile_helper_base.h"

class LC_ActionGroupManager;
class QC_ApplicationWindow;

inline const QString APPLICATION_BACKUP_FILE_IDENTIFIER = "LibreCAD Application Backup v_1";

class LC_ApplicationBackupService : public LC_ProfileExchangeHelperBase {
    Q_DECLARE_TR_FUNCTIONS(LC_ApplicationBackupService)
public:
    explicit LC_ApplicationBackupService(QC_ApplicationWindow* appWin);
    ~LC_ApplicationBackupService() override = default;

    bool exportBackup(QWidget* parent);
    bool importBackup(QWidget* parent);

private:
    static QString makeUniquePresetName(const QString& baseName, const QStringList& existingNames);
    static QJsonObject exportPortableSettings();
    static void importPortableSettings(const QJsonObject& groupsObj);

    QC_ApplicationWindow* m_appWin{nullptr};
};

#endif
