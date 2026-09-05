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

#include "lc_palette_color_utils.h"

#include <algorithm> // for qBound

#include <QApplication>
#include <QFont>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QPixmapCache>
#include "lc_icons_style_shared.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_colors_semantics.h"
#include "rs_debug.h"
#include "rs_settings.h"

bool LC_PaletteColorUtils::isPaletteDarkMode() {
    // Fallback: Query standard operating system theme

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme != Qt::ColorScheme::Unknown) {
        return scheme == Qt::ColorScheme::Dark;
    }
#endif
    return QGuiApplication::palette().color(QPalette::Window).lightnessF() < 0.5;
}

bool LC_PaletteColorUtils::isSystemInDarkMode() {
    ThemeModeOverride themeMode = CFG_AppStyling::o_ThemeModeOverride;

    if (themeMode == ThemeModeOverride::ForceLight) {
        return false; // Force Light Palette and Icons
    }
    if (themeMode == ThemeModeOverride::ForceDark) {
        return true;  // Force Dark Palette and Icons
    }

    return isPaletteDarkMode();
}

#ifdef DEBUG_BEVEL_GENERATION
    // Real-time Palette Engine Diagnostics Logger [3]
void LC_PaletteColorUtils::debugBevelCalculation(const QPalette &palette, BevelStyle style, QPalette::ColorRole seedRole) {
    QColor btnColor = palette.color(QPalette::Active, seedRole);
    if (!btnColor.isValid()) return;

    bool isDark = btnColor.value() < 120;

    LC_ERR << "\n======================================================================";
    LC_ERR << "=== PALETTE ENGINE DIAGNOSTICS (ALL CONTRAST WEIGHTS) ===";
    LC_ERR << "Theme Mode: " << (isDark ? "DARK MODE" : "LIGHT MODE");
    LC_ERR << "Bevel Style: " << (style == BevelStyle::Flat ? "FLAT" : (style == BevelStyle::Sharp ? "SHARP/GLASSY" : "SOFT/SATIN"));
    LC_ERR << "Base Seed Color (" << (seedRole == QPalette::Button ? "Button" : (seedRole == QPalette::Window ? "Window" : "Highlight")) << "): "
           << btnColor.name().toUtf8().constData() << " | HSV: " << btnColor.hue() << "," << btnColor.saturation() << "," << btnColor.value();
    LC_ERR << "----------------------------------------------------------------------";

    struct WeightInfo {
        ContrastWeight weight;
        const char* label;
    };

    const QList<WeightInfo> weightConfigs = {
        { ContrastWeight::None,     "NONE (FLAT)" },
        { ContrastWeight::Soft,     "SOFT CONTRAST" },
        { ContrastWeight::Balanced, "BALANCED (DEFAULT)" },
        { ContrastWeight::Hard,     "HARD CONTRAST" }
    };

    for (const auto &cfg : weightConfigs) {
        // Copy the compiled palette to prevent mutating the active application state
        QPalette tempPalette = palette;
        calculate3DHelperRoles(tempPalette, style, cfg.weight, seedRole);

        QColor lightColor     = tempPalette.color(QPalette::Active, QPalette::Light);
        QColor midlightColor  = tempPalette.color(QPalette::Active, QPalette::Midlight);
        QColor darkColor      = tempPalette.color(QPalette::Active, QPalette::Dark);
        QColor midColor       = tempPalette.color(QPalette::Active, QPalette::Mid);
        QColor shadowColor    = tempPalette.color(QPalette::Active, QPalette::Shadow);

        LC_ERR << " [Profile: " << cfg.label << "]";
        LC_ERR << "  -> Light:    " << lightColor.name().toUtf8().constData()    << " | HSV: " << lightColor.hue() << "," << lightColor.saturation() << "," << lightColor.value();
        LC_ERR << "  -> Midlight: " << midlightColor.name().toUtf8().constData() << " | HSV: " << midlightColor.hue() << "," << midlightColor.saturation() << "," << midlightColor.value();
        LC_ERR << "  -> Dark:     " << darkColor.name().toUtf8().constData()     << " | HSV: " << darkColor.hue() << "," << darkColor.saturation() << "," << darkColor.value();
        LC_ERR << "  -> Mid:      " << midColor.name().toUtf8().constData()      << " | HSV: " << midColor.hue() << "," << midColor.saturation() << "," << midColor.value();
        LC_ERR << "  -> Shadow:   " << shadowColor.name().toUtf8().constData()   << " | HSV: " << shadowColor.hue() << "," << shadowColor.saturation() << "," << shadowColor.value();
        LC_ERR << "----------------------------------------------------------------------";
    }
    LC_ERR << "======================================================================\n";
}
#endif


void LC_PaletteColorUtils::initializeDefaultControlStyle(ControlStyleConfig &config) {
    config.name = DEFAULT_THEME_NAME;
    config.styleArchetype = StyleArchetype::ClassicFusion;
    config.boxDecoration  = BoxDecoration::DividingHairline;
    config.customDockTitleBar = false;
    config.dockTitleBarStyle  = DockTitleBarStyle::Native;
    config.accentedScrollbars = false;
    config.transparentScrollbars = false;
    config.customGroupBoxBar = false;
    config.groupBoxHeaderStyle = GroupBoxHeaderStyle::Plain;
    config.groupBoxBoundaryStyle = GroupBoxBoundaryStyle::Full;
    config.groupBoxUseAccent = false;
    config.showActiveRowSpotlight = false;
    config.showItemViewHover = true;
    config.branchIndicatorStyle = BranchIndicatorStyle::MutedChevrons;
    config.showTreeConnectingLines = false;
    config.customSplitterGrip = false;
    config.splitterGripStyle = SplitterGripStyle::MutedDots;
    config.highlightSplitterOnDrag = false;
    config.showGripBackgroundWell = false;
    config.accentGrips = false;
    config.persistentDockSplitter = true;
    config.useFocusedInputGlow = false;
    config.useToolButtonUnderline = false;
    config.toolButtonIndicatorStyle = ToolButtonIndicatorStyle::ContextStripe;
    config.customToolTipCard = false;
    config.customVectorIconsInButtons = false;
    config.tabStripeAtBottom = false;
    config.customToolbarOverflowGrip = false;
    config.autoPopupToolbarOverflow = false;
    config.autoPopupInstantButtons = false;
    config.autoPopupMenuBar = false;
    config.useMenuBarHoverCard = false;
    config.showMenuCommandAliases = false;
    config.useSegmentedToolButtons = false;
    config.segmentedSeparationStyle = SegmentedSeparationStyle::ContinuousCard;
    config.segmentedColorPolicy = SegmentedColorPolicy::HarmonizedAnalogous;
    config.useSpinBoxProgressBar = false;
    config.useStatusPillChips = false;
    config.customMenuTearOff = false;
    config.syncCheckedMenuState = false;
    config.useFloatingHUDMenus = false;
    config.useFloatingHUDDocks = false;
    config.closeButtonColorPolicy = CloseButtonColorPolicy::AccentColor;
    config.showGenericDockIcons = true;
    config.showSpecialDockIcons = true;
    config.customDialogTitleBar = false;
}

void LC_PaletteColorUtils::initializeDefaultPalette(PaletteConfig &config) {
    config.name = DEFAULT_THEME_NAME;
    config.linkedIconStyleName = "Default";
    config.useThemeDefaultIcons = false;

    // Light palette population
    QPalette lightPal = QApplication::palette();
    QStyle *lightStyle = QStyleFactory::create("Fusion");
    if (lightStyle) {
        lightPal = lightStyle->standardPalette();
        delete lightStyle;
    }

    calculate3DHelperRoles(lightPal, BevelStyle::Soft, ContrastWeight::Balanced, QPalette::Button);

    forEachRoleState([&](const PaletteRoleMapping &row, const PaletteStateMapping &state) {
        config.light.palette[row.name][state.name] = lightPal.color(state.group, row.role);
    });

    config.light.qss = "";
    config.light.contrastWeight = ContrastWeight::Balanced;
    config.light.contrastPolicy = ContrastPolicy::Standard;
    config.light.autoCalculate3DHelpers = true;
    config.light.bevelSeedRole = QPalette::Button;

    // Dark palette population
    QPalette darkPal;
    const QColor windowColor(45, 45, 45);
    const QColor baseColor(25, 25, 25);
    const QColor buttonColor(45, 45, 45);

    for (const PaletteStateMapping &state : PALETTE_STATES) {
        if (state.group != QPalette::Disabled) {
            darkPal.setColor(state.group, QPalette::Window, windowColor);
            darkPal.setColor(state.group, QPalette::WindowText, Qt::white);
            darkPal.setColor(state.group, QPalette::Base, baseColor);
            darkPal.setColor(state.group, QPalette::AlternateBase, windowColor);
            darkPal.setColor(state.group, QPalette::ToolTipBase, Qt::white);
            darkPal.setColor(state.group, QPalette::ToolTipText, Qt::black);
            darkPal.setColor(state.group, QPalette::Text, Qt::white);
            darkPal.setColor(state.group, QPalette::Button, buttonColor);
            darkPal.setColor(state.group, QPalette::ButtonText, Qt::white);
            darkPal.setColor(state.group, QPalette::BrightText, Qt::red);
            darkPal.setColor(state.group, QPalette::Link, QColor(42, 130, 218));
            darkPal.setColor(state.group, QPalette::Highlight, QColor(42, 130, 218));
            darkPal.setColor(state.group, QPalette::HighlightedText, Qt::black);
            darkPal.setColor(state.group, QPalette::PlaceholderText, QColor(128, 128, 128));
        }
    }
    darkPal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(110, 110, 110));
    darkPal.setColor(QPalette::Disabled, QPalette::Text, QColor(110, 110, 110));
    darkPal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(110, 110, 110));
    darkPal.setColor(QPalette::Disabled, QPalette::Base, QColor(35, 35, 35));
    darkPal.setColor(QPalette::Disabled, QPalette::Window, QColor(40, 40, 40));

    calculate3DHelperRoles(darkPal, BevelStyle::Soft, ContrastWeight::Balanced, QPalette::Button);

    forEachRoleState([&](const PaletteRoleMapping &row, const PaletteStateMapping &state) {
        config.dark.palette[row.name][state.name] = darkPal.color(state.group, row.role);
    });

    config.dark.qss = "";
    config.dark.contrastWeight = ContrastWeight::Balanced;
    config.dark.contrastPolicy = ContrastPolicy::Standard;
    config.dark.autoCalculate3DHelpers = true;
    config.dark.bevelSeedRole = QPalette::Button;

    config.light.semanticColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM]  = QColor("#2a82da");
    config.light.semanticColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]   = QColor("#d9534f");
    config.light.semanticColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]  = QColor("#d97706");

    config.dark.semanticColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM]   = QColor("#2a82da");
    config.dark.semanticColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]    = QColor("#e06c77");
    config.dark.semanticColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]   = QColor("#bd6313");
}

void LC_PaletteColorUtils::initializeDefaultConfig(ControlStyleConfig &config) {
    initializeDefaultControlStyle(config);
}


void LC_PaletteColorUtils::initializeWithPalette(PaletteConfig &config, const QPalette &palette) {
    config.name = "System Platform Palette";

    // Extract color roles symmetrically for both light and dark schema templates
    forEachRoleState([&](const PaletteRoleMapping &row, const PaletteStateMapping &state) {
        QColor color = palette.color(state.group, row.role);
        config.light.palette[row.name][state.name] = color;
        config.dark.palette[row.name][state.name] = color;
    });

    // Reset QCSS plain texts and configure active default contrast policies
    config.light.qss = "";
    config.light.contrastPolicy = ContrastPolicy::Standard;
    config.light.contrastWeight = ContrastWeight::Balanced;
    config.light.autoCalculate3DHelpers = true;
    config.light.bevelSeedRole = QPalette::Button;

    config.dark.qss = "";
    config.dark.contrastPolicy = ContrastPolicy::Standard;
    config.dark.contrastWeight = ContrastWeight::Balanced;
    config.dark.autoCalculate3DHelpers = true;
    config.dark.bevelSeedRole = QPalette::Button;
}

void LC_PaletteColorUtils::generateHarmonizedTheme(const QColor &baseColor, PaletteConfig  &config) {
    int h, s, v;
    baseColor.getHsv(&h, &s, &v);

    if (h < 0) h = 210;

    // ================= DARK VARIANT GENERATION (Modern Slate Hierarchy) =================
    ColorSchemeData &dark = config.dark;

    QColor darkBg;
    darkBg.setHsv(h, qMin(s, 35), 38);

    QColor darkBase;
    darkBase.setHsv(h, qMin(s, 40), 24);

    QColor darkBtn;
    darkBtn.setHsv(h, qMin(s, 30), 55);

    QColor darkHighlight;
    darkHighlight.setHsv(h, qMax(s, 160), qMax(v, 140));

    QColor darkText;
    darkText.setHsv(h, qMin(s, 10), 235);

    assignGroupHarmonics(dark, QPalette::Active, darkBg, darkBase, darkBtn, darkHighlight, darkText);
    assignGroupHarmonics(dark, QPalette::Inactive, darkBg, darkBase, darkBtn, darkHighlight, darkText.darker(110));
    assignGroupHarmonics(dark, QPalette::Disabled, darkBg.darker(110), darkBase.darker(110), darkBtn.darker(110), darkHighlight.darker(150), Qt::gray);
    generateSemanticColors(dark, darkHighlight, true, false);
    dark.qss = "";

    // ================= LIGHT VARIANT GENERATION (Modern Soft Tint Hierarchy) =================
    ColorSchemeData &light = config.light;

    QColor lightBg;
    lightBg.setHsv(h, qMin(s, 15), 245);

    QColor lightBase = Qt::white;

    QColor lightBtn;
    lightBtn.setHsv(h, qMin(s, 18), 225);

    QColor lightHighlight;
    lightHighlight.setHsv(h, qMax(s, 140), qMin(v, 180));

    QColor lightText;
    lightText.setHsv(h, qMin(s, 20), 40);

    assignGroupHarmonics(light, QPalette::Active, lightBg, lightBase, lightBtn, lightHighlight, lightText);
    assignGroupHarmonics(light, QPalette::Inactive, lightBg, lightBase, lightBtn, lightHighlight, lightText.lighter(120));
    assignGroupHarmonics(light, QPalette::Disabled, lightBg.lighter(105), lightBase, lightBtn.lighter(105), lightHighlight.lighter(150), Qt::gray);
    generateSemanticColors(light, lightHighlight, false, false);
    light.qss = "";
}

void LC_PaletteColorUtils::generateHarmonizedTheme(const QColor &surfaceColor,
                                                   const QColor &accentColor,
                                                   PaletteConfig  &config) {
    int sH, sS, sV;
    surfaceColor.getHsv(&sH, &sS, &sV);
    if (sH < 0) sH = 210;

    int aH, aS, aV;
    accentColor.getHsv(&aH, &aS, &aV);
    if (aH < 0) aH = 30;

    // ================= DYNAMIC DARK SCHEMA =================
    ColorSchemeData &dark = config.dark;

    QColor darkBg;
    darkBg.setHsv(sH, qMin(sS, 35), 38);

    QColor darkBase;
    darkBase.setHsv(sH, qMin(sS, 40), 24);

    QColor darkBtn;
    darkBtn.setHsv(sH, qMin(sS, 30), 55);

    QColor darkHighlight;
    darkHighlight.setHsv(aH, qMax(aS, 160), qMax(aV, 150));

    QColor darkText;
    darkText.setHsv(sH, qMin(sS, 10), 235);

    assignGroupHarmonics(dark, QPalette::Active, darkBg, darkBase, darkBtn, darkHighlight, darkText);
    assignGroupHarmonics(dark, QPalette::Inactive, darkBg, darkBase, darkBtn, darkHighlight.darker(120), darkText.darker(110));
    assignGroupHarmonics(dark, QPalette::Disabled, darkBg.darker(110), darkBase.darker(110), darkBtn.darker(110), darkHighlight.darker(200), Qt::gray);
    generateSemanticColors(dark, darkHighlight, true, false);
    dark.qss = "";

    // ================= DYNAMIC LIGHT SCHEMA =================
    ColorSchemeData &light = config.light;

    QColor lightBg;
    lightBg.setHsv(sH, qMin(sS, 15), 245);

    QColor lightBase = Qt::white;

    QColor lightBtn;
    lightBtn.setHsv(sH, qMin(sS, 18), 225);

    QColor lightHighlight;
    lightHighlight.setHsv(aH, qMax(aS, 140), qMin(aV, 130));

    QColor lightText;
    lightText.setHsv(sH, qMin(sS, 20), 40);

    assignGroupHarmonics(light, QPalette::Active, lightBg, lightBase, lightBtn, lightHighlight, lightText);
    assignGroupHarmonics(light, QPalette::Inactive, lightBg, lightBase, lightBtn, lightHighlight.lighter(120), lightText.lighter(120));
    assignGroupHarmonics(light, QPalette::Disabled, lightBg.lighter(105), lightBase, lightBtn.lighter(105), lightHighlight.lighter(150), Qt::gray);
    generateSemanticColors(light, lightHighlight, false, false);
    light.qss = "";
}

void LC_PaletteColorUtils::assignGroupHarmonics(ColorSchemeData &scheme, QPalette::ColorGroup group,
                                                const QColor &bg, const QColor &base, const QColor &btn,
                                                const QColor &highlight, const QColor &text) {
    const QString groupName = (group == QPalette::Active) ? "Active" :
                              (group == QPalette::Inactive) ? "Inactive" : "Disabled";

    scheme.palette["Window"][groupName] = bg;
    scheme.palette["WindowText"][groupName] = text;
    scheme.palette["Base"][groupName] = base;
    scheme.palette["AlternateBase"][groupName] = base.lighter(110);
    scheme.palette["ToolTipBase"][groupName] = bg;
    scheme.palette["ToolTipText"][groupName] = text;
    scheme.palette["Text"][groupName] = text;
    scheme.palette["Button"][groupName] = btn;
    scheme.palette["ButtonText"][groupName] = text;
    scheme.palette["BrightText"][groupName] = Qt::red;
    scheme.palette["Link"][groupName] = highlight;
    scheme.palette["Highlight"][groupName] = highlight;
    scheme.palette["HighlightedText"][groupName] = (text.value() < 120) ? Qt::white : Qt::black;
    scheme.palette["PlaceholderText"][groupName] = Qt::gray;
}

// Updated Bevel Lighting generator to derive boundaries from custom seed role
void LC_PaletteColorUtils::calculate3DHelperRoles(QPalette &palette, BevelStyle style, ContrastWeight weight, QPalette::ColorRole seedRole) {
    for (const PaletteStateMapping &state : PALETTE_STATES) {
        QPalette::ColorGroup group = state.group;
        QColor btnColor = palette.color(group, seedRole);
        if (!btnColor.isValid()) continue;

        // Decoupled contrast multipliers. None results in 0.0 (completely flat)
        double contrastMultiplier = 1.0;
        if (weight == ContrastWeight::None) {
            contrastMultiplier = 0.0;
        } else if (weight == ContrastWeight::Soft) {
            contrastMultiplier = 0.50; // Increased to 50% for more distinct soft edges
        } else if (weight == ContrastWeight::Hard) {
            contrastMultiplier = 1.65; // Balanced hard edges
        }

        // Expanded baseline ratios to ensure distinct highlights on bright backgrounds
        double lightRatio     = 0.45 * contrastMultiplier;
        double midlightRatio  = 0.22 * contrastMultiplier;
        double darkRatio      = 0.22 * contrastMultiplier;
        double midRatio       = 0.36 * contrastMultiplier;
        double shadowRatio    = 0.55 * contrastMultiplier;

        // Adjust ratios based on BevelStyle archetype
        if (style == BevelStyle::Flat) {
            // Flat modern styles use extremely subtle borders
            lightRatio     *= 0.25;
            midlightRatio  *= 0.25;
            darkRatio      *= 0.35;
            midRatio       *= 0.35;
            shadowRatio    *= 0.35;
        } else if (style == BevelStyle::Sharp) {
            // Glassy/Sharp styles amplify contrast transitions
            lightRatio     *= 1.25;
            midlightRatio  *= 1.15;
            darkRatio      *= 1.15;
            midRatio       *= 1.25;
            shadowRatio    *= 1.35;
        }

        // Mix with white/black to generate contrast-stable colors
        bool isDark = btnColor.value() < 120;
        QColor lightColor, midlightColor, darkColor, midColor, shadowColor;

        if (isDark) {
            // Uses the central file-static interpolator
            lightColor    = interpolateColors(btnColor, Qt::white, lightRatio);
            midlightColor = interpolateColors(btnColor, Qt::white, midlightRatio);
            darkColor     = interpolateColors(btnColor, Qt::black, darkRatio);
            midColor      = interpolateColors(btnColor, Qt::black, midRatio);
            shadowColor   = interpolateColors(btnColor, Qt::black, shadowRatio);
        } else {
            // Highlights on bright backgrounds no longer dampened; full scale applied [3]
            lightColor    = interpolateColors(btnColor, Qt::white, lightRatio);
            midlightColor = interpolateColors(btnColor, Qt::white, midlightRatio);
            darkColor     = interpolateColors(btnColor, Qt::black, darkRatio);
            midColor      = interpolateColors(btnColor, Qt::black, midRatio);
            shadowColor   = interpolateColors(btnColor, Qt::black, shadowRatio);
        }

        palette.setColor(group, QPalette::Light, lightColor);
        palette.setColor(group, QPalette::Midlight, midlightColor);
        palette.setColor(group, QPalette::Dark, darkColor);
        palette.setColor(group, QPalette::Mid, midColor);
        palette.setColor(group, QPalette::Shadow, shadowColor);
    }
}

QPalette LC_PaletteColorUtils::createPaletteFromScheme(const ColorSchemeData &scheme,
                                                       StyleArchetype archetype,
                                                       CVDType cvd) {
    QPalette palette = QApplication::palette();

    // 1. Populate basic colors into the palette sequentially using our unified iterator
    forEachRoleState([&](const PaletteRoleMapping &row, const PaletteStateMapping &state) {
        QColor col = scheme.palette.value(row.name).value(state.name);
        if (col.isValid()) {
            if (cvd != CVDType::Normal) {
                col = simulateCVD(col, cvd);
            }
            palette.setColor(state.group, row.role, col);
        }
    });

    // Stable background canvas constants (decoupled from edge intensity)
    constexpr int recessFactor = 120;
    constexpr int elevateFactor = 112;

    // ContrastWeight now exclusively controls text-to-background contrast margins (mixFactor)
    double mixFactor = 1.0;

    if (scheme.contrastWeight == ContrastWeight::None) {
        mixFactor = 0.50; // Keep baseline text contrast safe and readable on flat canvases
    } else if (scheme.contrastWeight == ContrastWeight::Soft) {
        mixFactor = 0.65;
    } else if (scheme.contrastWeight == ContrastWeight::Hard) {
        mixFactor = 1.35;
    }

    auto recessColor = [](const QColor &c) { return c.darker(recessFactor); };
    auto elevateColor = [](const QColor &c) { return c.lighter(elevateFactor); };

    // 2. Apply contrast policy modifications directly to standard palette roles
    for (const PaletteStateMapping &state : PALETTE_STATES) {
        QPalette::ColorGroup group = state.group;

        QColor winColor    = palette.color(group, QPalette::Window);
        QColor buttonColor    = palette.color(group, QPalette::Button);
        QColor highlightColor = palette.color(group, QPalette::Highlight);
        QColor textColor      = palette.color(group, QPalette::WindowText);

        QColor bg = winColor;
        QColor btn = buttonColor;
        QColor txt = textColor;
        // QColor baseCol = winColor.darker(110);
        QColor baseCol = palette.color(group, QPalette::Base);

        switch (scheme.contrastPolicy) {
            case ContrastPolicy::RecessedWindow:
                bg = recessColor(winColor);
                btn = buttonColor;
                txt = interpolateColors(textColor, bg, 0.35 * mixFactor);
                baseCol = recessColor(bg);
                break;

            case ContrastPolicy::ElevatedWindow:
                bg = elevateColor(winColor);
                btn = recessColor(buttonColor);
                txt = textColor;
                baseCol = bg.darker(110);
                break;

            case ContrastPolicy::AccentTinted:
                bg = interpolateColors(winColor, highlightColor, 0.12);
                btn = interpolateColors(buttonColor, highlightColor, 0.12);
                txt = textColor;
                baseCol = bg.darker(110);
                break;

            case ContrastPolicy::Standard:
            default:
                break;
        }

        palette.setColor(group, QPalette::Window, bg);
        palette.setColor(group, QPalette::Button, btn);
        palette.setColor(group, QPalette::WindowText, txt);
        palette.setColor(group, QPalette::Text, txt);
        palette.setColor(group, QPalette::Base, baseCol);
        palette.setColor(group, QPalette::AlternateBase, baseCol.lighter(110));
    }

    // 3. Resolve the active bevel style archetype and run 3D Helper calculation if active
    if (scheme.autoCalculate3DHelpers) {
        BevelStyle calculatedBevel = BevelStyle::Soft;
        if (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline) {
            calculatedBevel = BevelStyle::Flat;
        } else if (archetype == StyleArchetype::SoftSatin) {
            calculatedBevel = BevelStyle::Soft;
        } else if (archetype == StyleArchetype::GlassyGloss) {
            calculatedBevel = BevelStyle::Sharp;
        } else if (archetype == StyleArchetype::ClassicFusion) {
            calculatedBevel = BevelStyle::Soft; // Classic Fusion always uses standard Soft chiseled bevel borders
        }

        // Pass the scheme's specific custom seed color role directly to the lighting generator
        calculate3DHelperRoles(palette, calculatedBevel, scheme.contrastWeight, scheme.bevelSeedRole);
    }

#ifdef DEBUG_BEVEL_GENERATION
    BevelStyle debugBevel = BevelStyle::Soft;
    if (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline) {
        debugBevel = BevelStyle::Flat;
    } else if (archetype == StyleArchetype::SoftSatin) {
        debugBevel = BevelStyle::Soft;
    } else if (archetype == StyleArchetype::GlassyGloss) {
        debugBevel = BevelStyle::Sharp;
    } else if (archetype == StyleArchetype::ClassicFusion) {
        debugBevel = BevelStyle::Soft;
    }
    debugBevelCalculation(palette, debugBevel, scheme.bevelSeedRole);
#endif

    return palette;
}

void LC_PaletteColorUtils::generateHighContrastTheme(const QColor &baseColor, PaletteConfig &config, ControlStyleConfig* controlStyle) {
    int h, s, v;
    baseColor.getHsv(&h, &s, &v);
    if (h < 0) h = 120; // Default to green if grayscale

    // 1. Configure High-Contrast Metric Overrides
    if (controlStyle != nullptr) {
        controlStyle->styleArchetype = StyleArchetype::FlatModern; // Clean flat geometry
        controlStyle->boxDecoration  = BoxDecoration::BoxOutline;  // Stark 1px outlines
    }

    // ================= HIGH-CONTRAST DARK VARIANT =================
    ColorSchemeData &dark = config.dark;

    QColor hcDarkBg   = Qt::black;
    QColor hcDarkBase = Qt::black;
    QColor hcDarkBtn  = Qt::black;

    QColor hcDarkHighlight;
    hcDarkHighlight.setHsv(h, 255, 255);

    QColor hcDarkText = Qt::white;

    assignGroupHarmonics(dark, QPalette::Active, hcDarkBg, hcDarkBase, hcDarkBtn, hcDarkHighlight, hcDarkText);
    assignGroupHarmonics(dark, QPalette::Inactive, hcDarkBg, hcDarkBase, hcDarkBtn, hcDarkHighlight.darker(120), hcDarkText);

    auto hcDarkDisabledText = QColor(128, 128, 128);
    assignGroupHarmonics(dark, QPalette::Disabled, hcDarkBg, hcDarkBase, hcDarkBtn, hcDarkHighlight.darker(200), hcDarkDisabledText);

    dark.qss = ""; // Clear QCSS entirely to keep the preview window fully responsive
    dark.autoCalculate3DHelpers = true;
    dark.contrastWeight = ContrastWeight::Hard; // Maximum border intensity
    dark.contrastPolicy = ContrastPolicy::Standard;
    dark.bevelSeedRole = QPalette::Button;
    generateSemanticColors(dark, hcDarkHighlight, true, true);

    // ================= HIGH-CONTRAST LIGHT VARIANT =================
    ColorSchemeData &light = config.light;

    QColor hcLightBg   = Qt::white;
    QColor hcLightBase = Qt::white;
    QColor hcLightBtn  = Qt::white;

    QColor hcLightHighlight;
    hcLightHighlight.setHsv(h, 255, 120);

    QColor hcLightText = Qt::black;

    assignGroupHarmonics(light, QPalette::Active, hcLightBg, hcLightBase, hcLightBtn, hcLightHighlight, hcLightText);
    assignGroupHarmonics(light, QPalette::Inactive, hcLightBg, hcLightBase, hcLightBtn, hcLightHighlight.lighter(120), hcLightText);

    auto hcLightDisabledText = QColor(128, 128, 128);
    assignGroupHarmonics(light, QPalette::Disabled, hcLightBg, hcLightBase, hcLightBtn, hcLightHighlight.lighter(200), hcLightDisabledText);

    light.qss = "";
    light.autoCalculate3DHelpers = true;
    light.contrastWeight = ContrastWeight::Hard; // Maximum border intensity
    light.contrastPolicy = ContrastPolicy::Standard;
    light.bevelSeedRole = QPalette::Button;
    generateSemanticColors(dark, hcLightHighlight, false, true);
}

QColor LC_PaletteColorUtils::simulateCVD(const QColor &color, CVDType type) {
    if (!color.isValid() || type == CVDType::Normal) return color;

    const double r = color.red();
    const double g = color.green();
    const double b = color.blue();

    double rPrime = r;
    double gPrime = g;
    double bPrime = b;

    if (type == CVDType::Protanopia) {
        rPrime = 0.567 * r + 0.433 * g + 0.000 * b;
        gPrime = 0.558 * r + 0.442 * g + 0.000 * b;
        bPrime = 0.000 * r + 0.242 * g + 0.758 * b;
    } else if (type == CVDType::Deuteranopia) {
        rPrime = 0.625 * r + 0.375 * g + 0.000 * b;
        gPrime = 0.700 * r + 0.300 * g + 0.000 * b;
        bPrime = 0.000 * r + 0.300 * g + 0.700 * b;
    } else if (type == CVDType::Tritanopia) {
        rPrime = 0.950 * r + 0.050 * g + 0.000 * b;
        gPrime = 0.000 * r + 0.433 * g + 0.567 * b;
        bPrime = 0.000 * r + 0.475 * g + 0.525 * b;
    }

    const int rOut = qBound(0, static_cast<int>(rPrime), 255);
    const int gOut = qBound(0, static_cast<int>(gPrime), 255);
    const int bOut = qBound(0, static_cast<int>(bPrime), 255);

    return QColor(rOut, gOut, bOut, color.alpha());
}

void LC_PaletteColorUtils::calculateSegmentedGroupColors(
    QPalette::ColorGroup group,
    StyleArchetype archetype,
    const QPalette &palette,
    SegmentedColorPolicy policy,
    int totalGroups,
    QMap<int, QColor> &groupBgStart,
    QMap<int, QColor> &groupBgEnd,
    QMap<int, GroupGradientStops> &groupGradientStops,
    QMap<int, QColor> &groupBgStartHovered,
    QMap<int, QColor> &groupBgEndHovered,
    QMap<int, GroupGradientStops> &groupGradientStopsHovered) {

    QColor windowColor = palette.color(group, QPalette::Window);
    QColor highlightColor = palette.color(group, QPalette::Highlight);
    bool isDark = (windowColor.value() < 120);

    int h, s, v;
    highlightColor.getHsv(&h, &s, &v);
    if (h < 0) h = 210; // Default to slate-blue

    for (int i = 1; i <= qMax(1, totalGroups); ++i) {
        QColor baseCol;

        if (policy == SegmentedColorPolicy::NeutralAlternating) {
            // POLICY 0: Modulo-4 Ping-Pong Neutral Staircase (Solving Issue 1)
            int hVal, sVal, vVal;
            windowColor.getHsv(&hVal, &sVal, &vVal);
            if (hVal < 0) hVal = 0;

            // Maps sequence smoothly to: Shade 0 -> Shade 1 -> Shade 2 -> Shade 1 -> Shade 0...
            int index = (i - 1) % 4;
            if (index == 3) {
                index = 1; // Return smoothly to the intermediate contrast step
            }

            if (index == 0) {
                baseCol = windowColor; // Base canvas
            } else if (index == 1) {
                if (isDark) {
                    baseCol.setHsv(hVal, qMax(0, sVal - 2), qMin(255, vVal + 10)); // Softly lighter
                } else {
                    baseCol.setHsv(hVal, qMin(255, sVal + 3), qMax(0, vVal - 10)); // Softly darker
                }
            } else if (index == 2) {
                if (isDark) {
                    baseCol.setHsv(hVal, qMax(0, sVal - 4), qMin(255, vVal + 20)); // Moderately lighter
                } else {
                    baseCol.setHsv(hVal, qMin(255, sVal + 6), qMax(0, vVal - 20)); // Moderately darker
                }
            } else { // index == 3
                if (isDark) {
                    baseCol.setHsv(hVal, qMax(0, sVal - 6), qMin(255, vVal + 30)); // Deeply lighter
                } else {
                    baseCol.setHsv(hVal, qMin(255, sVal + 9), qMax(0, vVal - 30)); // Deeply darker
                }
            }
        }
        else {
            // --- UNIVERSAL INTERLEAVED HALF-SHIFT SHUFFLER (Solving Issue 2) ---
            int permutedIndex = (i - 1);
            if (totalGroups > 1) {
                int half = (totalGroups + 1) / 2;
                if (i % 2 != 0) {
                    permutedIndex = (i - 1) / 2;
                } else {
                    permutedIndex = half + (i / 2 - 1);
                }
            }

            if (policy == SegmentedColorPolicy::HarmonizedAnalogous) {
                // POLICY 1: Palette-matched blend with Interleaved Shuffle
                double minFactor = 0.03;
                double maxFactor = 0.16; // Softer peach-apricot tints in light mode
                if (isDark) {
                    minFactor = 0.10;
                    maxFactor = 0.42; // Higher blend factor making active dark cards lighter and more visible
                }

                double factor = minFactor;
                if (totalGroups > 1) {
                    factor = minFactor + permutedIndex * (maxFactor - minFactor) / (totalGroups - 1);
                }
                baseCol = interpolateColors(windowColor, highlightColor, factor);
            }
            else if (policy == SegmentedColorPolicy::VibrantSpectrum) {
                // POLICY 2: Shuffled full-color pastel color-wheel spectrum
                int targetH = h;
                if (totalGroups > 1) {
                    targetH = (h - 45 + permutedIndex * 360 / totalGroups) % 360;
                }
                if (targetH < 0) {
                    targetH += 360;
                }

                int targetS = isDark ? 22 : 12;
                int targetV = isDark ? 44 : 240;
                baseCol.setHsv(targetH, targetS, targetV);
            }
            else { // SegmentedColorPolicy::AnalogousHueShift
                // POLICY 3: Shuffled dynamic analogous arc slicing (90-degree total span)
                int targetH = h;
                if (totalGroups > 1) {
                    targetH = (h - 45 + permutedIndex * 90 / (totalGroups - 1)) % 360;
                }
                if (targetH < 0) {
                    targetH += 360;
                }

                int targetS = isDark ? 22 : 12;
                int targetV = isDark ? 44 : 240;
                baseCol.setHsv(targetH, targetS, targetV);
            }
        }

        QColor endCol = baseCol.darker(isDark ? 108 : 106);

        // Precompute Gradient Stops
        GroupGradientStops stops;
        stops.stop0   = baseCol.lighter(isDark ? 116 : 106);
        stops.stop42  = baseCol.lighter(isDark ? 103 : 101);
        stops.stop55  = endCol.darker(isDark ? 102 : 101);
        stops.stop100 = endCol.darker(isDark ? 106 : 104);
        groupGradientStops[i] = stops;

        GroupGradientStops hoverStops;
        hoverStops.stop0   = stops.stop0.lighter(105);
        hoverStops.stop42  = stops.stop42.lighter(105);
        hoverStops.stop55  = stops.stop55.lighter(105);
        hoverStops.stop100 = stops.stop100.lighter(105);
        groupGradientStopsHovered[i] = hoverStops;

        // Adapt precalculated colors to style archetype
        if (archetype == StyleArchetype::AccentOutline) {
            QColor accentGlow = highlightColor;
            accentGlow.setAlpha(isDark ? 25 : 35);
            QColor blended = interpolateColors(accentGlow, baseCol, 0.15);
            groupBgStart[i] = blended;
            groupBgEnd[i]   = blended;
            groupBgStartHovered[i] = blended.lighter(105);
            groupBgEndHovered[i]   = blended.lighter(105);
        } else if (archetype == StyleArchetype::SoftSatin || archetype == StyleArchetype::GlassyGloss) {
            groupBgStart[i] = baseCol;
            groupBgEnd[i]   = endCol;
            groupBgStartHovered[i] = baseCol.lighter(105);
            groupBgEndHovered[i]   = endCol.lighter(105);
        } else if (archetype == StyleArchetype::FlatModern) {
            groupBgStart[i] = baseCol;
            groupBgEnd[i]   = baseCol;
            groupBgStartHovered[i] = baseCol.lighter(105);
            groupBgEndHovered[i]   = baseCol.lighter(105);
        } else { // ClassicFusion fallback
            groupBgStart[i] = baseCol;
            groupBgEnd[i]   = endCol;
            groupBgStartHovered[i] = baseCol.lighter(105);
            groupBgEndHovered[i]   = endCol.lighter(105);
        }
    }
}

QColor LC_PaletteColorUtils::interpolateColors(const QColor &c1, const QColor &c2, double factor) {
    if (!c1.isValid()) return c2;
    if (!c2.isValid()) return c1;
    double f = qBound(0.0, factor, 1.0);
    int r = c1.red()   + static_cast<int>((c2.red()   - c1.red())   * f);
    int g = c1.green() + static_cast<int>((c2.green() - c1.green()) * f);
    int b = c1.blue()  + static_cast<int>((c2.blue()  - c1.blue())  * f);
    int a = c1.alpha() + static_cast<int>((c2.alpha() - c1.alpha()) * f);
    return QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255), qBound(0, a, 255));
}

namespace {
    QMap<LC_SemanticColors, QColor> s_resolvedSemanticColors;
}

void LC_PaletteColorUtils::resolveSemanticColors(bool hasPalette, const ColorSchemeData& scheme, bool isDark) {
    QMap<LC_SemanticColors, QColor> resolved;

    for (const auto& def : SEMANTIC_COLOR_DEFS) {
        if (hasPalette && scheme.semanticColors.contains(QLatin1String(def.key))) {
            resolved[def.role] = scheme.semanticColors.value(QLatin1String(def.key));
        } else {
            // Native/Fallback setting
            QColor fallback = isDark ? QColor(def.defaultDarkHex) : QColor(def.defaultLightHex);
            if (def.role == LC_SemanticColors::FilteredItem) {
                fallback = CFG_AppStyling::o_ColorFilteredItem.get();
            } else if (def.role == LC_SemanticColors::ConflictingItem) {
                fallback = CFG_AppStyling::o_ColorConflictingItem.get();
            } else if (def.role == LC_SemanticColors::SearchResultItem) {
                fallback = CFG_AppStyling::o_ColorSearchResultItem.get();
            }
            resolved[def.role] = fallback;
        }
    }

    setResolvedSemanticColors(resolved);
}

void LC_PaletteColorUtils::setResolvedSemanticColors(const QMap<LC_SemanticColors, QColor>& colors) {
    s_resolvedSemanticColors = colors;
}

QColor LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors role, const QPalette& palette) {
    // 1. Fast O(1) static cache lookup
    if (s_resolvedSemanticColors.contains(role)) {
        const QColor cached = s_resolvedSemanticColors.value(role);
        if (cached.isValid()) {
            return cached;
        }
    }

    // 2. Adaptive fallback
    const bool isDark = palette.color(QPalette::Window).value() < 120;
    switch (role) {
        case LC_SemanticColors::FilteredItem:
            return CFG_AppStyling::o_ColorFilteredItem.get();
        case LC_SemanticColors::ConflictingItem:
            return isDark ? QColor("#e06c77") : QColor("#d9534f");
        case LC_SemanticColors::SearchResultItem:
            return isDark ? QColor("#bd6313") : QColor("#d97706");
    }
    return palette.color(QPalette::Highlight);
}

void LC_PaletteColorUtils::generateSemanticColors(ColorSchemeData& scheme,
                                                   const QColor& accentColor,
                                                   bool isDark,
                                                   bool highContrast) {
    if (highContrast) {
        if (isDark) {
            scheme.semanticColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM] = accentColor.isValid() ? accentColor : QColor("#00ffff");
            scheme.semanticColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]  = QColor("#ff4d4d");
            scheme.semanticColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]     = QColor("#ffaa00");
        } else {
            scheme.semanticColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM] = accentColor.isValid() ? accentColor : QColor("#0000ff");
            scheme.semanticColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]  = QColor("#cc0000");
            scheme.semanticColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]     = QColor("#b35900");
        }
    } else {
        // Harmonized / Standard contrast:
        scheme.semanticColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM] = accentColor;
        scheme.semanticColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]  = isDark ? QColor("#e06c77") : QColor("#d9534f");
        scheme.semanticColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]     = isDark ? QColor("#bd6313") : QColor("#d97706");
    }
}
