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

#include "lc_settings_page_workspace_profiles.h"
#include "ui_lc_settings_page_workspace_profiles.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "lc_dlg_styles_presets_generator.h"
#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_metrics_repository.h"
#include "lc_typography_repository.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_SettingsPageWorkspaceProfiles::LC_SettingsPageWorkspaceProfiles(QObject* parent)
    : LC_SettingsPageBase(tr("Profiles Exchange"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageWorkspaceProfiles>()) {
    m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageWorkspaceProfiles::~LC_SettingsPageWorkspaceProfiles() = default;

void LC_SettingsPageWorkspaceProfiles::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageWorkspaceProfiles::setupBehavior() {
    connect(ui->pbImportProfile, &QPushButton::clicked, this, &LC_SettingsPageWorkspaceProfiles::onImportProfileClicked);
    connect(ui->pbExportProfile, &QPushButton::clicked, this, &LC_SettingsPageWorkspaceProfiles::onExportProfileClicked);
    connect(ui->pbPresetsGenerator, &QPushButton::clicked, this, &LC_SettingsPageWorkspaceProfiles::onLaunchGeneratorClicked);
}

void LC_SettingsPageWorkspaceProfiles::onImportProfileClicked() {
    if (m_styleManager == nullptr) return;

    const QString path = QFileDialog::getOpenFileName(
        getEditingWidget(), tr("Import Workspace Profile"), QString(), tr("LibreCAD Workspace Profiles (*.lcws)"));
    if (path.isEmpty()) return;

    QString profileName, skinFile, iconFile, typographyFile, metricsFile;
    if (m_styleManager->importProfile(path, profileName, skinFile, iconFile, typographyFile, metricsFile)) {
        if (!skinFile.isEmpty()) m_styleManager->setActiveSkin(skinFile);
        if (!iconFile.isEmpty()) m_styleManager->setActiveIconStyle(iconFile);
        if (!typographyFile.isEmpty()) m_styleManager->setActiveTypography(typographyFile);
        if (!metricsFile.isEmpty()) m_styleManager->setActiveMetrics(metricsFile);

        m_styleManager->applyActiveStyleAndTheme();
        QMessageBox::information(getEditingWidget(), tr("Profile Imported"),
                                 tr("Workspace profile '%1' has been successfully imported and applied.").arg(profileName));
    }
    else {
        QMessageBox::critical(getEditingWidget(), tr("Import Error"), tr("Could not parse or import the workspace profile."));
    }
}

void LC_SettingsPageWorkspaceProfiles::onExportProfileClicked() {
    if (m_styleManager == nullptr) return;

    const QString path = QFileDialog::getSaveFileName(
        getEditingWidget(), tr("Export Workspace Profile"), QString(), tr("LibreCAD Workspace Profiles (*.lcws)"));
    if (path.isEmpty()) return;

    bool ok = false;
    const QString profileName = QInputDialog::getText(
        getEditingWidget(), tr("Export Profile"), tr("Enter profile name:"), QLineEdit::Normal, tr("My Custom Profile"), &ok);
    if (!ok || profileName.trimmed().isEmpty()) return;

    SkinConfig skin;
    const bool hasSkin = m_styleManager->getSkinsRepository() &&
                         m_styleManager->getSkinsRepository()->loadByKey(m_styleManager->getActiveSkin(), skin);

    IconStyleConfig icon;
    const bool hasIcon = m_styleManager->getIconsStyleRepository() &&
                         m_styleManager->getIconsStyleRepository()->loadByKey(m_styleManager->getActiveIconStyle(), icon);

    FontConfig font;
    const bool hasFont = m_styleManager->getTypographyRepository() &&
                         m_styleManager->getTypographyRepository()->loadByKey(m_styleManager->getActiveTypography(), font);

    StyleMetricsConfig metrics;
    const bool hasMetrics = m_styleManager->getMetricsRepository() &&
                            m_styleManager->getMetricsRepository()->loadByKey(m_styleManager->getActiveMetrics(), metrics);

    if (m_styleManager->exportProfile(path, profileName.trimmed(),
                                      hasSkin ? &skin : nullptr,
                                      hasIcon ? &icon : nullptr,
                                      hasFont ? &font : nullptr,
                                      hasMetrics ? &metrics : nullptr)) {
        QMessageBox::information(getEditingWidget(), tr("Profile Exported"),
                                 tr("Workspace profile '%1' has been successfully exported.").arg(profileName));
    }
    else {
        QMessageBox::critical(getEditingWidget(), tr("Export Error"), tr("Could not export the workspace profile."));
    }
}

void LC_SettingsPageWorkspaceProfiles::onLaunchGeneratorClicked() {
    if (m_styleManager == nullptr) return;
    LC_DlgStylesPresetsGenerator dlg(getEditingWidget(), m_styleManager);
    dlg.exec();
}
