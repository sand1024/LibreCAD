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
#include <QMessageBox>
#include "lc_icons_style_manager.h"
#include "lc_icons_style_repository.h"
#include "lc_palette_color_utils.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_PresetManagerIconsStyle::LC_PresetManagerIconsStyle(QObject* parent)
    : QObject(parent){
    m_styleManager=  QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    m_repository = m_styleManager->getIconsStyleRepository();
    m_currentVariantDark = LC_PaletteColorUtils::isSystemInDarkMode();
    m_originalActiveKey = m_styleManager ? m_styleManager->getActiveIconStyle() : DEFAULT_THEME_KEY;
    m_activeKey = m_originalActiveKey;
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerIconsStyle::presetStrings() const {
    LC_PresetManagerUIStrings s;
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
    s.saveModifiedPromptTitle = tr("Unsaved Changes");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to icon style preset '%1'. Do you want to save them before applying?");
    s.discardConfirmTitle = tr("Unsaved Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to icon style preset '%1'. Do you want to discard these changes?");
    return s;
}

bool LC_PresetManagerIconsStyle::loadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        m_iconColorsOptions.resetToDefaults();
        m_workingConfig = IconStyleConfig();
        m_workingConfig.name = tr("Default");
        m_iconColorsOptions.exportStyleConfig(m_workingConfig, true);
        m_iconColorsOptions.exportStyleConfig(m_workingConfig, false);
        m_activeKey = DEFAULT_THEME_KEY;
    }
    else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            m_iconColorsOptions.resetToDefaults();
            m_workingConfig = IconStyleConfig();
            m_workingConfig.name = tr("Default");
        }
        m_iconColorsOptions.importStyleConfig(m_workingConfig, m_currentVariantDark);
        m_activeKey = key;
    }

    m_isDirty = false;
    if (m_changedCallback) {
        m_changedCallback(false);
    }

    emit configLoaded();
    applyTransientStyle();
    return true;
}

bool LC_PresetManagerIconsStyle::saveCurrentPreset() {
    if (m_activeKey == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
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

bool LC_PresetManagerIconsStyle::savePresetAs(const QString& name, QString& outKey) {
    if (m_repository == nullptr) {
        return false;
    }
    m_workingConfig.name = name;
    m_iconColorsOptions.exportStyleConfig(m_workingConfig, m_currentVariantDark);
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

bool LC_PresetManagerIconsStyle::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
        return false;
    }
    if (key == m_originalActiveKey) {
        return false;
    }
    return m_repository->removeByKey(key);
}

QList<QPair<QString, QString>> LC_PresetManagerIconsStyle::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default (Classic)"), DEFAULT_THEME_KEY));
    if (m_repository != nullptr) {
        choices.append(m_repository->getPresetChoices());
    }
    return choices;
}

QString LC_PresetManagerIconsStyle::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_PresetManagerIconsStyle::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveIconStyle() : m_originalActiveKey;
}

void LC_PresetManagerIconsStyle::applyCurrentPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveIconStyle(m_activeKey);
        m_styleManager->applyActiveIconStyle();
        m_originalActiveKey = m_activeKey;
        m_isDirty = false;
        if (m_changedCallback) {
            m_changedCallback(false);
        }
    }
}

void LC_PresetManagerIconsStyle::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_previewController != nullptr) {
        connect(m_previewController, &LC_StylingPreviewController::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType) {
            applyTransientStyle();
        });
    }
}

QWidget* LC_PresetManagerIconsStyle::getSharedBottomWidget() {
    return (m_previewController != nullptr)
               ? m_previewController->createBottomWidget(false, true)
               : nullptr;
}

void LC_PresetManagerIconsStyle::rollbackState() {
    loadPreset(m_originalActiveKey);
}

void LC_PresetManagerIconsStyle::setChangedCallback(std::function<void(bool)> callback) {
    m_changedCallback = std::move(callback);
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

void LC_PresetManagerIconsStyle::notifyWorkingConfigChanged() {
    m_isDirty = true;
    if (m_changedCallback) {
        m_changedCallback(true);
    }
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

bool LC_PresetManagerIconsStyle::onDialogAccept(QWidget* parentDialog) {
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false;
    }
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveIconStyle(m_activeKey);
    }
    return true;
}

bool LC_PresetManagerIconsStyle::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}
