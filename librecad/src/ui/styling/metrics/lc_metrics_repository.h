
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

#ifndef LC_METRICS_REPOSITORY_H
#define LC_METRICS_REPOSITORY_H

#include "lc_style_repository_base.h"

class LC_MetricsRepository : public LC_StyleRepositoryBase<StyleMetricsConfig> {
public:
    explicit LC_MetricsRepository(const QString& configDir)
        : LC_StyleRepositoryBase<StyleMetricsConfig>(configDir, METRICS_EXTENSION, METRICS_FILE_IDENTIFIER, "metrics_index.lcix") {}

    QJsonObject configToJson(const StyleMetricsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, StyleMetricsConfig& config) const override;
};

#endif
