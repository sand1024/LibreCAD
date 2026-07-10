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

#ifndef LC_ICONS_COLOR_UTILS_H
#define LC_ICONS_COLOR_UTILS_H

#include <QColor>
#include "lc_icon_engine_shared.h"

enum class IconContrastMode {
    Soft = 0,
    Balanced,
    Contrast
};

enum class AccentHarmonizationPolicy {
    ComplementaryShift = 0, // Option A: Shift hue by 180 degrees (Complementary)
    LuminosityBoost,        // Option B: Same hue, but increased saturation and value
    PaletteHighlightLink    // Option C: Dynamic link to the active theme's highlight
};

class LC_IconsColorUtils {
public:

    static QColor generateCheckedAccent(const QColor &baseAccent,
                                        AccentHarmonizationPolicy policy,
                                        bool isDarkMode);

    // Generates harmonized Main and Back seed colors based on the selected Accent color and active background
    static void harmonizeSeeds(const QColor &accentColor,
                           bool isDarkMode,
                           const QColor &bgWindow,
                           IconContrastMode contrastMode,
                           AccentHarmonizationPolicy accentPolicy,
                           QColor &outMain,
                           QColor &outAccent,
                           QColor &outAccentChecked,
                           QColor &outBack);

    // Computes the dynamic state override color based on baseline seeds and active system colors
    static QColor calculateStateColor(LC_SVGIconEngineAPI::IconMode mode,
                                      LC_SVGIconEngineAPI::IconState state,
                                      LC_SVGIconEngineAPI::ColorType type,
                                      const QColor &mainSeed,
                                      const QColor &accentSeedNormal,
                                      const QColor &accentSeedChecked,
                                      const QColor &backSeed,
                                      const QColor &highlightColor,
                                      const QColor &highlightedTextColor,
                                      const QColor &windowColor);


    // Utility to linearly interpolate between two colors
    static QColor interpolateColor(const QColor &c1, const QColor &c2, double factor);
};

#endif
