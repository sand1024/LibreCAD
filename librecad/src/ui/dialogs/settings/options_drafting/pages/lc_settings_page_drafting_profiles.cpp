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

#include "lc_settings_page_drafting_profiles.h"

#include <QSettings>

#include "lc_dlg_checklist_selection.h"
#include "lc_repository_drafting_metrics.h"
#include "lc_repository_drafting_theme.h"
#include "lc_settings_app_styling.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"
#include "ui_lc_settings_page_drafting_profiles.h"

#include "lc_ui_style_manager.h"

LC_SettingsPageDraftingProfiles::LC_SettingsPageDraftingProfiles(QObject* parent)
    : LC_SettingsPageProfileExchangeBase(tr("Profiles Exchange"), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingProfiles>()) {
    setSortWeight(120);

    auto* appWin = QC_ApplicationWindow::getAppWindow();
    m_uiStyleManager = appWin->getUiStyleManager();

    auto* themeRepo = m_uiStyleManager->getDraftingThemeRepository();
    auto* metricsRepo = m_uiStyleManager->getDraftingMetricsRepository();

    m_helper = std::make_unique<LC_DraftingProfileImportExportHelper>(themeRepo, metricsRepo);
}

LC_SettingsPageDraftingProfiles::~LC_SettingsPageDraftingProfiles() = default;

void LC_SettingsPageDraftingProfiles::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingProfiles::setupBehavior() {
    connect(ui->pbImportProfile, &QPushButton::clicked, this, &LC_SettingsPageDraftingProfiles::onImportProfileClicked);
    connect(ui->pbExportProfile, &QPushButton::clicked, this, &LC_SettingsPageDraftingProfiles::onExportProfileClicked);
}

void LC_SettingsPageDraftingProfiles::doLoadSettings() {
    updateSummaryCard();
}

QString LC_SettingsPageDraftingProfiles::profileFileFilter() const {
    return tr("LibreCAD Drafting Profiles (*.lcdp);;All Files (*.*)");
}

QString LC_SettingsPageDraftingProfiles::profileDefaultName() const {
    return tr("My Drafting Profile");
}

QString LC_SettingsPageDraftingProfiles::profileDescriptionText() const {
    return tr("A Drafting Profile (.lcdp) bundles active visual themes and viewport sizing metrics into a single shareable package.");
}

QString LC_SettingsPageDraftingProfiles::profileBoxTitle() const {
    return tr("Consolidated Drafting Profiles (.lcdp)");
}

QList<LC_ChecklistChoice> LC_SettingsPageDraftingProfiles::getExportChoices() const {
    return {
        { "theme",   tr("Visual Theme"),     tr("Export active canvas colors, line patterns, and marker styles"), true, true },
        { "metrics", tr("Viewport Metrics"), tr("Export active handle sizes, marker radii, and line widths"),     true, true }
    };
}

bool LC_SettingsPageDraftingProfiles::inspectProfileFile(const QString& filePath,
                                                         QString& outProfileName,
                                                         QList<LC_ChecklistChoice>& outChoices) const {
    if (m_helper == nullptr) {
        return false;
    }
    return m_helper->inspectProfile(filePath, outProfileName, outChoices);
}

bool LC_SettingsPageDraftingProfiles::executeExport(const QString& filePath,
                                                    const QString& profileName,
                                                    const QList<LC_ChecklistChoice>& choices) {
    if (m_helper == nullptr || m_uiStyleManager == nullptr) {
        return false;
    }

    auto isChecked = [&choices](const QString& id) -> bool {
        for (const auto& c : choices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    LC_ViewportThemeConfig themeConfig;
    const LC_ViewportThemeConfig* pTheme = nullptr;
    if (isChecked("theme")) {
        auto* themeRepo = m_uiStyleManager->getDraftingThemeRepository();
        const QString activeThemeKey = CFG_AppStyling::o_ActiveDraftingThemeName.get();
        if (themeRepo != nullptr && themeRepo->loadByKey(activeThemeKey, themeConfig)) {
            pTheme = &themeConfig;
        }
    }

    LC_DraftingMetricsConfig metricsConfig;
    const LC_DraftingMetricsConfig* pMetrics = nullptr;
    if (isChecked("metrics")) {
        auto* metricsRepo = m_uiStyleManager->getDraftingMetricsRepository();

        const QString activeMetricsKey = CFG_AppStyling::o_ActiveDraftingMetricsName.get();
        if (metricsRepo != nullptr && metricsRepo->loadByKey(activeMetricsKey, metricsConfig)) {
            pMetrics = &metricsConfig;
        }
    }

    return m_helper->exportProfile(filePath, profileName, pTheme, pMetrics);
}

bool LC_SettingsPageDraftingProfiles::executeImport(const QString& filePath,
                                                    const QList<LC_ChecklistChoice>& choices,
                                                    QString& outProfileName) {
    if (m_helper == nullptr) {
        return false;
    }

    m_pendingThemeKey.clear();
    m_pendingMetricsKey.clear();

    return m_helper->importProfile(filePath, choices, outProfileName,
                                   m_pendingThemeKey,
                                   m_pendingMetricsKey);
}

void LC_SettingsPageDraftingProfiles::activateImportedProfile() {
    using namespace CFG_AppStyling;
    if (!m_pendingThemeKey.isEmpty()) {
        o_ActiveDraftingThemeName.set(m_pendingThemeKey);
    }
    if (!m_pendingMetricsKey.isEmpty()) {
        o_ActiveDraftingMetricsName = m_pendingMetricsKey;
    }

    RS_Settings::instance()->emitOptionsChanged();
    updateSummaryCard();
}

void LC_SettingsPageDraftingProfiles::updateSummaryCard() const {
    if (ui == nullptr) {
        return;
    }

    using namespace CFG_AppStyling;
    // 1. Active Theme
    const QString activeThemeKey = o_ActiveDraftingThemeName;
    QString themeDisplayName = activeThemeKey;
    if (activeThemeKey == CFG_AppState::DEFAULT_THEME_KEY) { // fixme - sand - code duplication with default themes, eliminate!
        themeDisplayName = tr("Default Theme (Built-in)");
    }
    else if (m_uiStyleManager != nullptr) {
        auto* themeRepo = m_uiStyleManager->getDraftingThemeRepository();
        if (themeRepo != nullptr) {
            themeDisplayName = themeRepo->getDisplayName(activeThemeKey);
        }
    }
    ui->lblActiveTheme->setText(themeDisplayName);

    // 2. Active Metrics
    const QString activeMetricsKey = o_ActiveDraftingMetricsName.get();
    QString metricsDisplayName = activeMetricsKey;
    if (activeMetricsKey == CFG_AppState::DEFAULT_THEME_KEY) {
        metricsDisplayName = tr("Standard (96–120 DPI)");
    }
    else if (activeMetricsKey == "__hidpi__") {
        metricsDisplayName = tr("HiDPI / 4K (150–200%)");
    }
    else {
        auto* metricsRepo = m_uiStyleManager->getDraftingMetricsRepository();
        if (metricsRepo != nullptr) {
            metricsDisplayName = metricsRepo->getDisplayName(activeMetricsKey);
        }
    }
    ui->lblActiveMetrics->setText(metricsDisplayName);
}
