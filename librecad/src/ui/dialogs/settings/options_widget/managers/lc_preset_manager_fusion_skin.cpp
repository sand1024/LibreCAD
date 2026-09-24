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

#include "lc_preset_manager_fusion_skin.h"

#include "lc_palette_color_utils.h"

LC_PresetManagerFusionSkin::LC_PresetManagerFusionSkin(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<ControlStyleConfig, LC_RepositoryFusionSkin>(
          styleManager,
          styleManager != nullptr ? styleManager->getSkinsRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActiveSkin() : QString(),
          nullptr) {
    m_headerBar = std::make_unique<LC_SkinArchetypeHeaderBar>();
    auto* bar = m_headerBar.get();
    connect(bar, &LC_SkinArchetypeHeaderBar::archetypeChanged, this, &LC_PresetManagerFusionSkin::onArchetypeChanged);
    connect(bar, &LC_SkinArchetypeHeaderBar::decorationChanged, this, &LC_PresetManagerFusionSkin::onDecorationChanged);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerFusionSkin::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Fusion Style");
    s.labelText = tr("Controls style preset:");
    s.selectToolTip = tr("Select a saved controls style configuration or load standard defaults.");
    s.saveToolTip = tr("Save changes directly to active controls style preset.");
    s.saveAsToolTip = tr("Save current controls style configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom style preset from disk.");
    s.applyToolTip = tr("Apply the active controls style configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the controls style preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Controls Style Preset As");
    s.saveAsDialogLabel = tr("Enter unique controls style preset name:");
    s.defaultNewPresetName = tr("Custom Controls Style");
    s.deleteConfirmTitle = tr("Delete Controls Style Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the controls style preset '%1'?");
    s.presetFileFilter = tr("LibreCAD Fusion Theme configuration (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".json"));

    s.defaultReadOnlyMessage = tr(
        "The Default controls style is a read-only template. To customize widget decorators and styling, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Style...");
    return s;
}

QString LC_PresetManagerFusionSkin::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActiveSkin();
    }
    return m_originalActiveKey;
}

QWidget* LC_PresetManagerFusionSkin::getSharedHeaderWidget() {
    return m_headerBar.get();
}

QString LC_PresetManagerFusionSkin::fusionGatingSubject() const {
    return tr("controls styling");
}

QString LC_PresetManagerFusionSkin::gatedMessage() const {
    if (isClassicFusion()) {
        return tr("The Classic Fusion archetype uses native Qt widget drawing. Sub-controls for custom grips, outlines, and item view hover are not applicable.");
    }
    return LC_PresetManagerStylingBase::gatedMessage();
}

void LC_PresetManagerFusionSkin::onPostLoadPreset() {
    if (m_headerBar != nullptr) {
        m_headerBar->populateFromConfig(m_workingConfig);
    }
}

void LC_PresetManagerFusionSkin::emitConfigLoaded() {
    emit configLoaded(m_workingConfig);
}

void LC_PresetManagerFusionSkin::onArchetypeChanged(StyleArchetype archetype) {
    m_workingConfig.styleArchetype = archetype;
    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionSkin::onDecorationChanged(BoxDecoration decoration) {
    m_workingConfig.boxDecoration = decoration;
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionSkin::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewSkin(m_workingConfig);
    }
}

void LC_PresetManagerFusionSkin::resetToDefaults(ControlStyleConfig& config) {
    LC_PaletteColorUtils::initializeDefaultControlStyle(config);
}

void LC_PresetManagerFusionSkin::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveSkin(activeKey);
    }
}
