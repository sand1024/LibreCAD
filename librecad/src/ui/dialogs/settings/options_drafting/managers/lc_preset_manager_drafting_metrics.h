
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

#ifndef LC_PRESET_MANAGER_DRAFTING_METRICS_H
#define LC_PRESET_MANAGER_DRAFTING_METRICS_H

#include "lc_preset_manager_config_base.h"
#include "lc_repository_drafting_metrics.h"

class LC_DraftingPreviewBottomBar;

class LC_PresetManagerDraftingMetrics : public LC_PresetManagerConfigBase<LC_DraftingMetricsConfig, LC_RepositoryDraftingMetrics> {
    Q_OBJECT
public:
    explicit LC_PresetManagerDraftingMetrics(LC_RepositoryDraftingMetrics* repository,
                                            QObject* parent = nullptr,
                                            QWidget* previewWidget = nullptr);
    ~LC_PresetManagerDraftingMetrics() override;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool isDefaultPreset(const QString& key) const override;
    bool loadPreset(const QString& key) override;
    bool isPresetModified() override;
    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;

    void setSharedPreviewWidget(QWidget* widget) {m_previewWidget = widget;}
    QWidget* getSharedPreviewWidget() override;
    QWidget* getSharedBottomWidget()  override;
    void setSharedBottomWidget(LC_DraftingPreviewBottomBar* bar) { m_bottomBar = bar; }
    bool isPreviewVisible() const;

protected:
    bool supportsMissingPresetRecovery() const override {return true;}
    void prepareWorkingConfigForSave() override;
    void onPostSavePreset(const QString& outKey) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void prependDefaultPresets(QList<QPair<QString, QString>>& choices) const override;

    bool resetToDefaults(const QString& key);
    bool resetToHiDpiDefaults(const QString& key);
    QJsonObject serializeCurrentSettings() const;
    void applyMetricsJson(const QJsonObject& obj);
    QWidget* m_previewWidget = nullptr;
    mutable LC_DraftingPreviewBottomBar* m_bottomBar = nullptr;
};

#endif
