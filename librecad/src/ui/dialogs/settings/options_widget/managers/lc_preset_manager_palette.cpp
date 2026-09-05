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

#include "lc_preset_manager_palette.h"
#include "lc_palette_color_utils.h"
#include "lc_settings_manager_styling.h"

LC_PresetManagerPalette::LC_PresetManagerPalette(QObject* parent)
    : LC_PresetManagerBase(
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getPaletteRepository(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getActivePalette(),
          parent)
    , m_currentVariantDark(LC_PaletteColorUtils::isSystemInDarkMode()) {
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerPalette::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Palette");
    s.labelText = tr("Color palette preset:");
    s.selectToolTip = tr("Select a saved color palette configuration or load standard defaults.");
    s.saveToolTip = tr("Save changes directly to active color palette preset.");
    s.saveAsToolTip = tr("Save current color palette configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom color palette from disk.");
    s.applyToolTip = tr("Apply the active color palette globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the color palette preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Palette Preset As");
    s.saveAsDialogLabel = tr("Enter unique color palette preset name:");
    s.defaultNewPresetName = tr("Custom Palette");
    s.deleteConfirmTitle = tr("Delete Color Palette");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the color palette '%1'?");
    s.presetFileFilter = tr("Color Palette Files (*.lcpl)");

    s.defaultReadOnlyMessage = tr("The Default color palette is a read-only template. To customize palette colors, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Palette...");
    return s;
}

bool LC_PresetManagerPalette::loadPreset(const QString& key) {
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

QString LC_PresetManagerPalette::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActivePalette() : m_originalActiveKey;
}

void LC_PresetManagerPalette::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActivePalette(activeKey);
    }
}

void LC_PresetManagerPalette::applyCurrentPreset() {
    // Reuses the core registration
    applyActiveConfigToSystem(m_activeKey);

    // Forces immediate live theme redraw on main window
    if (m_styleManager != nullptr) {
        m_styleManager->applyActiveStyleAndTheme();
    }
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}

void LC_PresetManagerPalette::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        connect(m_previewController, &LC_StylingPreviewController::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType) {
            updatePreview();
        });
    }
    updatePreview();
}

void LC_PresetManagerPalette::setCurrentVariantDark(bool dark) {
    if (m_currentVariantDark == dark) {
        return;
    }
    m_currentVariantDark = dark;
    emit variantChanged(m_currentVariantDark);
    updatePreview();
}

void LC_PresetManagerPalette::updatePreview() {
    const ColorSchemeData& scheme = m_currentVariantDark ? m_workingConfig.dark : m_workingConfig.light;
    LC_PaletteColorUtils::resolveSemanticColors(true, scheme, m_currentVariantDark);
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewPalette(m_workingConfig, m_currentVariantDark);
    }
}

void LC_PresetManagerPalette::resetToDefaults(PaletteConfig& config) {
    LC_PaletteColorUtils::initializeDefaultPalette(config);
}

void LC_PresetManagerPalette::calculateProceduralBevels(bool isDarkMode, StyleArchetype archetype) {
    ColorSchemeData& scheme = isDarkMode ? m_workingConfig.dark : m_workingConfig.light;

    QPalette tempPalette;
    for (const auto& roleMapping : BASE_INTERFACE_ROLES) {
        for (const auto& stateMapping : PALETTE_STATES) {
            const QColor col = scheme.palette[roleMapping.name][stateMapping.name];
            tempPalette.setColor(stateMapping.group, roleMapping.role, col);
        }
    }

    auto calculatedBevel = BevelStyle::Soft;
    if (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline) {
        calculatedBevel = BevelStyle::Flat;
    } else if (archetype == StyleArchetype::SoftSatin) {
        calculatedBevel = BevelStyle::Soft;
    } else if (archetype == StyleArchetype::GlassyGloss) {
        calculatedBevel = BevelStyle::Sharp;
    }

    LC_PaletteColorUtils::calculate3DHelperRoles(tempPalette, calculatedBevel, scheme.contrastWeight, scheme.bevelSeedRole);

    for (const auto& roleMapping : BEVEL_HELPER_ROLES) {
        for (const auto& stateMapping : PALETTE_STATES) {
            const QColor col = tempPalette.color(stateMapping.group, roleMapping.role);
            scheme.palette[roleMapping.name][stateMapping.name] = col;
        }
    }

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerPalette::generateHarmonizedTheme(const QColor& baseColor) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHarmonizedTheme(baseColor, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerPalette::generateTwoColorTheme(const QColor& surface, const QColor& accent) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHarmonizedTheme(surface, accent, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerPalette::generateHighContrastTheme(const QColor& baseColor) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHighContrastTheme(baseColor, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerPalette::activatePreviewTab(const QString& tag) {
    if (m_previewController != nullptr) {
        m_previewController->activatePreviewTab(tag);
    }
}

bool LC_PresetManagerPalette::isGated() const {
    return LC_SettingsManagerStyling::isFusionGated() || isReadOnlyDefault();
}

QString LC_PresetManagerPalette::gatedMessage() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("color palettes"));
    }
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    return QString();
}

QString LC_PresetManagerPalette::gatedActionText() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_PresetManagerPalette::gatedActionCallback() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return [this]() {
            LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        };
    }
    return nullptr;
}
