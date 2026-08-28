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

#include "lc_settings_page_metrics_layout.h"
#include "ui_lc_settings_page_metrics_layout.h"
#include "lc_preset_manager_metrics.h"
#include <QSpinBox>

LC_SettingsPageMetricsLayout::LC_SettingsPageMetricsLayout(QObject* parent)
    : LC_SettingsPageBase(tr("Layout & Spacing"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageMetricsLayout>()) {
    setSortWeight(10);
}

LC_SettingsPageMetricsLayout::~LC_SettingsPageMetricsLayout() = default;

void LC_SettingsPageMetricsLayout::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMetrics*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerMetrics::configLoaded, this, [this](const StyleMetricsConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageMetricsLayout::setupUi() {
    ui->setupUi(m_widget);

    ui->sbDockButtonMargin->setMinimum(-1);
    ui->sbDockButtonMargin->setSpecialValueText(tr("Auto-Center"));

    ui->sbDockTitleHeight->setMinimum(-1);
    ui->sbDockTitleHeight->setSpecialValueText(tr("Font-Adaptive"));

    ui->sbSplitterHandleLength->setMinimum(-1);
    ui->sbSplitterHandleLength->setSpecialValueText(tr("Full Length"));
}

void LC_SettingsPageMetricsLayout::setupBehavior() {
    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageMetricsLayout::onControlChanged);
    }
}

void LC_SettingsPageMetricsLayout::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageMetricsLayout::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageMetricsLayout::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageMetricsLayout::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageMetricsLayout::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->sbLayoutMargin->setValue(config.layoutMargin);
    ui->sbLayoutSpacing->setValue(config.layoutSpacing);
    ui->sbSplitterWidth->setValue(config.splitterWidth);
    ui->sbSplitterHandleLength->setValue(config.splitterHandleLength);
    ui->sbDockTitleHeight->setValue(config.dockTitleBarHeight);
    ui->sbDockButtonMargin->setValue(config.dockWidgetTitleBarButtonMargin);
    ui->sbGroupBoxTitleLeftPadding->setValue(config.groupBoxTitleLeftPadding);
    ui->sbGroupBoxTitleLineGap->setValue(config.groupBoxTitleLineGap);
    ui->sbTitleBarButtonSize->setValue(config.titleBarButtonSize);

    m_blockSignals = false;
}

void LC_SettingsPageMetricsLayout::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.layoutMargin = ui->sbLayoutMargin->value();
    config.layoutSpacing = ui->sbLayoutSpacing->value();
    config.splitterWidth = ui->sbSplitterWidth->value();
    config.splitterHandleLength = ui->sbSplitterHandleLength->value();
    config.dockTitleBarHeight = ui->sbDockTitleHeight->value();
    config.dockWidgetTitleBarButtonMargin = ui->sbDockButtonMargin->value();
    config.groupBoxTitleLeftPadding = ui->sbGroupBoxTitleLeftPadding->value();
    config.groupBoxTitleLineGap = ui->sbGroupBoxTitleLineGap->value();
    config.titleBarButtonSize = ui->sbTitleBarButtonSize->value();
}
