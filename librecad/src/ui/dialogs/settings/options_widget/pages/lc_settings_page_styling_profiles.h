/*******************************************************************************
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

#ifndef LC_SETTINGS_PAGE_WORKSPACE_PROFILES_H
#define LC_SETTINGS_PAGE_WORKSPACE_PROFILES_H

#include "lc_settings_page_base.h"
#include "lc_settings_page_profile_exchange_base.h"

namespace Ui {
    class LC_SettingsPageStylingProfiles;
}

class LC_UIStyleManager;

class LC_SettingsPageStylingProfiles : public LC_SettingsPageProfileExchangeBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageStylingProfiles(QObject* parent = nullptr);
    ~LC_SettingsPageStylingProfiles() override;
    bool acceptsSharedPreview() override {return false;}
protected:
    void setupUi() override;
    void setupBehavior() override;

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

    QString profileExtension() const override { return ".lcsp"; }

private slots:
    void onLaunchGeneratorClicked();

private:
    std::unique_ptr<Ui::LC_SettingsPageStylingProfiles> ui;
    LC_UIStyleManager* m_styleManager {nullptr};
    QString m_pendingPaletteName;
    QString m_pendingSkinName;
    QString m_pendingIconName;
    QString m_pendingTypographyName;
    QString m_pendingMetricsName;

};

#endif
