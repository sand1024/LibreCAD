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

#include "lc_palette_repository.h"

LC_PaletteRepository::LC_PaletteRepository(const QString& configDir)
    : LC_StyleRepositoryBase<PaletteConfig>(configDir, PALETTE_EXTENSION, PALETTE_FILE_IDENTIFIER, "palettes_index.lcix") {}

QJsonObject LC_PaletteRepository::serializeScheme(const ColorSchemeData& scheme) const {
    QJsonObject obj;
    obj["qss"] = scheme.qss;
    obj["contrast_policy"] = static_cast<int>(scheme.contrastPolicy);
    obj["contrast_weight"] = static_cast<int>(scheme.contrastWeight);
    obj["auto_calculate_3d_helpers"] = scheme.autoCalculate3DHelpers;
    obj["bevel_seed_role"] = static_cast<int>(scheme.bevelSeedRole);

    QJsonObject paletteObj;
    for (auto roleIt = scheme.palette.begin(); roleIt != scheme.palette.end(); ++roleIt) {
        QJsonObject stateObj;
        for (auto stateIt = roleIt.value().begin(); stateIt != roleIt.value().end(); ++stateIt) {
            stateObj[stateIt.key()] = stateIt.value().name(QColor::HexArgb);
        }
        paletteObj[roleIt.key()] = stateObj;
    }
    obj["palette"] = paletteObj;

    QJsonObject semanticObj;
    for (auto it = scheme.semanticColors.begin(); it != scheme.semanticColors.end(); ++it) {
        semanticObj[it.key()] = it.value().name(QColor::HexArgb);
    }

    obj["semantic_colors"] = semanticObj;
    return obj;
}

void LC_PaletteRepository::deserializeScheme(const QJsonObject& json, ColorSchemeData& scheme) const {
    scheme.qss = json["qss"].toString();
    scheme.contrastPolicy = static_cast<ContrastPolicy>(json["contrast_policy"].toInt(static_cast<int>(ContrastPolicy::Standard)));
    scheme.contrastWeight = static_cast<ContrastWeight>(json["contrast_weight"].toInt(static_cast<int>(ContrastWeight::Balanced)));
    scheme.autoCalculate3DHelpers = json["auto_calculate_3d_helpers"].toBool(true);
    scheme.bevelSeedRole = static_cast<QPalette::ColorRole>(json["bevel_seed_role"].toInt(static_cast<int>(QPalette::Button)));

    scheme.palette.clear();
    const QJsonObject paletteObj = json["palette"].toObject();
    for (auto roleIt = paletteObj.begin(); roleIt != paletteObj.end(); ++roleIt) {
        const QJsonObject stateObj = roleIt.value().toObject();
        for (auto stateIt = stateObj.begin(); stateIt != stateObj.end(); ++stateIt) {
            scheme.palette[roleIt.key()][stateIt.key()] = QColor(stateIt.value().toString());
        }
    }

    scheme.semanticColors.clear();
    const QJsonObject semanticObj = json["semantic_colors"].toObject();
    for (auto it = semanticObj.begin(); it != semanticObj.end(); ++it) {
        scheme.semanticColors[it.key()] = QColor(it.value().toString());
    }
}

QJsonObject LC_PaletteRepository::configToJson(const PaletteConfig& config) const {
    QJsonObject root;
    root["name"] = config.name;
    root["linked_icon_style"] = config.linkedIconStyleName;
    root["use_theme_default_icons"] = config.useThemeDefaultIcons;
    root["light_scheme"] = serializeScheme(config.light);
    root["dark_scheme"] = serializeScheme(config.dark);
    return root;
}

bool LC_PaletteRepository::configFromJson(const QJsonObject& json, PaletteConfig& config) const {
    config.name = json["name"].toString();
    config.linkedIconStyleName = json["linked_icon_style"].toString("Default");
    config.useThemeDefaultIcons = json["use_theme_default_icons"].toBool(false);
    deserializeScheme(json["light_scheme"].toObject(), config.light);
    deserializeScheme(json["dark_scheme"].toObject(), config.dark);
    return true;
}
