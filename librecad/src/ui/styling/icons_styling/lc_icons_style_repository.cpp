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

#include "lc_icons_style_repository.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

#include "lc_icon_engine_shared.h"

namespace {
    // Symmetrical field mapper to bridge hash keys with type-safe struct fields
    QString* getSchemeColorRef(IconSchemeConfig &scheme, LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, LC_SVGIconEngineAPI::ColorType type) {
        if (mode == LC_SVGIconEngineAPI::AnyMode) {
            if (type == LC_SVGIconEngineAPI::Main) return &scheme.genericMain;
            if (type == LC_SVGIconEngineAPI::Accent) {
                return (state == LC_SVGIconEngineAPI::On) ? &scheme.genericAccentChecked : &scheme.genericAccent;
            }
            return &scheme.genericBack;
        }
        IconStateColors *states = nullptr;
        switch (mode) {
            case LC_SVGIconEngineAPI::Normal:   states = (state == LC_SVGIconEngineAPI::On) ? &scheme.normalOn : &scheme.normalOff; break;
            case LC_SVGIconEngineAPI::Active:   states = (state == LC_SVGIconEngineAPI::On) ? &scheme.activeOn : &scheme.activeOff; break;
            case LC_SVGIconEngineAPI::Selected: states = (state == LC_SVGIconEngineAPI::On) ? &scheme.selectedOn : &scheme.selectedOff; break;
            case LC_SVGIconEngineAPI::Disabled: states = (state == LC_SVGIconEngineAPI::On) ? &scheme.disabledOn : &scheme.disabledOff; break;
            default: break;
        }
        if (states) {
            if (type == LC_SVGIconEngineAPI::Main) return &states->main;
            if (type == LC_SVGIconEngineAPI::Accent) return &states->accent;
            return &states->back;
        }
        return nullptr;
    }

    const QString STYLE_FILE_MARK = "LibreCAD Icons Style";

    // Dynamic, Human-Readable String Translators
    QString getModeStr(LC_SVGIconEngineAPI::IconMode mode) {
        switch (mode) {
            case LC_SVGIconEngineAPI::Active:   return "active";
            case LC_SVGIconEngineAPI::AnyMode:  return "any";
            case LC_SVGIconEngineAPI::Normal:   return "normal";
            case LC_SVGIconEngineAPI::Disabled: return "disabled";
            case LC_SVGIconEngineAPI::Selected: return "selected";
        }
        return "";
    }

    bool parseIconMode(const QString &val, LC_SVGIconEngineAPI::IconMode &mode) {
        QString trimmed = val.trimmed().toLower();
        if (trimmed == "active")   { mode = LC_SVGIconEngineAPI::Active;   return true; }
        if (trimmed == "any")      { mode = LC_SVGIconEngineAPI::AnyMode;  return true; }
        if (trimmed == "normal")   { mode = LC_SVGIconEngineAPI::Normal;   return true; }
        if (trimmed == "disabled") { mode = LC_SVGIconEngineAPI::Disabled; return true; }
        if (trimmed == "selected") { mode = LC_SVGIconEngineAPI::Selected; return true; }
        return false;
    }

    QString getStateStr(LC_SVGIconEngineAPI::IconState state) {
        switch (state) {
            case LC_SVGIconEngineAPI::Off:      return "off";
            case LC_SVGIconEngineAPI::On:       return "on";
            case LC_SVGIconEngineAPI::AnyState: return "any";
        }
        return "";
    }

    bool parseIconState(const QString &val, LC_SVGIconEngineAPI::IconState &state) {
        QString trimmed = val.trimmed().toLower();
        if (trimmed == "off") { state = LC_SVGIconEngineAPI::Off;      return true; }
        if (trimmed == "on")  { state = LC_SVGIconEngineAPI::On;       return true; }
        if (trimmed == "any") { state = LC_SVGIconEngineAPI::AnyState; return true; }
        return false;
    }

    QString getTypeStr(LC_SVGIconEngineAPI::ColorType type) {
        switch (type) {
            case LC_SVGIconEngineAPI::Background: return "background";
            case LC_SVGIconEngineAPI::Main:       return "main";
            case LC_SVGIconEngineAPI::Accent:     return "accent";
        }
        return "";
    }

    bool parseColorType(const QString &val, LC_SVGIconEngineAPI::ColorType &type) {
        QString trimmed = val.trimmed().toLower();
        if (trimmed == "background") { type = LC_SVGIconEngineAPI::ColorType::Background; return true; }
        if ("main" == trimmed)       { type = LC_SVGIconEngineAPI::ColorType::Main;       return true; }
        if ("accent" == trimmed)     { type = LC_SVGIconEngineAPI::ColorType::Accent;     return true; }
        return false;
    }
}


QJsonObject LC_IconsStyleRepository::configToJson(const IconStyleConfig& config) const {
    QJsonObject root;
    root["type"] = m_fileIdentifier;
    root["auto_calculate_states"] = config.autoCalculateStates;

    auto serializeScheme = [this](const IconSchemeConfig &scheme) {
        QJsonObject obj;
        obj["generic_main"]   = scheme.genericMain;
        obj["generic_accent"] = scheme.genericAccent;
        obj["generic_accent_checked"] = scheme.genericAccentChecked;
        obj["generic_back"]   = scheme.genericBack;

        QJsonArray settings;
        auto appendColorSetting = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, LC_SVGIconEngineAPI::ColorType type) {
            QString *colorRef = getSchemeColorRef(const_cast<IconSchemeConfig&>(scheme), mode, state, type);
            if (colorRef && !colorRef->isEmpty()) {
                QJsonObject setting;
                setting["iconMode"]       = getModeStr(mode);
                setting["iconState"]      = getStateStr(state);
                setting["colorType"]      = getTypeStr(type);
                setting["colorSvgString"] = *colorRef;
                settings.append(setting);
            }
        };

        for (int m = 0; m < 4; ++m) {
            for (int s = 0; s < 2; ++s) {
                for (int t = 0; t < 3; ++t) {
                    appendColorSetting(static_cast<LC_SVGIconEngineAPI::IconMode>(m), static_cast<LC_SVGIconEngineAPI::IconState>(s), static_cast<LC_SVGIconEngineAPI::ColorType>(t));
                }
            }
        }
        obj["settings"] = settings;
        return obj;
    };

    root["light_scheme"] = serializeScheme(config.light);
    root["dark_scheme"] = serializeScheme(config.dark);

    return root;
}

bool LC_IconsStyleRepository::configFromJson(const QJsonObject& json, IconStyleConfig& config) const {
    config.autoCalculateStates = json["auto_calculate_states"].toBool(true);

    auto deserializeScheme = [this](const QJsonObject &obj, IconSchemeConfig &scheme) {
        scheme.genericMain   = obj["generic_main"].toString();
        scheme.genericAccent = obj["generic_accent"].toString();
        scheme.genericAccentChecked = obj.contains("generic_accent_checked")
                                    ? obj["generic_accent_checked"].toString()
                                    : scheme.genericAccent;
        scheme.genericBack   = obj["generic_back"].toString();

        QJsonArray settings = obj["settings"].toArray();
        for (int i = 0; i < settings.size(); ++i) {
            QJsonObject settingObj = settings[i].toObject();
            QString modeStr  = settingObj["iconMode"].toString();
            QString stateStr = settingObj["iconState"].toString();
            QString typeStr  = settingObj["colorType"].toString();
            QString colorStr = settingObj["colorSvgString"].toString();

            LC_SVGIconEngineAPI::IconMode mode;
            LC_SVGIconEngineAPI::IconState state;
            LC_SVGIconEngineAPI::ColorType type;

            if (parseIconMode(modeStr, mode) && parseIconState(stateStr, state) && parseColorType(typeStr, type)) {
                QString *colorRef = getSchemeColorRef(scheme, mode, state, type);
                if (colorRef) {
                    *colorRef = colorStr;
                }
            }
        }
    };


    deserializeScheme(json["light_scheme"].toObject(), config.light);
    deserializeScheme(json["dark_scheme"].toObject(), config.dark);
    return true;
}
