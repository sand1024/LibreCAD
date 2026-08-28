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

#include "lc_preset_manager_metrics.h"
#include <QMessageBox>
#include "lc_fusion_skins_repository.h"
#include "lc_metrics_density_header_bar.h"
#include "lc_metrics_repository.h"
#include "lc_settings_app_styling.h"
#include "lc_style_metrics_utils.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_PresetManagerMetrics::LC_PresetManagerMetrics(QObject* parent)
    : QObject(parent){

    m_styleManager=  QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    m_repository = m_styleManager->getMetricsRepository();
    m_originalActiveKey = m_styleManager ? m_styleManager->getActiveMetrics() : DEFAULT_THEME_KEY;
    m_activeKey = m_originalActiveKey;

    m_headerBar = std::make_unique<LC_MetricsDensityHeaderBar>();
    connect(m_headerBar.get(), &LC_MetricsDensityHeaderBar::densityPresetSelected, this, &LC_PresetManagerMetrics::onDensityPresetSelected);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerMetrics::presetStrings() const {
    LC_PresetManagerUIStrings s;
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
    s.presetFileFilter = tr("Metrics Files (*.lcms)");

    s.defaultReadOnlyMessage = tr("The Default metrics preset is a read-only template. To customize layout metrics, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Metrics...");
    s.saveModifiedPromptTitle = tr("Unsaved Changes");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to metrics preset '%1'. Do you want to save them before applying?");
    s.discardConfirmTitle = tr("Unsaved Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to metrics preset '%1'. Do you want to discard these changes?");
    return s;
}

bool LC_PresetManagerMetrics::isGated() const {
    return LC_SettingsManagerStyling::isFusionGated() || isReadOnlyDefault();
}

QString LC_PresetManagerMetrics::gatedMessage() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("metrics"));
    }
    return presetStrings().defaultReadOnlyMessage;
}

QString LC_PresetManagerMetrics::gatedActionText() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    return presetStrings().duplicateActionText;
}

std::function<void()> LC_PresetManagerMetrics::gatedActionCallback() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return [this]() {
            LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        };
    }
    return nullptr;
}

bool LC_PresetManagerMetrics::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        LC_StyleMetricsUtils::initializeDefault(m_workingConfig);
        m_activeKey = DEFAULT_THEME_KEY;
        if (m_headerBar != nullptr) {
            m_headerBar->setDensityPreset("standard");
        }
    }
    else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            LC_StyleMetricsUtils::initializeDefault(m_workingConfig);
        }
        m_activeKey = key;
        if (m_headerBar != nullptr) {
            m_headerBar->setDensityToCustom();
        }
    }

    m_isDirty = false;
    if (m_changedCallback) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);
    updatePreview();
    return true;
}

bool LC_PresetManagerMetrics::saveCurrentPreset() {
    if (m_activeKey == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    QString outKey;
    if (m_repository->save(m_workingConfig.name, m_workingConfig, outKey)) {
        m_activeKey = outKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
        return true;
    }
    return false;
}

bool LC_PresetManagerMetrics::savePresetAs(const QString& name, QString& outKey) {
    if (m_repository == nullptr) {
        return false;
    }
    m_workingConfig.name = name;
    if (m_repository->save(name, m_workingConfig, outKey)) {
        m_activeKey = outKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
        return true;
    }
    return false;
}

bool LC_PresetManagerMetrics::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    if (key == m_originalActiveKey) {
        return false;
    }
    return m_repository->removeByKey(key);
}

QList<QPair<QString, QString>> LC_PresetManagerMetrics::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Fusion Metrics"), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

QString LC_PresetManagerMetrics::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_PresetManagerMetrics::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveMetrics() : m_originalActiveKey;
}

void LC_PresetManagerMetrics::applyCurrentPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveMetrics(m_activeKey);
        m_styleManager->applyActiveStyleAndTheme();
        m_originalActiveKey = m_activeKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
    }
}

void LC_PresetManagerMetrics::rollbackState() {
    loadPreset(m_originalActiveKey);
}

void LC_PresetManagerMetrics::setChangedCallback(std::function<void(bool)> callback) {
    m_changedCallback = std::move(callback);
}

void LC_PresetManagerMetrics::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    updatePreview();
}

QWidget* LC_PresetManagerMetrics::getSharedBottomWidget() {
    return (m_previewController != nullptr) ? m_previewController->createBottomWidget(true, false) : nullptr;
}

QWidget* LC_PresetManagerMetrics::getSharedHeaderWidget() {
    return m_headerBar.get();
}

void LC_PresetManagerMetrics::onSubPageControlChanged() {
    if (m_headerBar != nullptr) {
        m_headerBar->setDensityToCustom();
    }
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    updatePreview();
}

void LC_PresetManagerMetrics::onDensityPresetSelected(const QString& presetName) {
    if (presetName == "custom") {
        return;
    }
    LC_StyleMetricsUtils::applyDensityPreset(m_workingConfig, presetName);
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    emit configLoaded(m_workingConfig);
    updatePreview();
}

void LC_PresetManagerMetrics::updatePreview() const {
    if (m_previewController != nullptr && m_styleManager != nullptr) {
        SkinConfig activeSkin;
        if (!m_styleManager->getSkinsRepository()->loadByKey(m_styleManager->getActiveSkin(), activeSkin)) {
            LC_PaletteColorUtils::initializeDefaultConfig(activeSkin);
        }
        m_previewController->updatePreviewMetrics(m_workingConfig, activeSkin);
    }
}

bool LC_PresetManagerMetrics::onDialogAccept(QWidget* parentDialog) {
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false;
    }
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveMetrics(m_activeKey);
    }
    return true;
}

bool LC_PresetManagerMetrics::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}
