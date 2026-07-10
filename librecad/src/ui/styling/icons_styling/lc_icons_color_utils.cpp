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

#include "lc_icons_color_utils.h"
#include "rs_debug.h"
#include "lc_palette_color_utils.h"


namespace {
    // Encapsulated, non-recursive core calculation helper with perceptual luminance protection
    void calculateHarmonizedColors(const QColor &accentColor,
                                   bool isDarkMode,
                                   const QColor &bgWindow,
                                   IconContrastMode contrastMode,
                                   QColor &outMain,
                                   QColor &outAccent,
                                   QColor &outBack) {
        int h, s, v;
        accentColor.getHsv(&h, &s, &v);
        if (h < 0) h = 120; // Default to neutral green if grayscale

        int bgH, bgS, bgV;
        bgWindow.getHsv(&bgH, &bgS, &bgV);

        outAccent = accentColor;

        if (isDarkMode) {
            // --- DARK MODE HARMONIZATION & ACCENT POLICIES ---

            // Perceptual Floor: Blues, Violets, and Indigos [200, 290] require higher brightness
            // to remain distinct against dark window backgrounds
            int minAccentV = 160;
            if (h >= 200 && h <= 290) {
                minAccentV = 190; // Elevate deep blues to a vibrant, visible neon spectrum
            }

            if (contrastMode == IconContrastMode::Soft) {
                outAccent.setHsv(h, qMax(s, 120), qBound(minAccentV, v, 210));

                // Soft Dark Main (Strokes): Soft, neon-colored strokes (e.g., soft blue-gray instead of white)
                int mainV = qBound(180, bgV + 100, 205);
                outMain.setHsv(h, 25, mainV);

                int backV = qBound(15, bgV - 10, 50);
                outBack.setHsv(h, 10, backV);
            }
            else if (contrastMode == IconContrastMode::Balanced) {
                int targetV = qBound(minAccentV, v, 225);
                int mainV = qBound(220, bgV + 140, 240);
                if (qAbs(targetV - mainV) < 45) {
                    targetV = qMax(minAccentV, mainV - 45);
                }
                outAccent.setHsv(h, qMax(s, 140), targetV);

                // Balanced Dark Main: Clean, highly readable off-white strokes with a subtle tint
                outMain.setHsv(h, 10, mainV);

                int backV = qBound(35, bgV + 15, 60);
                outBack.setHsv(h, 25, backV);
            }
            else { // Contrast Mode
                outAccent.setHsv(h, qMax(s, 180), qMax(minAccentV, 180));
                outMain = QColor(255, 255, 255);
                outBack = QColor(); // Fully transparent
            }
        } else {
            // --- LIGHT MODE HARMONIZATION & ACCENT POLICIES ---

            // Perceptual Floor: Darker hues require a significantly higher brightness floor (165)
            // in Light Mode to keep details clearly distinct from dark charcoal/black strokes
            int minAccentV = 120;
            if (h >= 200 && h <= 290) {
                minAccentV = 165; // Elevate deep blues/purples to a clear sapphire/wisteria spectrum
            }

            if (contrastMode == IconContrastMode::Soft) {
                outAccent.setHsv(h, qMax(s, 160), qBound(minAccentV, v, 175));

                // Soft Light Main: Solid, readable dark strokes (Value <= 60) tinted with Accent's Hue
                int mainV = qBound(30, bgV - 185, 60);
                outMain.setHsv(h, 30, mainV);

                int backV = qBound(200, bgV - 25, 220);
                outBack.setHsv(h, 15, backV);
            }
            else if (contrastMode == IconContrastMode::Balanced) {
                // Ensure Accent remains solid and visible [120, 160]
                // We use qMax(minAccentV, 160) to prevent assertion failures if the dark hue floor is raised [3]
                int targetV = qBound(minAccentV, v, qMax(minAccentV, 160));
                int mainV = qBound(15, bgV - 220, 35);
                if (qAbs(targetV - mainV) < 45) {
                    targetV = qMin(160, mainV + 45);
                }
                outAccent.setHsv(h, qMax(s, 180), targetV);

                // Balanced Light Main: Stark, deep charcoal strokes (Value <= 35) with a very subtle tint
                outMain.setHsv(h, 12, mainV);

                // Soft, light-pastel background fills
                int backV = qBound(235, bgV + 10, 255);
                outBack.setHsv(h, 10, backV);
            }
            else { // Contrast Mode
                outAccent.setHsv(h, 255, qBound(minAccentV, v, 180));
                outMain = QColor(0, 0, 0);
                outBack = QColor(); // Fully transparent
            }
        }
    }
}


QColor LC_IconsColorUtils::interpolateColor(const QColor &c1, const QColor &c2, double factor) {
    return LC_PaletteColorUtils::interpolateColors(c1, c2, factor);
}

QColor LC_IconsColorUtils::generateCheckedAccent(const QColor &baseAccent, AccentHarmonizationPolicy policy, bool isDarkMode) {
    if (!baseAccent.isValid()) return QColor();

    int h, s, v;
    baseAccent.getHsv(&h, &s, &v);
    if (h < 0) h = 120; // Default to neutral green if grayscale

    QColor outColor;
    if (policy == AccentHarmonizationPolicy::ComplementaryShift) {
        // Option A: Shift Hue by 150 degrees (Analogous-Complementary) for sophisticated color pairs
        int targetH = (h + 150) % 360;
        int targetS = isDarkMode ? qMax(s, 160) : qMax(s, 140);
        int targetV = isDarkMode ? qMax(v, 180) : qMin(v, 160);
        outColor.setHsv(targetH, targetS, targetV);
    }
    else if (policy == AccentHarmonizationPolicy::LuminosityBoost) {
        // Option B: Keep same Hue, but dramatically boost Saturation/Value to indicate "Active"
        if (isDarkMode) {
            int targetS = qBound(180, s + 40, 255);
            int targetV = qBound(200, v + 40, 255);
            outColor.setHsv(h, targetS, targetV);
        } else {
            int targetS = qBound(200, s + 60, 255);
            int targetV = qBound(0, v - 40, 140); // Deepen value to remain solid
            outColor.setHsv(h, targetS, targetV);
        }
    }
    else if (policy == AccentHarmonizationPolicy::PaletteHighlightLink) {
        outColor = QColor(); // Transparent / Fallback
    }

    return outColor;
}


#define DEBUG_ICON_HARMONIZE_

void LC_IconsColorUtils::harmonizeSeeds(const QColor &accentColor,
                                        bool isDarkMode,
                                        const QColor &bgWindow,
                                        IconContrastMode contrastMode,
                                        AccentHarmonizationPolicy accentPolicy,
                                        QColor &outMain,
                                        QColor &outAccent,
                                        QColor &outAccentChecked,
                                        QColor &outBack) {
    // 1. Run calculation for the active contrast mode
    calculateHarmonizedColors(accentColor, isDarkMode, bgWindow, contrastMode, outMain, outAccent, outBack);
    outAccentChecked = generateCheckedAccent(outAccent, accentPolicy, isDarkMode);

#ifdef DEBUG_ICON_HARMONIZE
    // 2. Gather diagnostic data for ALL modes safely
    QColor mainSoft, accentSoft, backSoft;
    QColor mainBal, accentBal, backBal;
    QColor mainCont, accentCont, backCont;

    calculateHarmonizedColors(accentColor, isDarkMode, bgWindow, IconContrastMode::Soft, mainSoft, accentSoft, backSoft);
    calculateHarmonizedColors(accentColor, isDarkMode, bgWindow, IconContrastMode::Balanced, mainBal, accentBal, backBal);
    calculateHarmonizedColors(accentColor, isDarkMode, bgWindow, IconContrastMode::Contrast, mainCont, accentCont, backCont);

    // Helper lambda to print formatted HSV values
    auto hsvStr = [](const QColor &c) -> QString {
        if (!c.isValid()) return "Invalid";
        int h, s, v;
        c.getHsv(&h, &s, &v);
        return QString("%1 (H:%2, S:%3, V:%4)").arg(c.name(QColor::HexRgb)).arg(h).arg(s).arg(v);
    };

    // 3. Print the comprehensive matrix using the LC_ERR << stream macro
    LC_ERR << "\n========================================= [HARMONIZE SEEDS MATRIX] =========================================";
    LC_ERR << "Input Conditions:";
    LC_ERR << "  -> Target Mode:  " << (contrastMode == IconContrastMode::Soft ? "SOFT" : (contrastMode == IconContrastMode::Balanced ? "BALANCED" : "CONTRAST"));
    LC_ERR << "  -> Accent Seed:  " << hsvStr(accentColor).toUtf8().constData();
    LC_ERR << "  -> Dark Mode:    " << (isDarkMode ? "TRUE" : "FALSE");
    LC_ERR << "  -> bgWindow:     " << hsvStr(bgWindow).toUtf8().constData();
    LC_ERR << "------------------------------------------------------------------------------------------------------------";
    LC_ERR << "Evaluated Outputs for [SOFT] Contrast Mode:";
    LC_ERR << "  -> Main (Stroke): " << hsvStr(mainSoft).toUtf8().constData();
    LC_ERR << "  -> Accent (Detail):" << hsvStr(accentSoft).toUtf8().constData();
    LC_ERR << "  -> Back (Fill):   " << (backSoft.isValid() ? hsvStr(backSoft).toUtf8().constData() : "Transparent");
    LC_ERR << "------------------------------------------------------------------------------------------------------------";
    LC_ERR << "Evaluated Outputs for [BALANCED] Contrast Mode:";
    LC_ERR << "  -> Main (Stroke): " << hsvStr(mainBal).toUtf8().constData();
    LC_ERR << "  -> Accent (Detail):" << hsvStr(accentBal).toUtf8().constData();
    LC_ERR << "  -> Back (Fill):   " << (backBal.isValid() ? hsvStr(backBal).toUtf8().constData() : "Transparent");
    LC_ERR << "------------------------------------------------------------------------------------------------------------";
    LC_ERR << "Evaluated Outputs for [CONTRAST] Contrast Mode:";
    LC_ERR << "  -> Main (Stroke): " << hsvStr(mainCont).toUtf8().constData();
    LC_ERR << "  -> Accent (Detail):" << hsvStr(accentCont).toUtf8().constData();
    LC_ERR << "  -> Back (Fill):   " << (backCont.isValid() ? hsvStr(backCont).toUtf8().constData() : "Transparent");
    LC_ERR << "============================================================================================================\n";
#endif

}

QColor LC_IconsColorUtils::calculateStateColor(LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state,
                                               LC_SVGIconEngineAPI::ColorType type, const QColor& mainSeed, const QColor& accentSeedNormal,
                                               const QColor& accentSeedChecked, const QColor& backSeed, const QColor& highlightColor,
                                               const QColor& highlightedTextColor, const QColor& windowColor) {
    // Multiplex Accent color: Use the specialized AccentChecked for On states (if valid), otherwise fallback to Normal
    QColor activeAccent = (state == LC_SVGIconEngineAPI::On && accentSeedChecked.isValid())
                          ? accentSeedChecked
                          : accentSeedNormal;

    // Disabled desaturation
    auto disableColor = [windowColor](const QColor &c) {
        if (!c.isValid() || c == Qt::transparent) return QColor();
        int r = windowColor.red()   + static_cast<int>((c.red()   - windowColor.red())   * 0.35);
        int g = windowColor.green() + static_cast<int>((c.green() - windowColor.green()) * 0.35);
        int b = windowColor.blue()  + static_cast<int>((c.blue()  - windowColor.blue())  * 0.35);
        return QColor(r, g, b, c.alpha());
    };

    // Selection highlights
    QColor selectedBack = highlightColor;
    QColor selectedMain = highlightedTextColor;
    QColor selectedAccent = activeAccent;
    if (selectedAccent.isValid()) {
        selectedAccent = (selectedBack.value() < 120) ? selectedAccent.lighter(130) : selectedAccent.darker(130);
    }

    switch (mode) {
        case LC_SVGIconEngineAPI::Normal:
            if (type == LC_SVGIconEngineAPI::Main)       return mainSeed;
            else if (type == LC_SVGIconEngineAPI::Accent) return activeAccent;
            else                                         return backSeed;

        case LC_SVGIconEngineAPI::Active:
            // Symmetrically return mainSeed directly instead of interpolating with accentSeed
            if (type == LC_SVGIconEngineAPI::Main)       return mainSeed;
            else if (type == LC_SVGIconEngineAPI::Accent) return activeAccent;
            else                                         return backSeed;

        case LC_SVGIconEngineAPI::Selected:
            if (type == LC_SVGIconEngineAPI::Main)       return selectedMain;
            else if (type == LC_SVGIconEngineAPI::Accent) return selectedAccent;
            else                                         return selectedBack;

        case LC_SVGIconEngineAPI::Disabled:
            if (type == LC_SVGIconEngineAPI::Main)       return disableColor(mainSeed);
            else if (type == LC_SVGIconEngineAPI::Accent) return disableColor(activeAccent);
            else                                         return disableColor(backSeed);

        default:
            break;
    }

    return QColor();
}
