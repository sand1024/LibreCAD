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

#include "lc_icon_colors_options.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QPalette>
#include <QRegularExpression>
#include <QStyleHints>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "lc_palette_color_utils.h"
#include "lc_palette_editor_shared.h"


#include "rs_debug.h"
#include "rs_settings.h"

namespace {
    // Light-scheme template defaults — match the SVG template colors so that
    // the icon engine's replaceColor() short-circuits and the SVG renders
    // exactly as authored.
    constexpr const char* DEFAULT_MAIN_LIGHT       = "#000";
    constexpr const char* DEFAULT_ACCENT           = "#00ff7f";
    constexpr const char* DEFAULT_BACKGROUND_LIGHT = "#fff";

    // Dark-scheme defaults — picked to avoid substring collisions in the
    // engine's naive three-pass QString::replace() at lc_svgiconengine.cpp:368.
    // Main must not contain "#fff" (would be clobbered by the BG pass);
    // Background must not contain "#000" (would be clobbered by the Main pass).
    constexpr const char* DEFAULT_MAIN_DARK       = "#e6e6e6";
    constexpr const char* DEFAULT_BACKGROUND_DARK = "#1e1e1e";

    inline const char* defaultMain() {
        return LC_PaletteColorUtils::isSystemInDarkMode()
            ? DEFAULT_MAIN_DARK : DEFAULT_MAIN_LIGHT;
    }
    inline const char* defaultBackground() {
        return LC_PaletteColorUtils::isSystemInDarkMode()
            ? DEFAULT_BACKGROUND_DARK : DEFAULT_BACKGROUND_LIGHT;
    }

    // Symmetrical field mapper to bridge hash keys with type-safe struct fields [3]
    QString* getSchemeColorRef(IconSchemeConfig& scheme, LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state,
                           LC_SVGIconEngineAPI::ColorType type) {
        if (mode == LC_SVGIconEngineAPI::AnyMode) {
            if (type == LC_SVGIconEngineAPI::Main)
                return &scheme.genericMain;
            if (type == LC_SVGIconEngineAPI::Accent) {
                return (state == LC_SVGIconEngineAPI::On) ? &scheme.genericAccentChecked : &scheme.genericAccent;
            }
            return &scheme.genericBack;
        }
        IconStateColors* states = nullptr;
        switch (mode) {
            case LC_SVGIconEngineAPI::Normal:
                states = (state == LC_SVGIconEngineAPI::On) ? &scheme.normalOn : &scheme.normalOff;
                break;
            case LC_SVGIconEngineAPI::Active:
                states = (state == LC_SVGIconEngineAPI::On) ? &scheme.activeOn : &scheme.activeOff;
                break;
            case LC_SVGIconEngineAPI::Selected:
                states = (state == LC_SVGIconEngineAPI::On) ? &scheme.selectedOn : &scheme.selectedOff;
                break;
            case LC_SVGIconEngineAPI::Disabled:
                states = (state == LC_SVGIconEngineAPI::On) ? &scheme.disabledOn : &scheme.disabledOff;
                break;
            default:
                break;
        }
        if (states) {
            if (type == LC_SVGIconEngineAPI::Main)
                return &states->main;
            if (type == LC_SVGIconEngineAPI::Accent)
                return &states->accent;
            return &states->back;
        }
        return nullptr;
    }

}

LC_IconColorsOptions::LC_IconColorsOptions() = default;

LC_IconColorsOptions::LC_IconColorsOptions(const LC_IconColorsOptions& other) {
    m_colors.insert(other.m_colors);
    m_autoCalculateStates = other.m_autoCalculateStates;
    m_iconOverridesDir = other.m_iconOverridesDir;
}

void LC_IconColorsOptions::resetToDefaults() {
    m_colors.clear();
    setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Main, defaultMain());
    setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Accent, DEFAULT_ACCENT);
    setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Background, defaultBackground());
    m_autoCalculateStates = true;
}

void LC_IconColorsOptions::apply(const LC_IconColorsOptions& other) {
    m_colors.clear();
    m_colors.insert(other.m_colors);
    m_autoCalculateStates = other.m_autoCalculateStates;
    m_iconOverridesDir = other.m_iconOverridesDir;
}

void LC_IconColorsOptions::mark() {
    m_colorsMarkCopy.clear();
    m_colorsMarkCopy.insert(m_colors);
    m_iconOverridesDirMarkCopy = m_iconOverridesDir;
    m_autoCalculateStatesMarkCopy = m_autoCalculateStates;
}

void LC_IconColorsOptions::restore() {
    m_colors.clear();
    m_colors.insert(m_colorsMarkCopy);
    m_colorsMarkCopy.clear();
    m_iconOverridesDir = m_iconOverridesDirMarkCopy;
    m_autoCalculateStates = m_autoCalculateStatesMarkCopy;
}

bool LC_IconColorsOptions::isIconOverridesChanged() const {
    return m_iconOverridesDir != m_iconOverridesDirMarkCopy;
}

QString LC_IconColorsOptions::getKeyBaseName(const LC_SVGIconEngineAPI::ColorType type) {
    switch (type) {
        case LC_SVGIconEngineAPI::ColorType::Main:
            return LC_SVGIconEngineAPI::KEY_COLOR_MAIN;
        case LC_SVGIconEngineAPI::ColorType::Accent:
            return LC_SVGIconEngineAPI::KEY_COLOR_ACCENT;
        case LC_SVGIconEngineAPI::ColorType::Background:
            return LC_SVGIconEngineAPI::KEY_COLOR_BG;
        default:
            return "";
    }
}

QString LC_IconColorsOptions::getSettingsKeyName(const LC_SVGIconEngineAPI::IconMode mode, const LC_SVGIconEngineAPI::IconState state,
                                                 const LC_SVGIconEngineAPI::ColorType type) {
    const QString keyBase = getKeyBaseName(type);
    QString keyName = getColorAppKeyName(keyBase, mode, state);
    return keyName;
}

void LC_IconColorsOptions::loadColor(const LC_SVGIconEngineAPI::IconMode mode, const LC_SVGIconEngineAPI::IconState state,
                                     const LC_SVGIconEngineAPI::ColorType type, const QString& defaultValue) {
    const QString settingsKey = getSettingsKeyName(mode, state, type);
    const QString value = LC_GET_STR(settingsKey, defaultValue);
    const int hashKey = iconHashKey(mode, state, type);
    m_colors.insert(hashKey, value);
}

void LC_IconColorsOptions::saveColor(const LC_SVGIconEngineAPI::IconMode mode, const LC_SVGIconEngineAPI::IconState state,
                                     const LC_SVGIconEngineAPI::ColorType type) {
    const QString settingsKey = getSettingsKeyName(mode, state, type);
    const int hashKey = iconHashKey(mode, state, type);
    const QString value = m_colors.value(hashKey);
    LC_SET(settingsKey, value);
}


void LC_IconColorsOptions::loadSettings() {
    LC_GROUP("UiIconsStyling");
    {
        loadColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Main, defaultMain());
        loadColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Accent, DEFAULT_ACCENT);
        loadColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Background, defaultBackground());

        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background, "");
        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background, "");

        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background, "");
        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background, "");

        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background, "");
        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background, "");

        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background, "");
        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main, "");
        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent, "");
        loadColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background, "");

        m_iconOverridesDir = LC_GET_STR("IconOverridesDir", "");
        m_autoCalculateStates = LC_GET_BOOL("AutoCalculateStates", true);
    }
    LC_GROUP_END();
}

void LC_IconColorsOptions::save() {
    LC_GROUP("UiIconsStyling");
    {
        saveColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::ColorType::Background);

        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background);
        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Active, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background);

        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background);
        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Normal, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background);

        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background);
        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background);

        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::ColorType::Background);
        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Main);
        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Accent);
        saveColor(LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off, LC_SVGIconEngineAPI::ColorType::Background);

        LC_SET("IconOverridesDir", m_iconOverridesDir);
        LC_SET("AutoCalculateStates", m_autoCalculateStates);
    }
    LC_GROUP_END();
}


QString LC_IconColorsOptions::getColor(const LC_SVGIconEngineAPI::IconMode mode, const LC_SVGIconEngineAPI::IconState state,
                                       const LC_SVGIconEngineAPI::ColorType type) const {
    const int key = iconHashKey(mode, state, type);
    QString result = m_colors.value(key);
    return result;
}

void LC_IconColorsOptions::setColor(const LC_SVGIconEngineAPI::IconMode mode, const LC_SVGIconEngineAPI::IconState state,
                                    const LC_SVGIconEngineAPI::ColorType type, const QString& color) {
    const int key = iconHashKey(mode, state, type);
    m_colors.insert(key, color);
}

void LC_IconColorsOptions::importStyleConfig(const IconStyleConfig &style, bool isDarkMode) {
    m_autoCalculateStates = style.autoCalculateStates;

    // Retrieve the active scheme configuration (Light vs Dark)
    IconSchemeConfig scheme = isDarkMode ? style.dark : style.light;

    m_colors.clear();

    auto importSingleColor = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, LC_SVGIconEngineAPI::ColorType type) {
        QString *colorRef = getSchemeColorRef(scheme, mode, state, type);
        if (colorRef && !colorRef->isEmpty()) {
            setColor(mode, state, type, *colorRef); // Direct import of the raw config string
        } else {
            setColor(mode, state, type, "");
        }
    };

    // 1. Import generic colors
    importSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main);
    importSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent);
    importSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent);
    importSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background);

    // 2. Import all 24 state-specific overrides symmetrically
    for (int m = 0; m < 4; ++m) {
        for (int s = 0; s < 2; ++s) {
            for (int t = 0; t < 3; ++t) {
                importSingleColor(static_cast<LC_SVGIconEngineAPI::IconMode>(m), static_cast<LC_SVGIconEngineAPI::IconState>(s), static_cast<LC_SVGIconEngineAPI::ColorType>(t));
            }
        }
    }
}

void LC_IconColorsOptions::exportStyleConfig(IconStyleConfig &style, bool isDarkMode) const {
    style.autoCalculateStates = m_autoCalculateStates;

    // Retrieve the active target scheme configuration (Light vs Dark)
    IconSchemeConfig &scheme = isDarkMode ? style.dark : style.light;

    auto exportSingleColor = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, LC_SVGIconEngineAPI::ColorType type) {
        QString *colorRef = getSchemeColorRef(scheme, mode, state, type);
        if (colorRef) {
            *colorRef = getColor(mode, state, type); // Direct export of the active UI value
        }
    };

    // 1. Export generic colors
    exportSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main);
    exportSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent);
    exportSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent);
    exportSingleColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background);

    // 2. Export all 24 state-specific overrides symmetrically
    for (int m = 0; m < 4; ++m) {
        for (int s = 0; s < 2; ++s) {
            for (int t = 0; t < 3; ++t) {
                exportSingleColor(static_cast<LC_SVGIconEngineAPI::IconMode>(m), static_cast<LC_SVGIconEngineAPI::IconState>(s), static_cast<LC_SVGIconEngineAPI::ColorType>(t));
            }
        }
    }
}
