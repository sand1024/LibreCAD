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

#include "lc_skin_scaled_geometry_provider.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>

void LC_SkinScaledGeometryProvider::setBaseMetrics(const StyleMetricsConfig& metrics) {
    m_baseMetrics = metrics;
    invalidate();
}

void LC_SkinScaledGeometryProvider::invalidate() const {
    m_dpiCache.clear();
    m_lastDpr = 0.0;
    m_lastGeometries = SkinScaledGeometries();
}

const SkinScaledGeometries& LC_SkinScaledGeometryProvider::getGeometries(const QWidget* widget) const {
    qreal dpr = 1.0;
    if (widget) {
        dpr = widget->devicePixelRatioF();
    }
    else if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
    return getGeometriesForDpr(dpr);
}

const SkinScaledGeometries& LC_SkinScaledGeometryProvider::getGeometries(const QPainter *painter) const {
    qreal dpr = 1.0;
    if (painter && painter->device()) {
        dpr = painter->device()->devicePixelRatioF();
    } else if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
    return getGeometriesForDpr(dpr);
}

const SkinScaledGeometries& LC_SkinScaledGeometryProvider::getGeometriesForDpr(qreal dpr) const {
    if (qFuzzyCompare(dpr, m_lastDpr)) {
        return m_lastGeometries;
    }

    if (!m_dpiCache.contains(dpr)) {
        m_dpiCache.insert(dpr, resolveGeometries(dpr));
    }

    m_lastDpr = dpr;
    m_lastGeometries = m_dpiCache.value(dpr);
    return m_lastGeometries;
}

void LC_SkinScaledGeometryProvider::scaleInts(qreal dpr, SkinScaledGeometries& geoms) const {
    geoms.ints.scale1  = scaleMetric(1, dpr);
    geoms.ints.scale2  = scaleMetric(2, dpr);
    geoms.ints.scale3  = scaleMetric(3, dpr);
    geoms.ints.scale4  = scaleMetric(4, dpr);
    geoms.ints.scale5  = scaleMetric(5, dpr);
    geoms.ints.scale6  = scaleMetric(6, dpr);
    geoms.ints.scale8  = scaleMetric(8, dpr);
    geoms.ints.scale12 = scaleMetric(12, dpr);
    geoms.ints.scale15 = scaleMetric(15, dpr);
    geoms.ints.scale16 = scaleMetric(16, dpr);
    geoms.ints.scale20 = scaleMetric(20, dpr);
    geoms.ints.scale24 = scaleMetric(24, dpr);
    geoms.ints.scale48 = scaleMetric(48, dpr);
}

SkinScaledGeometries LC_SkinScaledGeometryProvider::resolveGeometries(qreal dpr) const {
    SkinScaledGeometries geoms;
    geoms.dpr = dpr;
    geoms.crispOffset = 0.5 / dpr;

    // Delegate standard metrics config scaling
    geoms.scaledMetrics = scaleMetricsConfig(m_baseMetrics, dpr);

    // Delegate domain-specific geometries scaling
    scaleTabs(geoms.tab, dpr);
    scaleCheckboxes(geoms.checkbox, dpr);
    scaleRadioButtons(geoms.radioButton, dpr);
    scaleScrollbars(geoms.scrollbar, dpr);
    scaleInputGlows(geoms.focusGlow, dpr);
    scaleSplitterGrips(geoms.splitterGrip, dpr);
    scaleProgressBars(geoms.progressBar, dpr);
    scaleGroupBoxes(geoms.groupBox, dpr);
    scaleSegmentedButtons(geoms.segmentedButton, dpr);
    scaleTooltips(geoms.tooltip, dpr);
    scaleMenuColumns(geoms.menuCol, dpr);

    scaleInts(dpr, geoms);

    return geoms;
}

int LC_SkinScaledGeometryProvider::scaleMetric(int val, qreal dpr) const {
    return (val < 0) ? val : qRound(val * dpr);
}

StyleMetricsConfig LC_SkinScaledGeometryProvider::scaleMetricsConfig(const StyleMetricsConfig &base, qreal dpr) const {
    StyleMetricsConfig scaled;
    scaled.name                           = base.name;
    scaled.scrollBarWidth                 = scaleMetric(base.scrollBarWidth, dpr);
    scaled.treeIndentation                = scaleMetric(base.treeIndentation, dpr);
    scaled.buttonPadding                  = scaleMetric(base.buttonPadding, dpr);
    scaled.scrollBarMinLength              = scaleMetric(base.scrollBarMinLength, dpr);
    scaled.dockTitleBarHeight             = scaleMetric(base.dockTitleBarHeight, dpr);
    scaled.splitterWidth                  = scaleMetric(base.splitterWidth, dpr);
    scaled.itemViewRowPadding             = scaleMetric(base.itemViewRowPadding, dpr);
    scaled.flatDisabledText               = base.flatDisabledText;
    scaled.hideShortcutUnderlines          = base.hideShortcutUnderlines;
    scaled.tabBarTabOverlap               = scaleMetric(base.tabBarTabOverlap, dpr);
    scaled.tabBarTabBaseOverlap           = scaleMetric(base.tabBarTabBaseOverlap, dpr);
    scaled.tabBarTabHSpace                = scaleMetric(base.tabBarTabHSpace, dpr);
    scaled.tabBarTabVSpace                = scaleMetric(base.tabBarTabVSpace, dpr);
    scaled.tabActiveExtraHeight           = scaleMetric(base.tabActiveExtraHeight, dpr);
    scaled.tabCloseIndicatorSize          = scaleMetric(base.tabCloseIndicatorSize, dpr);
    scaled.menuVerticalPadding            = scaleMetric(base.menuVerticalPadding, dpr);
    scaled.menuHorizontalPadding          = scaleMetric(base.menuHorizontalPadding, dpr);
    scaled.menuBorderWidth                = scaleMetric(base.menuBorderWidth, dpr);
    scaled.headerDefaultHeight            = scaleMetric(base.headerDefaultHeight, dpr);
    scaled.sliderHandleLength             = scaleMetric(base.sliderHandleLength, dpr);
    scaled.dockWidgetTitleBarButtonMargin = scaleMetric(base.dockWidgetTitleBarButtonMargin, dpr);
    scaled.sliderControlThickness         = scaleMetric(base.sliderControlThickness, dpr);
    scaled.focusFrameHMargin              = scaleMetric(base.focusFrameHMargin, dpr);
    scaled.focusFrameVMargin              = scaleMetric(base.focusFrameVMargin, dpr);
    scaled.layoutMargin                   = scaleMetric(base.layoutMargin, dpr);
    scaled.layoutSpacing                  = scaleMetric(base.layoutSpacing, dpr);
    scaled.toolbarItemSpacing             = scaleMetric(base.toolbarItemSpacing, dpr);
    scaled.toolbarSeparatorWidth          = scaleMetric(base.toolbarSeparatorWidth, dpr);
    scaled.menuBarItemSpacing             = scaleMetric(base.menuBarItemSpacing, dpr);
    scaled.menuBarVerticalMargin          = scaleMetric(base.menuBarVerticalMargin, dpr);
    scaled.indicatorBoxSize               = scaleMetric(base.indicatorBoxSize, dpr);
    scaled.indicatorLabelSpacing          = scaleMetric(base.indicatorLabelSpacing, dpr);
    scaled.titleBarButtonSpacing          = scaleMetric(base.titleBarButtonSpacing, dpr);
    scaled.titleBarButtonSize             = scaleMetric(base.titleBarButtonSize, dpr);
    scaled.subMenuOverlap                 = scaleMetric(base.subMenuOverlap, dpr);
    scaled.menuSupportsIcons              = base.menuSupportsIcons;
    scaled.menuAllowActiveAndDisabled     = base.menuAllowActiveAndDisabled;
    scaled.groupBoxTitleLeftPadding       = scaleMetric(base.groupBoxTitleLeftPadding, dpr);
    scaled.groupBoxTitleLineGap           = scaleMetric(base.groupBoxTitleLineGap, dpr);
    scaled.mnemonicUnderlineMode          = base.mnemonicUnderlineMode;
    scaled.treeBranchIndicatorSize        = scaleMetric(base.treeBranchIndicatorSize, dpr);
    scaled.splitterHandleLength           = scaleMetric(base.splitterHandleLength, dpr);
    scaled.toolbarPopupDelay              = base.toolbarPopupDelay;
    return scaled;
}

void LC_SkinScaledGeometryProvider::scaleTabs(SkinScaledGeometries::Tab &tab, qreal dpr) const {
    tab.roundedRadius     = 10.0 * dpr;
    tab.beveledInset      = 12.0 * dpr;
    tab.stripeThickness   = 2.5 * dpr;
    tab.stripeOffset      = 1.25 * dpr;
    tab.marginDefault     = 2.0 * dpr;
    tab.activeExtraHeight = 3.0 * dpr;
}

void LC_SkinScaledGeometryProvider::scaleCheckboxes(SkinScaledGeometries::Checkbox &cb, qreal dpr) const {
    cb.checkInset    = scaleMetric(3, dpr);
    cb.checkPenWidth = 1.8 * dpr;
}

void LC_SkinScaledGeometryProvider::scaleRadioButtons(SkinScaledGeometries::RadioButton &rb, qreal dpr) const {
    rb.dotOffset = scaleMetric(3, dpr);
}

void LC_SkinScaledGeometryProvider::scaleScrollbars(SkinScaledGeometries::Scrollbar &sb, qreal dpr) const {
    sb.paddingTransparent = scaleMetric(2, dpr);
    sb.paddingStandard    = scaleMetric(1, dpr);
    sb.radiusTransparent  = scaleMetric(3, dpr);
    sb.radiusStandard     = scaleMetric(2, dpr);
}

void LC_SkinScaledGeometryProvider::scaleInputGlows(SkinScaledGeometries::FocusGlow &fg, qreal dpr) const {
    fg.margin1 = 2.0 * dpr;
    fg.margin2 = 1.5 * dpr;
    fg.margin3 = 2.5 * dpr;
    fg.radius1 = 2.5 * dpr;
    fg.radius2 = 1.5 * dpr;
    fg.radius3 = 0.5 * dpr;
}

void LC_SkinScaledGeometryProvider::scaleSplitterGrips(SkinScaledGeometries::SplitterGrip &sg, qreal dpr) const {
    sg.wellWidth        = scaleMetric(6, dpr);
    sg.wellLengthOffset = scaleMetric(6, dpr);
    sg.gripPillHeight   = 3.0 * dpr;
    sg.gripSpacing      = 4.0 * dpr;
}

void LC_SkinScaledGeometryProvider::scaleProgressBars(SkinScaledGeometries::ProgressBar &pb, qreal dpr) const {
    pb.trackRadius = 2.0 * dpr;
    pb.fillRadius  = 1.5 * dpr;
    pb.fillInset   = scaleMetric(1, dpr);
}

void LC_SkinScaledGeometryProvider::scaleGroupBoxes(SkinScaledGeometries::GroupBox &gb, qreal dpr) const {
    gb.titleLeftSpacing  = scaleMetric(8, dpr);
    gb.titleRightSpacing = scaleMetric(6, dpr);
    gb.checkBoxTextGap   = scaleMetric(6, dpr);
}

void LC_SkinScaledGeometryProvider::scaleSegmentedButtons(SkinScaledGeometries::SegmentedButton &sb, qreal dpr) const {
    sb.dividerInset = scaleMetric(2, dpr);
}

void LC_SkinScaledGeometryProvider::scaleTooltips(SkinScaledGeometries::Tooltip &tt, qreal dpr) const {
    tt.marginLeft   = scaleMetric(8, dpr);
    tt.marginTop    = scaleMetric(4, dpr);
    tt.marginRight  = scaleMetric(8, dpr);
    tt.marginBottom = scaleMetric(4, dpr);
    tt.cornerRadius = 4.0 * dpr;
}

void LC_SkinScaledGeometryProvider::scaleMenuColumns(SkinScaledGeometries::MenuColumn &mc, qreal dpr) const {
    mc.leftMargin              = scaleMetric(12, dpr);
    mc.rightMargin             = scaleMetric(20, dpr);
    mc.gapBetweenCmdShortcut   = scaleMetric(15, dpr);
    mc.flexibleLabelGap        = scaleMetric(10, dpr);
    mc.titleBarClearanceOffset = scaleMetric(48, dpr);
}
