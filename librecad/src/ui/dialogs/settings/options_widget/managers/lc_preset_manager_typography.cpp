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

#include "lc_preset_manager_typography.h"

#include "lc_typography_utils.h"

LC_PresetManagerTypography::LC_PresetManagerTypography(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<FontConfig, LC_RepositoryTypography>(
          styleManager,
          styleManager != nullptr ? styleManager->getTypographyRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActiveTypography() : QString(),
          nullptr) {
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerTypography::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Typography");
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
    s.presetFileFilter = tr("LibreCAD Typography Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".json"));

    s.defaultReadOnlyMessage = tr(
        "The Default typography preset is a read-only template. To customize font settings, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Typography...");
    return s;
}

QString LC_PresetManagerTypography::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActiveTypography();
    }
    return m_originalActiveKey;
}

void LC_PresetManagerTypography::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewTypography(m_workingConfig);
    }
}

void LC_PresetManagerTypography::resetToDefaults(FontConfig& config) {
    LC_TypographyUtils::initializeDefaultConfig(config);
}

void LC_PresetManagerTypography::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveTypography(activeKey);
    }
}

void LC_PresetManagerTypography::emitConfigLoaded() {
    emit configLoaded(m_workingConfig);
}
