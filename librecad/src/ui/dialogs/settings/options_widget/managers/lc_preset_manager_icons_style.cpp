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

#include "lc_preset_manager_icons_style.h"

#include "lc_icons_style_manager.h"
#include "lc_palette_color_utils.h"
#include "qc_applicationwindow.h"

LC_PresetManagerIconsStyle::LC_PresetManagerIconsStyle(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<IconStyleConfig, LC_RepositoryIconsStyle>(
          styleManager,
          styleManager != nullptr ? styleManager->getIconsStyleRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActiveIconStyle() : QString(),
          nullptr)
    , m_currentVariantDark(LC_PaletteColorUtils::isSystemInDarkMode()) {
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerIconsStyle::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default (Classic)");
    s.labelText = tr("Icons style preset:");
    s.selectToolTip = tr("Select a saved custom icon color style or load system defaults.");
    s.saveToolTip = tr("Save changes directly to active icon style preset.");
    s.saveAsToolTip = tr("Save current icon color configuration as a new style preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom icon style from disk.");
    s.applyToolTip = tr("Apply the active icon style globally to application toolbars and docks.");
    s.revertToolTip = tr("Discard modifications and reload the icon style as saved on disk.");
    s.saveAsDialogTitle = tr("Save Icon Style As");
    s.saveAsDialogLabel = tr("Enter unique icon style name:");
    s.defaultNewPresetName = tr("Custom Icons Style");
    s.deleteConfirmTitle = tr("Delete Icon Style");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the icon style '%1'?");
    s.presetFileFilter = tr("LibreCAD Icon Style Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".json"));

    s.defaultReadOnlyMessage = tr(
        "The Default icon style is a read-only template. To customize icon colors, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Icon Style...");
    return s;
}

bool LC_PresetManagerIconsStyle::saveCurrentPreset() {
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    return LC_PresetManagerStylingBase<IconStyleConfig, LC_RepositoryIconsStyle>::saveCurrentPreset();
}

bool LC_PresetManagerIconsStyle::savePresetAs(const QString& name, QString& outKey) {
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    return LC_PresetManagerStylingBase<IconStyleConfig, LC_RepositoryIconsStyle>::savePresetAs(name, outKey);
}

QString LC_PresetManagerIconsStyle::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActiveIconStyle();
    }
    return m_originalActiveKey;
}

void LC_PresetManagerIconsStyle::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveIconStyle(activeKey);
    }
}

void LC_PresetManagerIconsStyle::onPostApplyPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->applyActiveIconStyle();
    }
}

void LC_PresetManagerIconsStyle::onPostLoadPreset() {
    m_iconColorsOptions.importStyleConfig(m_workingConfig, m_currentVariantDark);
}

void LC_PresetManagerIconsStyle::emitConfigLoaded() {
    emit configLoaded();
}

void LC_PresetManagerIconsStyle::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        connect(m_previewController, &LC_StylingPreviewController::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType) {
            applyTransientStyle();
        });
    }
    updatePreview();
}

void LC_PresetManagerIconsStyle::setCurrentVariantDark(bool dark) {
    if (m_currentVariantDark == dark) {
        return;
    }
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    m_currentVariantDark = dark;
    m_iconColorsOptions.importStyleConfig(m_workingConfig, m_currentVariantDark);

    emit variantChanged(m_currentVariantDark);
    applyTransientStyle();
}

void LC_PresetManagerIconsStyle::applyTransientStyle() {
    LC_IconColorsOptions tempOptions;
    tempOptions.loadSettings();

    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    tempOptions.importStyleConfig(m_workingConfig, m_currentVariantDark);

    const auto cvdType = (m_previewController != nullptr)
                             ? m_previewController->activeCvdType()
                             : LC_PaletteColorUtils::CVDType::Normal;

    auto* appWindow = QC_ApplicationWindow::getAppWindow();
    if (appWindow != nullptr) {
        LC_IconsStyleManager::applyStyle(appWindow, tempOptions, m_currentVariantDark, cvdType);
    }
}

void LC_PresetManagerIconsStyle::updatePreview() {
    applyTransientStyle();
}

void LC_PresetManagerIconsStyle::resetToDefaults(IconStyleConfig& config) {
    m_iconColorsOptions.resetToDefaults();
    config = IconStyleConfig();
    config.name = defaultPresetDisplayName();
    m_iconColorsOptions.exportStyleConfig(config, true);
    m_iconColorsOptions.exportStyleConfig(config, false);
}
