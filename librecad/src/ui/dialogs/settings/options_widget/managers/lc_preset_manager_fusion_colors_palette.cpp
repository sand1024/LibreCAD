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

#include "lc_preset_manager_fusion_colors_palette.h"

LC_PresetManagerFusionColorsPalette::LC_PresetManagerFusionColorsPalette(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<PaletteConfig, LC_RepositoryPalette>(
          styleManager,
          styleManager != nullptr ? styleManager->getPaletteRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActivePalette() : QString(),
          nullptr)
    , m_currentVariantDark(LC_PaletteColorUtils::isSystemInDarkMode()) {
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerFusionColorsPalette::presetStrings() const {
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
    s.presetFileFilter = tr("LibreCAD Fusion Theme Color Palette Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".json"));

    s.defaultReadOnlyMessage = tr("The Default color palette is a read-only template. To customize palette colors, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Palette...");
    return s;
}

QString LC_PresetManagerFusionColorsPalette::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActivePalette();
    }
    return m_originalActiveKey;
}

void LC_PresetManagerFusionColorsPalette::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        connect(m_previewController, &LC_StylingPreviewController::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType) {
            updatePreview();
        });
    }
    updatePreview();
}

void LC_PresetManagerFusionColorsPalette::setCurrentVariantDark(bool dark) {
    if (m_currentVariantDark == dark) {
        return;
    }
    m_currentVariantDark = dark;
    emit variantChanged(m_currentVariantDark);
    updatePreview();
}

void LC_PresetManagerFusionColorsPalette::updatePreview() {
    const ColorSchemeData& scheme = m_currentVariantDark ? m_workingConfig.dark : m_workingConfig.light;
    LC_PaletteColorUtils::resolveSemanticColors(true, scheme, m_currentVariantDark);
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewPalette(m_workingConfig, m_currentVariantDark);
    }
}

void LC_PresetManagerFusionColorsPalette::resetToDefaults(PaletteConfig& config) {
    LC_PaletteColorUtils::initializeDefaultPalette(config);
}

void LC_PresetManagerFusionColorsPalette::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActivePalette(activeKey);
    }
}

void LC_PresetManagerFusionColorsPalette::emitConfigLoaded() {
    emit configLoaded(m_workingConfig);
}

QString LC_PresetManagerFusionColorsPalette::fusionGatingSubject() const {
    return tr("color palettes");
}

void LC_PresetManagerFusionColorsPalette::calculateProceduralBevels(bool isDarkMode, StyleArchetype archetype) {
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
    }
    else if (archetype == StyleArchetype::SoftSatin) {
        calculatedBevel = BevelStyle::Soft;
    }
    else if (archetype == StyleArchetype::GlassyGloss) {
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

void LC_PresetManagerFusionColorsPalette::generateHarmonizedTheme(const QColor& baseColor) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHarmonizedTheme(baseColor, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionColorsPalette::generateTwoColorTheme(const QColor& surface, const QColor& accent) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHarmonizedTheme(surface, accent, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionColorsPalette::generateHighContrastTheme(const QColor& baseColor) {
    PaletteConfig tempSkin;
    tempSkin.light = m_workingConfig.light;
    tempSkin.dark = m_workingConfig.dark;
    LC_PaletteColorUtils::generateHighContrastTheme(baseColor, tempSkin);
    m_workingConfig.light = tempSkin.light;
    m_workingConfig.dark = tempSkin.dark;

    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionColorsPalette::activatePreviewTab(const QString& tag) {
    if (m_previewController != nullptr) {
        m_previewController->activatePreviewTab(tag);
    }
}
