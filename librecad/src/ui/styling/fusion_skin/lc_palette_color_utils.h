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

#ifndef LC_PALETTECOLORUTILS_H
#define LC_PALETTECOLORUTILS_H

#include <QApplication>

#include "lc_palette_editor_shared.h"

#define DEBUG_BEVEL_GENERATION_

enum class LC_SemanticColors;

class LC_PaletteColorUtils {
public:
    // Color Vision Deficiency Types
    enum class CVDType {
        Normal = 0,
        Protanopia,   // Red-Blind
        Deuteranopia, // Green-Blind
        Tritanopia    // Blue-Blind
    };

    struct GroupGradientStops {
        QColor stop0;
        QColor stop42;
        QColor stop55;
        QColor stop100;
    };

    // Default Initializers for Separated Configurations
    static void initializeDefaultPalette(PaletteConfig &config);
    static void initializeDefaultControlStyle(ControlStyleConfig &config);
    static void initializeDefaultConfig(ControlStyleConfig &config);
    static void initializeWithPalette(PaletteConfig &config, const QPalette &palette);

    // 1-Click Palette Generators
    static void generateHarmonizedTheme(const QColor &baseColor, PaletteConfig &config);
    static void generateHarmonizedTheme(const QColor &surfaceColor, const QColor &accentColor, PaletteConfig &config);
    static void generateHighContrastTheme(const QColor &baseColor, PaletteConfig &config, ControlStyleConfig* controlStyle = nullptr); // fixme - review

    static bool isPaletteDarkMode();
    static bool isSystemInDarkMode();
    static void calculate3DHelperRoles(QPalette &palette, BevelStyle style, ContrastWeight weight = ContrastWeight::Balanced, QPalette::ColorRole seedRole = QPalette::Button);

    // Unified palette generator that pre-calculates the 30 composition variants directly into standard roles
    static QPalette createPaletteFromScheme(const ColorSchemeData& scheme, StyleArchetype archetype, CVDType cvd);
    static QColor simulateCVD(const QColor &color, CVDType type);

    static void calculateSegmentedGroupColors(QPalette::ColorGroup group, StyleArchetype archetype, const QPalette& palette,
                                              SegmentedColorPolicy policy, int totalGroups,
                                              QMap<int, QColor>& groupBgStart, QMap<int, QColor>& groupBgEnd,
                                              QMap<int, GroupGradientStops>& groupGradientStops, QMap<int, QColor>& groupBgStartHovered,
                                              QMap<int, QColor>& groupBgEndHovered,
                                              QMap<int, GroupGradientStops>& groupGradientStopsHovered);


    static QColor interpolateColors(const QColor &c1, const QColor &c2, double factor);
    static void resolveSemanticColors(bool hasPalette, const ColorSchemeData& scheme, bool isDark);

    static void setResolvedSemanticColors(const QMap<LC_SemanticColors, QColor>& colors);
    static QColor getSemanticColor(LC_SemanticColors role, const QPalette& palette = QApplication::palette());
    static void generateSemanticColors(ColorSchemeData& scheme, const QColor& accentColor, bool isDark, bool highContrast);

private:
    static void assignGroupHarmonics(ColorSchemeData &scheme, QPalette::ColorGroup group,
                                     const QColor &bg, const QColor &base, const QColor &btn,
                                     const QColor &highlight, const QColor &text);

#ifdef DEBUG_BEVEL_GENERATION
    static void debugBevelCalculation(const QPalette &palette, BevelStyle style, QPalette::ColorRole seedRole);
#endif
};

#endif
