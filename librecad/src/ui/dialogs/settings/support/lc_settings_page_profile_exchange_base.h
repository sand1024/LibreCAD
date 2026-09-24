
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

#ifndef LC_SETTINGS_PAGE_PROFILE_EXCHANGE_BASE_H
#define LC_SETTINGS_PAGE_PROFILE_EXCHANGE_BASE_H

#include <QList>
#include <QString>

#include "lc_dlg_checklist_selection.h"
#include "lc_settings_page_base.h"

class LC_SettingsPageProfileExchangeBase : public LC_SettingsPageBase {
    Q_OBJECT
public:
    LC_SettingsPageProfileExchangeBase(const QString& displayName, QObject* parent = nullptr);
    ~LC_SettingsPageProfileExchangeBase() override = default;

protected slots:
    void onImportProfileClicked();
    void onExportProfileClicked();

protected:
    virtual QString profileFileFilter() const = 0;
    virtual QString profileDefaultName() const = 0;
    virtual QString profileDescriptionText() const = 0;
    virtual QString profileBoxTitle() const = 0;
    virtual QString profileExtension() const = 0;
    virtual QList<LC_ChecklistChoice> getExportChoices() const = 0;
    virtual bool inspectProfileFile(const QString& filePath,
                                    QString& outProfileName,
                                    QList<LC_ChecklistChoice>& outChoices) const = 0;

    virtual bool executeExport(const QString& filePath,
                               const QString& profileName,
                               const QList<LC_ChecklistChoice>& choices) = 0;

    virtual bool executeImport(const QString& filePath,
                               const QList<LC_ChecklistChoice>& choices,
                               QString& outProfileName) = 0;

    virtual void activateImportedProfile() = 0;

    void setupExchangeUi(QWidget* parentWidget, QWidget* extraWidget = nullptr);
};

#endif
