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

#ifndef LC_PRESET_MANAGER_METRICS_H
#define LC_PRESET_MANAGER_METRICS_H

#include <memory>
#include "lc_metrics_density_header_bar.h"
#include "lc_preset_manager_styling_base.h"
#include "lc_repository_metrics.h"

class LC_PresetManagerFusionMetrics : public LC_PresetManagerStylingBase<StyleMetricsConfig, LC_RepositoryMetrics> {
    Q_OBJECT
public:
    explicit LC_PresetManagerFusionMetrics(LC_UIStyleManager* styleManager);
    ~LC_PresetManagerFusionMetrics() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    QString getAppliedPresetKey() const override;
    QWidget* getSharedHeaderWidget() override;

    signals:
        void configLoaded(const StyleMetricsConfig& config);

public slots:
    void onDensityPresetSelected(const QString& presetName);

protected:
    void resetToDefaults(StyleMetricsConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void updatePreview() override;
    void onPostLoadPreset() override;
    void emitConfigLoaded() override;
    QString fusionGatingSubject() const override;

private:
    std::unique_ptr<LC_MetricsDensityHeaderBar> m_headerBar;
};

#endif
