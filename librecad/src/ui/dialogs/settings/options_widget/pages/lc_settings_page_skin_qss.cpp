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

#include "lc_settings_page_skin_qss.h"

#include <QTextEdit>

#include "ui_lc_settings_page_skin_qss.h"
#include "lc_preset_manager_fusion_skin.h"

LC_SettingsPageSkinQss::LC_SettingsPageSkinQss(QObject* parent)
    : LC_SettingsPageBase(tr("Stylesheet (QCSS)"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSkinQss>()) {
    setSortWeight(50);
}

LC_SettingsPageSkinQss::~LC_SettingsPageSkinQss() = default;

void LC_SettingsPageSkinQss::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerFusionSkin*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerFusionSkin::configLoaded, this, [this](const SkinConfig&) {
            populateUiFromWorkingConfig();
        });
        connect(m_presetManager, &LC_PresetManagerFusionSkin::variantChanged, this, [this](bool) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageSkinQss::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageSkinQss::setupBehavior() {
    connect(ui->qssEdit, &QTextEdit::textChanged, this, &LC_SettingsPageSkinQss::onControlChanged);
}

void LC_SettingsPageSkinQss::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageSkinQss::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageSkinQss::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageSkinQss::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageSkinQss::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const bool isDark = m_presetManager->isCurrentVariantDark();
    const auto& skin = m_presetManager->workingConfig();
    const ColorSchemeData& scheme = isDark ? skin.dark : skin.light;

    ui->qssEdit->setPlainText(scheme.qss);
    m_blockSignals = false;
}

void LC_SettingsPageSkinQss::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    const bool isDark = m_presetManager->isCurrentVariantDark();
    auto& skin = m_presetManager->workingConfig();
    ColorSchemeData& scheme = isDark ? skin.dark : skin.light;

    scheme.qss = ui->qssEdit->toPlainText();
}
