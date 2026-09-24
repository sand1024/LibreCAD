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

#include "lc_styling_profile_import_export_helper.h"
#include <QJsonObject>

#include "lc_dlg_checklist_selection.h"
#include "lc_repository_fusion_skin.h"
#include "lc_repository_icons_style.h"
#include "lc_repository_metrics.h"
#include "lc_repository_palette.h"
#include "lc_repository_typography.h"

LC_StylingProfileImportExportHelper::LC_StylingProfileImportExportHelper(LC_RepositoryPalette* paletteRepo,
                                                                         LC_RepositoryFusionSkin* skinRepo,
                                                                         LC_RepositoryIconsStyle* iconRepo,
                                                                         LC_RepositoryTypography* typographyRepo,
                                                                         LC_RepositoryMetrics* metricsRepo)
    : LC_ProfileExchangeHelperBase(PROFILE_FILE_IDENTIFIER), m_paletteRepo(paletteRepo), m_skinRepo(skinRepo), m_iconRepo(iconRepo),
      m_typographyRepo(typographyRepo), m_metricsRepo(metricsRepo) {
}

bool LC_StylingProfileImportExportHelper::exportProfile(const QString& exportFilePath, const QString& profileName,
                                                        const PaletteConfig* palette, const ControlStyleConfig* controlStyle,
                                                        const IconStyleConfig* icon, const FontConfig* font,
                                                        const StyleMetricsConfig* metrics) const {
    QJsonObject root;
    root["lc_file_format"] = m_fileFormatIdentifier;
    root["lc_file_format_version"] = "1.0";
    root["profile_name"] = profileName;

    if (palette != nullptr && m_paletteRepo != nullptr) {
        auto obj = m_paletteRepo->configToJson(*palette);
        obj["name"] = palette->name;
        root["inlined_palette"] = obj;
    }
    if (controlStyle != nullptr && m_skinRepo != nullptr) {
        auto obj = m_skinRepo->configToJson(*controlStyle);
        obj["name"] = controlStyle->name;
        root["inlined_control_style"] = obj;
    }
    if (icon != nullptr && m_iconRepo != nullptr) {
        auto obj = m_iconRepo->configToJson(*icon);
        obj["name"] = icon->name;
        root["inlined_icon"] = obj;
    }
    if (font != nullptr && m_typographyRepo != nullptr) {
        auto obj = m_typographyRepo->configToJson(*font);
        obj["name"] = font->name;
        root["inlined_font"] = obj;
    }
    if (metrics != nullptr && m_metricsRepo != nullptr) {
        auto obj = m_metricsRepo->configToJson(*metrics);
        obj["name"] = metrics->name;
        root["inlined_metrics"] = obj;
    }

    return writeJsonFile(exportFilePath, root);
}

bool LC_StylingProfileImportExportHelper::importProfile(const QString& importFilePath, const QList<LC_ChecklistChoice>& selectedChoices,
                                                        QString& outProfileName, QString& outPaletteName, QString& outControlStyleName,
                                                        QString& outIconStyleName, QString& outTypographyName,
                                                        QString& outMetricsName) const {
    QJsonObject root;
    if (!readJsonFile(importFilePath, root)) {
        return false;
    }

    outProfileName = root["profile_name"].toString();

    auto isSelected = [&selectedChoices](const QString& id) -> bool {
        for (const auto& c : selectedChoices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    // 1. Palette
    QJsonObject paletteObj = root["inlined_palette"].toObject();
    if (isSelected("palette") && m_paletteRepo != nullptr && !paletteObj.isEmpty()) {
        PaletteConfig palette;
        QString outPath;
        if (m_paletteRepo->configFromJson(paletteObj, palette) && m_paletteRepo->save(palette.name, palette, outPath)) {
            outPaletteName = palette.name;
        }
    }
    else {
        outPaletteName = "";
    }

    // 2. Control Style / Skin
    QJsonObject skinObj = root["inlined_control_style"].toObject();
    if (skinObj.isEmpty()) {
        skinObj = root["inlined_skin"].toObject();
    }
    if (isSelected("skin") && m_skinRepo != nullptr && !skinObj.isEmpty()) {
        ControlStyleConfig skin;
        QString outPath;
        if (m_skinRepo->configFromJson(skinObj, skin) && m_skinRepo->save(skin.name, skin, outPath)) {
            outControlStyleName = skin.name;
        }
    }
    else {
        outControlStyleName = "";
    }

    // 3. Icon Style
    QJsonObject iconObj = root["inlined_icon"].toObject();
    if (isSelected("icon") && m_iconRepo != nullptr && !iconObj.isEmpty()) {
        IconStyleConfig icon;
        QString outPath;
        if (m_iconRepo->configFromJson(iconObj, icon) && m_iconRepo->save(icon.name, icon, outPath)) {
            outIconStyleName = icon.name;
        }
    }
    else {
        outIconStyleName = "";
    }

    // 4. Typography
    QJsonObject fontObj = root["inlined_font"].toObject();
    if (isSelected("typography") && m_typographyRepo != nullptr && !fontObj.isEmpty()) {
        FontConfig font;
        QString outPath;
        if (m_typographyRepo->configFromJson(fontObj, font) && m_typographyRepo->save(font.name, font, outPath)) {
            outTypographyName = font.name;
        }
    }
    else {
        outTypographyName = "";
    }

    // 5. Metrics
    QJsonObject metricsObj = root["inlined_metrics"].toObject();
    if (isSelected("metrics") && m_metricsRepo != nullptr && !metricsObj.isEmpty()) {
        StyleMetricsConfig metrics;
        QString outPath;
        if (m_metricsRepo->configFromJson(metricsObj, metrics) && m_metricsRepo->save(metrics.name, metrics, outPath)) {
            outMetricsName = metrics.name;
        }
    }
    else {
        outMetricsName = "";
    }

    return true;
}
