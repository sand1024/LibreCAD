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

#include "lc_preset_manager_fusion_skin.h"
#include <QMessageBox>
#include "lc_fusion_skins_repository.h"
#include "lc_palette_color_utils.h"
#include "lc_settings_app_styling.h"
#include "lc_skin_archetype_header_bar.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_PresetManagerFusionSkin::LC_PresetManagerFusionSkin(QObject* parent)
    : QObject(parent){
    m_styleManager=  QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    m_repository = m_styleManager->getSkinsRepository();
    m_currentVariantDark = LC_PaletteColorUtils::isSystemInDarkMode();
    m_originalActiveKey = m_styleManager ? m_styleManager->getActiveSkin() : DEFAULT_THEME_KEY;
    m_activeKey = m_originalActiveKey;

    m_headerBar = std::make_unique<LC_SkinArchetypeHeaderBar>();
    connect(m_headerBar.get(), &LC_SkinArchetypeHeaderBar::archetypeChanged, this, &LC_PresetManagerFusionSkin::onArchetypeChanged);
    connect(m_headerBar.get(), &LC_SkinArchetypeHeaderBar::decorationChanged, this, &LC_PresetManagerFusionSkin::onDecorationChanged);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerFusionSkin::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.labelText = tr("Fusion skin preset:");
    s.selectToolTip = tr("Select a saved Fusion skin configuration or load standard defaults.");
    s.saveToolTip = tr("Save changes directly to active skin preset.");
    s.saveAsToolTip = tr("Save current skin configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom skin from disk.");
    s.applyToolTip = tr("Apply the active skin configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the skin preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Skin Preset As");
    s.saveAsDialogLabel = tr("Enter unique skin preset name:");
    s.defaultNewPresetName = tr("Custom Skin");
    s.deleteConfirmTitle = tr("Delete Skin Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the skin preset '%1'?");
    s.presetFileFilter = tr("Skin Files (*.lcsk)");

    s.defaultReadOnlyMessage = tr("The Default Fusion skin is a read-only template. To customize colors and styling, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Skin...");
    s.saveModifiedPromptTitle = tr("Unsaved Changes");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to skin preset '%1'. Do you want to save them before applying?");
    s.discardConfirmTitle = tr("Unsaved Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to skin preset '%1'. Do you want to discard these changes?");
    return s;
}

bool LC_PresetManagerFusionSkin::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        LC_PaletteColorUtils::initializeDefaultConfig(m_workingConfig);
        m_activeKey = DEFAULT_THEME_KEY;
    }
    else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            LC_PaletteColorUtils::initializeDefaultConfig(m_workingConfig);
        }
        m_activeKey = key;
    }

    if (m_headerBar != nullptr) {
        m_headerBar->populateFromConfig(m_workingConfig);
    }

    m_isDirty = false;
    if (m_changedCallback) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);
    updatePreview();
    return true;
}

bool LC_PresetManagerFusionSkin::saveCurrentPreset() {
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

bool LC_PresetManagerFusionSkin::savePresetAs(const QString& name, QString& outKey) {
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

bool LC_PresetManagerFusionSkin::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    if (key == m_originalActiveKey) {
        return false;
    }
    return m_repository->removeByKey(key);
}

QList<QPair<QString, QString>> LC_PresetManagerFusionSkin::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Fusion Skin"), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

QString LC_PresetManagerFusionSkin::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_PresetManagerFusionSkin::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveSkin() : m_originalActiveKey;
}

void LC_PresetManagerFusionSkin::applyCurrentPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveSkin(m_activeKey);
        m_styleManager->applyActiveStyleAndTheme();
        m_originalActiveKey = m_activeKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
    }
}

void LC_PresetManagerFusionSkin::rollbackState() {
    loadPreset(m_originalActiveKey);
}

void LC_PresetManagerFusionSkin::setChangedCallback(std::function<void(bool)> callback) {
    m_changedCallback = std::move(callback);
}

void LC_PresetManagerFusionSkin::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        connect(m_previewController, &LC_StylingPreviewController::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType) {
            updatePreview();
        });
    }
    updatePreview();
}

QWidget* LC_PresetManagerFusionSkin::getSharedBottomWidget() {
    return (m_previewController != nullptr) ? m_previewController->createBottomWidget(true, true) : nullptr;
}

QWidget* LC_PresetManagerFusionSkin::getSharedHeaderWidget() {
    return m_headerBar.get();
}

void LC_PresetManagerFusionSkin::setCurrentVariantDark(bool dark) {
    if (m_currentVariantDark == dark) {
        return;
    }
    m_currentVariantDark = dark;
    emit variantChanged(m_currentVariantDark);
    updatePreview();
}

void LC_PresetManagerFusionSkin::onArchetypeChanged(StyleArchetype archetype) {
    m_workingConfig.styleArchetype = archetype;
    if (m_workingConfig.light.autoCalculate3DHelpers) {
        calculateProceduralBevels(false);
    }
    if (m_workingConfig.dark.autoCalculate3DHelpers) {
        calculateProceduralBevels(true);
    }
    onSubPageControlChanged();
}

void LC_PresetManagerFusionSkin::onDecorationChanged(BoxDecoration decoration) {
    m_workingConfig.boxDecoration = decoration;
    onSubPageControlChanged();
}

void LC_PresetManagerFusionSkin::onSubPageControlChanged() {
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    updatePreview();
}

void LC_PresetManagerFusionSkin::updatePreview() {
    if (m_previewController != nullptr) {
        const auto cvd = m_previewController->activeCvdType();
        m_previewController->updatePreviewSkin(m_workingConfig, m_currentVariantDark, cvd);
    }
}

void LC_PresetManagerFusionSkin::calculateProceduralBevels(bool isDarkMode) {
    ColorSchemeData& scheme = isDarkMode ? m_workingConfig.dark : m_workingConfig.light;

    QPalette tempPalette;
    for (const auto& roleMapping : BASE_INTERFACE_ROLES) {
        for (const auto& stateMapping : PALETTE_STATES) {
            const QColor col = scheme.palette[roleMapping.name][stateMapping.name];
            tempPalette.setColor(stateMapping.group, roleMapping.role, col);
        }
    }

    auto calculatedBevel = BevelStyle::Soft;
    if (m_workingConfig.styleArchetype == StyleArchetype::FlatModern || m_workingConfig.styleArchetype == StyleArchetype::AccentOutline) {
        calculatedBevel = BevelStyle::Flat;
    }
    else if (m_workingConfig.styleArchetype == StyleArchetype::SoftSatin) {
        calculatedBevel = BevelStyle::Soft;
    }
    else if (m_workingConfig.styleArchetype == StyleArchetype::GlassyGloss) {
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
    updatePreview();
}

void LC_PresetManagerFusionSkin::generateHarmonizedTheme(const QColor& baseColor) {
    LC_PaletteColorUtils::generateHarmonizedTheme(baseColor, m_workingConfig);
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    emit configLoaded(m_workingConfig);
    updatePreview();
}

void LC_PresetManagerFusionSkin::generateTwoColorTheme(const QColor& surface, const QColor& accent) {
    LC_PaletteColorUtils::generateHarmonizedTheme(surface, accent, m_workingConfig);
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    emit configLoaded(m_workingConfig);
    updatePreview();
}

void LC_PresetManagerFusionSkin::generateHighContrastTheme(const QColor& baseColor) {
    LC_PaletteColorUtils::generateHighContrastTheme(baseColor, m_workingConfig);
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
    emit configLoaded(m_workingConfig);
    updatePreview();
}

bool LC_PresetManagerFusionSkin::onDialogAccept(QWidget* parentDialog) {
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false;
    }
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveSkin(m_activeKey);
    }
    return true;
}

bool LC_PresetManagerFusionSkin::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}

bool LC_PresetManagerFusionSkin::isGated() const {
    return LC_SettingsManagerStyling::isFusionGated() || isReadOnlyDefault();
}

QString LC_PresetManagerFusionSkin::gatedMessage() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("skins"));
    }
    return presetStrings().defaultReadOnlyMessage;
}

QString LC_PresetManagerFusionSkin::gatedActionText() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    return presetStrings().duplicateActionText;
}

std::function<void()> LC_PresetManagerFusionSkin::gatedActionCallback() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return [this]() {
            LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        };
    }
    return nullptr;
}
