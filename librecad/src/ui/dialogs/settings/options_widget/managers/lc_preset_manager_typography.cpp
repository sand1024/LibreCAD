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
#include "lc_preset_manager_typography.h"
#include <QMessageBox>
#include "lc_styling_preview_controller.h"
#include "lc_typography_repository.h"
#include "lc_typography_utils.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_PresetManagerTypography::LC_PresetManagerTypography(QObject* parent)
    : QObject(parent){
    m_styleManager=  QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    m_repository = m_styleManager->getTypographyRepository();
    m_originalActiveKey = m_styleManager ? m_styleManager->getActiveTypography() : DEFAULT_THEME_KEY;
    m_activeKey = m_originalActiveKey;
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerTypography::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.labelText = tr("Typography preset:");
    s.selectToolTip = tr("Select a typography configuration preset.");
    s.saveToolTip = tr("Save changes directly to active typography preset.");
    s.saveAsToolTip = tr("Save current typography configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected typography preset from disk.");
    s.applyToolTip = tr("Apply the active typography configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the typography preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Typography Preset As");
    s.saveAsDialogLabel = tr("Enter unique typography preset name:");
    s.defaultNewPresetName = tr("Custom Typography");
    s.deleteConfirmTitle = tr("Delete Typography Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the typography preset '%1'?");
    s.presetFileFilter = tr("Typography Files (*.lcft)");

    s.defaultReadOnlyMessage = tr("The Default typography preset is a read-only template. To customize font settings, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Typography...");
    s.saveModifiedPromptTitle = tr("Unsaved Changes");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to typography preset '%1'. Do you want to save them before applying?");
    s.discardConfirmTitle = tr("Unsaved Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to typography preset '%1'. Do you want to discard these changes?");
    return s;
}

bool LC_PresetManagerTypography::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        LC_TypographyUtils::initializeDefaultConfig(m_workingConfig);
        m_activeKey = DEFAULT_THEME_KEY;
    }
    else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            LC_TypographyUtils::initializeDefaultConfig(m_workingConfig);
        }
        m_activeKey = key;
    }

    m_isDirty = false;
    if (m_changedCallback) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);

    if (m_previewController) {
        m_previewController->updatePreviewTypography(m_workingConfig);
    }
    return true;
}

bool LC_PresetManagerTypography::saveCurrentPreset() {
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

bool LC_PresetManagerTypography::savePresetAs(const QString& name, QString& outKey) {
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

bool LC_PresetManagerTypography::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    if (key == m_originalActiveKey) {
        return false;
    }
    return m_repository->removeByKey(key);
}

QList<QPair<QString, QString>> LC_PresetManagerTypography::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Typography"), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

QString LC_PresetManagerTypography::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_PresetManagerTypography::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveTypography() : m_originalActiveKey;
}


void LC_PresetManagerTypography::applyCurrentPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveTypography(m_activeKey);
        m_styleManager->applyActiveStyleAndTheme();
        m_originalActiveKey = m_activeKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
    }
}

void LC_PresetManagerTypography::rollbackState() {
    loadPreset(m_originalActiveKey);
}

void LC_PresetManagerTypography::setChangedCallback(std::function<void(bool)> callback) {
    m_changedCallback = std::move(callback);
}

QWidget* LC_PresetManagerTypography::getSharedBottomWidget() {
    return (m_previewController != nullptr) ? m_previewController->createBottomWidget(true, false,nullptr) : nullptr;
}

void LC_PresetManagerTypography::notifyWorkingConfigChanged() {
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewTypography(m_workingConfig);
    }
}

bool LC_PresetManagerTypography::onDialogAccept(QWidget* parentDialog) {
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false;
    }
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveTypography(m_activeKey);
    }
    return true;
}

bool LC_PresetManagerTypography::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}
void LC_PresetManagerTypography::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewTypography(m_workingConfig);
    }
}
