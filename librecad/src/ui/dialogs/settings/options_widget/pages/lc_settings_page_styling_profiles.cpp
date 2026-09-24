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

#include "lc_settings_page_styling_profiles.h"
#include "ui_lc_settings_page_styling_profiles.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "lc_dlg_styles_presets_generator.h"
#include "lc_repository_fusion_skin.h"
#include "lc_repository_icons_style.h"
#include "lc_repository_metrics.h"
#include "lc_repository_palette.h"
#include "lc_repository_typography.h"
#include "lc_styling_profile_import_export_helper.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_SettingsPageStylingProfiles::LC_SettingsPageStylingProfiles(QObject* parent)
    : LC_SettingsPageProfileExchangeBase(tr("Profiles Exchange"), parent), ui(std::make_unique<Ui::LC_SettingsPageStylingProfiles>()) {
    m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageStylingProfiles::~LC_SettingsPageStylingProfiles() = default;

void LC_SettingsPageStylingProfiles::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageStylingProfiles::setupBehavior() {
    connect(ui->pbImportProfile, &QPushButton::clicked, this, &LC_SettingsPageStylingProfiles::onImportProfileClicked);
    connect(ui->pbExportProfile, &QPushButton::clicked, this, &LC_SettingsPageStylingProfiles::onExportProfileClicked);
    connect(ui->pbPresetsGenerator, &QPushButton::clicked, this, &LC_SettingsPageStylingProfiles::onLaunchGeneratorClicked);
}

QString LC_SettingsPageStylingProfiles::profileFileFilter() const {
    return tr("LibreCAD Workspace Profiles (*.lcws);;All Files (*.*)");
}

QString LC_SettingsPageStylingProfiles::profileDefaultName() const {
    return tr("My Custom Profile");
}

QString LC_SettingsPageStylingProfiles::profileDescriptionText() const {
    return tr("A Workspace Profile (.lcws) bundles active skin colors, icon styles, typography settings, "
              "and density metrics into a single shareable package.");
}

QString LC_SettingsPageStylingProfiles::profileBoxTitle() const {
    return tr("Consolidated Workspace Profiles (.lcws)");
}

QList<LC_ChecklistChoice> LC_SettingsPageStylingProfiles::getExportChoices() const {
    return {
        {"palette",    tr("Color Palette"),       tr("Export active color palette theme"),          true, true},
        {"skin",       tr("Control Style / Skin"),tr("Export active widget skins and highlight rules"), true, true},
        {"icon",       tr("Icon Style"),          tr("Export active toolbar and CAD icon themes"),  true, true},
        {"typography", tr("Typography & Fonts"),  tr("Export active application font metrics"),     true, true},
        {"metrics",    tr("Density & Metrics"),   tr("Export active toolbar and margin dimensions"), true, true}
    };
}

bool LC_SettingsPageStylingProfiles::inspectProfileFile(const QString& filePath,
                                                          QString& outProfileName,
                                                          QList<LC_ChecklistChoice>& outChoices) const {
    if (m_styleManager == nullptr || m_styleManager->getImportExportHelper() == nullptr) {
        return false;
    }

    QJsonObject root;
    if (!m_styleManager->getImportExportHelper()->readJsonFile(filePath, root)) {
        return false;
    }

    outProfileName = root.value("profile_name").toString();
    outChoices.clear();

    if (root.contains("inlined_palette")) {
        outChoices.append({"palette", tr("Color Palette"), tr("Color palette definitions"), true, true});
    }
    if (root.contains("inlined_control_style") || root.contains("inlined_skin")) {
        outChoices.append({"skin", tr("Control Style / Skin"), tr("Widget skins and controls styling"), true, true});
    }
    if (root.contains("inlined_icon")) {
        outChoices.append({"icon", tr("Icon Style"), tr("Toolbar and action icon style"), true, true});
    }
    if (root.contains("inlined_font")) {
        outChoices.append({"typography", tr("Typography & Fonts"), tr("Application font settings"), true, true});
    }
    if (root.contains("inlined_metrics")) {
        outChoices.append({"metrics", tr("Density & Metrics"), tr("Toolbar and widget size metrics"), true, true});
    }

    return !outChoices.isEmpty();
}

bool LC_SettingsPageStylingProfiles::executeExport(const QString& filePath,
                                                     const QString& profileName,
                                                     const QList<LC_ChecklistChoice>& choices) {
    auto stylingProfileImportExportHelper = m_styleManager->getImportExportHelper();
    if (m_styleManager == nullptr || stylingProfileImportExportHelper == nullptr) {
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

    PaletteConfig palette;
    const bool hasPalette = isChecked("palette") && m_styleManager->getPaletteRepository() &&
                            m_styleManager->getPaletteRepository()->loadByKey(m_styleManager->getActivePalette(), palette);

    ControlStyleConfig skin;
    const bool hasSkin = isChecked("skin") && m_styleManager->getSkinsRepository() &&
                         m_styleManager->getSkinsRepository()->loadByKey(m_styleManager->getActiveSkin(), skin);

    IconStyleConfig icon;
    const bool hasIcon = isChecked("icon") && m_styleManager->getIconsStyleRepository() &&
                         m_styleManager->getIconsStyleRepository()->loadByKey(m_styleManager->getActiveIconStyle(), icon);

    FontConfig font;
    const bool hasFont = isChecked("typography") && m_styleManager->getTypographyRepository() &&
                         m_styleManager->getTypographyRepository()->loadByKey(m_styleManager->getActiveTypography(), font);

    StyleMetricsConfig metrics;
    const bool hasMetrics = isChecked("metrics") && m_styleManager->getMetricsRepository() &&
                            m_styleManager->getMetricsRepository()->loadByKey(m_styleManager->getActiveMetrics(), metrics);

    return stylingProfileImportExportHelper->exportProfile(
        filePath, profileName,
        hasPalette ? &palette : nullptr,
        hasSkin ? &skin : nullptr,
        hasIcon ? &icon : nullptr,
        hasFont ? &font : nullptr,
        hasMetrics ? &metrics : nullptr);
}

bool LC_SettingsPageStylingProfiles::executeImport(const QString& filePath,
                                                     const QList<LC_ChecklistChoice>& choices,
                                                     QString& outProfileName) {
    if (m_styleManager == nullptr || m_styleManager->getImportExportHelper() == nullptr) {
        return false;
    }

    m_pendingPaletteName.clear();
    m_pendingSkinName.clear();
    m_pendingIconName.clear();
    m_pendingTypographyName.clear();
    m_pendingMetricsName.clear();

    return m_styleManager->getImportExportHelper()->importProfile(
        filePath, choices, outProfileName,
        m_pendingPaletteName,
        m_pendingSkinName,
        m_pendingIconName,
        m_pendingTypographyName,
        m_pendingMetricsName);
}

void LC_SettingsPageStylingProfiles::activateImportedProfile() {
    if (m_styleManager == nullptr) {
        return;
    }

    if (!m_pendingPaletteName.isEmpty()) {
        m_styleManager->setActivePalette(m_pendingPaletteName);
    }
    if (!m_pendingSkinName.isEmpty()) {
        m_styleManager->setActiveSkin(m_pendingSkinName);
    }
    if (!m_pendingIconName.isEmpty()) {
        m_styleManager->setActiveIconStyle(m_pendingIconName);
    }
    if (!m_pendingTypographyName.isEmpty()) {
        m_styleManager->setActiveTypography(m_pendingTypographyName);
    }
    if (!m_pendingMetricsName.isEmpty()) {
        m_styleManager->setActiveMetrics(m_pendingMetricsName);
    }

    m_styleManager->applyActiveStyleAndTheme();
}

void LC_SettingsPageStylingProfiles::onLaunchGeneratorClicked() {
    if (m_styleManager == nullptr) {
        return;
    }
    LC_DlgStylesPresetsGenerator dlg(getEditingWidget(), m_styleManager);
    dlg.exec();
}
