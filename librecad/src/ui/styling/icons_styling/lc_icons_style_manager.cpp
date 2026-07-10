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


#include "lc_icons_style_manager.h"

#include <QGuiApplication>
#include <QPalette>
#include <QPixmapCache>
#include "lc_icons_style_repository.h"
#include "lc_icon_colors_options.h"
#include "lc_icons_color_utils.h"
#include "lc_palette_color_utils.h"
#include "lc_palette_editor_shared.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"

void LC_IconsStyleManager::applyThemeLinkedIcons(const QString &linkedStyleName, bool useThemeDefaultIcons, bool isDarkMode) {
    if (!useThemeDefaultIcons) {
        return; // Preserve the user's active, standalone custom style [3]
    }

    bool applied = false;
    QString iconsDir = LC_GET_ONE_STR("UiIconsStyling", "IconOverridesDir", "");

    // Load the linked style if a custom folder and style are specified [3]
    if (!iconsDir.isEmpty() && !linkedStyleName.isEmpty() && linkedStyleName != "Default") {
        LC_IconsStyleRepository iconRepo(iconsDir);
        IconStyleConfig iconStyle;
        if (iconRepo.loadByKey(linkedStyleName, iconStyle)) {
            LC_IconColorsOptions iconOptions;
            iconOptions.loadSettings(); // Load directories and defaults
            iconOptions.importStyleConfig(iconStyle, isDarkMode);

            // Delegate completely to applyStyle() [3]
            applyStyle(iconOptions, isDarkMode);
            applied = true;
        }
    }

    // Symmetrical Procedural Fallback [3]
    if (!applied) {
        if (isDarkMode) {
            // Generate and apply dark-adapted procedural colors based on the current palette
            QPalette palette = QGuiApplication::palette();
            QColor darkMain   = palette.color(QPalette::Active, QPalette::WindowText);
            QColor darkAccent = palette.color(QPalette::Active, QPalette::Highlight);

            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_MAIN, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, darkMain.name());
            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_ACCENT, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, darkAccent.name());
            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_BG, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, "");
        } else {
            // Symmetrical Light Mode fallback: clear properties so standard resource icons render exactly as designed
            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_MAIN, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, "");
            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_ACCENT, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, "");
            LC_SVGIconEngineAPI::setColorAppProperty(LC_SVGIconEngineAPI::KEY_COLOR_BG, LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, "");
        }

        QPixmapCache::clear();
        const auto& appWindow = QC_ApplicationWindow::getAppWindow();
        if (appWindow != nullptr) {
            appWindow->fireIconsRefresh();
            appWindow->update();
        }
    }
}

#define DEBUG_APPLYING_COLORS_

void LC_IconsStyleManager::applyStyle(const LC_IconColorsOptions &options, bool isDarkMode, LC_PaletteColorUtils::CVDType cvd) {
    // 1. Pre-resolve active seeds and system colors
    QString mainSeedStr         = resolveColorValue(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main));
    QString accentNormalStr     = resolveColorValue(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent));
    QString accentCheckedStr    = resolveColorValue(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent));
    QString backSeedStr         = resolveColorValue(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background));

    QColor mainSeed            = QColor(mainSeedStr);
    QColor accentSeedNormal    = QColor(accentNormalStr);
    QColor accentSeedChecked   = QColor(accentCheckedStr);
    QColor backSeed            = QColor(backSeedStr);

    bool autoCalc = options.isAutoCalculateStates();
    if (!mainSeed.isValid()) {
        mainSeed = isDarkMode ? QColor("#e6e6e6") : QColor("#000000");
    }
    if (!accentSeedNormal.isValid()) {
        accentSeedNormal = QColor("#00ff7f"); // Default green accent template fallback
    }
    if (!accentSeedChecked.isValid()) {
        if (autoCalc) {
            // Symmetrical Legacy Fallback: Automatically generate a beautiful checked accent
            // on-the-fly using our ComplementaryShift policy [3]
            accentSeedChecked = LC_IconsColorUtils::generateCheckedAccent(
                accentSeedNormal,
                AccentHarmonizationPolicy::ComplementaryShift,
                isDarkMode
            );
        } else {
            // Symmetrical Manual Fallback: Revert to using the normal accent color
            accentSeedChecked = accentSeedNormal;
        }
    }

    QPalette activePalette = QGuiApplication::palette();
    QColor highlightColor = activePalette.color(QPalette::Highlight);
    QColor highlightedTextColor = activePalette.color(QPalette::HighlightedText);
    QColor windowColor = activePalette.color(QPalette::Window);

    auto applyColor = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, LC_SVGIconEngineAPI::ColorType type) {
        QString originalVal;
        QString resolvedValue;

        if (mode == LC_SVGIconEngineAPI::AnyMode) {
            originalVal = options.getColor(mode, state, type);
            resolvedValue = resolveColorValue(originalVal);
        }
        else if (autoCalc) {
            // Symmetrical call passing both 'state' and our dual accent seeds
            QColor targetColor = LC_IconsColorUtils::calculateStateColor(
                mode, state, type, mainSeed, accentSeedNormal, accentSeedChecked, backSeed,
                highlightColor, highlightedTextColor, windowColor
            );
            resolvedValue = targetColor.isValid() ? targetColor.name(QColor::HexRgb) : "";

            QString seedName = options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, type);
            originalVal = QString("Auto (%1)").arg(seedName.isEmpty() ? "None" : seedName);
        }
        else {
            originalVal = options.getColor(mode, state, type);
            resolvedValue = resolveColorValue(originalVal);
        }

        // --- ACCESSIBILITY CVD SIMULATION FILTER ---
        if (cvd != LC_PaletteColorUtils::CVDType::Normal && !resolvedValue.isEmpty() && resolvedValue.startsWith('#')) {
            QColor col = QColor(resolvedValue);

            // Reuses the exact same mathematical model used in the palette editor
            col = LC_PaletteColorUtils::simulateCVD(col, cvd);
            resolvedValue = col.name(QColor::HexRgb);
        }

        // --- COLOUR COLLISION SANITIZATION FILTER ---
        if (!resolvedValue.isEmpty() && resolvedValue.startsWith('#')) {
            QColor col = QColor(resolvedValue);
            if (col.isValid()) {
                int r = col.red();
                int g = col.green();
                int b = col.blue();
                bool modified = false;

                // 1. Prevent Main (Stroke) from matching the Accent Template (#00ff7f)
                //    or the Background Template (#ffffff)
                if (type == LC_SVGIconEngineAPI::Main) {
                    if (r == 0 && g == 255 && b == 127) { // Matches #00ff7f
                        g = 254; // Alter green slightly
                        modified = true;
                    }
                    else if (r == 255 && g == 255 && b == 255) { // Matches #ffffff
                        b = 254; // Alter blue slightly to #fffffe
                        modified = true;
                    }
                }
                // 2. Prevent Accent (Detail) from matching the Background Template (#ffffff)
                //    or the Main Template (#000000)
                else if (type == LC_SVGIconEngineAPI::Accent) {
                    if (r == 255 && g == 255 && b == 255) { // Matches #ffffff
                        b = 254; // Alter blue slightly to #fffffe
                        modified = true;
                    }
                    else if (r == 0 && g == 0 && b == 0) { // Matches #000000
                        r = 1; // Alter red slightly to #000001
                        modified = true;
                    }
                }

                if (modified) {
                    col.setRgb(r, g, b);
                    resolvedValue = col.name(QColor::HexRgb);
                }
            }
        }
        QString keyBase = "";
        switch (type) {
            case LC_SVGIconEngineAPI::Main:       keyBase = LC_SVGIconEngineAPI::KEY_COLOR_MAIN; break;
            case LC_SVGIconEngineAPI::Accent:     keyBase = LC_SVGIconEngineAPI::KEY_COLOR_ACCENT; break;
            case LC_SVGIconEngineAPI::Background: keyBase = LC_SVGIconEngineAPI::KEY_COLOR_BG; break;
        }

        LC_SVGIconEngineAPI::setColorAppProperty(keyBase, mode, state, resolvedValue);

#        ifdef DEBUG_APPLYING_COLORS
        // Debug logger (displays the sanitized output)
        auto modeToName = [](LC_SVGIconEngineAPI::IconMode m) -> const char* {
            switch (m) {
                case LC_SVGIconEngineAPI::Normal:   return "Normal";
                case LC_SVGIconEngineAPI::Disabled: return "Disabled";
                case LC_SVGIconEngineAPI::Active:   return "Active";
                case LC_SVGIconEngineAPI::Selected: return "Selected";
                case LC_SVGIconEngineAPI::AnyMode:  return "AnyMode";
            }
            return "Unknown";
        };

        auto stateToName = [](LC_SVGIconEngineAPI::IconState s) -> const char* {
            switch (s) {
                case LC_SVGIconEngineAPI::On:       return "On";
                case LC_SVGIconEngineAPI::Off:      return "Off";
                case LC_SVGIconEngineAPI::AnyState: return "AnyState";
            }
            return "Unknown";
        };

        LC_ERR << "[ICON-DEBUG] Color Property Applied:"
               << " Mode = " << modeToName(mode)
               << " | State = " << stateToName(state)
               << " | BaseKey = " << keyBase.toUtf8().constData()
               << " | Original = " << originalVal.toUtf8().constData()
               << " | Resolved = " << (resolvedValue.isEmpty() ? "Transparent/None" : resolvedValue.toUtf8().constData());

#        endif
    };

    // Apply generic seed variables
    applyColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main);
    applyColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent);
    applyColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background);

    // Apply state combinations
    for (int m = 0; m < 4; ++m) {
        for (int s = 0; s < 2; ++s) {
            for (int t = 0; t < 3; ++t) {
                applyColor(static_cast<LC_SVGIconEngineAPI::IconMode>(m), static_cast<LC_SVGIconEngineAPI::IconState>(s), static_cast<LC_SVGIconEngineAPI::ColorType>(t));
            }
        }
    }

    qApp->setProperty(LC_SVGIconEngineAPI::KEY_ICONS_OVERRIDES_DIR, options.getIconsOverridesDir());

    // Flush and repaint
    QPixmapCache::clear();
    const auto& appWindow = QC_ApplicationWindow::getAppWindow();
    if (appWindow != nullptr) {
        appWindow->fireIconsRefresh();
        appWindow->update();
        appWindow->repaint();
    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void LC_IconsStyleManager::applyCurrentStyle() {
    LC_IconColorsOptions options;
    options.loadSettings();
    bool darkMode = LC_PaletteColorUtils::isSystemInDarkMode();
    applyStyle(options, darkMode);
}

QString LC_IconsStyleManager::resolveColorValue(const QString &value) {
    if (value.startsWith("palette:")) {
        QString roleName = value.mid(8).trimmed();
        QPalette palette = QGuiApplication::palette();
        QPalette::ColorRole role = QPalette::NoRole;

        for (const auto &mapping : BASE_INTERFACE_ROLES) {
            if (mapping.name == roleName) {
                role = mapping.role;
                break;
            }
        }
        if (role == QPalette::NoRole) {
            for (const auto &mapping : BEVEL_HELPER_ROLES) {
                if (mapping.name == roleName) {
                    role = mapping.role;
                    break;
                }
            }
        }

        if (role != QPalette::NoRole) {
            return palette.color(QPalette::Active, role).name(QColor::HexRgb);
        }
    }

    if (value == "transparent") {
        return "";
    }

    return value;
}
