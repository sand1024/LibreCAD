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

#include "lc_settings_page_metrics_controls.h"
#include "ui_lc_settings_page_metrics_controls.h"
#include "lc_preset_manager_metrics.h"
#include <QSpinBox>

LC_SettingsPageMetricsControls::LC_SettingsPageMetricsControls(QObject* parent)
    : LC_SettingsPageBase(tr("Controls"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageMetricsControls>()) {
}

LC_SettingsPageMetricsControls::~LC_SettingsPageMetricsControls() = default;

void LC_SettingsPageMetricsControls::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMetrics*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerMetrics::configLoaded, this, [this](const StyleMetricsConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageMetricsControls::setupUi() {
    ui->setupUi(m_widget);

    ui->sbScrollbarWidth->setMinimum(-1);
    ui->sbScrollbarWidth->setSpecialValueText(tr("System Default"));
}

void LC_SettingsPageMetricsControls::setupBehavior() {
    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageMetricsControls::onControlChanged);
    }
}

void LC_SettingsPageMetricsControls::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageMetricsControls::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageMetricsControls::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageMetricsControls::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageMetricsControls::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->sbIndicatorBoxSize->setValue(config.indicatorBoxSize);
    ui->sbIndicatorLabelSpacing->setValue(config.indicatorLabelSpacing);
    ui->sbScrollbarWidth->setValue(config.scrollBarWidth);
    ui->sbScrollbarMinLength->setValue(config.scrollBarMinLength);
    ui->sbSliderThickness->setValue(config.sliderControlThickness);
    ui->sbSliderHandleLength->setValue(config.sliderHandleLength);
    ui->sbButtonPadding->setValue(config.buttonPadding);
    ui->sbFocusHMargin->setValue(config.focusFrameHMargin);
    ui->sbFocusVMargin->setValue(config.focusFrameVMargin);

    m_blockSignals = false;
}

void LC_SettingsPageMetricsControls::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.indicatorBoxSize = ui->sbIndicatorBoxSize->value();
    config.indicatorLabelSpacing = ui->sbIndicatorLabelSpacing->value();
    config.scrollBarWidth = ui->sbScrollbarWidth->value();
    config.scrollBarMinLength = ui->sbScrollbarMinLength->value();
    config.sliderControlThickness = ui->sbSliderThickness->value();
    config.sliderHandleLength = ui->sbSliderHandleLength->value();
    config.buttonPadding = ui->sbButtonPadding->value();
    config.focusFrameHMargin = ui->sbFocusHMargin->value();
    config.focusFrameVMargin = ui->sbFocusVMargin->value();
}
