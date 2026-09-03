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

#include "lc_settings_page_fusion_theme.h"
#include "ui_lc_settings_page_fusion_theme.h"

#include "lc_palette_editor_shared.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_backend.h"
#include "lc_settings_list_widget.h"
#include "lc_settings_manager_styling.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_SettingsPageFusionTheme::LC_SettingsPageFusionTheme(QObject* parent)
    : LC_SettingsPageBase(tr("Fusion Theme"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_AppStyling::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageFusionTheme>()) {
    setSortWeight(50);
    if (QC_ApplicationWindow::getAppWindow() != nullptr) {
        m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    }
}

LC_SettingsPageFusionTheme::~LC_SettingsPageFusionTheme() = default;

void LC_SettingsPageFusionTheme::setupUi() {
    m_blockSignals = true;
    ui->setupUi(m_widget);
    setupThemeModeCombobox();
    updateGatedControlsState();
    m_blockSignals = false;
}

void LC_SettingsPageFusionTheme::setupThemeModeCombobox() {
    ui->cbThemeModeOverride->clear();
    ui->cbThemeModeOverride->addItem(tr("Follow System Settings"), static_cast<int>(ThemeModeOverride::FollowSystem));
    ui->cbThemeModeOverride->addItem(tr("Force Light Mode"), static_cast<int>(ThemeModeOverride::ForceLight));
    ui->cbThemeModeOverride->addItem(tr("Force Dark Mode"), static_cast<int>(ThemeModeOverride::ForceDark));
}

void LC_SettingsPageFusionTheme::setupBehavior() {
    connect(ui->cbThemeModeOverride, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageFusionTheme::onControlChanged);
    connect(ui->cbIgnoreIconStylingInTheme, &QCheckBox::toggled, this, &LC_SettingsPageFusionTheme::onControlChanged);
}

void LC_SettingsPageFusionTheme::setupBindings() {
    using namespace CFG_AppStyling;

    bindCustom<QComboBox, int>(
        ui->cbThemeModeOverride, o_ThemeModeOverride.fullKey(), static_cast<int>(o_ThemeModeOverride.defaultValue()), false,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](QComboBox* w) { return w->currentData().toInt(); },
        [](QComboBox* w, int val) {
            const int idx = w->findData(val);
            if (idx >= 0) {
                w->setCurrentIndex(idx);
            }
        });

    bindCustom<QCheckBox, bool>(
        ui->cbIgnoreIconStylingInTheme, "Widgets/IgnoreIconStylingInTheme", false, false,
        &QCheckBox::toggled,
        [](QCheckBox* w) { return w->isChecked(); },
        [](QCheckBox* w, bool val) { w->setChecked(val); });
}

void LC_SettingsPageFusionTheme::setChildPages(const QList<LC_SettingsPageInterface*>& children) {
    if (children.isEmpty()) {
        return;
    }

    QList<QPair<QString, QString>> links;
    for (const auto* child : children) {
        if (child != nullptr) {
            links.append({child->id(), child->displayName()});
        }
    }

    auto* linksWidget = new LC_SettingsLinksWidget(links, m_widget);
    if (ui->gbSubPages->layout() != nullptr) {
        ui->gbSubPages->layout()->addWidget(linksWidget);
    }

    connect(linksWidget, &LC_SettingsLinksWidget::pageSelected,
            this, &LC_SettingsPageBase::navigateToPage);
}

void LC_SettingsPageFusionTheme::onAboutToShow() {
    LC_SettingsPageBase::onAboutToShow();
    updateGatedControlsState();
}

bool LC_SettingsPageFusionTheme::isPageGated() const {
    return LC_SettingsManagerStyling::isFusionGated();
}

QString LC_SettingsPageFusionTheme::gatedMessage() const {
    if (isPageGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("Enable Fusion theme"));
    }
    return QString();
}

QString LC_SettingsPageFusionTheme::gatedActionText() const {
    if (isPageGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    return QString();
}
std::function<void()> LC_SettingsPageFusionTheme::gatedActionCallback() const {
    return [this]() {
        LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        updateGatedControlsState();
    };
}

void LC_SettingsPageFusionTheme::updateGatedControlsState() const {
    const bool gated = isPageGated();
    ui->gbThemeMode->setEnabled(!gated);
    ui->gbIconThemePolicy->setEnabled(!gated);
    ui->gbSubPages->setEnabled(true);
}

void LC_SettingsPageFusionTheme::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    applyTransientTheme();
    updateLivePreview();
}

void LC_SettingsPageFusionTheme::applyTransientTheme() {
    if (m_styleManager == nullptr) {
        return;
    }

    const bool allowStyle = m_styleManager->isStyleAllowed();
    const QString styleName = m_styleManager->getActiveStyle();
    const auto themeModeOverride = static_cast<ThemeModeOverride>(ui->cbThemeModeOverride->currentData().toInt());
    const bool ignoreIcons = ui->cbIgnoreIconStylingInTheme->isChecked();

    CFG_AppStyling::o_ThemeModeOverride.set(themeModeOverride);
    m_styleManager->setThemeModeOverride(themeModeOverride);
    m_styleManager->setIgnoreIconStylingInTheme(ignoreIcons);

    const QString paletteKey = m_styleManager->getActivePalette();
    const QString skinKey = m_styleManager->getActiveSkin();
    const QString metricsKey = m_styleManager->getActiveMetrics();
    const QString typographyKey = m_styleManager->getActiveTypography();
    const QString iconStyleKey = m_styleManager->getActiveIconStyle();

    m_styleManager->applyTransientTheme(allowStyle, styleName,
                                        paletteKey, skinKey, metricsKey,
                                        typographyKey, iconStyleKey,
                                        themeModeOverride);
}

void LC_SettingsPageFusionTheme::loadSettings() {
    m_blockSignals = true;
    LC_SettingsPageBase::loadSettings();
    updateGatedControlsState();
    m_blockSignals = false;
}

bool LC_SettingsPageFusionTheme::saveSettings() {
    const bool success = LC_SettingsPageBase::saveSettings();
    if (success && m_styleManager != nullptr) {
        m_styleManager->setThemeModeOverride(static_cast<ThemeModeOverride>(ui->cbThemeModeOverride->currentData().toInt()));
        m_styleManager->setIgnoreIconStylingInTheme(ui->cbIgnoreIconStylingInTheme->isChecked());
        m_styleManager->applyActiveStyleAndTheme();
    }
    return success;
}
