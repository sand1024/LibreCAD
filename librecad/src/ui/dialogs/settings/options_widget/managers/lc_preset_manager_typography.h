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

#ifndef LC_PRESET_MANAGER_TYPOGRAPHY_H
#define LC_PRESET_MANAGER_TYPOGRAPHY_H

#include <QObject>
#include <memory>
#include "lc_palette_editor_shared.h"
#include "lc_preset_manager_interface.h"
#include "lc_styling_preview_controller.h"

class LC_UIStyleManager;
class LC_TypographyRepository;
class LC_StylingPreviewController;

class LC_PresetManagerTypography : public QObject, public LC_PresetManagerInterface, public LC_StylingPreviewAware {
    Q_OBJECT
public:
    explicit LC_PresetManagerTypography(QObject* parent = nullptr);
    ~LC_PresetManagerTypography() override = default;

    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    bool deletePreset(const QString& key) override;
    QList<QPair<QString, QString>> getAvailablePresets() const override;
    QString getActivePresetKey() const override;
    QString getAppliedPresetKey() const override;

    void applyCurrentPreset() override;
    void rollbackState() override;

    void setChangedCallback(std::function<void(bool isDirty)> callback) override;
    QWidget* getSharedBottomWidget() override;
    bool isPresetModified() override { return m_isDirty; }

    LC_PresetAcceptPolicy acceptPolicy() const override { return LC_PresetAcceptPolicy::PromptSaveIfModified; }
    bool onDialogAccept(QWidget* parentDialog) override;
    bool onDialogReject(QWidget* parentDialog) override;

    LC_PresetManagerUIStrings presetStrings() const override;

    FontConfig& workingConfig() { return m_workingConfig; }
    const FontConfig& workingConfig() const { return m_workingConfig; }
    void notifyWorkingConfigChanged();

    void setPreviewController(LC_StylingPreviewController* controller) override;
    LC_StylingPreviewController* getPreviewController() const { return m_previewController; }

    bool supportsImportExport() const override { return false; }
    bool supportsApply() const override { return true; }
 signals:
    void configLoaded(const FontConfig& config);
private:
    LC_UIStyleManager* m_styleManager = nullptr;
    LC_TypographyRepository* m_repository = nullptr;
    LC_StylingPreviewController* m_previewController = nullptr;

    FontConfig m_workingConfig;
    QString m_activeKey;
    QString m_originalActiveKey;
    bool m_isDirty = false;
    std::function<void(bool)> m_changedCallback;
};

#endif
