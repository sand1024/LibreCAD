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

LC_PresetManagerIconsStyle::LC_PresetManagerIconsStyle(QObject* parent)
    : LC_PresetManagerBase(
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getIconsStyleRepository(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getActiveIconStyle(),
          parent)
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
    s.presetFileFilter = tr("Icon Style Files (*.lcis)");

    s.defaultReadOnlyMessage = tr("The Default icon style is a read-only template. To customize icon colors, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Icon Style...");
    return s;
}

bool LC_PresetManagerIconsStyle::loadPreset(const QString& key) {
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
            m_iconColorsOptions.importStyleConfig(m_workingConfig, m_currentVariantDark);
            m_activeKey = key;
        }
    }

    m_isDirty = false;
    if (m_changedCallback != nullptr) {
        m_changedCallback(false);
    }

    emit configLoaded();
    applyTransientStyle();
    return true;
}

bool LC_PresetManagerIconsStyle::saveCurrentPreset() {
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    return LC_PresetManagerBase::saveCurrentPreset();
}

bool LC_PresetManagerIconsStyle::savePresetAs(const QString& name, QString& outKey) {
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
    return LC_PresetManagerBase::savePresetAs(name, outKey);
}

QString LC_PresetManagerIconsStyle::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveIconStyle() : m_originalActiveKey;
}

void LC_PresetManagerIconsStyle::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveIconStyle(activeKey);
    }
}

void LC_PresetManagerIconsStyle::applyCurrentPreset() {
    applyActiveConfigToSystem(m_activeKey);
    if (m_styleManager != nullptr) {
        m_styleManager->applyActiveIconStyle();
    }
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
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

    LC_IconsStyleManager::applyStyle(tempOptions, m_currentVariantDark, cvdType);
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

bool LC_PresetManagerIconsStyle::isGated() const {
    return isReadOnlyDefault();
}

QString LC_PresetManagerIconsStyle::gatedMessage() const {
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    return QString();
}

QString LC_PresetManagerIconsStyle::gatedActionText() const {
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_PresetManagerIconsStyle::gatedActionCallback() const {
    return nullptr;
}
