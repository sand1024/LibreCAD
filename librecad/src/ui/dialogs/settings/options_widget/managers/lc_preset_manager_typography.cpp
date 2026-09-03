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

LC_PresetManagerTypography::LC_PresetManagerTypography(QObject* parent)
    : LC_PresetManagerBase(
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getTypographyRepository(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getActiveTypography(),
          parent) {
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
    s.presetFileFilter = tr("Typography Files (*.lcft)");

    s.defaultReadOnlyMessage = tr("The Default typography preset is a read-only template. To customize font settings, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Typography...");
    return s;
}

bool LC_PresetManagerTypography::loadPreset(const QString& key) {
    const bool isDefault = (key == DEFAULT_THEME_KEY || key.isEmpty() || key == DEFAULT_THEME_NAME ||
                            (m_repository != nullptr && !m_repository->exists(key)));

    if (isDefault) {
        resetToDefaults(m_workingConfig);
        m_workingConfig.name = defaultPresetDisplayName();
        m_activeKey = DEFAULT_THEME_KEY;
    } else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            resetToDefaults(m_workingConfig);
            m_workingConfig.name = defaultPresetDisplayName();
            m_activeKey = DEFAULT_THEME_KEY;
        } else {
            m_activeKey = key;
        }
    }

    m_isDirty = false;
    if (m_changedCallback != nullptr) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);
    updatePreview();
    return true;
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
        if (m_changedCallback != nullptr) {
            m_changedCallback(false);
        }
    }
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
