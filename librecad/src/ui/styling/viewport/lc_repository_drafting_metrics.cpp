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

#include "lc_repository_drafting_metrics.h"

#include "rs_system.h"

LC_RepositoryDraftingMetrics::LC_RepositoryDraftingMetrics(const QString& configDir)
    : LC_PresetRepositoryBase<LC_DraftingMetricsConfig>(
          configDir,
          ".lcdm",
          "drafting_metrics",
          "metrics_index.json") {
}


QJsonObject LC_RepositoryDraftingMetrics::configToJson(const LC_DraftingMetricsConfig& config) const {
    return config.rootObject;
}

bool LC_RepositoryDraftingMetrics::configFromJson(const QJsonObject& json, LC_DraftingMetricsConfig& config) const {
    config.rootObject = json;
    return true;
}
