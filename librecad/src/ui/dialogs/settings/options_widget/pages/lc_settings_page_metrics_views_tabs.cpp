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

#include "lc_settings_page_metrics_views_tabs.h"
#include "ui_lc_settings_page_metrics_views_tabs.h"
#include "lc_preset_manager_metrics.h"
#include <QSpinBox>

LC_SettingsPageMetricsViewsTabs::LC_SettingsPageMetricsViewsTabs(QObject* parent)
    : LC_SettingsPageBase(tr("Views"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageMetricsViewsTabs>()) {
}

LC_SettingsPageMetricsViewsTabs::~LC_SettingsPageMetricsViewsTabs() = default;

void LC_SettingsPageMetricsViewsTabs::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMetrics*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerMetrics::configLoaded, this, [this](const StyleMetricsConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageMetricsViewsTabs::setupUi() {
    ui->setupUi(m_widget);

    ui->sbTreeBranchIndicatorSize->setMinimum(-1);
    ui->sbTreeBranchIndicatorSize->setSpecialValueText(tr("Auto-Scale"));
}

void LC_SettingsPageMetricsViewsTabs::setupBehavior() {
    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageMetricsViewsTabs::onControlChanged);
    }
}

void LC_SettingsPageMetricsViewsTabs::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageMetricsViewsTabs::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageMetricsViewsTabs::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageMetricsViewsTabs::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageMetricsViewsTabs::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->sbTabHSpace->setValue(config.tabBarTabHSpace);
    ui->sbTabVSpace->setValue(config.tabBarTabVSpace);
    ui->sbTabOverlapHorizontal->setValue(config.tabBarTabOverlap);
    ui->sbTabCloseSize->setValue(config.tabCloseIndicatorSize);
    ui->sbTabOverlap->setValue(config.tabBarTabBaseOverlap);
    ui->sbHeaderDefaultHeight->setValue(config.headerDefaultHeight);
    ui->sbRowPadding->setValue(config.itemViewRowPadding);
    ui->sbTreeIndentation->setValue(config.treeIndentation);
    ui->sbTreeBranchIndicatorSize->setValue(config.treeBranchIndicatorSize);

    m_blockSignals = false;
}

void LC_SettingsPageMetricsViewsTabs::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.tabBarTabHSpace = ui->sbTabHSpace->value();
    config.tabBarTabVSpace = ui->sbTabVSpace->value();
    config.tabBarTabOverlap = ui->sbTabOverlapHorizontal->value();
    config.tabCloseIndicatorSize = ui->sbTabCloseSize->value();
    config.tabBarTabBaseOverlap = ui->sbTabOverlap->value();
    config.headerDefaultHeight = ui->sbHeaderDefaultHeight->value();
    config.itemViewRowPadding = ui->sbRowPadding->value();
    config.treeIndentation = ui->sbTreeIndentation->value();
    config.treeBranchIndicatorSize = ui->sbTreeBranchIndicatorSize->value();
}
