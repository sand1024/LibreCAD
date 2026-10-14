
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

#ifndef LC_REPOSITORY_DRAFTING_METRICS_H
#define LC_REPOSITORY_DRAFTING_METRICS_H

#include <QJsonObject>
#include <QString>

#include "lc_preset_repository_base.h"
#include "lc_setting.h"

struct LC_DraftingMetricsConfig {
    QString name;
    QJsonObject rootObject;
};

struct LC_DraftingMetricsMapEntry {
    QString jsonPath;
    const LC_SettingBase* setting = nullptr;

    LC_DraftingMetricsMapEntry(const QString& path, const LC_SettingBase& settingRef)
        : jsonPath(path)
        , setting(&settingRef) {
    }

    LC_DraftingMetricsMapEntry(const LC_SettingBase& settingRef)
        : jsonPath(settingRef.group() + "." + settingRef.key())
        , setting(&settingRef) {
    }
};

class LC_RepositoryDraftingMetrics : public LC_PresetRepositoryBase<LC_DraftingMetricsConfig> {
public:
    explicit LC_RepositoryDraftingMetrics(const QString& configDir = QString());
    ~LC_RepositoryDraftingMetrics() override = default;

    QJsonObject configToJson(const LC_DraftingMetricsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, LC_DraftingMetricsConfig& config) const override;
};

#endif
