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

#ifndef LC_FUSION_PROXY_STYLE_SHARED_H
#define LC_FUSION_PROXY_STYLE_SHARED_H
#include <QAbstractButton>
#include <QPainterPath>
#include <QToolBar>
#include <QToolButton>

#include "lc_palette_color_utils.h"

class QAbstractButton;
class QColor;

struct GroupBoxLayout {
    QRect drawFrameRect;
    QRect drawTotalRect;
    QRect drawCheckBoxRect;
    QRect drawTextRect;
    int topY = 0;
    QRect headerRect;
    QRect fillHeaderRect;
    int underlineY = 0;
    QRect bgCoverRect;
};

struct SkinColors {
    struct Common {
        QColor bgStart;
        QColor bgEnd;
        QColor textColor;
        QColor selectionHighlight;
        QColor highlightColor;
        bool useGradient = false;
        bool useGlassyGloss = false;
    } common;

    struct Frame {
        QColor borderButton;
        QColor borderHovered;
        QColor borderTop;
        QColor borderBottom;
        QColor borderLeft;
        QColor borderRight;
        QColor accentBarColor;
        bool hasFullBorder = false;
        bool hasTopBottomBorder = false;
        bool hasSideBorders = false;
        bool hasLeftAccentBar = false;
    } frame;

    struct Button {
        QColor bgButton;
        QColor bgButtonEnd;
        QColor bgHovered;
        QColor bgSunken;
        QColor bgChecked;
        QColor glassStart;
        QColor glassMidStart;
        QColor glassMidEnd;
        QColor glassEnd;
        QColor glassHoverStart;
        QColor glassHoverMidStart;
        QColor glassHoverMidEnd;
        QColor glassHoverEnd;
        QColor glassSunkenStart;
        QColor glassSunkenMidStart;
        QColor glassSunkenMidEnd;
        QColor glassSunkenEnd;
    } button;

    struct SegmentedGroup {
        QMap<int, QColor> groupBgStart;
        QMap<int, QColor> groupBgEnd;
        QMap<int, LC_PaletteColorUtils::GroupGradientStops> groupGradientStops;
        QMap<int, QColor> groupBgStartHovered;
        QMap<int, QColor> groupBgEndHovered;
        QMap<int, LC_PaletteColorUtils::GroupGradientStops> groupGradientStopsHovered;
    } segmentedGroup;

    struct Input {
        QColor bgInput;
        QColor bgInputEnd;
        QColor bgInputHovered;
        QColor bgInputHoveredEnd;
        QColor bgInputFocused;
        QColor inputFocusGlowOuter;
        QColor inputFocusGlowInner;
    } input;

    struct CheckBox {
        QColor bg;
        QColor border;
        QColor checkMark;
    } checkbox;

    struct RadioButton {
        QColor bg;
        QColor border;
        QColor dot;
    } radioButton;

    struct Arrow {
        QColor arrowColor;
        QColor arrowColorHovered;
        QColor menuArrowColorSelected;
    } arrow;

    struct SpinBox {
        QColor spinBoxProgressBarTrack;
        QColor spinBoxProgressBarTrackFocused;
        QColor spinBoxProgressBarFill;
        QColor spinBoxProgressBarSatinTop;
        QColor spinBoxProgressBarSatinBottom;
        QColor spinBoxProgressBarGlass0;
        QColor spinBoxProgressBarGlass42;
        QColor spinBoxProgressBarGlass55;
        QColor spinBoxProgressBarGlass100;
        QColor spinBoxProgressBarSatinFocusedTop;
        QColor spinBoxProgressBarSatinFocusedBottom;
        QColor spinBoxProgressBarGlassFocused0;
        QColor spinBoxProgressBarGlassFocused42;
        QColor spinBoxProgressBarGlassFocused55;
        QColor spinBoxProgressBarGlassFocused100;
        QColor spinBoxProgressBarFocusedFill;
    } spinBox;

    struct ProgressBar {
        QColor spinBoxProgressBarTrack;
        QColor spinBoxProgressBarFill;
        QColor spinBoxProgressBarSatinTop;
        QColor spinBoxProgressBarSatinBottom;
        QColor spinBoxProgressBarGlass0;
        QColor spinBoxProgressBarGlass42;
        QColor spinBoxProgressBarGlass55;
        QColor spinBoxProgressBarGlass100;
        QColor spinBoxProgressBarSatinFocusedTop;
        QColor spinBoxProgressBarSatinFocusedBottom;
        QColor spinBoxProgressBarGlassFocused0;
        QColor spinBoxProgressBarGlassFocused42;
        QColor spinBoxProgressBarGlassFocused55;
        QColor spinBoxProgressBarGlassFocused100;
        QColor spinBoxProgressBarFocusedFill;
        QColor trackColor;
        QColor fillColor;
        QColor borderColor;
    } progressBar;

    struct Tab {
        QColor bgTabInactive;
        QColor bgTabInactiveDock;
        QColor textTabInactive;
        QColor borderTabInactiveTop;
        QColor bgTabInactiveHovered;
        QColor bgTabInactiveHoveredDock;
        QColor borderTabInactiveHovered;
        QColor activeTabAccentBg;
        QColor accentOutlineTabBg;
        QColor activeTabBgStart;
        QColor activeTabBgEnd;
        QColor tabWidgetFrameBorder;
        QColor closeButtonHoverColor;
        QColor tearIndicatorColor;
        QColor tearIndicatorColorHover;
    } tab;

    struct GroupBox {
        GroupBoxHeaderStyle groupBoxHeaderStyle;
        GroupBoxBoundaryStyle groupBoxBoundaryStyle;
        QColor groupBoxFrameColor;
        QColor groupBoxBannerFillColor;
        QColor groupBoxTextColor;
        QColor groupBoxGlassStart;
        QColor groupBoxGlassMidStart;
        QColor groupBoxGlassMidEnd;
        QColor groupBoxGlassEnd;
    } groupBox;

    struct Splitter {
        QColor splitterGripColor;
        QColor splitterGripDark;
        QColor splitterGripLight;
        QColor splitterGripColorIdle;
        QColor splitterGripWellColor;
        QColor splitterHoverGlowColor;
    } splitter;

    struct ToolBar {
        QColor toolbarSeparatorPen;
        QColor toolbarSeparatorHighlightPen;
        QColor toolbarOverflowIndicatorColor;
        QColor bg;
        QColor flatBorder;
        QColor borderLight;
        QColor borderDark;
    } toolBar;

    struct MenuBar {
        QColor bg;
        QColor borderBottom;
    } menuBar;

    struct ToolButton {
        QColor toolButtonWellColor;
        QColor toolButtonIndicatorColor;
        QColor toolButtonIndicatorColorHover;
        QColor toolButtonWellColorHover;
        QColor toolButtonBgCheckedHover;
    } toolButton;

    struct DockTitleBar {
        QColor dockTitleBgAccentOutline;
        QColor borderTop;
        QColor borderBottom;
        QColor accentBarColor;
        QColor highlightBorder;
        QColor midlightBorder;
    } dockTitleBar;

    struct Slider {
        QColor sliderInnerDotColor;
        QColor sliderHandleColor;
        QColor sliderHandleColorHovered;
        QColor grooveBg;
        QColor grooveBorder;
        QColor handleBorder;
    } slider;

    struct ScrollBar {
        QColor scrollBarHandleBorder;
        QColor scrollBarTrackColor;
        QColor scrollBarHandleColor;
        QColor scrollBarHandleColorHovered;
        QColor scrollBarBorderColor;
        QColor scrollBarBorderColorHovered;
    } scrollBar;

    struct Header {
        QColor headerSeparator;
        QColor headerBorderBottom;
    } header;

    struct ItemView {
        QColor bgItemHovered;
        QColor bgItemAlternateHovered;
        QColor indicatorHoverWellColor;
        QColor branchIndicatorHoverColor;
        QColor menuAliasColorNormal;
        QColor menuAliasColorSelected;
        QColor branchIndicatorSelectedColor;
        QColor branchLineSelectedColor;
    } itemView;

    struct Tooltip {
        QColor tooltipBg;
        QColor tooltipBorder;
    } tooltip;
};

struct TabPaths {
    QPainterPath fillPath;
    QPainterPath borderPath;
};

struct SkinScaledGeometries {
    qreal dpr;
    qreal crispOffset;

    StyleMetricsConfig scaledMetrics;

    struct Tab {
        qreal roundedRadius;
        qreal beveledInset;
        qreal stripeThickness;
        qreal stripeOffset;
        qreal marginDefault;
        qreal activeExtraHeight;
    } tab;

    struct Checkbox {
        int checkInset;
        qreal checkPenWidth;
    } checkbox;

    struct RadioButton {
        int dotOffset;
    } radioButton;

    struct Scrollbar {
        int paddingTransparent;
        int paddingStandard;
        int radiusTransparent;
        int radiusStandard;
    } scrollbar;

    struct FocusGlow {
        qreal margin1;
        qreal margin2;
        qreal margin3;
        qreal radius1;
        qreal radius2;
        qreal radius3;
    } focusGlow;

    struct SplitterGrip {
        int wellWidth;
        int wellLengthOffset;
        qreal gripPillHeight;
        qreal gripSpacing;
    } splitterGrip;

    struct ProgressBar {
        qreal trackRadius;
        qreal fillRadius;
        int fillInset;
    } progressBar;

    struct GroupBox {
        int titleLeftSpacing;
        int titleRightSpacing;
        int checkBoxTextGap;
    } groupBox;

    struct SegmentedButton {
        int dividerInset;
    } segmentedButton;

    struct Tooltip {
        int marginLeft;
        int marginTop;
        int marginRight;
        int marginBottom;
        qreal cornerRadius;
    } tooltip;

    struct MenuColumn {
        int leftMargin;
        int rightMargin;
        int gapBetweenCmdShortcut;
        int flexibleLabelGap;
        int titleBarClearanceOffset;
    } menuCol;

    // Fast look-up registers for pre-scaled integer dimensions
    struct ScaledInts {
        int scale1; // qRound(1.0 * dpr)
        int scale2; // qRound(2.0 * dpr)
        int scale3; // qRound(3.0 * dpr)
        int scale4; // qRound(4.0 * dpr)
        int scale5; // qRound(5.0 * dpr)
        int scale6; // qRound(6.0 * dpr)
        int scale8; // qRound(8.0 * dpr)
        int scale12; // qRound(12.0 * dpr)
        int scale15; // qRound(15.0 * dpr)
        int scale16; // qRound(16.0 * dpr)
        int scale20; // qRound(20.0 * dpr)
        int scale24; // qRound(24.0 * dpr)
        int scale48; // qRound(48.0 * dpr)
    } ints;
};

// Precomputed Segmented Group Background Properties
constexpr auto PROP_GROUP_BG_START = "lcfs_groupBgStart";
constexpr auto PROP_GROUP_BG_END = "lcfs_groupBgEnd";
constexpr auto PROP_GROUP_STOP0 = "lcfs_groupStop0";
constexpr auto PROP_GROUP_STOP42 = "lcfs_groupStop42";
constexpr auto PROP_GROUP_STOP55 = "lcfs_groupStop55";
constexpr auto PROP_GROUP_STOP100 = "lcfs_groupStop100";
constexpr auto PROP_GROUP_BG_START_HOVERED = "lcfs_groupBgStartHovered";
constexpr auto PROP_GROUP_BG_END_HOVERED = "lcfs_groupBgEndHovered";
constexpr auto PROP_GROUP_STOP0_HOVERED = "lcfs_groupStop0Hovered";
constexpr auto PROP_GROUP_STOP42_HOVERED = "lcfs_groupStop42Hovered";
constexpr auto PROP_GROUP_STOP55_HOVERED = "lcfs_groupStop55Hovered";
constexpr auto PROP_GROUP_STOP100_HOVERED = "lcfs_groupStop100Hovered";

constexpr auto PROP_IS_INPUT_COMPONENT       = "lcfs_isInputComponent";
constexpr auto PROP_IS_DOCK_TITLE_BUTTON     = "lcfs_isDockTitleButton";
constexpr auto PROP_IS_WINDOW_TITLE_BUTTON   = "lcfs_isWindowTitleButton";
constexpr auto PROP_GROUP_NEIGHBORS          = "groupNeighbors";


constexpr auto PROP_HOVERED_GROUP_ID         = "lcfs_hoveredGroupId";
constexpr auto PROP_LINK_ACTIVE_HOVER        = "lcfs_linkActiveHover";
constexpr auto PROP_ORIGINAL_HTML            = "lcfs_originalHtml";
constexpr auto PROP_EXT_BUTTON               = "lcfs_extButton";
constexpr auto PROP_STYLE_PRE_CLOAKED        = "lcfs_styleWindowPreCloaked";

constexpr auto PROP_GROUP_PALETTE_COLOR      = "lcfs_groupPaletteColor";
constexpr auto PROP_FLOATING_HUD_FILTER_INSTALLED = "lcfs_floatingHUDFilterInstalled";
constexpr auto PROP_CLOSE_BUTTON_HOVERED     = "lcfs_closeButtonHovered";


// constants that may be used by the application
constexpr auto PROP_BUTTON_GROUP             = "buttonGroup";
constexpr auto PROP_DISABLE_AUTO_EXPANSION   = "disableAutoExpansion";
constexpr auto PROP_USE_STATUS_PILL_CHIPS    = "lcfs_useStatusPillChips";
constexpr auto PROP_ACTIVE                   = "active";





inline bool isTitleOrDockButton(const QWidget* widget) {
    if (!widget) {
        return false;
    }
    return widget->property(PROP_IS_DOCK_TITLE_BUTTON).toBool() || widget->property(PROP_IS_WINDOW_TITLE_BUTTON).toBool();
}

inline bool isWidgetCheckable(const QWidget* widget) {
    if (!widget) {
        return true; // Default behavior
    }
    if (const auto* btn = qobject_cast<const QAbstractButton*>(widget)) {
        return btn->isCheckable();
    }
    return true;
}

inline QToolButton* findToolbarExtensionButton(const QToolBar* toolBar) {
    if (!toolBar)
        return nullptr;
    return toolBar->findChild<QToolButton*>(QLatin1String("qt_toolbar_ext_button"));
}

// Identifies if a widget is the private toolbar expansion button
inline bool isToolbarExtensionButton(const QWidget* widget) {
    return widget && widget->objectName() == QLatin1String("qt_toolbar_ext_button");
}

#endif
