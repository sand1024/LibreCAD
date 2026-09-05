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
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "lc_palette_repository.h"
#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_typography_repository.h"
#include "lc_metrics_repository.h"

LC_WorkspaceImportExportHelper::LC_WorkspaceImportExportHelper(
    LC_PaletteRepository* paletteRepo,
    LC_FusionSkinsRepository* skinRepo,
    LC_IconsStyleRepository* iconRepo,
    LC_TypographyRepository* typographyRepo,
    LC_MetricsRepository* metricsRepo)
    : m_paletteRepo(paletteRepo)
    , m_skinRepo(skinRepo)
    , m_iconRepo(iconRepo)
    , m_typographyRepo(typographyRepo)
    , m_metricsRepo(metricsRepo) {}

bool LC_WorkspaceImportExportHelper::exportProfile(const QString& exportFilePath,
                                                  const QString& profileName,
                                                  const PaletteConfig* palette,
                                                  const ControlStyleConfig* controlStyle,
                                                  const IconStyleConfig* icon,
                                                  const FontConfig* font,
                                                  const StyleMetricsConfig* metrics) const {
    QJsonObject root;
    root["file_identifier"] = PROFILE_FILE_IDENTIFIER;
    root["profile_name"] = profileName;

    if (palette && m_paletteRepo) {
        auto obj = m_paletteRepo->configToJson(*palette);
        obj["name"] = palette->name;
        root["inlined_palette"] = obj;
    }
    if (controlStyle && m_skinRepo) {
        auto obj = m_skinRepo->configToJson(*controlStyle);
        obj["name"] = controlStyle->name;
        root["inlined_control_style"] = obj;
    }
    if (icon && m_iconRepo) {
        auto obj = m_iconRepo->configToJson(*icon);
        obj["name"] = icon->name;
        root["inlined_icon"] = obj;
    }
    if (font && m_typographyRepo) {
        auto obj = m_typographyRepo->configToJson(*font);
        obj["name"] = font->name;
        root["inlined_font"] = obj;
    }
    if (metrics && m_metricsRepo) {
        auto obj = m_metricsRepo->configToJson(*metrics);
        obj["name"] = metrics->name;
        root["inlined_metrics"] = obj;
    }

    QFile file(exportFilePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(QJsonDocument(root).toJson());
    return true;
}

bool LC_WorkspaceImportExportHelper::importProfile(const QString& importFilePath,
                                                  QString& outProfileName,
                                                  QString& outPaletteName,
                                                  QString& outControlStyleName,
                                                  QString& outIconStyleName,
                                                  QString& outTypographyName,
                                                  QString& outMetricsName) const {
    QFile file(importFilePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = doc.object();
    if (root["file_identifier"].toString() != PROFILE_FILE_IDENTIFIER) return false;

    outProfileName = root["profile_name"].toString();

    // 1. Palette
    QJsonObject paletteObj = root["inlined_palette"].toObject();
    if (m_paletteRepo && !paletteObj.isEmpty()) {
        PaletteConfig palette;
        QString outPath;
        if (m_paletteRepo->configFromJson(paletteObj, palette) && m_paletteRepo->save(palette.name, palette, outPath)) {
            outPaletteName = palette.name;
        }
    } else {
        outPaletteName = "";
    }

    // 2. Control Style / Skin
    QJsonObject skinObj = root["inlined_control_style"].toObject();
    if (skinObj.isEmpty()) {
        skinObj = root["inlined_skin"].toObject(); // Backward compatibility
    }
    if (m_skinRepo && !skinObj.isEmpty()) {
        ControlStyleConfig skin;
        QString outPath;
        if (m_skinRepo->configFromJson(skinObj, skin) && m_skinRepo->save(skin.name, skin, outPath)) {
            outControlStyleName = skin.name;
        }
    } else {
        outControlStyleName = "";
    }

    // 3. Icon Style
    QJsonObject iconObj = root["inlined_icon"].toObject();
    if (m_iconRepo && !iconObj.isEmpty()) {
        IconStyleConfig icon;
        QString outPath;
        if (m_iconRepo->configFromJson(iconObj, icon) && m_iconRepo->save(icon.name, icon, outPath)) {
            outIconStyleName = icon.name;
        }
    } else {
        outIconStyleName = "";
    }

    // 4. Typography
    QJsonObject fontObj = root["inlined_font"].toObject();
    if (m_typographyRepo && !fontObj.isEmpty()) {
        FontConfig font;
        QString outPath;
        if (m_typographyRepo->configFromJson(fontObj, font) && m_typographyRepo->save(font.name, font, outPath)) {
            outTypographyName = font.name;
        }
    } else {
        outTypographyName = "";
    }

    // 5. Metrics
    QJsonObject metricsObj = root["inlined_metrics"].toObject();
    if (m_metricsRepo && !metricsObj.isEmpty()) {
        StyleMetricsConfig metrics;
        QString outPath;
        if (m_metricsRepo->configFromJson(metricsObj, metrics) && m_metricsRepo->save(metrics.name, metrics, outPath)) {
            outMetricsName = metrics.name;
        }
    } else {
        outMetricsName = "";
    }

    return true;
}
