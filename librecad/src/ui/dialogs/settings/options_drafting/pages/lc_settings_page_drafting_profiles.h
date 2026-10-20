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

#ifndef LC_SETTINGS_PAGE_DRAFTING_PROFILES_H
#define LC_SETTINGS_PAGE_DRAFTING_PROFILES_H

#include <memory>
#include "lc_drafting_profile_import_export_helper.h"
#include "lc_settings_page_profile_exchange_base.h"

namespace Ui {
    class LC_SettingsPageDraftingProfiles;
}

class LC_UIStyleManager;

class LC_SettingsPageDraftingProfiles : public LC_SettingsPageProfileExchangeBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageDraftingProfiles(QObject* parent = nullptr);
    ~LC_SettingsPageDraftingProfiles() override;

protected:
    void setupUi() override;
    void setupBehavior() override;
    void doLoadSettings() override;

    QString profileFileFilter() const override;
    QString profileDefaultName() const override;
    QString profileDescriptionText() const override;
    QString profileBoxTitle() const override;
    QList<LC_ChecklistChoice> getExportChoices() const override;

    bool inspectProfileFile(const QString& filePath,
                            QString& outProfileName,
                            QList<LC_ChecklistChoice>& outChoices) const override;

    bool executeExport(const QString& filePath,
                       const QString& profileName,
                       const QList<LC_ChecklistChoice>& choices) override;

    bool executeImport(const QString& filePath,
                       const QList<LC_ChecklistChoice>& choices,
                       QString& outProfileName) override;

    void activateImportedProfile() override;

    QString profileExtension() const override { return ".lcdp"; }

private:
    void updateSummaryCard() const;

    std::unique_ptr<Ui::LC_SettingsPageDraftingProfiles> ui;
    std::unique_ptr<LC_DraftingProfileImportExportHelper> m_helper;
    LC_UIStyleManager* m_uiStyleManager = nullptr;

    QString m_pendingThemeKey;
    QString m_pendingMetricsKey;
};

#endif
