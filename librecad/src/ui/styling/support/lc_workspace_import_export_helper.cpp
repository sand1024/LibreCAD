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


#include "lc_workspace_import_export_helper.h"
#include "lc_workspace_import_export_helper.h"
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>

#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_typography_repository.h"
#include "lc_metrics_repository.h"

LC_WorkspaceImportExportHelper::LC_WorkspaceImportExportHelper(
    LC_FusionSkinsRepository* skinRepo,
    LC_IconsStyleRepository* iconRepo,
    LC_TypographyRepository* typographyRepo,
    LC_MetricsRepository* metricsRepo)
    : m_skinRepo(skinRepo)
    , m_iconRepo(iconRepo)
    , m_typographyRepo(typographyRepo)
    , m_metricsRepo(metricsRepo) {}

bool LC_WorkspaceImportExportHelper::exportProfile(const QString& exportFilePath,
                                                  const QString& profileName,
                                                  const SkinConfig* skin,
                                                  const IconStyleConfig* icon,
                                                  const FontConfig* font,
                                                  const StyleMetricsConfig* metrics) const {
    QJsonObject root;
    root["file_identifier"] = PROFILE_FILE_IDENTIFIER;
    root["profile_name"] = profileName;

    // Delegate serialization entirely to the repositories if configs are present
    if (skin && m_skinRepo) {
        auto obj = m_skinRepo->configToJson(*skin);
        obj["name"] = skin->name;
        root["inlined_skin"] = obj;
    }
    if (icon && m_iconRepo) {
        auto obj = m_iconRepo->configToJson(*icon);
        obj["name"] = skin->name;
        root["inlined_icon"] = obj;
    }
    if (font && m_typographyRepo) {
        auto obj = m_typographyRepo->configToJson(*font);
        obj["name"] = skin->name;
        root["inlined_font"] = obj;
    }
    if (metrics && m_metricsRepo) {
        auto obj = m_metricsRepo->configToJson(*metrics);
        obj["name"] = skin->name;
        root["inlined_metrics"] = obj;
    }

    QFile file(exportFilePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(QJsonDocument(root).toJson());
    return true;
}

bool LC_WorkspaceImportExportHelper::importProfile(const QString& importFilePath,
                                                  QString& outProfileName,
                                                  QString& outSkinName,
                                                  QString& outIconStyleName,
                                                  QString& outTypographyName,
                                                  QString& outMetricsName) const {
    QFile file(importFilePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = doc.object();
    if (root["file_identifier"].toString() != PROFILE_FILE_IDENTIFIER) return false;

    outProfileName = root["profile_name"].toString();

    // 1. Skin (Optional, Fusion-only)
    QJsonObject skinObj = root["inlined_skin"].toObject();
    if (m_skinRepo) {
        SkinConfig skin;
        QString outPath;
        if (m_skinRepo->configFromJson(skinObj, skin) && m_skinRepo->save(skin.name, skin, outPath)) {
            outSkinName = skin.name;
        }
    } else {
        outSkinName = "";
    }

    // 2. Icon Style (Optional)
    QJsonObject iconObj = root["inlined_icon"].toObject();
    if (m_iconRepo) {
        IconStyleConfig icon;
        icon.name = iconObj["name"].toString();
        QString outPath;
        if (m_iconRepo->configFromJson(iconObj, icon) && m_iconRepo->save(icon.name, icon, outPath)) {
            outIconStyleName = icon.name;
        }
    } else {
        outIconStyleName = "";
    }

    // 3. Typography (Optional)
    QJsonObject fontObj = root["inlined_font"].toObject();
    if (m_typographyRepo) {
        FontConfig font;
        font.name = iconObj["name"].toString();
        QString outPath;
        if (m_typographyRepo->configFromJson(fontObj, font) && m_typographyRepo->save(font.name, font, outPath)) {
            outTypographyName = font.name;
        }
    } else {
        outTypographyName = "";
    }

    // 4. Metrics (Optional, Fusion-only)
    QJsonObject metricsObj = root["inlined_metrics"].toObject();
    if (m_metricsRepo) {
        StyleMetricsConfig metrics;
        metrics.name = iconObj["name"].toString();
        QString outPath;
        if (m_metricsRepo->configFromJson(metricsObj, metrics) && m_metricsRepo->save(metrics.name, metrics, outPath)) {
            outMetricsName = metrics.name;
        }
    } else {
        outMetricsName = "";
    }

    return true;
}
