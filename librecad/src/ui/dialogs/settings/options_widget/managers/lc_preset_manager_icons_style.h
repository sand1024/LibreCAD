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

#ifndef LC_PRESET_MANAGER_ICONS_STYLE_H
#define LC_PRESET_MANAGER_ICONS_STYLE_H

#include <QObject>
#include "lc_icon_colors_options.h"
#include "lc_icons_style_shared.h"
#include "lc_palette_editor_shared.h"
#include "lc_preset_manager_interface.h"
#include "lc_styling_preview_controller.h"

class LC_UIStyleManager;
class LC_IconsStyleRepository;

class LC_PresetManagerIconsStyle : public QObject, public LC_PresetManagerInterface, public LC_StylingPreviewAware {
    Q_OBJECT
public:
    explicit LC_PresetManagerIconsStyle(QObject* parent = nullptr);
    ~LC_PresetManagerIconsStyle() override = default;

    // --- LC_StylingPreviewAware Override ---
    void setPreviewController(LC_StylingPreviewController* controller) override;
    LC_StylingPreviewController* getPreviewController() const { return m_previewController; }

    // --- LC_PresetManagerInterface Overrides ---
    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    bool deletePreset(const QString& key) override;
    QList<QPair<QString, QString>> getAvailablePresets() const override;
    QString getActivePresetKey() const override;
    QString getAppliedPresetKey() const override;

    bool supportsApply() const override { return true; }
    bool supportsImportExport() const override { return false; }

    void applyCurrentPreset() override;
    void rollbackState() override;

    void setChangedCallback(std::function<void(bool isDirty)> callback) override;
    QWidget* getSharedBottomWidget() override;
    QWidget* getSharedHeaderWidget() override { return nullptr; }
    bool isPresetModified() override { return m_isDirty; }

    LC_PresetAcceptPolicy acceptPolicy() const override { return LC_PresetAcceptPolicy::PromptSaveIfModified; }
    bool onDialogAccept(QWidget* parentDialog) override;
    bool onDialogReject(QWidget* parentDialog) override;

    LC_PresetManagerUIStrings presetStrings() const override;

    IconStyleConfig& workingConfig() { return m_workingConfig; }
    const IconStyleConfig& workingConfig() const { return m_workingConfig; }

    LC_IconColorsOptions& iconColorsOptions() { return m_iconColorsOptions; }
    const LC_IconColorsOptions& iconColorsOptions() const { return m_iconColorsOptions; }

    bool isCurrentVariantDark() const { return m_currentVariantDark; }
    void setCurrentVariantDark(bool dark);

    void notifyWorkingConfigChanged();
    void applyTransientStyle();

signals:
    void configLoaded();
    void variantChanged(bool isDark);

private:
    LC_UIStyleManager* m_styleManager = nullptr;
    LC_IconsStyleRepository* m_repository = nullptr;
    LC_StylingPreviewController* m_previewController = nullptr;

    IconStyleConfig m_workingConfig;
    LC_IconColorsOptions m_iconColorsOptions;

    QString m_activeKey;
    QString m_originalActiveKey;
    bool m_currentVariantDark = false;
    bool m_isDirty = false;
    std::function<void(bool)> m_changedCallback;
};

#endif
