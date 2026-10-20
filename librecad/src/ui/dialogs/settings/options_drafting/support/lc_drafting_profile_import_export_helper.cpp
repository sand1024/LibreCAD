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

#include "lc_drafting_profile_import_export_helper.h"

#include <QCoreApplication>
#include <QJsonObject>

LC_DraftingProfileImportExportHelper::LC_DraftingProfileImportExportHelper(
    LC_RepositoryDraftingTheme* themeRepo,
    LC_RepositoryDraftingMetrics* metricsRepo)
    : LC_ProfileExchangeHelperBase(DRAFTING_PROFILE_FILE_IDENTIFIER)
    , m_themeRepo(themeRepo)
    , m_metricsRepo(metricsRepo) {
}

bool LC_DraftingProfileImportExportHelper::exportProfile(const QString& exportFilePath,
                                                         const QString& profileName,
                                                         const LC_ViewportThemeConfig* theme,
                                                         const LC_DraftingMetricsConfig* metrics) const {
    QJsonObject root;
    root["lc_file_format"] = m_fileFormatIdentifier;
    root["lc_file_format_version"] = "1.0";
    root["profile_name"] = profileName;

    if (theme != nullptr && m_themeRepo != nullptr) {
        QJsonObject obj = m_themeRepo->configToJson(*theme);
        obj["name"] = theme->name;
        root["inlined_theme"] = obj;
    }

    if (metrics != nullptr && m_metricsRepo != nullptr) {
        QJsonObject obj = m_metricsRepo->configToJson(*metrics);
        obj["name"] = metrics->name;
        root["inlined_metrics"] = obj;
    }

    return writeJsonFile(exportFilePath, root);
}

bool LC_DraftingProfileImportExportHelper::inspectProfile(const QString& importFilePath,
                                                          QString& outProfileName,
                                                          QList<LC_ChecklistChoice>& outChoices) const {
    QJsonObject root;
    if (!readJsonFile(importFilePath, root)) {
        return false;
    }

    outProfileName = root.value("profile_name").toString();
    outChoices.clear();

    if (root.contains("inlined_theme")) {
        outChoices.append({ "theme",
                           QCoreApplication::translate("LC_DraftingProfile", "Visual Theme"),
                           QCoreApplication::translate("LC_DraftingProfile", "Canvas colors, line patterns, and marker styles etc."),
                           true, true });
    }
    if (root.contains("inlined_metrics")) {
        outChoices.append({ "metrics",
                           QCoreApplication::translate("LC_DraftingProfile", "Viewport Metrics"),
                           QCoreApplication::translate("LC_DraftingProfile", "Handle sizes, marker radii, and line thicknesses etc."),
                           true, true });
    }

    return !outChoices.isEmpty();
}

bool LC_DraftingProfileImportExportHelper::importProfile(const QString& importFilePath,
                                                         const QList<LC_ChecklistChoice>& selectedChoices,
                                                         QString& outProfileName,
                                                         QString& outThemeKey,
                                                         QString& outMetricsKey) const {
    QJsonObject root;
    if (!readJsonFile(importFilePath, root)) {
        return false;
    }

    outProfileName = root.value("profile_name").toString();

    auto isSelected = [&selectedChoices](const QString& id) -> bool {
        for (const auto& c : selectedChoices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    // 1. Visual Theme
    if (isSelected("theme") && m_themeRepo != nullptr && root.contains("inlined_theme")) {
        LC_ViewportThemeConfig config;
        const QJsonObject obj = root.value("inlined_theme").toObject();
        if (m_themeRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_themeRepo->save(config.name, config, outKey)) {
                outThemeKey = outKey;
            }
        }
    }

    // 2. Viewport Metrics
    if (isSelected("metrics") && m_metricsRepo != nullptr && root.contains("inlined_metrics")) {
        LC_DraftingMetricsConfig config;
        const QJsonObject obj = root.value("inlined_metrics").toObject();
        if (m_metricsRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_metricsRepo->save(config.name, config, outKey)) {
                outMetricsKey = outKey;
            }
        }
    }

    return true;
}
