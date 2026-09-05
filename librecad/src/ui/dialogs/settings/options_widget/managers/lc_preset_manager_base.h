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

#include <QObject>
#include "lc_preset_manager_interface.h"
#include "lc_styling_preview_controller.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

#ifndef LC_PRESET_MANAGER_BASE_H
#define LC_PRESET_MANAGER_BASE_H

#include "lc_abstract_preset_manager.h"


template <typename TConfig, typename TRepo>
class LC_PresetManagerBase : public LC_AbstractPresetManager, public LC_StylingPreviewAware {
public:
    explicit LC_PresetManagerBase(LC_UIStyleManager* styleManager, TRepo* repo, const QString& initialKey, QObject* parent = nullptr)
        : LC_AbstractPresetManager(initialKey, parent)
        , m_repository(repo)
        , m_styleManager(styleManager) {
        if (m_repository != nullptr) {
            m_repository->initializeIndex();
        }

        if (m_originalActiveKey.isEmpty() || m_originalActiveKey == DEFAULT_THEME_NAME ||
            (m_repository != nullptr && !m_repository->exists(m_originalActiveKey))) {
            m_originalActiveKey = DEFAULT_THEME_KEY;
        }
        m_activeKey = m_originalActiveKey;
    }

    ~LC_PresetManagerBase() override = default;

    // --- Preview Window & Accessibility Capabilities ---
    bool supportsPreviewWindow() const override { return true; }
    bool supportsAccessibilityCheck() const override { return false; }
    bool supportsImportExport() const override { return false; }

    // --- Common Preset Implementations ---
    QList<QPair<QString, QString>> getAvailablePresets() const override {
        QList<QPair<QString, QString>> choices;
        choices.append(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
        if (m_repository != nullptr) {
            choices.append(m_repository->getPresetChoices());
        }
        return choices;
    }

    bool isPresetModified() override {
        return m_isDirty;
    }

    bool saveCurrentPreset() override {
        if (isReadOnlyDefault() || m_repository == nullptr) {
            return false;
        }
        QString outKey;
        if (m_repository->save(m_workingConfig.name, m_workingConfig, outKey)) {
            m_activeKey = outKey;
            setDirtyState(false);
            return true;
        }
        return false;
    }

    bool savePresetAs(const QString& name, QString& outKey) override {
        if (m_repository == nullptr) {
            return false;
        }
        m_workingConfig.name = name;
        if (m_repository->save(name, m_workingConfig, outKey)) {
            m_activeKey = outKey;
            setDirtyState(false);
            return true;
        }
        return false;
    }

    bool deletePreset(const QString& key) override {
        if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
            return false;
        }
        if (key == m_originalActiveKey) {
            return false;
        }
        return m_repository->removeByKey(key);
    }



    // --- Preview Controller Wiring ---
    void setPreviewController(LC_StylingPreviewController* controller) override {
        m_previewController = controller;
        updatePreview();
    }

    QWidget* getSharedBottomWidget() override {
        return (m_previewController != nullptr)
                   ? m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck())
                   : nullptr;
    }

    // --- Working Config & Change Notifications ---
    const TConfig& workingConfig() const { return m_workingConfig; }
    TConfig& workingConfig() { return m_workingConfig; }

    virtual void onSubPageControlChanged() {
        notifyWorkingConfigChanged();
    }

    void notifyWorkingConfigChanged() override {
        LC_AbstractPresetManager::notifyWorkingConfigChanged();
        updatePreview(); // Styling-specific preview update
    }

protected:
    virtual void updatePreview() {}
    virtual void resetToDefaults(TConfig& config) = 0;

    TConfig m_workingConfig;
    TRepo* m_repository = nullptr;
    LC_UIStyleManager* m_styleManager = nullptr;
    LC_StylingPreviewController* m_previewController = nullptr;
};

#endif
