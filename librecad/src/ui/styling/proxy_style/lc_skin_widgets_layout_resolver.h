
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

#ifndef LC_LayoutResolver_H
#define LC_LayoutResolver_H
#include <QDockWidget>
#include <QPainterPath>
#include <QStyle>
#include <QTabBar>

#include "lc_proxy_style_shared.h"

class QAbstractSpinBox;
class QStyleOption;
class QStyleOptionGroupBox;
class QStyleOptionMenuItem;
class QStyleOptionProgressBar;
class QStyleOptionSlider;

namespace LC_SkinWidgetsLayoutResolver {
    struct SliderLayout {
        QRect grooveRect;
        QRect handleRect;
        QRectF circleRect;
        QRectF innerDotRect;
    };

    struct MenuItemColumnsLayout {
        QRect labelRect;
        QRect aliasRect;
        QRect shortcutRect;
    };

    struct ProgressBarLayout {
        QRect rect;
        QRectF fillRect;
    };

    struct TabLayout {
        QRectF adjustedRect;
        TabPaths paths;
        QLineF stripeLine;
        bool hasActiveStripe = false;
    };

    struct SegmentedToolButtonLayout {
        QPainterPath buttonPath;
        QList<QLineF> dividerLines;
        bool drawIndicator = false;
        bool isIndicatorDot = false;
        QRectF indicatorRect;
        QPointF indicatorDotCenter;
    };

    struct ToolButtonAccessoryLayout {
        bool drawWell = false;
        QRect wellRect;
        bool drawIndicator = false;
        bool isIndicatorDot = false;
        QRectF indicatorRect;
        QPointF indicatorDotCenter;
    };

    struct GripLayout {
        bool showWell = false;
        QRect wellRect;
        int wellRadius = 0;
        int handleLength = 0;
        bool horizontalPattern = false;
    };

    struct BranchLayout {
        QPoint center;
        int size = 0;
        bool drawLines = false;
        QLine verticalLine;
        QLine horizontalLine;
        QRect plusMinusBox;
        QPolygonF chevronPolygon;
        bool isExpanded = false;
    };

    struct TabWidgetFrameClipLayout {
        bool needClipping = false;
        QRegion clipRegion;
    };

    struct SpinBoxProgressBarLayout {
        QRect editRect;
        QRect bgRect;
        QRect fillRect;
        QRect cursorRect;
    };

    struct ScrollBarLayout {
        QRect subPageRect;
        QRect addPageRect;
        QRect handleRect;
        QRectF adjustedHandleRect;
        int handleRadius = 0;
        bool handleValid = false;
    };

    struct ArrowLayout {
        bool isPolygon = false;
        QPolygonF polygon;
        bool isLines = false;
        QLineF line1;
        QLineF line2;
    };

    struct TitleBarLayout {
        QRect titleBarRect;
        QRect dragHandleRect;
        QRect closeButtonRect;
    };
    struct TitleBarButtonLayout {
        QRect closeRect;
        QRect floatRect;
        QRect textRect;
    };



    SliderLayout resolveSliderLayout(const QStyle* style, const QStyleOptionSlider* option, const SkinScaledGeometries& geoms,
                                     const QWidget* widget);
    ProgressBarLayout resolveProgressBarLayout(const QStyleOptionProgressBar* option, const SkinScaledGeometries& geoms);
    MenuItemColumnsLayout resolveMenuItemLayout(const QStyleOptionMenuItem* option, const SkinScaledGeometries& geoms, int maxCmdWidth,
                                                int maxShortcutWidth);
    TabLayout resolveTabLayout(const QStyleOptionTab* option, const QWidget* widget, const SkinScaledGeometries& geoms, const QStyle* style,
                               bool isLeftAccentBar, bool tabStripeAtBottom);
    GroupBoxLayout resolveGroupBoxLayout(const QStyle* style, const QStyleOptionGroupBox* option, const SkinColors& desc,
                                         const SkinScaledGeometries& geoms, const QRect& textRect, const QRect& checkBoxRect,
                                         const QWidget* widget);
    QPainterPath resolveSegmentedGroupPath(const QList<QToolButton*>& buttons, qreal dpr);
    SegmentedToolButtonLayout resolveSegmentedToolButtonLayout(const QStyleOption* option, const SkinScaledGeometries& geoms, int mask,
                                                               bool useToolButtonUnderline, ToolButtonIndicatorStyle indicatorStyle,
                                                               const QWidget* widget);

    ToolButtonAccessoryLayout resolveToolButtonAccessoryLayout(const QStyleOption* option, const SkinScaledGeometries& geoms,
                                                               bool useToolButtonUnderline, ToolButtonIndicatorStyle indicatorStyle,
                                                               const QWidget* widget, bool autoRaise, bool showGripBackgroundWell);

    GripLayout resolveGripLayout(const QStyleOption* option, const SkinScaledGeometries& geoms, bool showGripBackgroundWell, int baseHandleLen,
                                 bool handleIsHorizontal);

    BranchLayout resolveBranchLayout(const QStyleOption* option, const SkinScaledGeometries& geoms);

    TabWidgetFrameClipLayout resolveTabWidgetFrameClipLayout(const QStyleOption* option, const SkinScaledGeometries& geoms,
                                                             const QWidget* widget, bool tabStripeAtBottom);

    SpinBoxProgressBarLayout resolveSpinBoxProgressBarLayout(const QStyle* style, const QRect& rect, const SkinScaledGeometries& geoms,
                                                             qreal pct, bool hasFocus, const QWidget* widget,
                                                             const QAbstractSpinBox* spinBox, bool useFocusedInputGlow);

    ScrollBarLayout resolveScrollBarLayout(const QStyle* style, const QStyleOptionSlider* option, const SkinScaledGeometries& geoms,
                                           const QWidget* widget, bool transparentScrollbars);

    ArrowLayout resolveArrowLayout(QStyle::PrimitiveElement element, const QStyleOption* option, const SkinScaledGeometries& geoms,
                                   const QWidget* widget, bool customVectorIcons);

    TitleBarLayout resolveTitleBarMetrics(const QRect &windowRect, const SkinScaledGeometries &geoms);

    // fixme - move to proper location
    void drawTitleBar(QPainter* painter, const TitleBarLayout& metrics, const SkinColors& desc, const SkinScaledGeometries& geoms,
                      bool active, bool closeHovered, CloseButtonColorPolicy closePolicy, const QString& titleText);

    TitleBarButtonLayout resolveTitleBarButtonLayout(const QRect& titleRect, const SkinScaledGeometries& geoms,
                                                     QDockWidget::DockWidgetFeatures features, bool isVertical);

}

#endif
