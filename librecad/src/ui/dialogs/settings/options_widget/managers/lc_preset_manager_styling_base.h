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

#ifndef LC_PRESET_MANAGER_STYLING_BASE_H
#define LC_PRESET_MANAGER_STYLING_BASE_H

#include "lc_preset_manager_config_base.h"
#include "lc_settings_manager_styling.h"
#include "lc_styling_preview_controller.h"
#include "lc_ui_style_manager.h"

template <typename TConfig, typename TRepo>
class LC_PresetManagerStylingBase : public LC_PresetManagerConfigBase<TConfig, TRepo>, public LC_StylingPreviewAware {
public:
    explicit LC_PresetManagerStylingBase(LC_UIStyleManager* styleManager, TRepo* repo, const QString& initialKey, QObject* parent = nullptr)
        : LC_PresetManagerConfigBase<TConfig, TRepo>(repo, initialKey, parent)
        , m_styleManager(styleManager) {
        if (this->m_originalActiveKey.isEmpty() ||
            (this->m_repository != nullptr && !this->m_repository->exists(this->m_originalActiveKey))) {
            this->m_originalActiveKey = CFG_AppState::DEFAULT_THEME_KEY;
        }
        this->setActivePresetKey(this->m_originalActiveKey);
    }

    ~LC_PresetManagerStylingBase() override = default;

    bool supportsPreviewWindow() const override { return true; }
    bool supportsAccessibilityCheck() const override { return false; }
    bool supportsImportExport() const override { return false; }

    bool loadPreset(const QString& key) override {
        using namespace CFG_AppState;
        bool success = false;
        const bool isDefault = this->isDefaultPreset(key) ||
            (this->m_repository != nullptr && !this->m_repository->exists(key));

        if (isDefault) {
            resetToDefaults(this->m_workingConfig);
            this->m_workingConfig.name = this->defaultPresetDisplayName();
            this->setActivePresetKeyDefault();
            success = true;
        }
        else if (this->m_repository != nullptr && this->m_repository->loadByKey(key, this->m_workingConfig)) {
            this->setActivePresetKey(key);
            success = true;
        }
        else {
            // Fallback: loadByKey failed or repository unavailable
                resetToDefaults(this->m_workingConfig);
                this->m_workingConfig.name = this->defaultPresetDisplayName();
                this->setActivePresetKeyDefault();
            success = false;
        }

        onPostLoadPreset();
        this->setDirtyState(false);
        emitConfigLoaded();
        updatePreview();
        return success;
    }

    void setPreviewController(LC_StylingPreviewController* controller) override {
        m_previewController = controller;
        updatePreview();
    }

    QWidget* getSharedBottomWidget() override {
        if (m_previewController != nullptr) {
            return m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck());
        }
        return nullptr;
    }

    virtual void onSubPageControlChanged() {
        notifyWorkingConfigChanged();
    }

    void notifyWorkingConfigChanged() override {
        LC_AbstractPresetManager::notifyWorkingConfigChanged();
        updatePreview();
    }

    bool isGated() const override {
        if (!this->isStorageAvailable()) {
            return true;
        }
        return LC_SettingsManagerStyling::isFusionGated() || this->isReadOnlyDefault();
    }

    QString gatedMessage() const override {
        if (!this->isStorageAvailable()) {
            return LC_AbstractPresetManager::gatedMessage();
        }
        if (LC_SettingsManagerStyling::isFusionGated()) {
            return LC_SettingsManagerStyling::fusionGatedMessage(fusionGatingSubject());
        }
        return LC_AbstractPresetManager::gatedMessage();
    }

    QString gatedActionText() const override {
        if (LC_SettingsManagerStyling::isFusionGated()) {
            return LC_SettingsManagerStyling::fusionGatedActionText();
        }
        return LC_AbstractPresetManager::gatedActionText();
    }

    std::function<void()> gatedActionCallback() const override {
        if (LC_SettingsManagerStyling::isFusionGated()) {
            return [this]() {
                LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
            };
        }
        return nullptr;
    }

    QString gatedIcon() const override {
        if (LC_SettingsManagerStyling::isFusionGated()) {
            return LC_SettingsManagerStyling::fusionGatedIcon();
        }
        return LC_AbstractPresetManager::gatedIcon();
    }

protected:
    virtual void updatePreview() {}
    virtual void resetToDefaults(TConfig& config) = 0;
    virtual void onPostLoadPreset() {}
    virtual void emitConfigLoaded() {}
    virtual QString fusionGatingSubject() const { return QString(); }

    void onPostApplyPreset() override {
        if (m_styleManager != nullptr) {
            m_styleManager->applyActiveStyleAndTheme();
        }
    }

    LC_UIStyleManager* m_styleManager = nullptr;
    LC_StylingPreviewController* m_previewController = nullptr;
};

#endif
