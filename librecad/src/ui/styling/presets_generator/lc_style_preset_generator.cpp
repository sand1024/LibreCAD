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


#include "lc_style_preset_generator.h"

#include "lc_palette_color_utils.h"
#include "lc_icons_color_utils.h"
#include <QDir>
#include <QDateTime>
#include <qrandom.h>

#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_metrics_repository.h"
#include "lc_style_metrics_utils.h"
#include "lc_typography_repository.h"
#include "rs_debug.h"

LC_StylePresetGenerator::LC_StylePresetGenerator(const QString &skinsOutputDir, const QString &iconsOutputDir)
    : m_skinsDir(skinsOutputDir)
    , m_iconsDir(iconsOutputDir) {
    m_skinsRepo = std::make_unique<LC_FusionSkinsRepository>(m_skinsDir);
    m_iconsRepo = std::make_unique<LC_IconsStyleRepository>(m_iconsDir);
}

LC_StylePresetGenerator::~LC_StylePresetGenerator() = default;

int LC_StylePresetGenerator::generateSkins(int count) {
    // 1. Color Hues definitions (H, S, V)
    struct ColorBase { QString label; int h; int s; int v; QStringList adjectives; };
    const QList<ColorBase> bases = {
        { "Slate",  210, 20,  180, { "Steel", "Industrial", "Graphite", "Titanium", "Obsidian" } },
        { "Green",  130, 35,  170, { "Sage", "Forest", "Moss", "Juniper", "Emerald", "Verdant" } },
        { "Red",    350, 45,  160, { "Crimson", "Burgundy", "Velvet", "Ruby", "Terracotta" } },
        { "Orange", 35,  60,  210, { "Amber", "Copper", "Autumn", "Saffron", "Bronze" } },
        { "Blue",   225, 55,  180, { "Midnight", "Cobalt", "Ocean", "Sapphire", "Polaris" } },
        { "Purple", 278, 40,  170, { "Plum", "Amethyst", "Orchid", "Lavender", "Violet" } },
        { "Teal",   185, 50,  190, { "Teal", "Oceanic", "Abyss", "Lagoon", "Glacier" } },
        { "Sepia",  48,  15,  190, { "Sand", "Sepia", "Desert", "Flint", "Parchment" } }
    };

    // 2. Style Archetype Combinations
    struct StyleCombination { StyleArchetype arch; BoxDecoration dec; QStringList nouns; };
    const QList<StyleCombination> styles = {
        { StyleArchetype::FlatModern,    BoxDecoration::Frameless,         { "Minimal", "Flat", "Clean", "Draft" } },
        { StyleArchetype::FlatModern,    BoxDecoration::BoxOutline,        { "Studio", "Blueprint", "Technical", "Grid" } },
        { StyleArchetype::SoftSatin,     BoxDecoration::DividingHairline,  { "Satin", "Workspace", "Flow", "Ambient" } },
        { StyleArchetype::SoftSatin,     BoxDecoration::LeftAccentBar,     { "Sidebar", "Structure", "Accent", "Active" } },
        { StyleArchetype::GlassyGloss,    BoxDecoration::RecessedWell,      { "Gloss", "Prism", "Sheen", "Reflect" } },
        { StyleArchetype::AccentOutline, BoxDecoration::ActiveAccentFrame, { "Outline", "Stark", "Accent", "Neon" } }
    };

    int generatedCount = 0;

    for (int i = 0; i < count; ++i) {
        // Symmetrical Selection: Cycle through hues and styles to guarantee maximum visual variety
        const ColorBase &colorBase = bases[i % bases.size()];
        const StyleCombination &styleComb = styles[(i / bases.size()) % styles.size()];

        QString adjective = colorBase.adjectives[(i / (bases.size() * styles.size())) % colorBase.adjectives.size()];
        QString noun = styleComb.nouns[(i / 2) % styleComb.nouns.size()];
        QString themeName = QString("%1 %2").arg(adjective, noun);

        SkinConfig config;
        config.name = themeName;
        config.styleArchetype = styleComb.arch;
        config.boxDecoration  = styleComb.dec;

        // Custom aesthetic overrides randomly jittered for index diversity testing
        config.customDockTitleBar = (i % 3 != 0);
        config.dockTitleBarStyle  = static_cast<DockTitleBarStyle>(i % 6);
        config.accentedScrollbars = (i % 2 == 0);
        config.transparentScrollbars = (i % 3 == 0);

        // 3. Generate the light/dark ColorSchemeData palettes
        QColor baseColor;
        baseColor.setHsv(colorBase.h, colorBase.s, colorBase.v);
        LC_PaletteColorUtils::generateHarmonizedTheme(baseColor, config);

        // 4. Save using the updated Skins Repository
        QString path;
        if (m_skinsRepo->save(themeName, config, path)) {
            generatedCount++;
        }
    }

    return generatedCount;
}

#include <QSet>

int LC_StylePresetGenerator::generateIconStyles(int count, bool shortWheel) {
    // Expanded baseline of 24 highly distinct, beautifully curated accents
    struct AccentBase { QString label; QColor accent; };
    // 72 unique variants
    const QList<AccentBase> accentBasesShort = {
        { "Ruby",        QColor("#e6194b") }, // Vivid Red
        { "Coral",       QColor("#f58231") }, // Vivid Orange
        { "Amber",       QColor("#f5a623") }, // Golden Orange
        { "Lemon",       QColor("#ffe119") }, // Warm Yellow
        { "Lime",        QColor("#bfef45") }, // Chartreuse/Spring Green
        { "Emerald",     QColor("#3cb44b") }, // Pure Green
        { "Teal",        QColor("#469990") }, // Deep Teal
        { "Cyan",        QColor("#00ffff") }, // Pure Electric Cyan
        { "Cobalt",      QColor("#0082c8") }, // Deep Royal Blue
        { "Wisteria",    QColor("#911eb4") }, // Deep Purple
        { "Pink",        QColor("#ff69b4") }, // Vivid Hot Pink
        { "Sepia",       QColor("#9a7b56") }  // Warm Drafting Earth
    };

    // Curated Large List (45 distinct halftone, warm/cool, and pastel variants)
    // 270 variants
    const QList<AccentBase> accentBasesLarge = {
        // Reds & Pinks
        { "Ruby",        QColor("#e6194b") },
        { "Crimson",     QColor("#b30000") },
        { "Terracotta",  QColor("#d9534f") },
        { "Salmon",      QColor("#ff8080") },
        { "Hot Pink",    QColor("#ff69b4") },
        { "Rose",        QColor("#fabed4") },
        { "Magenta",     QColor("#f032e6") },
        { "Plum",        QColor("#800040") },
        // Oranges & Browns
        { "Coral",       QColor("#f58231") },
        { "Rust",        QColor("#c44d00") },
        { "Amber",       QColor("#f5a623") },
        { "Copper",      QColor("#b87333") },
        { "Sepia",       QColor("#9a7b56") },
        { "Peach",       QColor("#ffd8b1") },
        { "Sand",        QColor("#e5c158") },
        // Yellows & Chartreuses
        { "Saffron",     QColor("#ffe119") },
        { "Lemon",       QColor("#fff700") },
        { "Cream",       QColor("#fffac8") },
        { "Lime",        QColor("#bfef45") },
        { "Absinthe",    QColor("#9acd32") },
        // Greens
        { "Emerald",     QColor("#3cb44b") },
        { "Forest",      QColor("#228b22") },
        { "Sage",        QColor("#8db600") },
        { "Mint",        QColor("#aaffc3") },
        { "Celadon",     QColor("#ace1af") },
        { "Olive-Gray",  QColor("#aaaa7f") },
        { "Jade",        QColor("#00a86b") },
        // Teals & Cyans
        { "Teal",        QColor("#469990") },
        { "Ocean Teal",  QColor("#00aa7f") },
        { "Cyan",        QColor("#00ffff") },
        { "Turquoise",   QColor("#42d4f4") },
        { "Celeste",     QColor("#aaffff") },
        { "Aqua",        QColor("#00d6d6") },
        // Blues
        { "Sky Blue",    QColor("#87ceeb") },
        { "Cobalt",      QColor("#0082c8") },
        { "Azure",       QColor("#00aaff") },
        { "Steel Blue",  QColor("#4682b4") },
        { "Indigo",      QColor("#4b0082") },
        { "Pewter",      QColor("#cbdade") },
        { "Periwinkle",  QColor("#9999ff") },
        // Purples
        { "Wisteria",    QColor("#911eb4") },
        { "Amethyst",    QColor("#9966cc") },
        { "Lavender",    QColor("#e6beff") },
        { "Orchid",      QColor("#da70d6") },
        { "Slate Blue",  QColor("#708090") }
    };

    QList<AccentBase> accentBases = shortWheel ? accentBasesShort: accentBasesLarge;

    QSet<QString> generatedNames;
    int generatedCount = 0;

    for (int i = 0; i < 5000 && generatedCount < count; ++i) {
        const auto &base = accentBases[i % accentBases.size()];
        IconContrastMode contrastMode = static_cast<IconContrastMode>((i / accentBases.size()) % 3);
        AccentHarmonizationPolicy policy = static_cast<AccentHarmonizationPolicy>((i / (accentBases.size() * 3)) % 2);

        // Extract HSV and apply controlled random jitter
        int h, s, v;
        base.accent.getHsv(&h, &s, &v);
        if (h < 0) h = 120;

        int hJitter = QRandomGenerator::global()->bounded(-5, 6);
        int sJitter = QRandomGenerator::global()->bounded(-10, 11);
        int vJitter = QRandomGenerator::global()->bounded(-10, 11);

        int finalH = (h + hJitter + 360) % 360;
        int finalS = qBound(50, s + sJitter, 255);
        int finalV = qBound(80, v + vJitter, 255);

        QColor jitteredAccent;
        jitteredAccent.setHsv(finalH, finalS, finalV);

        // Generate unique names
        QString modeStr = (contrastMode == IconContrastMode::Soft) ? "Soft" :
                          (contrastMode == IconContrastMode::Balanced) ? "Balanced" : "Stark";
        QString policyStr = (policy == AccentHarmonizationPolicy::ComplementaryShift) ? "Complement" : "Active";
        QString styleName = QString("%1 %2 %3").arg(base.label, modeStr, policyStr);

        // Symmetrical Uniqueness Loop: If the name is already taken, append a sequential suffix
        int suffix = 2;
        QString baseStyleName = styleName;
        while (generatedNames.contains(styleName) || m_iconsRepo->exists(styleName)) {
            styleName = QString("%1 #%2").arg(baseStyleName).arg(suffix++);
        }
        generatedNames.insert(styleName);

        IconStyleConfig style;
        style.name = styleName;
        style.autoCalculateStates = true;

        QColor bgDark(30, 30, 30);
        QColor bgLight(245, 245, 245);

        QColor darkMain, darkAccent, darkAccentChecked, darkBack;
        LC_IconsColorUtils::harmonizeSeeds(jitteredAccent, true, bgDark, contrastMode, policy,
                                           darkMain, darkAccent, darkAccentChecked, darkBack);

        style.dark.genericMain          = darkMain.name(QColor::HexRgb);
        style.dark.genericAccent        = darkAccent.name(QColor::HexRgb);
        style.dark.genericAccentChecked = darkAccentChecked.name(QColor::HexRgb);
        style.dark.genericBack          = darkBack.isValid() ? darkBack.name(QColor::HexRgb) : "";

        QColor lightMain, lightAccent, lightAccentChecked, lightBack;
        LC_IconsColorUtils::harmonizeSeeds(jitteredAccent, false, bgLight, contrastMode, policy,
                                           lightMain, lightAccent, lightAccentChecked, lightBack);

        style.light.genericMain          = lightMain.name(QColor::HexRgb);
        style.light.genericAccent        = lightAccent.name(QColor::HexRgb);
        style.light.genericAccentChecked = lightAccentChecked.name(QColor::HexRgb);
        style.light.genericBack          = lightBack.isValid() ? lightBack.name(QColor::HexRgb) : "";

        QString path;
        if (m_iconsRepo->save(styleName, style, path)) {
            generatedCount++;

            // Symmetrical Logging: Prints the raw QString values directly
            LC_ERR << "[PRESET-GENERATOR] New Icon Style Saved: " << styleName.toUtf8().constData()
                   << " | Dark Main = " << style.dark.genericMain.toUtf8().constData()
                   << " | Dark Accent = " << style.dark.genericAccent.toUtf8().constData()
                   << " | Dark Accent Checked = " << style.dark.genericAccentChecked.toUtf8().constData()
                   << " | Light Main = " << style.light.genericMain.toUtf8().constData()
                   << " | Light Accent = " << style.light.genericAccent.toUtf8().constData()
                   << " | Light Accent Checked = " << style.light.genericAccentChecked.toUtf8().constData();
        }
    }

    return generatedCount;
}

int LC_StylePresetGenerator::generateTypography(int count) {
    struct FontBase { QString label; QString family; QString techFamily; };
    const QList<FontBase> fontBases = {
        { "Standard",  "Segoe UI",       "Consolas" },
        { "Draft",     "Arial",          "Courier New" },
        { "Legacy",    "Times New Roman", "Fixedsys" },
        { "Technical", "Helvetica",      "Source Code Pro" },
        { "Corporate", "Roboto",         "Inconsolata" },
        { "Futuristic", "Futura",         "Fira Code" },
        { "Geometric", "Century Gothic", "Monaco" }
    };

    int generatedCount = 0;
    for (int i = 0; i < count; ++i) {
        const auto& base = fontBases[i % fontBases.size()];

        QString presetName = QString("%1 Layout Font").arg(base.label);
        if (i >= fontBases.size()) {
            presetName += QString(" #%1").arg(i / fontBases.size() + 1);
        }

        FontConfig config;
        config.name = presetName;
        config.mainFamily = base.family;
        config.mainSize   = 8 + (i % 5); // Diverse base point sizes (8pt to 12pt)
        config.techFamily = base.techFamily;

        // Synthesize diverse relative offsets for robust layout testing
        config.headings.setup(2, (i % 2 == 0), false);
        config.menuBar.setup(1, false, (i % 3 == 0));
        config.menus.setup(0, false, false);
        config.buttons.setup((i % 2 == 0) ? -1 : 0, false, false);
        config.inputs.setup(0, false, false);
        config.technical.setup(0, false, false);

        QString path;
        if (m_typographyRepo->save(presetName, config, path)) {
            generatedCount++;
        }
    }
    return generatedCount;
}

int LC_StylePresetGenerator::generateMetrics(int count) {
    struct DensityBase { QString label; QString presetName; };
    const QList<DensityBase> densityBases = {
        { "Stark Compact", "compact" },
        { "Draft Standard", "standard" },
        { "Spacious Presentation", "spacious" }
    };

    int generatedCount = 0;
    for (int i = 0; i < count; ++i) {
        const auto& base = densityBases[i % densityBases.size()];

        QString presetName = QString("%1 Metric").arg(base.label);
        if (i >= densityBases.size()) {
            presetName += QString(" #%1").arg(i / densityBases.size() + 1);
        }

        StyleMetricsConfig config;
        config.name = presetName;

        // 1. Apply baseline layout density configurations
        LC_StyleMetricsUtils::applyDensityPreset(config, base.presetName);

        // 2. Add controlled random jitter to physical values to guarantee layout index variability
        config.layoutMargin = qBound(2, config.layoutMargin + (i % 3) - 1, 15);
        config.layoutSpacing = qBound(2, config.layoutSpacing + (i % 3) - 1, 12);
        config.scrollBarWidth = qBound(8, config.scrollBarWidth + (i % 5) - 2, 22);
        config.buttonPadding = qBound(2, config.buttonPadding + (i % 3) - 1, 10);


        QString path;
        if (m_metricsRepo->save(presetName, config, path)) {
            generatedCount++;
        }
    }
    return generatedCount;
}
