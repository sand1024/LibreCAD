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

#include "lc_skin_colors_resolver.h"
#include <QApplication>
#include <QDockWidget>
#include <QIconEngine>
#include <QPainter>
#include "lc_palette_color_utils.h"

namespace {
    class LC_VectorIconEngine : public QIconEngine {
    public:
        enum IconType { Close = 0, Float, Min, Max };

        LC_VectorIconEngine(IconType type, bool useAccent)
            : m_type(type), m_useAccent(useAccent) {
        }

        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override {
            Q_UNUSED(state);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            const QWidget* widget = nullptr;
            if (painter->device() && painter->device()->devType() == QInternal::Widget) {
                widget = static_cast<const QWidget*>(painter->device());
            }
            QPalette palette = widget ? widget->palette() : QApplication::palette();

            SkinColors desc;
            const QStyle* style = QApplication::style();
            if (style) {
                QPalette::ColorGroup group = (mode == QIcon::Disabled) ? QPalette::Disabled : QPalette::Active;
                // Safely resolve the descriptor from style's pass-through
                const QMetaObject* meta = style->metaObject();
                if (meta) {
                    QMetaObject::invokeMethod(const_cast<QStyle*>(style), "getCachedStyleDescriptor", Q_RETURN_ARG(SkinColors, desc),
                                              Q_ARG(QPalette, palette), Q_ARG(QPalette::ColorGroup, group));
                }
            }

            QColor strokeColor = desc.groupBox.groupBoxFrameColor;
            if (m_useAccent) {
                strokeColor = palette.color(QPalette::Active, QPalette::Highlight);
            }

            if (mode == QIcon::Active || mode == QIcon::Selected) {
                strokeColor = palette.color(QPalette::Active, QPalette::Highlight);
            }
            else if (mode == QIcon::Disabled) {
                strokeColor = palette.color(QPalette::Disabled, QPalette::WindowText);
            }

            qreal scaleFactor = static_cast<qreal>(rect.width()) / 16.0;
            qreal penWidth = 1.25 * scaleFactor;
            QPoint cx = rect.center();
            int hSize = qMax(3, rect.width() * 80 / 200);

            // Fix: Delegate rendering directly to static utility functions to eliminate code duplication
            if (m_type == Close) {
                LC_SkinColorsResolver::drawCloseIcon(painter, QRectF(rect), strokeColor, penWidth);
            }
            else if (m_type == Float) {
                LC_SkinColorsResolver::drawFloatIcon(painter, QRectF(rect), strokeColor, penWidth);
            }
            else if (m_type == Min) {
                painter->setPen(QPen(strokeColor, penWidth, Qt::SolidLine, Qt::RoundCap));
                painter->drawLine(cx.x() - hSize, cx.y() + hSize / 2, cx.x() + hSize, cx.y() + hSize / 2);
            }
            else if (m_type == Max) {
                painter->setPen(QPen(strokeColor, penWidth * 0.85, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(QRect(cx.x() - hSize, cx.y() - hSize, hSize * 2, hSize * 2));
            }
            painter->restore();
        }

        QIconEngine* clone() const override {
            return new LC_VectorIconEngine(m_type, m_useAccent);
        }

    private:
        IconType m_type;
        bool m_useAccent;
    };
}

LC_SkinColorsResolver::LC_SkinColorsResolver() {
    invalidate();
}

LC_SkinColorsResolver::~LC_SkinColorsResolver() = default;

void LC_SkinColorsResolver::setSkin(const ControlStyleConfig& skin) {
    m_skin = skin;

    m_isFlat = (skin.styleArchetype == StyleArchetype::FlatModern || skin.styleArchetype == StyleArchetype::AccentOutline);
    m_isClassic = (skin.styleArchetype == StyleArchetype::ClassicFusion);
    m_isFlatModern = (skin.styleArchetype == StyleArchetype::FlatModern);
    m_isGlossy = (skin.styleArchetype == StyleArchetype::GlassyGloss);
    m_isSoftSatin = (skin.styleArchetype == StyleArchetype::SoftSatin);
    m_isAccentOnline = (skin.styleArchetype == StyleArchetype::AccentOutline);

    m_activeDockTitleStyle = skin.dockTitleBarStyle;
    if (!skin.customDockTitleBar) {
        switch (skin.styleArchetype) {
            case StyleArchetype::FlatModern:
                m_activeDockTitleStyle = DockTitleBarStyle::CustomSolid;
                break;
            case StyleArchetype::SoftSatin:
                m_activeDockTitleStyle = DockTitleBarStyle::CustomSatin;
                break;
            case StyleArchetype::GlassyGloss:
                m_activeDockTitleStyle = DockTitleBarStyle::CustomGlassy;
                break;
            case StyleArchetype::AccentOutline:
                m_activeDockTitleStyle = DockTitleBarStyle::CustomAccentOutline;
                break;
            case StyleArchetype::ClassicFusion: default:
                m_activeDockTitleStyle = DockTitleBarStyle::Native;
                break;
        }
    }

    invalidate();
}

void LC_SkinColorsResolver::invalidate() const {
    m_globalSlot.palette = QPalette();
    m_globalSlot.descriptors[0] = SkinColors();
    m_globalSlot.descriptors[1] = SkinColors();
    m_globalSlot.descriptors[2] = SkinColors();

    m_customSlot.palette = QPalette();
    m_customSlot.descriptors[0] = SkinColors();
    m_customSlot.descriptors[1] = SkinColors();
    m_customSlot.descriptors[2] = SkinColors();

    m_iconCache.clear();
    m_cachedMonoPointSize = -1;
    m_cachedMonoFont = QFont();
}

SkinColors LC_SkinColorsResolver::getDescriptor(const QPalette& palette, QPalette::ColorGroup group) const {
    const int idx = static_cast<int>(group);

    // Verify both the palette matches AND the specific color-group descriptor has been calculated
    if (palette == m_globalSlot.palette && m_globalSlot.descriptors[idx].common.bgStart.isValid()) {
        return m_globalSlot.descriptors[idx];
    }
    if (palette == m_customSlot.palette && m_customSlot.descriptors[idx].common.bgStart.isValid()) {
        return m_customSlot.descriptors[idx];
    }

    const SkinColors desc = resolveStyleDescriptor(m_skin.styleArchetype, palette, group);

    if (palette == QApplication::palette()) {
        m_globalSlot.palette = palette;
        m_globalSlot.descriptors[idx] = desc;
    }
    else {
        m_customSlot.palette = palette;
        m_customSlot.descriptors[idx] = desc;
    }

    return desc;
}

QIcon LC_SkinColorsResolver::getStandardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const {
    if (m_skin.customVectorIconsInButtons) {
        const bool isClose = (standardIcon == QStyle::SP_TitleBarCloseButton || standardIcon == QStyle::SP_DockWidgetCloseButton);
        const bool isFloat = (standardIcon == QStyle::SP_TitleBarNormalButton);
        const bool isMin = (standardIcon == QStyle::SP_TitleBarMinButton);
        const bool isMax = (standardIcon == QStyle::SP_TitleBarMaxButton);

        if (isClose || isFloat || isMin || isMax) {
            const QPalette::ColorGroup group = (option && (option->state & QStyle::State_Enabled)) ? QPalette::Active : QPalette::Disabled;
            const int iconId = isClose ? 1 : (isFloat ? 2 : (isMin ? 3 : 4));
            const int key = (iconId << 8) | (static_cast<int>(group) << 4) | (m_skin.accentGrips ? 1 : 0);

            if (m_iconCache.contains(key)) {
                return m_iconCache.value(key);
            }

            QIcon icon;
            if (isClose) {
                icon = QIcon(new LC_VectorIconEngine(LC_VectorIconEngine::Close, m_skin.accentGrips));
            }
            else if (isFloat) {
                icon = QIcon(new LC_VectorIconEngine(LC_VectorIconEngine::Float, m_skin.accentGrips));
            }
            else if (isMin) {
                icon = QIcon(new LC_VectorIconEngine(LC_VectorIconEngine::Min, m_skin.accentGrips));
            }
            else {
                icon = QIcon(new LC_VectorIconEngine(LC_VectorIconEngine::Max, m_skin.accentGrips));
            }

            m_iconCache.insert(key, icon);
            return icon;
        }
    }
    return QIcon();
}

const QFont& LC_SkinColorsResolver::getResolvedMonoFont(const QFont& baseFont) const {
    const int targetSize = qMax(6, baseFont.pointSize() - 1);
    if (m_cachedMonoPointSize != targetSize) {
        m_cachedMonoFont = QFont("monospace", targetSize);
        m_cachedMonoPointSize = targetSize;
    }
    return m_cachedMonoFont;
}

QColor LC_SkinColorsResolver::mixColors(const QColor& c1, const QColor& c2, double factor) {
    return LC_PaletteColorUtils::interpolateColors(c1, c2, factor);
}

QPalette::ColorGroup LC_SkinColorsResolver::resolveColorGroup(QStyle::State state) {
    QPalette::ColorGroup group = (state & QStyle::State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(state & QStyle::State_Active)) {
        group = QPalette::Inactive;
    }
    return group;
}

SkinColors LC_SkinColorsResolver::resolveStyleDescriptor(StyleArchetype archetype, const QPalette& palette,
                                                         QPalette::ColorGroup group) const {
    SkinColors desc;

    QColor windowColor = palette.color(group, QPalette::Window);
    QColor buttonColor = palette.color(group, QPalette::Button);
    QColor baseColor = palette.color(group, QPalette::Base);
    QColor highlightColor = palette.color(group, QPalette::Highlight);

    bool isDark = windowColor.value() < 120;

    if (archetype == StyleArchetype::ClassicFusion) {
        desc.common.selectionHighlight = highlightColor;
    }
    else if (archetype == StyleArchetype::AccentOutline) {
        desc.common.selectionHighlight = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), isDark ? 40 : 50);
    }
    else {
        // FlatModern, SoftSatin, GlassyGloss: use a beautiful, lightened/darkened pastel tint [74]
        desc.common.selectionHighlight = mixColors(isDark ? windowColor : baseColor, highlightColor, isDark ? 0.15 : 0.09);
    }

    QColor textColor = palette.color(group, QPalette::WindowText);
    QColor midColor = palette.color(group, QPalette::Mid);

    QColor lightColor = palette.color(group, QPalette::Light);
    QColor darkColor = palette.color(group, QPalette::Dark);
    const QColor midlightColor = palette.color(group, QPalette::Midlight);

    desc.common.bgStart = windowColor;
    desc.common.textColor = textColor;

    // Define baseline borders before the BoxDecoration switch block
    QColor baseBorder = (archetype == StyleArchetype::FlatModern) ? mixColors(windowColor, textColor, 0.15) : midColor;

    desc.frame.borderButton = mixColors(windowColor, textColor, isDark ? 0.12 : 0.15);

    desc.button.glassStart = windowColor.lighter(isDark ? 125 : 110);
    desc.button.glassMidStart = windowColor.lighter(isDark ? 108 : 104);
    desc.button.glassMidEnd = windowColor.darker(isDark ? 104 : 103);
    desc.button.glassEnd = windowColor.darker(isDark ? 115 : 108);

    // ================= RESOLVE REFINED BOX DECORATIONS =================
    switch (m_skin.boxDecoration) {
        case BoxDecoration::Frameless:
            break;

        case BoxDecoration::DividingHairline:
            if (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline) {
                desc.frame.borderBottom = buttonColor;
                desc.frame.hasTopBottomBorder = true;
            }
            else {
                desc.frame.borderTop = lightColor;
                desc.frame.borderBottom = darkColor;
                desc.frame.hasTopBottomBorder = true;
            }
            break;

        case BoxDecoration::BoxOutline:
            if (archetype == StyleArchetype::AccentOutline) {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = mixColors(
                    windowColor, textColor, 0.20);
            }
            else if (archetype == StyleArchetype::FlatModern) {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = desc.frame.borderButton;
            }
            else {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = midColor;
            }
            desc.frame.hasFullBorder = true;
            break;

        case BoxDecoration::RecessedWell:
            if (archetype == StyleArchetype::FlatModern) {
                desc.common.bgStart = mixColors(windowColor, Qt::black, isDark ? 0.38 : 0.08); // matches bgTabInactive
                desc.common.bgEnd = desc.common.bgStart;
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = desc.frame.borderButton;
                desc.frame.hasFullBorder = true;
            }
            else if (archetype == StyleArchetype::AccentOutline) {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = mixColors(
                    windowColor, textColor, 0.35);
                desc.frame.hasFullBorder = true;
            }
            else {
                desc.common.bgEnd = windowColor.lighter(106);
                desc.common.useGradient = true;
                desc.frame.borderTop = lightColor;
                desc.frame.borderBottom = darkColor;
                desc.frame.hasTopBottomBorder = true;
            }
            break;

        case BoxDecoration::LeftAccentBar:
            desc.frame.hasLeftAccentBar = true;
            desc.frame.accentBarColor = highlightColor;

            if (archetype == StyleArchetype::SoftSatin || archetype == StyleArchetype::GlassyGloss) {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderRight = lightColor;
                desc.frame.hasTopBottomBorder = true;
                desc.frame.hasSideBorders = true;
                desc.frame.borderLeft = QColor();
            }
            else {
                desc.frame.borderBottom = palette.color(group, QPalette::Midlight);
                desc.frame.hasTopBottomBorder = true;
            }
            break;

        case BoxDecoration::ActiveAccentFrame:
            if (archetype == StyleArchetype::AccentOutline) {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = QColor(
                    highlightColor.red(), highlightColor.green(), highlightColor.blue(), 90);
            }
            else {
                desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = highlightColor;
            }
            desc.frame.hasFullBorder = true;
            break;
    }

    // ================= OPTIMIZED INTERACTIVE STATE RESOLUTION =================
    if (archetype == StyleArchetype::FlatModern) {
        desc.common.useGradient = false;
        desc.common.useGlassyGloss = false;

        if (desc.frame.hasFullBorder) {
            desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = baseBorder;
        }

        if (isDark) {
            desc.button.bgButton = mixColors(windowColor, Qt::white, 0.06);
            desc.button.bgButtonEnd = desc.button.bgButton;
            desc.button.bgHovered = mixColors(desc.button.bgButton, highlightColor, 0.22);
            desc.button.bgChecked = mixColors(desc.button.bgButton, highlightColor, 0.14);
            desc.button.bgSunken = mixColors(desc.button.bgButton, highlightColor, 0.35);
        }
        else {
            desc.button.bgButton = windowColor.darker(106);
            desc.button.bgButtonEnd = desc.button.bgButton;
            desc.button.bgHovered = windowColor.darker(112);
            desc.button.bgChecked = windowColor.darker(110);
            desc.button.bgSunken = windowColor.darker(118);
        }
    }
    else if (archetype == StyleArchetype::SoftSatin) {
        desc.common.useGradient = true;
        if (!desc.common.bgEnd.isValid()) {
            desc.common.bgEnd = desc.common.bgStart.darker(isDark ? 106 : 108);
        }

        if (isDark) {
            desc.button.bgButton = mixColors(windowColor, Qt::black, 0.18);
            desc.button.bgButtonEnd = mixColors(windowColor, Qt::black, 0.35);
            desc.button.bgHovered = mixColors(windowColor, Qt::white, 0.16);
            desc.button.bgSunken = mixColors(windowColor, Qt::black, 0.50);
            desc.button.bgChecked = mixColors(windowColor, highlightColor, 0.15);
        }
        else {
            desc.button.bgButton = desc.common.bgStart;
            desc.button.bgButtonEnd = desc.common.bgEnd;
            desc.button.bgHovered = desc.common.bgStart.lighter(105);
            desc.button.bgSunken = mixColors(desc.common.bgStart, baseColor, 0.20);
            desc.button.bgChecked = mixColors(desc.common.bgStart, highlightColor, 0.15);
        }
    }
    else if (archetype == StyleArchetype::GlassyGloss) {
        desc.common.useGradient = false;
        desc.common.useGlassyGloss = true;

        if (isDark) {
            desc.button.glassStart = mixColors(windowColor, Qt::white, 0.22);
            desc.button.glassMidStart = mixColors(windowColor, Qt::white, 0.10);
            desc.button.glassMidEnd = mixColors(windowColor, Qt::white, 0.02);
            desc.button.glassEnd = mixColors(windowColor, Qt::black, 0.15);

            desc.button.glassHoverStart = mixColors(desc.button.glassStart, highlightColor, 0.25);
            desc.button.glassHoverMidStart = mixColors(desc.button.glassMidStart, highlightColor, 0.20);
            desc.button.glassHoverMidEnd = mixColors(desc.button.glassMidEnd, highlightColor, 0.15);
            desc.button.glassHoverEnd = mixColors(desc.button.glassEnd, highlightColor, 0.10);

            desc.button.glassSunkenStart = mixColors(windowColor, Qt::black, 0.30);
            desc.button.glassSunkenMidStart = mixColors(windowColor, Qt::black, 0.15);
            desc.button.glassSunkenMidEnd = mixColors(windowColor, Qt::white, 0.10);
            desc.button.glassSunkenEnd = mixColors(windowColor, Qt::white, 0.20);
        }
        else {
            desc.button.glassStart = windowColor.lighter(110);
            desc.button.glassMidStart = windowColor.lighter(104);
            desc.button.glassMidEnd = windowColor.darker(103);
            desc.button.glassEnd = windowColor.darker(108);

            desc.button.glassHoverStart = windowColor.lighter(112);
            desc.button.glassHoverMidStart = windowColor.lighter(106);
            desc.button.glassHoverMidEnd = windowColor.darker(101);
            desc.button.glassHoverEnd = windowColor.darker(105);

            desc.button.glassSunkenStart = windowColor.darker(115);
            desc.button.glassSunkenMidStart = windowColor.darker(108);
            desc.button.glassSunkenMidEnd = windowColor.lighter(102);
            desc.button.glassSunkenEnd = windowColor.lighter(106);
        }

        if (m_skin.boxDecoration == BoxDecoration::RecessedWell) {
            desc.common.useGradient = true;
            desc.common.bgStart = windowColor.darker(isDark ? 110 : 108);
            desc.common.bgEnd = windowColor.darker(isDark ? 118 : 115);
        }

        desc.button.bgSunken = mixColors(desc.common.bgStart, palette.color(group, QPalette::Base), isDark ? 0.40 : 0.20);
        desc.button.bgHovered = desc.common.bgStart.lighter(105);
        desc.button.bgChecked = mixColors(desc.common.bgStart, highlightColor, 0.15);

        desc.button.bgButton = desc.common.bgStart;
        desc.button.bgButtonEnd = desc.common.bgEnd.isValid() ? desc.common.bgEnd : desc.common.bgStart;
    }
    else if (archetype == StyleArchetype::AccentOutline) {
        desc.common.useGradient = false;
        desc.common.useGlassyGloss = false;

        desc.common.bgStart = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 15);

        desc.button.bgHovered = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 65);
        desc.button.bgChecked = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 50);
        desc.button.bgSunken = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 90);

        desc.button.bgButton = desc.common.bgStart;
        desc.button.bgButtonEnd = desc.common.bgStart;

        baseBorder = mixColors(windowColor, textColor, 0.20);
        desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = baseBorder;
        desc.frame.hasFullBorder = true;
        desc.common.textColor = highlightColor;
    }
    else {
        desc.button.bgButton = desc.common.bgStart;
        desc.button.bgButtonEnd = desc.common.bgEnd.isValid() ? desc.common.bgEnd : desc.common.bgStart;
        desc.button.bgSunken = mixColors(desc.common.bgStart, baseColor, isDark ? 0.40 : 0.20);
        desc.button.bgHovered = desc.common.bgStart.lighter(105);
        desc.button.bgChecked = mixColors(desc.common.bgStart, highlightColor, 0.15);

        desc.button.glassStart = windowColor.lighter(isDark ? 125 : 110);
        desc.button.glassMidStart = windowColor.lighter(isDark ? 108 : 104);
        desc.button.glassMidEnd = windowColor.darker(isDark ? 104 : 103);
        desc.button.glassEnd = windowColor.darker(isDark ? 115 : 108);
    }

    desc.frame.borderHovered = mixColors(baseBorder, highlightColor, 0.35);

    if (m_skin.boxDecoration == BoxDecoration::RecessedWell) {
        desc.input.bgInput = mixColors(baseColor, desc.button.bgButton, isDark ? 0.30 : 0.08);
        desc.input.bgInputEnd = mixColors(baseColor, desc.button.bgButtonEnd, isDark ? 0.30 : 0.08);
        if (desc.input.bgInput == desc.input.bgInputEnd) {
            desc.input.bgInputEnd = desc.input.bgInput.darker(isDark ? 104 : 102);
        }
    }
    else {
        desc.input.bgInput = baseColor;
        // If SoftSatin archetype is selected, apply a beautifully subtle satin gradient to input backgrounds
        if (archetype == StyleArchetype::SoftSatin) {
            desc.input.bgInputEnd = baseColor.darker(isDark ? 105 : 103);
        }
        else {
            desc.input.bgInputEnd = desc.input.bgInput;
        }
    }
    desc.input.bgInputHovered = mixColors(desc.input.bgInput, highlightColor, isDark ? 0.08 : 0.04);

    desc.input.bgInputHovered = mixColors(desc.input.bgInput, highlightColor, isDark ? 0.08 : 0.04);
    if (m_skin.boxDecoration == BoxDecoration::RecessedWell) {
        desc.input.bgInputHoveredEnd = desc.input.bgInputHovered.darker(isDark ? 104 : 103);
    }
    else {
        desc.input.bgInputHoveredEnd = desc.input.bgInputHovered;
    }

    desc.input.bgInputFocused = mixColors(baseColor, highlightColor, isDark ? 0.12 : 0.06);
    desc.itemView.bgItemHovered = desc.common.selectionHighlight;

    QColor alternateBaseColor = palette.color(group, QPalette::AlternateBase);
    if (!alternateBaseColor.isValid() || alternateBaseColor == Qt::black) {
        alternateBaseColor = windowColor;
    }
    desc.itemView.bgItemAlternateHovered = mixColors(alternateBaseColor, highlightColor, isDark ? 0.14 : 0.09);

    desc.tab.bgTabInactive = mixColors(windowColor, Qt::black, isDark ? 0.38 : 0.08);
    desc.tab.bgTabInactiveDock = mixColors(windowColor, Qt::black, isDark ? 0.20 : 0.04);

    desc.tab.textTabInactive = mixColors(textColor, windowColor, 0.45);
    desc.tab.borderTabInactiveTop = isDark ? desc.tab.bgTabInactive.lighter(115) : desc.tab.bgTabInactive.darker(108);

    desc.tab.bgTabInactiveHovered = mixColors(windowColor, Qt::black, isDark ? 0.20 : 0.04);
    desc.tab.bgTabInactiveHoveredDock = mixColors(windowColor, Qt::black, isDark ? 0.10 : 0.02);

    desc.tab.borderTabInactiveHovered = mixColors(desc.frame.borderButton, highlightColor, isDark ? 0.18 : 0.25);
    desc.scrollBar.scrollBarHandleBorder = isDark ? highlightColor.lighter(115) : highlightColor.darker(115);
    desc.slider.sliderInnerDotColor = (highlightColor.value() < 120) ? Qt::white : Qt::black;

    desc.tab.tabWidgetFrameBorder = (archetype == StyleArchetype::FlatModern)
                                        ? desc.frame.borderButton
                                        : (desc.frame.borderTop.isValid() ? desc.frame.borderTop : desc.frame.borderHovered);

    desc.dockTitleBar.dockTitleBgAccentOutline = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(),
                                                        isDark ? 20 : 35);

    bool isFlat = (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline);
    if (isFlat) {
        desc.toolBar.toolbarSeparatorPen = palette.color(group, QPalette::Midlight);
    }
    else {
        desc.toolBar.toolbarSeparatorPen = palette.color(group, QPalette::Dark);
        desc.toolBar.toolbarSeparatorHighlightPen = palette.color(group, QPalette::Light);
    }

    if (desc.common.bgStart.isValid() && desc.common.textColor.isValid()) {
        desc.header.headerSeparator = mixColors(desc.common.bgStart, desc.common.textColor, isDark ? 0.18 : 0.12);
    }
    else {
        desc.header.headerSeparator = midColor;
    }

    resolveDynamicGroupBoxFallback(desc, palette, group);

    LC_PaletteColorUtils::calculateSegmentedGroupColors(group, archetype, palette, m_skin.segmentedColorPolicy, 15,
                                                        desc.segmentedGroup.groupBgStart, desc.segmentedGroup.groupBgEnd,
                                                        desc.segmentedGroup.groupGradientStops, desc.segmentedGroup.groupBgStartHovered,
                                                        desc.segmentedGroup.groupBgEndHovered,
                                                        desc.segmentedGroup.groupGradientStopsHovered);

    desc.splitter.splitterGripColor = mixColors(windowColor, textColor, isDark ? 0.18 : 0.28);
    desc.splitter.splitterGripDark = desc.splitter.splitterGripColor.darker(isDark ? 115 : 120);
    desc.splitter.splitterGripLight = desc.splitter.splitterGripColor.lighter(isDark ? 120 : 115);
    desc.splitter.splitterGripColorIdle = QColor(desc.splitter.splitterGripColor.red(), desc.splitter.splitterGripColor.green(),
                                                 desc.splitter.splitterGripColor.blue(), 90);
    desc.splitter.splitterGripWellColor = QColor(desc.splitter.splitterGripColor.red(), desc.splitter.splitterGripColor.green(),
                                                 desc.splitter.splitterGripColor.blue(), 50);
    desc.itemView.indicatorHoverWellColor = QColor(desc.splitter.splitterGripColor.red(), desc.splitter.splitterGripColor.green(),
                                                   desc.splitter.splitterGripColor.blue(), 30);
    desc.splitter.splitterHoverGlowColor = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 25);

    if (isDark) {
        desc.input.inputFocusGlowOuter = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 20);
        desc.input.inputFocusGlowInner = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 64);
    }
    else {
        desc.input.inputFocusGlowOuter = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 46);
        desc.input.inputFocusGlowInner = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), 115);
    }

    bool activeAccent = m_skin.groupBoxUseAccent || (archetype == StyleArchetype::AccentOutline);
    if (activeAccent) {
        desc.toolButton.toolButtonWellColor = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), isDark ? 40 : 50);
    }
    else {
        desc.toolButton.toolButtonWellColor = QColor(desc.splitter.splitterGripColor.red(), desc.splitter.splitterGripColor.green(),
                                                     desc.splitter.splitterGripColor.blue(), isDark ? 50 : 60);
    }

    desc.toolButton.toolButtonIndicatorColor = highlightColor;
    desc.toolButton.toolButtonIndicatorColorHover = highlightColor.lighter(115);
    desc.toolButton.toolButtonWellColorHover = mixColors(desc.toolButton.toolButtonWellColor, highlightColor, isDark ? 0.35 : 0.65);
    desc.toolButton.toolButtonBgCheckedHover = isDark ? desc.button.bgChecked.lighter(116) : desc.button.bgChecked.darker(112);

    if (m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame || archetype == StyleArchetype::AccentOutline) {
        desc.toolBar.toolbarOverflowIndicatorColor = highlightColor;
    }
    else {
        desc.toolBar.toolbarOverflowIndicatorColor = mixColors(windowColor, textColor, isDark ? 0.60 : 0.65);
    }

    desc.tab.activeTabAccentBg = mixColors(windowColor, highlightColor, 0.12);

    if (archetype == StyleArchetype::AccentOutline) {
        int alpha = 25;
        switch (m_skin.boxDecoration) {
            case BoxDecoration::Frameless:
                alpha = 15;
                break;
            case BoxDecoration::DividingHairline:
                alpha = 20;
                break;
            case BoxDecoration::BoxOutline:
            case BoxDecoration::LeftAccentBar:
                alpha = 25;
                break;
            case BoxDecoration::RecessedWell:
                alpha = 40;
                break;
            case BoxDecoration::ActiveAccentFrame:
                alpha = 45;
                break;
        }
        desc.tab.accentOutlineTabBg = QColor(highlightColor.red(), highlightColor.green(), highlightColor.blue(), alpha);
    }

    if (archetype == StyleArchetype::GlassyGloss) {
        desc.groupBox.groupBoxGlassStart = mixColors(windowColor, desc.button.glassStart, 0.35);
        desc.groupBox.groupBoxGlassMidStart = mixColors(windowColor, desc.button.glassMidStart, 0.35);
        desc.groupBox.groupBoxGlassMidEnd = mixColors(windowColor, desc.button.glassMidEnd, 0.35);
        desc.groupBox.groupBoxGlassEnd = mixColors(windowColor, desc.button.glassEnd, 0.35);
    }

    QColor hl = highlightColor;
    desc.spinBox.spinBoxProgressBarTrack = hl;
    desc.spinBox.spinBoxProgressBarTrack.setAlpha(isDark ? 15 : 6);

    int fillAlpha = isDark
                        ? ((m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame) ? 90 : 72)
                        : ((m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame) ? 42 : 32);

    desc.spinBox.spinBoxProgressBarFill = hl;
    desc.spinBox.spinBoxProgressBarFill.setAlpha(fillAlpha);

    desc.spinBox.spinBoxProgressBarSatinTop = hl;
    desc.spinBox.spinBoxProgressBarSatinTop.setAlpha(qRound(fillAlpha * 0.7));
    desc.spinBox.spinBoxProgressBarSatinBottom = hl;
    desc.spinBox.spinBoxProgressBarSatinBottom.setAlpha(qRound(fillAlpha * 1.3));

    desc.spinBox.spinBoxProgressBarGlass0 = hl;
    desc.spinBox.spinBoxProgressBarGlass0.setAlpha(qRound(fillAlpha * 1.4));
    desc.spinBox.spinBoxProgressBarGlass42 = hl;
    desc.spinBox.spinBoxProgressBarGlass42.setAlpha(qRound(fillAlpha * 1.0));
    desc.spinBox.spinBoxProgressBarGlass55 = hl;
    desc.spinBox.spinBoxProgressBarGlass55.setAlpha(qRound(fillAlpha * 0.6));
    desc.spinBox.spinBoxProgressBarGlass100 = hl;
    desc.spinBox.spinBoxProgressBarGlass100.setAlpha(qRound(fillAlpha * 1.2));

    int fillAlphaFocused = isDark ? 128 : 50;

    desc.spinBox.spinBoxProgressBarFocusedFill = hl;
    desc.spinBox.spinBoxProgressBarFocusedFill.setAlpha(fillAlphaFocused);

    desc.spinBox.spinBoxProgressBarSatinFocusedTop = hl;
    desc.spinBox.spinBoxProgressBarSatinFocusedTop.setAlpha(qRound(fillAlphaFocused * 0.7));
    desc.spinBox.spinBoxProgressBarSatinFocusedBottom = hl;
    desc.spinBox.spinBoxProgressBarSatinFocusedBottom.setAlpha(qRound(fillAlphaFocused * 1.3));

    desc.spinBox.spinBoxProgressBarGlassFocused0 = hl;
    desc.spinBox.spinBoxProgressBarGlassFocused0.setAlpha(qRound(fillAlphaFocused * 1.4));
    desc.spinBox.spinBoxProgressBarGlassFocused42 = hl;
    desc.spinBox.spinBoxProgressBarGlassFocused42.setAlpha(qRound(fillAlphaFocused * 1.0));
    desc.spinBox.spinBoxProgressBarGlassFocused55 = hl;
    desc.spinBox.spinBoxProgressBarGlassFocused55.setAlpha(qRound(fillAlphaFocused * 0.6));
    desc.spinBox.spinBoxProgressBarGlassFocused100 = hl;
    desc.spinBox.spinBoxProgressBarGlassFocused100.setAlpha(qRound(fillAlphaFocused * 1.2));

    desc.spinBox.spinBoxProgressBarTrackFocused = hl;
    desc.spinBox.spinBoxProgressBarTrackFocused.setAlpha(45);

    desc.slider.sliderHandleColor = highlightColor;
    desc.slider.sliderHandleColorHovered = highlightColor.lighter(115);

    QColor baseScrollTrack = palette.color(group, QPalette::Base);
    QColor scrollHandle = m_skin.accentedScrollbars ? highlightColor : buttonColor;
    QColor scrollBorder = m_skin.accentedScrollbars ? desc.scrollBar.scrollBarHandleBorder : desc.frame.borderButton;

    if (m_skin.transparentScrollbars) {
        baseScrollTrack.setAlpha(12);
        desc.scrollBar.scrollBarTrackColor = baseScrollTrack;

        desc.scrollBar.scrollBarHandleColor = scrollHandle;
        desc.scrollBar.scrollBarHandleColor.setAlpha(80);
        desc.scrollBar.scrollBarHandleColorHovered = scrollHandle;

        desc.scrollBar.scrollBarBorderColor = scrollBorder;
        desc.scrollBar.scrollBarBorderColor.setAlpha(90);
        desc.scrollBar.scrollBarBorderColorHovered = scrollBorder;
    }
    else {
        desc.scrollBar.scrollBarTrackColor = baseScrollTrack;
        desc.scrollBar.scrollBarHandleColor = scrollHandle;
        desc.scrollBar.scrollBarHandleColorHovered = scrollHandle;
        desc.scrollBar.scrollBarBorderColor = scrollBorder;
        desc.scrollBar.scrollBarBorderColorHovered = scrollBorder;
    }

    desc.header.headerBorderBottom = windowColor.darker(115);

    QColor tooltipBaseColor = palette.color(group, QPalette::ToolTipBase);
    tooltipBaseColor.setAlpha(isDark ? 235 : 240);
    desc.tooltip.tooltipBg = tooltipBaseColor;

    QColor tooltipBorderColor = palette.color(group, QPalette::ToolTipText);
    tooltipBorderColor.setAlpha(38);
    desc.tooltip.tooltipBorder = tooltipBorderColor;

    desc.itemView.branchIndicatorHoverColor = highlightColor.lighter(115);

    QColor textCol = desc.groupBox.groupBoxTextColor;
    QColor selectedTextCol = palette.color(group, QPalette::HighlightedText);

    // ================= NEW SEMANTIC PRECOMPUTATIONS =================

    // CheckBox and RadioButton Indicator precomputations
    desc.checkbox.bg = baseColor;
    desc.checkbox.border = midColor;
    desc.checkbox.checkMark = highlightColor;

    desc.radioButton.bg = baseColor;
    desc.radioButton.border = midColor;
    desc.radioButton.dot = highlightColor;

    // Standard arrow icon precomputations
    desc.arrow.arrowColor = palette.color(group, QPalette::ButtonText);
    desc.arrow.arrowColorHovered = highlightColor;

    desc.arrow.arrowColor = palette.color(group, QPalette::ButtonText);
    desc.arrow.arrowColorHovered = highlightColor;
    desc.arrow.menuArrowColorSelected = palette.color(group, QPalette::HighlightedText);

    desc.itemView.menuAliasColorNormal = textCol;
    desc.itemView.menuAliasColorNormal.setAlpha(128);

    desc.itemView.menuAliasColorSelected = selectedTextCol;
    desc.itemView.menuAliasColorSelected.setAlpha(180);

    if (m_skin.showActiveRowSpotlight) {
        desc.itemView.branchIndicatorSelectedColor = highlightColor;
        desc.itemView.branchLineSelectedColor = desc.groupBox.groupBoxFrameColor;
    }
    else {
        desc.itemView.branchIndicatorSelectedColor = palette.color(group, QPalette::HighlightedText);
        desc.itemView.branchLineSelectedColor = palette.color(group, QPalette::HighlightedText);
    }

    // Slider groove precomputations
    desc.slider.grooveBg = baseColor;
    desc.slider.grooveBorder = midColor;
    desc.slider.handleBorder = midColor;

    // ProgressBar precomputations
    desc.progressBar.trackColor = baseColor;
    desc.progressBar.fillColor = highlightColor;
    desc.progressBar.borderColor = midColor;

    // ToolBar boundary and 3D border precomputations
    desc.toolBar.bg = windowColor;
    desc.toolBar.flatBorder = midlightColor;
    desc.toolBar.borderLight = lightColor;
    desc.toolBar.borderDark = darkColor;

    // MenuBar empty space boundary precomputations
    desc.menuBar.bg = windowColor;
    desc.menuBar.borderBottom = darkColor;

    // DockWidget titlebar border and line precomputations
    desc.dockTitleBar.borderTop = lightColor;
    desc.dockTitleBar.borderBottom = midColor;
    desc.dockTitleBar.accentBarColor = highlightColor;
    desc.dockTitleBar.highlightBorder = lightColor;
    desc.dockTitleBar.midlightBorder = midlightColor;
    desc.dockTitleBar.titleBarButtonStrokeIdle = mixColors(desc.common.bgStart, desc.common.textColor, 0.5);
    desc.dockTitleBar.titleBarButtonStrokeIdleCad = mixColors(desc.common.bgStart, desc.common.textColor, 0.15);

    // Tab accessory precomputations (tear guidelines and hover close states)
    desc.tab.closeButtonHoverColor = highlightColor;
    desc.tab.tearIndicatorColor = palette.color(group, QPalette::ButtonText);
    desc.tab.tearIndicatorColorHover = highlightColor;

    return desc;
}

void LC_SkinColorsResolver::resolveDynamicGroupBoxFallback(SkinColors& desc, const QPalette& palette, QPalette::ColorGroup group) const {
    QColor highlightColor = palette.color(group, QPalette::Highlight);
    QColor windowColor = palette.color(group, QPalette::Window);
    QColor buttonColor = palette.color(group, QPalette::Button);

    GroupBoxHeaderStyle activeHeader = GroupBoxHeaderStyle::Overlapping;
    GroupBoxBoundaryStyle activeBoundary = GroupBoxBoundaryStyle::Full;
    bool activeAccent = false;

    if (m_skin.customGroupBoxBar) {
        activeHeader = m_skin.groupBoxHeaderStyle;
        activeBoundary = m_skin.groupBoxBoundaryStyle;
        activeAccent = m_skin.groupBoxUseAccent;
    }
    else if (!m_isClassic) {
        switch (m_skin.boxDecoration) {
            case BoxDecoration::Frameless:
                activeBoundary = GroupBoxBoundaryStyle::None;
                activeHeader = GroupBoxHeaderStyle::Plain;
                break;

            case BoxDecoration::DividingHairline:
                activeBoundary = GroupBoxBoundaryStyle::TopLine;
                activeHeader = GroupBoxHeaderStyle::BreakBorder;
                break;

            case BoxDecoration::BoxOutline:
            case BoxDecoration::RecessedWell:
                activeBoundary = GroupBoxBoundaryStyle::Full;
                activeHeader = GroupBoxHeaderStyle::BreakBorder;
                break;

            case BoxDecoration::LeftAccentBar:
                activeBoundary = GroupBoxBoundaryStyle::LeftStripe;
                activeHeader = GroupBoxHeaderStyle::Plain;
                break;

            case BoxDecoration::ActiveAccentFrame:
                activeBoundary = GroupBoxBoundaryStyle::Full;
                activeHeader = GroupBoxHeaderStyle::BreakBorder;
                activeAccent = true;
                break;
        }

        if (m_isAccentOnline) {
            activeAccent = true;
        }
    }

    if (activeBoundary == GroupBoxBoundaryStyle::None || activeBoundary == GroupBoxBoundaryStyle::LeftStripe) {
        if (activeHeader == GroupBoxHeaderStyle::Overlapping || activeHeader == GroupBoxHeaderStyle::BreakBorder) {
            activeHeader = GroupBoxHeaderStyle::BreakBorder;
            activeBoundary = GroupBoxBoundaryStyle::TopLine;
        }
    }

    desc.groupBox.groupBoxHeaderStyle = activeHeader;
    desc.groupBox.groupBoxBoundaryStyle = activeBoundary;
    desc.common.highlightColor = highlightColor;

    if (activeAccent) {
        desc.groupBox.groupBoxFrameColor = highlightColor;
    }
    else {
        desc.groupBox.groupBoxFrameColor = palette.color(group, QPalette::Dark);
    }

    QColor bannerBaseColor = desc.button.bgButton;
    if (m_isSoftSatin || m_isGlossy) {
        bannerBaseColor = mixColors(windowColor, buttonColor, 0.35);
    }

    if (activeAccent) {
        desc.groupBox.groupBoxBannerFillColor = mixColors(bannerBaseColor, highlightColor, 0.12);
    }
    else {
        desc.groupBox.groupBoxBannerFillColor = bannerBaseColor;
    }

    if (m_isGlossy) {
        desc.groupBox.groupBoxGlassStart = mixColors(windowColor, desc.button.glassStart, 0.35);
        desc.groupBox.groupBoxGlassMidStart = mixColors(windowColor, desc.button.glassMidStart, 0.35);
        desc.groupBox.groupBoxGlassMidEnd = mixColors(windowColor, desc.button.glassMidEnd, 0.35);
        desc.groupBox.groupBoxGlassEnd = mixColors(windowColor, desc.button.glassEnd, 0.35);
    }

    QColor groupTextColor = palette.color(group, QPalette::WindowText);
    if (activeHeader == GroupBoxHeaderStyle::Plain && (activeBoundary == GroupBoxBoundaryStyle::LeftStripe || activeBoundary ==
        GroupBoxBoundaryStyle::None) && activeAccent) {
        groupTextColor = highlightColor;
    }
    desc.groupBox.groupBoxTextColor = groupTextColor;
}

void LC_SkinColorsResolver::precomputeSegmentedGroupColors(QWidget* widget, int totalGroups) const {
    if (!widget)
        return;
    const QPalette::ColorGroup group = widget->isEnabled() ? QPalette::Active : QPalette::Disabled;

    QMap<int, QColor> groupBgStart;
    QMap<int, QColor> groupBgEnd;
    QMap<int, LC_PaletteColorUtils::GroupGradientStops> groupGradientStops;
    QMap<int, QColor> groupBgStartHovered;
    QMap<int, QColor> groupBgEndHovered;
    QMap<int, LC_PaletteColorUtils::GroupGradientStops> groupGradientStopsHovered;

    LC_PaletteColorUtils::calculateSegmentedGroupColors(group, m_skin.styleArchetype, widget->palette(), m_skin.segmentedColorPolicy,
                                                        totalGroups, groupBgStart, groupBgEnd, groupGradientStops, groupBgStartHovered,
                                                        groupBgEndHovered, groupGradientStopsHovered);

    QVariantMap bgStartMap, bgEndMap, stop0Map, stop42Map, stop55Map, stop100Map;
    QVariantMap bgStartHoveredMap, bgEndHoveredMap, stop0HoveredMap, stop42HoveredMap, stop55HoveredMap, stop100HoveredMap;

    for (auto it = groupBgStart.begin(); it != groupBgStart.end(); ++it) {
        QString key = QString::number(it.key());
        bgStartMap[key] = it.value();
        bgEndMap[key] = groupBgEnd.value(it.key());

        const LC_PaletteColorUtils::GroupGradientStops s = groupGradientStops.value(it.key());
        stop0Map[key] = s.stop0;
        stop42Map[key] = s.stop42;
        stop55Map[key] = s.stop55;
        stop100Map[key] = s.stop100;

        bgStartHoveredMap[key] = groupBgStartHovered.value(it.key());
        bgEndHoveredMap[key] = groupBgEndHovered.value(it.key());

        const LC_PaletteColorUtils::GroupGradientStops sh = groupGradientStopsHovered.value(it.key());
        stop0HoveredMap[key] = sh.stop0;
        stop42HoveredMap[key] = sh.stop42;
        stop55HoveredMap[key] = sh.stop55;
        stop100HoveredMap[key] = sh.stop100;
    }

    widget->setProperty(PROP_GROUP_BG_START, bgStartMap);
    widget->setProperty(PROP_GROUP_BG_END, bgEndMap);
    widget->setProperty(PROP_GROUP_STOP0, stop0Map);
    widget->setProperty(PROP_GROUP_STOP42, stop42Map);
    widget->setProperty(PROP_GROUP_STOP55, stop55Map);
    widget->setProperty(PROP_GROUP_STOP100, stop100Map);

    widget->setProperty(PROP_GROUP_BG_START_HOVERED, bgStartHoveredMap);
    widget->setProperty(PROP_GROUP_BG_END_HOVERED, bgEndHoveredMap);
    widget->setProperty(PROP_GROUP_STOP0_HOVERED, stop0HoveredMap);
    widget->setProperty(PROP_GROUP_STOP42_HOVERED, stop42HoveredMap);
    widget->setProperty(PROP_GROUP_STOP55_HOVERED, stop55HoveredMap);
    widget->setProperty(PROP_GROUP_STOP100_HOVERED, stop100HoveredMap);

    // Cache the window color used to prevent redundant updates during paint passes
    widget->setProperty(PROP_GROUP_PALETTE_COLOR, widget->palette().color(QPalette::Window));
}

SkinColors LC_SkinColorsResolver::resolveStandardButtonDescriptor(const QStyleOptionButton* option, const QWidget* widget) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);

    SkinColors buttonDesc = getDescriptor(option->palette, finalGroup);

    const bool sunken = (option->state & QStyle::State_Sunken);
    const bool checked = (option->state & QStyle::State_On);
    const bool hovered = (option->state & QStyle::State_MouseOver);

    if (sunken || checked) {
        if (checked && !sunken) {
            buttonDesc.common.bgStart = buttonDesc.button.bgChecked;
            buttonDesc.common.bgEnd = buttonDesc.button.bgChecked;

            if (!m_isFlatModern) {
                buttonDesc.frame.borderBottom = option->palette.color(finalGroup, QPalette::Mid);
                buttonDesc.frame.hasTopBottomBorder = true;
            }

            if (buttonDesc.common.useGlassyGloss) {
                buttonDesc.button.glassStart = buttonDesc.button.glassSunkenStart;
                buttonDesc.button.glassMidStart = buttonDesc.button.glassSunkenMidStart;
                buttonDesc.button.glassMidEnd = buttonDesc.button.glassSunkenMidEnd;
                buttonDesc.button.glassEnd = buttonDesc.button.glassSunkenEnd;
            }
        }
        else {
            // sunken (actively pressed)
            buttonDesc.common.bgStart = buttonDesc.button.bgSunken;
            buttonDesc.common.bgEnd = buttonDesc.button.bgSunken;

            if (buttonDesc.common.useGlassyGloss) {
                buttonDesc.button.glassStart = buttonDesc.button.glassSunkenStart;
                buttonDesc.button.glassMidStart = buttonDesc.button.glassSunkenMidStart;
                buttonDesc.button.glassMidEnd = buttonDesc.button.glassSunkenMidEnd;
                buttonDesc.button.glassEnd = buttonDesc.button.glassSunkenEnd;
            }
        }
    }
    else if (hovered) {
        buttonDesc.common.bgStart = buttonDesc.button.bgHovered;
        buttonDesc.common.bgEnd = buttonDesc.button.bgHovered;

        if (buttonDesc.common.useGlassyGloss) {
            buttonDesc.button.glassStart = buttonDesc.button.glassHoverStart;
            buttonDesc.button.glassMidStart = buttonDesc.button.glassHoverMidStart;
            buttonDesc.button.glassMidEnd = buttonDesc.button.glassHoverMidEnd;
            buttonDesc.button.glassEnd = buttonDesc.button.glassHoverEnd;
        }
    }
    else if (m_isFlatModern) {
        buttonDesc.common.bgStart = buttonDesc.button.bgButton;
        buttonDesc.common.bgEnd = buttonDesc.button.bgButtonEnd;

        bool needsBorder = true;
        if (widget && widget->parentWidget()) {
            const QColor parentBg = widget->parentWidget()->palette().color(widget->parentWidget()->backgroundRole());
            const int delta = qAbs(buttonDesc.common.bgStart.value() - parentBg.value());
            if (delta > 12) {
                needsBorder = false;
            }
        }

        if (needsBorder) {
            buttonDesc.frame.borderTop = buttonDesc.frame.borderBottom = buttonDesc.frame.borderLeft = buttonDesc.frame.borderRight =
                buttonDesc.frame.borderButton;
            buttonDesc.frame.hasFullBorder = true;
        }
    }
    else {
        buttonDesc.common.bgStart = buttonDesc.button.bgButton;
        buttonDesc.common.bgEnd = buttonDesc.button.bgButtonEnd;
    }

    if (!m_isFlatModern) {
        buttonDesc.frame.hasFullBorder = true;
        buttonDesc.frame.hasTopBottomBorder = false;
        buttonDesc.frame.hasSideBorders = false;
        buttonDesc.frame.hasLeftAccentBar = false;

        if (m_skin.styleArchetype != StyleArchetype::AccentOutline) {
            buttonDesc.frame.borderTop = buttonDesc.frame.borderBottom = buttonDesc.frame.borderLeft = buttonDesc.frame.borderRight =
                buttonDesc.frame.borderButton;
        }
    }

    return buttonDesc;
}

SkinColors LC_SkinColorsResolver::resolveToolButtonDescriptor(const QStyleOption* option, const QWidget* widget) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);

    SkinColors buttonDesc = getDescriptor(option->palette, finalGroup);

    const bool sunken = (option->state & QStyle::State_Sunken);
    const bool checked = (option->state & QStyle::State_On);
    const bool hovered = (option->state & QStyle::State_MouseOver);
    const bool autoRaise = (option->state & QStyle::State_AutoRaise);

    if (sunken || checked) {
        if (checked && !sunken) {
            buttonDesc.common.bgStart = buttonDesc.button.bgChecked;
            buttonDesc.common.bgEnd = buttonDesc.button.bgChecked;

            if (buttonDesc.common.useGlassyGloss) {
                buttonDesc.button.glassStart = buttonDesc.button.glassSunkenStart;
                buttonDesc.button.glassMidStart = buttonDesc.button.glassSunkenMidStart;
                buttonDesc.button.glassMidEnd = buttonDesc.button.glassSunkenMidEnd;
                buttonDesc.button.glassEnd = buttonDesc.button.glassSunkenEnd;
            }
        }
        else {
            // sunken
            buttonDesc.common.bgStart = buttonDesc.button.bgSunken;
            buttonDesc.common.bgEnd = buttonDesc.button.bgSunken;

            if (buttonDesc.common.useGlassyGloss) {
                buttonDesc.button.glassStart = buttonDesc.button.glassSunkenStart;
                buttonDesc.button.glassMidStart = buttonDesc.button.glassSunkenMidStart;
                buttonDesc.button.glassMidEnd = buttonDesc.button.glassSunkenMidEnd;
                buttonDesc.button.glassEnd = buttonDesc.button.glassSunkenEnd;
            }
        }
    }
    else if (hovered) {
        buttonDesc.common.bgStart = buttonDesc.button.bgHovered;
        buttonDesc.common.bgEnd = buttonDesc.button.bgHovered;

        if (buttonDesc.common.useGlassyGloss) {
            buttonDesc.button.glassStart = buttonDesc.button.glassHoverStart;
            buttonDesc.button.glassMidStart = buttonDesc.button.glassHoverMidStart;
            buttonDesc.button.glassMidEnd = buttonDesc.button.glassHoverMidEnd;
            buttonDesc.button.glassEnd = buttonDesc.button.glassHoverEnd;
        }
    } else if (autoRaise) {
        // Flat autoRaise button when idle: transparent background and no borders
        buttonDesc.common.bgStart = Qt::transparent;
        buttonDesc.common.bgEnd = Qt::transparent;
        buttonDesc.common.useGradient = false;
        buttonDesc.common.useGlassyGloss = false;
        buttonDesc.frame.hasFullBorder = false;
        buttonDesc.frame.hasTopBottomBorder = false;
        buttonDesc.frame.hasSideBorders = false;
        buttonDesc.frame.hasLeftAccentBar = false;
    } else {
        buttonDesc.common.bgStart = buttonDesc.button.bgButton;
        buttonDesc.common.bgEnd = buttonDesc.button.bgButtonEnd;
    }

    if (isTitleOrDockButton(widget)) {
        buttonDesc.frame.hasFullBorder = false;
        buttonDesc.frame.hasTopBottomBorder = false;
        buttonDesc.frame.hasSideBorders = false;
        buttonDesc.frame.hasLeftAccentBar = false;
    }

    return buttonDesc;
}

SkinColors LC_SkinColorsResolver::resolveTabBorders(const QStyleOptionTab* option, const QWidget* widget) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);

    const SkinColors desc = getDescriptor(option->palette, finalGroup);
    SkinColors tabDesc = desc;

    const bool selected = (option->state & QStyle::State_Selected);
    const bool hovered = (option->state & QStyle::State_MouseOver);

    if (!selected) {
        tabDesc.common.bgStart = hovered ? desc.tab.bgTabInactiveHovered : desc.tab.bgTabInactive;
        tabDesc.common.bgEnd = tabDesc.common.bgStart;
        tabDesc.frame.hasLeftAccentBar = false;

        if (!m_isClassic && m_skin.boxDecoration == BoxDecoration::DividingHairline) {
            tabDesc.frame.borderTop = desc.tab.borderTabInactiveTop;
        }
    }
    else {
        if (m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame) {
            tabDesc.common.bgStart = desc.tab.activeTabAccentBg;
        }
        else if (m_isAccentOnline) {
            tabDesc.common.bgStart = desc.tab.accentOutlineTabBg;
        }
        else {
            tabDesc.common.bgStart = option->palette.color(finalGroup, QPalette::Window);
        }
        tabDesc.common.bgEnd = tabDesc.common.bgStart;
    }

    QColor flatBorder = tabDesc.frame.borderButton;
    if (m_isFlatModern && hovered && !selected) {
        flatBorder = tabDesc.tab.borderTabInactiveHovered;
    }

    if (m_isFlatModern) {
        tabDesc.frame.hasFullBorder = false;
        tabDesc.frame.hasTopBottomBorder = false;
        tabDesc.frame.hasSideBorders = false;
        tabDesc.frame.hasLeftAccentBar = false;

        if (selected) {
            if (m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame) {
                tabDesc.frame.borderTop = tabDesc.frame.borderBottom = tabDesc.frame.borderLeft = tabDesc.frame.borderRight = option->
                    palette.color(finalGroup, QPalette::Highlight);
                tabDesc.frame.hasFullBorder = true;
            }
            else {
                if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth || option->shape ==
                    QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth) {
                    tabDesc.frame.borderLeft = tabDesc.frame.borderRight = flatBorder;
                    tabDesc.frame.hasSideBorders = true;
                }
                else {
                    tabDesc.frame.borderTop = tabDesc.frame.borderBottom = flatBorder;
                    tabDesc.frame.hasTopBottomBorder = true;
                }
            }
        }
        else {
            if (m_skin.boxDecoration != BoxDecoration::Frameless) {
                if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
                    tabDesc.frame.borderBottom = flatBorder;
                    tabDesc.frame.hasTopBottomBorder = true;
                }
                else if (option->shape == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth) {
                    tabDesc.frame.borderTop = flatBorder;
                    tabDesc.frame.hasTopBottomBorder = true;
                }
                else if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
                    tabDesc.frame.borderRight = flatBorder;
                    tabDesc.frame.hasSideBorders = true;
                }
                else if (option->shape == QTabBar::RoundedEast || option->shape == QTabBar::TriangularEast) {
                    tabDesc.frame.borderLeft = flatBorder;
                    tabDesc.frame.hasSideBorders = true;
                }
            }
        }
    }
    else {
        if (selected) {
            if (m_skin.boxDecoration == BoxDecoration::LeftAccentBar) {
                tabDesc.frame.hasLeftAccentBar = true;
                tabDesc.frame.accentBarColor = option->palette.color(finalGroup, QPalette::Highlight);
            }
            else if (m_skin.boxDecoration == BoxDecoration::ActiveAccentFrame) {
                tabDesc.frame.borderTop = tabDesc.frame.borderBottom = tabDesc.frame.borderLeft = tabDesc.frame.borderRight = option->
                    palette.color(finalGroup, QPalette::Highlight);
                tabDesc.frame.hasFullBorder = true;
            }
        }
    }

    if (selected && m_isAccentOnline) {
        if (m_skin.boxDecoration != BoxDecoration::Frameless && m_skin.boxDecoration != BoxDecoration::DividingHairline) {
            tabDesc.frame.borderTop = tabDesc.frame.borderBottom = tabDesc.frame.borderLeft = tabDesc.frame.borderRight = option->palette.
                color(finalGroup, QPalette::Highlight);
            tabDesc.frame.hasFullBorder = true;
        }
        else if (m_skin.boxDecoration == BoxDecoration::DividingHairline) {
            tabDesc.frame.borderTop = option->palette.color(finalGroup, QPalette::Highlight);
            tabDesc.frame.hasTopBottomBorder = true;
        }
    }

    const bool isDockTab = option->rect.isValid() && widget && widget->inherits("QDockAreaTabBar");
    if (isDockTab) {
        tabDesc.frame.hasFullBorder = false;
        tabDesc.frame.hasTopBottomBorder = false;
        tabDesc.frame.hasLeftAccentBar = false;

        const bool isLeftmost = (option->position == QStyleOptionTab::Beginning || option->position == QStyleOptionTab::OnlyOneTab);
        if (isLeftmost && !m_isFlatModern) {
            tabDesc.frame.borderLeft = option->palette.color(finalGroup, QPalette::Mid);
            tabDesc.frame.hasSideBorders = true;
        }
        else {
            tabDesc.frame.hasSideBorders = false;
        }

        if (!selected) {
            tabDesc.common.bgStart = hovered ? desc.tab.bgTabInactiveHoveredDock : desc.tab.bgTabInactiveDock;
            tabDesc.common.bgEnd = tabDesc.common.bgStart;
        }
    }
    return tabDesc;
}

SkinColors LC_SkinColorsResolver::resolveHeaderSectionDescriptor(const QStyleOptionHeader* option) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);
    SkinColors desc = getDescriptor(option->palette, finalGroup);

    desc.frame.hasLeftAccentBar = false;
    desc.frame.hasFullBorder = false;
    desc.frame.hasSideBorders = false;
    desc.frame.hasTopBottomBorder = true;

    if (m_isFlat) {
        desc.frame.borderBottom = option->palette.color(finalGroup, QPalette::Button);
        desc.frame.borderTop = QColor();
    }
    else {
        desc.frame.borderTop = option->palette.color(finalGroup, QPalette::Light);
        desc.frame.borderBottom = desc.header.headerBorderBottom;
    }

    return desc;
}

SkinColors LC_SkinColorsResolver::resolveLineEditDescriptor(const QStyleOption* option, const QWidget* widget) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);
    SkinColors desc = getDescriptor(option->palette, finalGroup);

    // Allow input fields to inherit useGradient and useGlassyGloss directly from the style archetype,
    // but force useGradient if the RecessedWell box decoration is selected.
    if (m_skin.boxDecoration == BoxDecoration::RecessedWell) {
        desc.common.useGradient = true;
        desc.common.useGlassyGloss = false;
    }

    // Ensure LineEdits and SpinBoxes always maintain full, closed borders (matching ComboBoxes)
    if (!desc.frame.hasFullBorder) {
        desc.frame.hasFullBorder = true;
        desc.frame.hasTopBottomBorder = false;
        desc.frame.hasSideBorders = false;
        desc.frame.hasLeftAccentBar = false;
        desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = desc.frame.borderButton;
    }

    bool hasFocus = (option->state & QStyle::State_HasFocus);
    if (widget && widget->isEnabled()) {
        if (!hasFocus) {
            const QWidget* activeFocusWidget = widget->window() ? widget->window()->focusWidget() : nullptr;
            if (activeFocusWidget && (activeFocusWidget == widget || widget->isAncestorOf(activeFocusWidget))) {
                hasFocus = true;
            }
        }
    }

    bool isHovered = (option->state & QStyle::State_MouseOver);
    if (widget && widget->isEnabled()) {
        isHovered = false;
        const QPoint globalCursorPos = QCursor::pos();
        const QWidget* currWidget = widget;

        while (currWidget) {
            if (!currWidget->property(PROP_IS_INPUT_COMPONENT).toBool()) {
                break;
            }

            const QPoint localPos = currWidget->mapFromGlobal(globalCursorPos);
            if (currWidget->rect().contains(localPos)) {
                isHovered = true;
                break;
            }
            currWidget = currWidget->parentWidget();
        }
    }

    if (hasFocus) {
        desc.common.bgStart = desc.input.bgInputFocused;
        desc.common.bgEnd = desc.input.bgInputFocused;
    }
    else if (isHovered && (option->state & QStyle::State_Enabled)) {
        desc.common.bgStart = desc.input.bgInputHovered;
        desc.common.bgEnd = desc.input.bgInputHoveredEnd;
    }
    else {
        desc.common.bgStart = desc.input.bgInput;
        desc.common.bgEnd = desc.input.bgInputEnd;
    }

    return desc;
}

SkinColors LC_SkinColorsResolver::resolveDockTitleBarDescriptor(const QStyleOptionDockWidget* option, const QWidget* widget) const {
    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);
    SkinColors desc = getDescriptor(option->palette, finalGroup);

    if (m_isClassic && m_skin.customDockTitleBar) {
        desc.common.bgStart = desc.tab.bgTabInactive;
        desc.common.bgEnd = desc.tab.bgTabInactive;
        desc.frame.borderTop = desc.dockTitleBar.borderTop;
        desc.frame.borderBottom = desc.dockTitleBar.borderBottom;
        desc.frame.hasTopBottomBorder = true;
    }

    if (m_activeDockTitleStyle != DockTitleBarStyle::Native) {
        if (m_isClassic) {
            desc.frame.hasFullBorder = false;
            desc.frame.hasLeftAccentBar = false;
            desc.frame.hasSideBorders = false;
            desc.frame.hasTopBottomBorder = true;
            desc.frame.borderTop = desc.dockTitleBar.borderTop;
            desc.frame.borderBottom = desc.dockTitleBar.borderBottom;
        }

        if (m_activeDockTitleStyle == DockTitleBarStyle::CustomSolid) {
            desc.common.bgStart = desc.button.bgButton;
            desc.common.bgEnd = desc.button.bgButtonEnd;
            desc.common.useGradient = false;
            desc.common.useGlassyGloss = false;
        }
        else if (m_activeDockTitleStyle == DockTitleBarStyle::CustomSatin) {
            desc.common.bgStart = desc.button.bgButton;
            desc.common.bgEnd = desc.button.bgButtonEnd;
            desc.common.useGradient = true;
            desc.common.useGlassyGloss = false;
        }
        else if (m_activeDockTitleStyle == DockTitleBarStyle::CustomGlassy) {
            desc.common.useGradient = false;
            desc.common.useGlassyGloss = true;
        }
        else if (m_activeDockTitleStyle == DockTitleBarStyle::CustomAccentOutline) {
            desc.common.bgStart = desc.dockTitleBar.dockTitleBgAccentOutline;
            desc.common.bgEnd = desc.common.bgStart;
            desc.common.useGradient = false;
            desc.common.useGlassyGloss = false;
        }
        else if (m_activeDockTitleStyle == DockTitleBarStyle::CustomAccentLine) {
            desc.common.bgEnd = desc.common.bgStart;
            desc.common.useGradient = false;
            desc.common.useGlassyGloss = false;
            desc.frame.hasLeftAccentBar = true;
            desc.frame.accentBarColor = desc.dockTitleBar.accentBarColor;

            desc.frame.borderBottom = desc.dockTitleBar.midlightBorder;
            desc.frame.hasTopBottomBorder = true;
        }
    }

        // Safe parenting check: Resolve the parent QDockWidget safely to prevent header circularity
        const QDockWidget *dock = nullptr;
        if (widget) {
            if (widget->inherits("LC_CustomTitleBarWidget") || widget->inherits("LC_DockTitleBar")) {
                dock = qobject_cast<const QDockWidget*>(widget->parent());
            } else {
                dock = qobject_cast<const QDockWidget*>(widget);
            }
        }

        bool isFocused = false;
        if (dock) {
            QWidget *fw = QApplication::focusWidget();
            // A dock panel is focused if its containing window is active AND either the dock or its children holds focus
            isFocused = dock->isActiveWindow() && (dock->hasFocus() || (fw && (dock == fw || dock->isAncestorOf(fw))));
        } else {
            isFocused = (option->state & QStyle::State_Active);
        }

        // If the dock panel has active keyboard/input focus, dynamically apply a soft accent tint background
        if (isFocused) {
        const bool isDark = desc.common.bgStart.value() < 120;
        const qreal blendFactor = isDark ? 0.18 : 0.12;

        // Blend both bgStart and bgEnd to preserve underlying solid or gradient characteristics
        desc.common.bgStart = mixColors(desc.common.bgStart, desc.common.highlightColor, blendFactor);
        desc.common.bgEnd = mixColors(desc.common.bgEnd.isValid() ? desc.common.bgEnd : desc.common.bgStart, desc.common.highlightColor,
                                      blendFactor);
    }

    return desc;
}

void LC_SkinColorsResolver::drawCloseIcon(QPainter* painter, const QRectF& rect, const QColor& color, qreal penWidth) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);

    const QPointF cx = rect.center();
    const qreal hSize = qMax(3.0, rect.width() * 80.0 / 200.0);

    painter->drawLine(QPointF(cx.x() - hSize, cx.y() - hSize), QPointF(cx.x() + hSize, cx.y() + hSize));
    painter->drawLine(QPointF(cx.x() - hSize, cx.y() + hSize), QPointF(cx.x() + hSize, cx.y() - hSize));
    painter->restore();
}

void LC_SkinColorsResolver::drawFloatIcon(QPainter* painter, const QRectF& rect, const QColor& color, qreal penWidth,
                                          const QColor& fillColor) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(color, penWidth * 0.85, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const QPointF cx = rect.center();
    const qreal size = qMin(rect.width(), rect.height()) * 0.55;
    const QRectF box1(cx.x() - size + 1.0, cx.y() - size, size, size);
    const QRectF box2(cx.x() - 1.0, cx.y() - 1.0, size, size);

    painter->drawRect(box1);
    if (fillColor != Qt::transparent) {
        painter->fillRect(box2.adjusted(0.5, 0.5, -0.5, -0.5), fillColor);
    }
    painter->drawRect(box2);
    painter->restore();
}
