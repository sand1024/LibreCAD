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

#include "lc_preset_manager_fusion_metrics.h"

#include "lc_style_metrics_utils.h"

LC_PresetManagerFusionMetrics::LC_PresetManagerFusionMetrics(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<StyleMetricsConfig, LC_RepositoryMetrics>(
          styleManager,
          styleManager != nullptr ? styleManager->getMetricsRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActiveMetrics() : QString(),
          nullptr) {
    m_headerBar = std::make_unique<LC_MetricsDensityHeaderBar>();
    connect(m_headerBar.get(), &LC_MetricsDensityHeaderBar::densityPresetSelected,
            this, &LC_PresetManagerFusionMetrics::onDensityPresetSelected);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerFusionMetrics::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Fusion Metrics");
    s.labelText = tr("Metrics preset:");
    s.selectToolTip = tr("Select a saved metrics density configuration or load system defaults.");
    s.saveToolTip = tr("Save changes directly to active metrics preset.");
    s.saveAsToolTip = tr("Save current metrics configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom metrics preset from disk.");
    s.applyToolTip = tr("Apply the active metrics density configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the metrics preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Metrics Preset As");
    s.saveAsDialogLabel = tr("Enter unique metrics preset name:");
    s.defaultNewPresetName = tr("Custom Metrics");
    s.deleteConfirmTitle = tr("Delete Metrics Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the metrics preset '%1'?");
    s.presetFileFilter = tr("LibreCAD Fusion Theme Metrics Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".json"));

    s.defaultReadOnlyMessage = tr(
        "The Default metrics preset is a read-only template. To customize layout metrics, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Metrics...");
    return s;
}

QString LC_PresetManagerFusionMetrics::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActiveMetrics();
    }
    return m_originalActiveKey;
}

QWidget* LC_PresetManagerFusionMetrics::getSharedHeaderWidget() {
    return m_headerBar.get();
}

QString LC_PresetManagerFusionMetrics::fusionGatingSubject() const {
    return tr("metrics");
}

void LC_PresetManagerFusionMetrics::onPostLoadPreset() {
    if (m_headerBar != nullptr) {
        if (isReadOnlyDefault()) {
            m_headerBar->setDensityPreset("standard");
        }
        else {
            m_headerBar->setDensityToCustom();
        }
    }
}

void LC_PresetManagerFusionMetrics::emitConfigLoaded() {
    emit configLoaded(m_workingConfig);
}

void LC_PresetManagerFusionMetrics::onDensityPresetSelected(const QString& presetName) {
    if (presetName == "custom") {
        return;
    }
    LC_StyleMetricsUtils::applyDensityPreset(m_workingConfig, presetName);
    notifyWorkingConfigChanged();
    emit configLoaded(m_workingConfig);
}

void LC_PresetManagerFusionMetrics::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewMetrics(m_workingConfig);
    }
}

void LC_PresetManagerFusionMetrics::resetToDefaults(StyleMetricsConfig& config) {
    LC_StyleMetricsUtils::initializeDefault(config);
}

void LC_PresetManagerFusionMetrics::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveMetrics(activeKey);
    }
}
