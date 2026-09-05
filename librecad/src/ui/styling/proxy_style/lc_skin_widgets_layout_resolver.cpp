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

#include "lc_skin_widgets_layout_resolver.h"

#include <QStyle>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionSlider>

#include "lc_proxy_style_shared.h"

namespace {
    QPainterPath getTabRoundedPath(const QRectF& rF, qreal r, QTabBar::Shape orientation) {
        QPainterPath path;
        if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
            path.moveTo(rF.left(), rF.bottom());
            path.lineTo(rF.left(), rF.top() + r);
            path.quadTo(rF.left(), rF.top(), rF.left() + r, rF.top());
            path.lineTo(rF.right() - r, rF.top());
            path.quadTo(rF.right(), rF.top(), rF.right(), rF.top() + r);
            path.lineTo(rF.right(), rF.bottom());
            path.closeSubpath();
        }
        else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
            path.moveTo(rF.left(), rF.top());
            path.lineTo(rF.left(), rF.bottom() - r);
            path.quadTo(rF.left(), rF.bottom(), rF.left() + r, rF.bottom());
            path.lineTo(rF.right() - r, rF.bottom());
            path.quadTo(rF.right(), rF.bottom(), rF.right(), rF.bottom() - r);
            path.lineTo(rF.right(), rF.top());
            path.closeSubpath();
        }
        else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
            path.moveTo(rF.right(), rF.top());
            path.lineTo(rF.left() + r, rF.top());
            path.quadTo(rF.left(), rF.top(), rF.left(), rF.top() + r);
            path.lineTo(rF.left(), rF.bottom() - r);
            path.quadTo(rF.left(), rF.bottom(), rF.left() + r, rF.bottom());
            path.lineTo(rF.right(), rF.bottom());
            path.closeSubpath();
        }
        else {
            // RoundedEast / TriangularEast
            path.moveTo(rF.left(), rF.top());
            path.lineTo(rF.right() - r, rF.top());
            path.quadTo(rF.right(), rF.top(), rF.right(), rF.top() + r);
            path.lineTo(rF.right(), rF.bottom() - r);
            path.quadTo(rF.right(), rF.bottom(), rF.right() - r, rF.bottom());
            path.lineTo(rF.left(), rF.bottom());
            path.closeSubpath();
        }
        return path;
    }

    TabPaths getTabPaths(const QRectF& rF, QTabBar::Shape orientation, bool selected, const SkinScaledGeometries& geoms) {
        TabPaths paths;
        const qreal r = geoms.tab.roundedRadius;
        const qreal inset = geoms.tab.beveledInset;

        TabShape shape = TabShape::Rounded;
        if (orientation == QTabBar::TriangularNorth || orientation == QTabBar::TriangularSouth || orientation == QTabBar::TriangularWest ||
            orientation == QTabBar::TriangularEast) {
            shape = TabShape::Beveled;
        }

        // 1. Build the CLOSED background fill path
        if (shape == TabShape::Rectangular) {
            paths.fillPath.addRect(rF);
        }
        else if (shape == TabShape::Rounded) {
            paths.fillPath = getTabRoundedPath(rF, r, orientation);
        }
        else if (shape == TabShape::Beveled) {
            QPolygonF polygon;
            if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
                polygon << QPointF(rF.left(), rF.bottom()) << QPointF(rF.left() + inset, rF.top()) << QPointF(rF.right() - inset, rF.top())
                    << QPointF(rF.right(), rF.bottom());
            }
            else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
                polygon << QPointF(rF.left(), rF.top()) << QPointF(rF.left() + inset, rF.bottom()) << QPointF(
                    rF.right() - inset, rF.bottom()) << QPointF(rF.right(), rF.top());
            }
            else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
                polygon << QPointF(rF.right(), rF.top()) << QPointF(rF.left(), rF.top() + inset) << QPointF(rF.left(), rF.bottom() - inset)
                    << QPointF(rF.right(), rF.bottom());
            }
            else {
                // East
                polygon << QPointF(rF.left(), rF.top()) << QPointF(rF.right(), rF.top() + inset) << QPointF(rF.right(), rF.bottom() - inset)
                    << QPointF(rF.left(), rF.bottom());
            }
            paths.fillPath.addPolygon(polygon);
        }

        // 2. Build the OPEN border outline path
        if (selected) {
            if (shape == TabShape::Rectangular) {
                if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
                    paths.borderPath.moveTo(rF.left(), rF.bottom());
                    paths.borderPath.lineTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.left(), rF.bottom());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                    paths.borderPath.lineTo(rF.right(), rF.top());
                }
                else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
                    paths.borderPath.moveTo(rF.right(), rF.top());
                    paths.borderPath.lineTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.left(), rF.bottom());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else {
                    // East
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                    paths.borderPath.lineTo(rF.left(), rF.bottom());
                }
            }
            else if (shape == TabShape::Rounded) {
                if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
                    paths.borderPath.moveTo(rF.left(), rF.bottom());
                    paths.borderPath.lineTo(rF.left(), rF.top() + r);
                    paths.borderPath.quadTo(rF.left(), rF.top(), rF.left() + r, rF.top());
                    paths.borderPath.lineTo(rF.right() - r, rF.top());
                    paths.borderPath.quadTo(rF.right(), rF.top(), rF.right(), rF.top() + r);
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.left(), rF.bottom() - r);
                    paths.borderPath.quadTo(rF.left(), rF.bottom(), rF.left() + r, rF.bottom());
                    paths.borderPath.lineTo(rF.right() - r, rF.bottom());
                    paths.borderPath.quadTo(rF.right(), rF.bottom(), rF.right(), rF.bottom() - r);
                    paths.borderPath.lineTo(rF.right(), rF.top());
                }
                else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
                    paths.borderPath.moveTo(rF.right(), rF.top());
                    paths.borderPath.lineTo(rF.left() + r, rF.top());
                    paths.borderPath.quadTo(rF.left(), rF.top(), rF.left(), rF.top() + r);
                    paths.borderPath.lineTo(rF.left(), rF.bottom() - r);
                    paths.borderPath.quadTo(rF.left(), rF.bottom(), rF.left() + r, rF.bottom());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else {
                    // East
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.right() - r, rF.top());
                    paths.borderPath.quadTo(rF.right(), rF.top(), rF.right(), rF.top() + r);
                    paths.borderPath.lineTo(rF.right(), rF.bottom() - r);
                    paths.borderPath.quadTo(rF.right(), rF.bottom(), rF.right() - r, rF.bottom());
                    paths.borderPath.lineTo(rF.left(), rF.bottom());
                }
            }
            else if (shape == TabShape::Beveled) {
                if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
                    paths.borderPath.moveTo(rF.left(), rF.bottom());
                    paths.borderPath.lineTo(rF.left() + inset, rF.top());
                    paths.borderPath.lineTo(rF.right() - inset, rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.left() + inset, rF.bottom());
                    paths.borderPath.lineTo(rF.right() - inset, rF.bottom());
                    paths.borderPath.lineTo(rF.right(), rF.top());
                }
                else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
                    paths.borderPath.moveTo(rF.right(), rF.top());
                    paths.borderPath.lineTo(rF.left(), rF.top() + inset);
                    paths.borderPath.lineTo(rF.left(), rF.bottom() - inset);
                    paths.borderPath.lineTo(rF.right(), rF.bottom());
                }
                else {
                    // East
                    paths.borderPath.moveTo(rF.left(), rF.top());
                    paths.borderPath.lineTo(rF.right(), rF.top() + inset);
                    paths.borderPath.lineTo(rF.right(), rF.bottom() - inset);
                    paths.borderPath.lineTo(rF.left(), rF.bottom());
                }
            }
        }
        else {
            if (orientation == QTabBar::RoundedNorth || orientation == QTabBar::TriangularNorth) {
                paths.borderPath.moveTo(rF.left(), rF.bottom());
                paths.borderPath.lineTo(rF.right(), rF.bottom());
            }
            else if (orientation == QTabBar::RoundedSouth || orientation == QTabBar::TriangularSouth) {
                paths.borderPath.moveTo(rF.left(), rF.top());
                paths.borderPath.lineTo(rF.right(), rF.top());
            }
            else if (orientation == QTabBar::RoundedWest || orientation == QTabBar::TriangularWest) {
                paths.borderPath.moveTo(rF.right(), rF.top());
                paths.borderPath.lineTo(rF.right(), rF.bottom());
            }
            else {
                // East
                paths.borderPath.moveTo(rF.left(), rF.top());
                paths.borderPath.lineTo(rF.left(), rF.bottom());
            }
        }

        return paths;
    }

    QPainterPath getSegmentedRectPath(const QRectF& r, qreal rTL, qreal rTR, qreal rBL, qreal rBR) {
        QPainterPath path;
        path.moveTo(r.left() + rTL, r.top());
        path.lineTo(r.right() - rTR, r.top());
        if (rTR > 0) {
            path.quadTo(r.topRight(), QPointF(r.right(), r.top() + rTR));
        }
        else {
            path.lineTo(r.topRight());
        }

        path.lineTo(r.right(), r.bottom() - rBR);
        if (rBR > 0) {
            path.quadTo(r.bottomRight(), QPointF(r.right() - rBR, r.bottom()));
        }
        else {
            path.lineTo(r.bottomRight());
        }

        path.lineTo(r.left() + rBL, r.bottom());
        if (rBL > 0) {
            path.quadTo(r.bottomLeft(), QPointF(r.left(), r.bottom() - rBL));
        }
        else {
            path.lineTo(r.bottomLeft());
        }

        path.lineTo(r.left(), r.top() + rTL);
        if (rTL > 0) {
            path.quadTo(r.topLeft(), QPointF(r.left() + rTL, r.top()));
        }
        else {
            path.lineTo(r.topLeft());
        }

        path.closeSubpath();
        return path;
    }
} // namespace

LC_SkinWidgetsLayoutResolver::SliderLayout LC_SkinWidgetsLayoutResolver::resolveSliderLayout(const QStyle* style, const QStyleOptionSlider* option,
                                                                       const SkinScaledGeometries& geoms, const QWidget* widget) {
    SliderLayout layout;
    const QRect grooveRect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderGroove, widget);
    const QRect handleRect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderHandle, widget);

    layout.grooveRect = grooveRect;
    layout.handleRect = handleRect;

    const int halfGroove = qMax(1, geoms.ints.scale2);

    if (option->orientation == Qt::Horizontal) {
        layout.grooveRect.adjust(0, grooveRect.height() / 2 - halfGroove, 0, -grooveRect.height() / 2 + halfGroove);
    }
    else {
        layout.grooveRect.adjust(grooveRect.width() / 2 - halfGroove, 0, -grooveRect.width() / 2 + halfGroove, 0);
    }

    const int handleMargin = qMax(1, geoms.ints.scale1);
    layout.circleRect = QRectF(handleRect).adjusted(handleMargin, handleMargin, -handleMargin, -handleMargin);

    const int dotOffset = qMax(2, geoms.ints.scale4);
    layout.innerDotRect = layout.circleRect.adjusted(dotOffset, dotOffset, -dotOffset, -dotOffset);

    return layout;
}

LC_SkinWidgetsLayoutResolver::ProgressBarLayout LC_SkinWidgetsLayoutResolver::resolveProgressBarLayout(const QStyleOptionProgressBar* option,
                                                                                 const SkinScaledGeometries& geoms) {
    LC_SkinWidgetsLayoutResolver::ProgressBarLayout layout;
    layout.rect = option->rect;

    double factor = (option->maximum == option->minimum)
                        ? 0.0
                        : static_cast<double>(option->progress - option->minimum) / (option->maximum - option->minimum);
    factor = qBound(0.0, factor, 1.0);

    const int fillWidth = static_cast<int>(option->rect.width() * factor);

    if (fillWidth > (geoms.progressBar.fillInset * 2)) {
        layout.fillRect = QRectF(option->rect.left() + geoms.progressBar.fillInset, option->rect.top() + geoms.progressBar.fillInset,
                                 fillWidth - (geoms.progressBar.fillInset * 2), option->rect.height() - (geoms.progressBar.fillInset * 2));
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::MenuItemColumnsLayout LC_SkinWidgetsLayoutResolver::resolveMenuItemLayout(const QStyleOptionMenuItem* option,
                                                                                  const SkinScaledGeometries& geoms, int maxCmdWidth,
                                                                                  int maxShortcutWidth) {
    MenuItemColumnsLayout layout;

    const int leftMargin = option->rect.left() + option->maxIconWidth + geoms.menuCol.leftMargin;
    const int rightMargin = option->rect.right() - geoms.menuCol.rightMargin;

    const int col3_width = maxShortcutWidth;
    const int col3_left = rightMargin - col3_width;

    const int col2_right = col3_left - geoms.menuCol.gapBetweenCmdShortcut;
    const int col2_left = col2_right - maxCmdWidth;

    const int col1_width = col2_left - leftMargin - geoms.menuCol.flexibleLabelGap;

    layout.labelRect = QRect(leftMargin, option->rect.top(), col1_width, option->rect.height());
    layout.aliasRect = QRect(col2_left, option->rect.top(), maxCmdWidth, option->rect.height());
    layout.shortcutRect = QRect(col3_left, option->rect.top(), col3_width, option->rect.height());

    return layout;
}

LC_SkinWidgetsLayoutResolver::TabLayout LC_SkinWidgetsLayoutResolver::resolveTabLayout(const QStyleOptionTab* option, const QWidget* widget,
                                                                 const SkinScaledGeometries& geoms, const QStyle* style, bool isLeftAccentBar,
                                                                 bool tabStripeAtBottom) {
    TabLayout layout;
    QRect tabRect = option->rect;

    // Calculate horizontal tab overlaps
    const int tabOverlap = style->pixelMetric(QStyle::PM_TabBarTabOverlap, option, widget);
    const bool isLastTab = (option->position == QStyleOptionTab::End || option->position == QStyleOptionTab::OnlyOneTab);
    if (tabOverlap > 0 && !isLastTab) {
        if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth || option->shape == QTabBar::RoundedSouth ||
            option->shape == QTabBar::TriangularSouth) {
            tabRect.adjust(0, 0, tabOverlap, 0);
        }
        else {
            tabRect.adjust(0, 0, 0, tabOverlap);
        }
    }

    const int extra = qRound(geoms.tab.activeExtraHeight);
    int overlap = style->pixelMetric(QStyle::PM_TabBarBaseOverlap, option, widget);
    if (overlap <= 0)
        overlap = 2;

    const bool selected = (option->state & QStyle::State_Selected);
    if (selected) {
        if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
            tabRect.adjust(0, 0, 0, overlap);
        }
        else if (option->shape == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth) {
            tabRect.adjust(0, -overlap, 0, 0);
        }
        else if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
            tabRect.adjust(0, 0, overlap, 0);
        }
        else if (option->shape == QTabBar::RoundedEast || option->shape == QTabBar::TriangularEast) {
            tabRect.adjust(-overlap, 0, 0, 0);
        }
    }
    else {
        if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
            tabRect.adjust(0, extra, 0, 0);
        }
        else if (option->shape == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth) {
            tabRect.adjust(0, 0, 0, -extra);
        }
        else if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
            tabRect.adjust(extra, 0, 0, 0);
        }
        else if (option->shape == QTabBar::RoundedEast || option->shape == QTabBar::TriangularEast) {
            tabRect.adjust(0, 0, -extra, 0);
        }
    }

    layout.adjustedRect = QRectF(tabRect);
    layout.paths = getTabPaths(layout.adjustedRect, option->shape, selected || (option->state & QStyle::State_MouseOver), geoms);

    if (selected) {
        layout.hasActiveStripe = true;
        const qreal r = geoms.tab.roundedRadius;
        const qreal inset = geoms.tab.beveledInset;

        const TabShape shape = (option->shape == QTabBar::TriangularNorth || option->shape == QTabBar::TriangularSouth || option->shape ==
                                   QTabBar::TriangularWest || option->shape == QTabBar::TriangularEast)
                                   ? TabShape::Beveled
                                   : TabShape::Rounded;

        if (isLeftAccentBar && !tabStripeAtBottom) {
            const bool isHorizontal = (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth || option->shape
                == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth);

            if (isHorizontal) {
                if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
                    if (shape == TabShape::Beveled) {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset,
                                    layout.adjustedRect.bottom() - geoms.tab.marginDefault),
                            QPointF(layout.adjustedRect.left() + inset + geoms.tab.stripeOffset,
                                    layout.adjustedRect.top() + geoms.tab.marginDefault));
                    }
                    else {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset,
                                    layout.adjustedRect.bottom() - geoms.tab.marginDefault),
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset, layout.adjustedRect.top() + r));
                    }
                }
                else {
                    // South
                    if (shape == TabShape::Beveled) {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset,
                                    layout.adjustedRect.top() + geoms.tab.marginDefault),
                            QPointF(layout.adjustedRect.left() + inset + geoms.tab.stripeOffset,
                                    layout.adjustedRect.bottom() - geoms.tab.marginDefault));
                    }
                    else {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset,
                                    layout.adjustedRect.top() + geoms.tab.marginDefault),
                            QPointF(layout.adjustedRect.left() + geoms.tab.stripeOffset, layout.adjustedRect.bottom() - r));
                    }
                }
            }
            else {
                if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
                    if (shape == TabShape::Beveled) {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.right() - geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + geoms.tab.stripeOffset),
                            QPointF(layout.adjustedRect.left() + geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + inset + geoms.tab.stripeOffset));
                    }
                    else {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.right() - geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + geoms.tab.stripeOffset),
                            QPointF(layout.adjustedRect.left() + r, layout.adjustedRect.top() + geoms.tab.stripeOffset));
                    }
                }
                else {
                    // East
                    if (shape == TabShape::Beveled) {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + geoms.tab.stripeOffset),
                            QPointF(layout.adjustedRect.right() - geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + inset + geoms.tab.stripeOffset));
                    }
                    else {
                        layout.stripeLine = QLineF(
                            QPointF(layout.adjustedRect.left() + geoms.tab.marginDefault,
                                    layout.adjustedRect.top() + geoms.tab.stripeOffset),
                            QPointF(layout.adjustedRect.right() - r, layout.adjustedRect.top() + geoms.tab.stripeOffset));
                    }
                }
            }
        }
        else {
            const bool isHorizontal = (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth || option->shape
                == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth);

            if (isHorizontal) {
                qreal y = 0.0;
                bool stripeIsAtBaseEdge = false;

                if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
                    if (tabStripeAtBottom) {
                        y = layout.adjustedRect.bottom() - geoms.tab.stripeOffset;
                        stripeIsAtBaseEdge = true;
                    }
                    else {
                        y = layout.adjustedRect.top() + geoms.tab.stripeOffset;
                    }
                }
                else {
                    // South
                    if (tabStripeAtBottom) {
                        y = layout.adjustedRect.top() + geoms.tab.stripeOffset;
                        stripeIsAtBaseEdge = true;
                    }
                    else {
                        y = layout.adjustedRect.bottom() - geoms.tab.stripeOffset;
                    }
                }

                qreal xStart = layout.adjustedRect.left() + geoms.tab.marginDefault;
                qreal xEnd = layout.adjustedRect.right() - geoms.tab.marginDefault;

                if (!stripeIsAtBaseEdge) {
                    if (shape == TabShape::Rounded) {
                        xStart = layout.adjustedRect.left() + r;
                        xEnd = layout.adjustedRect.right() - r;
                    }
                    else if (shape == TabShape::Beveled) {
                        xStart = layout.adjustedRect.left() + inset;
                        xEnd = layout.adjustedRect.right() - inset;
                    }
                }
                layout.stripeLine = QLineF(QPointF(xStart, y), QPointF(xEnd, y));
            }
            else {
                // Vertical
                qreal x = 0.0;
                bool stripeIsAtBaseEdge = false;

                if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
                    if (tabStripeAtBottom) {
                        x = layout.adjustedRect.right() - geoms.tab.stripeOffset;
                        stripeIsAtBaseEdge = true;
                    }
                    else {
                        x = layout.adjustedRect.left() + geoms.tab.stripeOffset;
                    }
                }
                else {
                    // East
                    if (tabStripeAtBottom) {
                        x = layout.adjustedRect.left() + geoms.tab.stripeOffset;
                        stripeIsAtBaseEdge = true;
                    }
                    else {
                        x = layout.adjustedRect.right() - geoms.tab.stripeOffset;
                    }
                }

                qreal yStart = layout.adjustedRect.top() + geoms.tab.marginDefault;
                qreal yEnd = layout.adjustedRect.bottom() - geoms.tab.marginDefault;

                if (!stripeIsAtBaseEdge) {
                    if (shape == TabShape::Rounded) {
                        yStart = layout.adjustedRect.top() + r;
                        yEnd = layout.adjustedRect.bottom() - r;
                    }
                    else if (shape == TabShape::Beveled) {
                        yStart = layout.adjustedRect.top() + inset;
                        yEnd = layout.adjustedRect.bottom() - inset;
                    }
                }
                layout.stripeLine = QLineF(QPointF(x, yStart), QPointF(x, yEnd));
            }
        }
    }

    return layout;
}

GroupBoxLayout LC_SkinWidgetsLayoutResolver::resolveGroupBoxLayout(const QStyle* style, const QStyleOptionGroupBox* option,
                                                        const SkinColors& desc, const SkinScaledGeometries& geoms, const QRect& textRect,
                                                        const QRect& checkBoxRect, const QWidget* widget) {
    GroupBoxLayout layout;
    const QRect rect = option->rect;

    QRect frameRect = style->subControlRect(QStyle::CC_GroupBox, option, QStyle::SC_GroupBoxFrame, widget);
    if (frameRect.isEmpty()) {
        frameRect = rect.adjusted(0, textRect.height() / 2, 0, 0);
    }

    const int centerY = textRect.center().y();
    layout.topY = centerY;

    if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::Plain) {
        layout.topY = textRect.bottom() + geoms.ints.scale4;
    }
    else if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::Overlapping) {
        layout.topY = textRect.bottom() - geoms.ints.scale2;
    }

    layout.drawFrameRect = frameRect.adjusted(0, 0, -1, -1);
    layout.drawFrameRect.setTop(layout.topY);
    layout.drawTotalRect = rect.adjusted(0, 0, -1, -1);

    int leftMargin = geoms.scaledMetrics.groupBoxTitleLeftPadding;
    if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::LeftStripe) {
        leftMargin += geoms.ints.scale3;
    }

    int activeCenterY = centerY;
        const int headerHeight = qMax(textRect.height(), checkBoxRect.height()) + geoms.ints.scale8;

        // Dynamic sub-geometry coordinate resolutions
        layout.headerRect = QRect(rect.left(), rect.top(), rect.width(), headerHeight);
        if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::LeftStripe) {
            layout.headerRect.setLeft(rect.left() + geoms.ints.scale3);
        }
        layout.fillHeaderRect = layout.headerRect.adjusted(1, 1, -1, 0);
        layout.underlineY = qMax(textRect.bottom(), checkBoxRect.bottom()) + geoms.ints.scale2;

        if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::HeaderBanner) {
            activeCenterY = layout.headerRect.center().y();
    }

    layout.drawCheckBoxRect = checkBoxRect;
    layout.drawCheckBoxRect.moveTop(activeCenterY - layout.drawCheckBoxRect.height() / 2);
    layout.drawCheckBoxRect.moveLeft(rect.left() + leftMargin);

    layout.drawTextRect = textRect.adjusted(geoms.scaledMetrics.groupBoxTitleLineGap, 0, -geoms.scaledMetrics.groupBoxTitleLineGap, 0);
    layout.drawTextRect.moveTop(activeCenterY - layout.drawTextRect.height() / 2);

    if (option->subControls & QStyle::SC_GroupBoxCheckBox) {
        layout.drawTextRect.moveLeft(layout.drawCheckBoxRect.right() + geoms.groupBox.checkBoxTextGap);
    }
    else {
        layout.drawTextRect.moveLeft(rect.left() + leftMargin);
    }

        // Overlay backing cover bounds (breaks borders in overlapping mode)
        layout.bgCoverRect = layout.drawTextRect.adjusted(-geoms.ints.scale2, 0, geoms.ints.scale2, geoms.ints.scale2);
        if (option->subControls & QStyle::SC_GroupBoxCheckBox) {
            layout.bgCoverRect.setLeft(layout.drawCheckBoxRect.left() - geoms.ints.scale2);
        }

    return layout;
}

QPainterPath LC_SkinWidgetsLayoutResolver::resolveSegmentedGroupPath(const QList<QToolButton*>& buttons, qreal dpr) {
    QPainterPath groupPath;
    const qreal r = qMax(2.5, static_cast<qreal>(5 - qRound(dpr))); // 5 reflects typical button Padding metric scale

    for (QToolButton* btn : buttons) {
        const QRectF rect = btn->geometry();

        const QVariant neighborsVar = btn->property(PROP_GROUP_NEIGHBORS);
        const int mask = neighborsVar.isValid() ? neighborsVar.toInt() : 0;
        const bool sameT = (mask & 0x01);
        const bool sameB = (mask & 0x02);
        const bool sameL = (mask & 0x04);
        const bool sameR = (mask & 0x08);

        const bool hasT = (mask & 0x10);
        const bool hasB = (mask & 0x20);
        const bool hasL = (mask & 0x40);
        const bool hasR = (mask & 0x80);

        const bool diffT = (hasT && !sameT);
        const bool diffB = (hasB && !sameB);
        const bool diffL = (hasL && !sameL);
        const bool diffR = (hasR && !sameR);

        const qreal rTL = (!sameT && !sameL && !diffT && !diffL) ? r : 0.0;
        const qreal rTR = (!sameT && !sameR && !diffT && !diffR) ? r : 0.0;
        const qreal rBL = (!sameB && !sameL && !diffB && !diffL) ? r : 0.0;
        const qreal rBR = (!sameB && !sameR && !diffB && !diffR) ? r : 0.0;

        const QRectF overlapRect = rect.adjusted(-0.1, -0.1, 0.1, 0.1);
        const QPainterPath btnPath = getSegmentedRectPath(overlapRect, rTL, rTR, rBL, rBR);

        groupPath = groupPath.united(btnPath);
    }

    return groupPath;
}

LC_SkinWidgetsLayoutResolver::SegmentedToolButtonLayout LC_SkinWidgetsLayoutResolver::resolveSegmentedToolButtonLayout(
    const QStyleOption* option, const SkinScaledGeometries& geoms, int mask, bool useToolButtonUnderline,
    ToolButtonIndicatorStyle indicatorStyle, const QWidget* widget) {
    SegmentedToolButtonLayout layout;

    const bool sameT = (mask & 0x01);
    const bool sameB = (mask & 0x02);
    const bool sameL = (mask & 0x04);
    const bool sameR = (mask & 0x08);

    const bool hasT = (mask & 0x10);
    const bool hasB = (mask & 0x20);
    const bool hasL = (mask & 0x40);
    const bool hasR = (mask & 0x80);

    const bool diffT = (hasT && !sameT);
    const bool diffB = (hasB && !sameB);
    const bool diffL = (hasL && !sameL);
    const bool diffR = (hasR && !sameR);

    const qreal r = qMax(2.5, static_cast<qreal>(geoms.scaledMetrics.buttonPadding - geoms.ints.scale1));
    const qreal rTL = (!sameT && !sameL && !diffT && !diffL) ? r : 0.0;
    const qreal rTR = (!sameT && !sameR && !diffT && !diffR) ? r : 0.0;
    const qreal rBL = (!sameB && !sameL && !diffB && !diffL) ? r : 0.0;
    const qreal rBR = (!sameB && !sameR && !diffB && !diffR) ? r : 0.0;

    const QRectF rectF = QRectF(option->rect).adjusted(geoms.crispOffset, geoms.crispOffset, -geoms.crispOffset, -geoms.crispOffset);
    layout.buttonPath = getSegmentedRectPath(rectF, rTL, rTR, rBL, rBR);

    const bool selected = (option->state & QStyle::State_On);
    const bool hovered = (option->state & QStyle::State_MouseOver);
    const bool sunken = (option->state & QStyle::State_Sunken);

    const bool drawDividers = selected || hovered || sunken;
    if (drawDividers) {
        const int inset = geoms.segmentedButton.dividerInset;
        if (sameR) {
            layout.dividerLines.append(QLineF(QPointF(rectF.right(), rectF.top() + inset), QPointF(rectF.right(), rectF.bottom() - inset)));
        }
        if (sameB) {
            layout.dividerLines.append(
                QLineF(QPointF(rectF.left() + inset, rectF.bottom()), QPointF(rectF.right() - inset, rectF.bottom())));
        }
    }

    if (selected && useToolButtonUnderline) {
        layout.drawIndicator = true;
        const QRect rect = option->rect;

        if (indicatorStyle == ToolButtonIndicatorStyle::ContextStripe) {
            bool verticalToolbar = false;
            if (widget && widget->parentWidget()) {
                if (const auto* toolBar = qobject_cast<const QToolBar*>(widget->parentWidget())) {
                    verticalToolbar = (toolBar->orientation() == Qt::Vertical);
                }
            }
            if (verticalToolbar) {
                layout.indicatorRect = QRectF(rect.left() + geoms.ints.scale2, rect.top() + geoms.ints.scale2, geoms.ints.scale2,
                                              rect.height() - geoms.ints.scale4);
            }
            else {
                layout.indicatorRect = QRectF(rect.left() + geoms.ints.scale2, rect.bottom() - geoms.ints.scale3,
                                              rect.width() - geoms.ints.scale4, geoms.ints.scale2);
            }
        }
        else if (indicatorStyle == ToolButtonIndicatorStyle::AccentDot) {
            layout.isIndicatorDot = true;
            layout.indicatorDotCenter = QPointF(QRectF(rect).center().x(),
                                                rect.bottom() - (geoms.tab.stripeOffset + geoms.tab.stripeThickness));
        }
        else if (indicatorStyle == ToolButtonIndicatorStyle::AccentFrame) {
            layout.indicatorRect = QRectF(rect).adjusted(geoms.crispOffset, geoms.crispOffset, -geoms.crispOffset, -geoms.crispOffset);
        }
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::ToolButtonAccessoryLayout LC_SkinWidgetsLayoutResolver::resolveToolButtonAccessoryLayout(
    const QStyleOption* option, const SkinScaledGeometries& geoms, bool useToolButtonUnderline, ToolButtonIndicatorStyle indicatorStyle,
    const QWidget* widget, bool autoRaise, bool showGripBackgroundWell) {
    ToolButtonAccessoryLayout layout;
    const QRect rect = option->rect;
    const bool selected = (option->state & QStyle::State_On);

    if (selected && autoRaise && showGripBackgroundWell) {
        layout.drawWell = true;
        if (indicatorStyle == ToolButtonIndicatorStyle::ContextStripe) {
            bool verticalToolbar = false;
            if (widget && widget->parentWidget()) {
                if (const auto* toolBar = qobject_cast<const QToolBar*>(widget->parentWidget())) {
                    verticalToolbar = (toolBar->orientation() == Qt::Vertical);
                }
            }
            if (verticalToolbar) {
                layout.wellRect = QRect(rect.left() + geoms.ints.scale1, rect.top() + geoms.ints.scale2, geoms.ints.scale5,
                                        rect.height() - geoms.ints.scale4);
            }
            else {
                layout.wellRect = QRect(rect.left() + geoms.ints.scale2, rect.bottom() - geoms.ints.scale5,
                                        rect.width() - geoms.ints.scale4, geoms.ints.scale5);
            }
        }
        else if (indicatorStyle == ToolButtonIndicatorStyle::AccentDot) {
            layout.wellRect = QRect(rect.center().x() - geoms.ints.scale6, rect.bottom() - geoms.ints.scale5, geoms.ints.scale12,
                                    geoms.ints.scale5);
        }
        else {
            layout.wellRect = rect.adjusted(geoms.ints.scale1, geoms.ints.scale1, -geoms.ints.scale1, -geoms.ints.scale1);
        }
    }

    if (selected && useToolButtonUnderline) {
        layout.drawIndicator = true;
        if (indicatorStyle == ToolButtonIndicatorStyle::ContextStripe) {
            bool verticalToolbar = false;
            if (widget && widget->parentWidget()) {
                if (const auto* toolBar = qobject_cast<const QToolBar*>(widget->parentWidget())) {
                    verticalToolbar = (toolBar->orientation() == Qt::Vertical);
                }
            }
            if (verticalToolbar) {
                layout.indicatorRect = QRectF(rect.left() + geoms.ints.scale2, rect.top() + geoms.ints.scale2, geoms.ints.scale2,
                                              rect.height() - geoms.ints.scale4);
            }
            else {
                layout.indicatorRect = QRectF(rect.left() + geoms.ints.scale2, rect.bottom() - geoms.ints.scale3,
                                              rect.width() - geoms.ints.scale4, geoms.ints.scale2);
            }
        }
        else if (indicatorStyle == ToolButtonIndicatorStyle::AccentDot) {
            layout.isIndicatorDot = true;
            layout.indicatorDotCenter = QPointF(rect.center().x(), rect.bottom() - (geoms.tab.stripeOffset + geoms.tab.stripeThickness));
        }
        else if (indicatorStyle == ToolButtonIndicatorStyle::AccentFrame) {
            layout.indicatorRect = QRectF(rect).adjusted(geoms.crispOffset, geoms.crispOffset, -geoms.crispOffset, -geoms.crispOffset);
        }
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::GripLayout LC_SkinWidgetsLayoutResolver::resolveGripLayout(const QStyleOption* option, const SkinScaledGeometries& geoms,
                                                                   bool showGripBackgroundWell, int baseHandleLen,
                                                                   bool handleIsHorizontal) {
    GripLayout layout;
    const QRect rect = option->rect;
    const QPoint cx = rect.center();

    layout.horizontalPattern = handleIsHorizontal;

    if (showGripBackgroundWell) {
        layout.showWell = true;
        const int wellWidth = qMin(geoms.splitterGrip.wellWidth, (handleIsHorizontal ? rect.height() : rect.width()) - 1);
        const int halfWell = wellWidth / 2;
        const int wellLength = baseHandleLen + geoms.splitterGrip.wellLengthOffset;
        const int halfWellLen = wellLength / 2;
        layout.wellRadius = qMax(1, geoms.ints.scale2);

        if (handleIsHorizontal) {
            layout.wellRect = QRect(cx.x() - halfWellLen, cx.y() - halfWell, wellLength, wellWidth);
        }
        else {
            layout.wellRect = QRect(cx.x() - halfWell, cx.y() - halfWellLen, wellWidth, wellLength);
        }
    }

    layout.handleLength = baseHandleLen;
    return layout;
}

LC_SkinWidgetsLayoutResolver::BranchLayout LC_SkinWidgetsLayoutResolver::resolveBranchLayout(const QStyleOption* option, const SkinScaledGeometries& geoms) {
    LC_SkinWidgetsLayoutResolver::BranchLayout layout;
    const QRect rect = option->rect;
    const int midH = rect.x() + rect.width() / 2;
    const int midV = rect.y() + rect.height() / 2;
    layout.center = QPoint(midH, midV);

    if (option->state & QStyle::State_Sibling) {
        layout.drawLines = true;
        layout.verticalLine = QLine(midH, rect.top(), midH, rect.bottom());
    }

    if (option->state & QStyle::State_Item) {
        layout.drawLines = true;
        if (option->direction == Qt::RightToLeft) {
            layout.horizontalLine = QLine(rect.left(), midV, midH, midV);
        }
        else {
            layout.horizontalLine = QLine(midH, midV, rect.right(), midV);
        }
    }

    int size = geoms.scaledMetrics.treeBranchIndicatorSize;
    if (size < 0) {
        size = geoms.scaledMetrics.treeIndentation * 40 / 100;
    }
    size = qMin(size, rect.width() - geoms.ints.scale4);
    layout.size = size;

    if (option->state & QStyle::State_Children) {
        layout.plusMinusBox = QRect(midH - size / 2, midV - size / 2, size, size);
        layout.isExpanded = (option->state & QStyle::State_Open);

        if (option->state & QStyle::State_Open) {
            layout.chevronPolygon << QPointF(midH - size / 2.0, midV - size / 3.0) << QPointF(midH, midV + size / 3.0) << QPointF(
                midH + size / 2.0, midV - size / 3.0);
        }
        else {
            layout.chevronPolygon << QPointF(midH - size / 3.0, midV - size / 2.0) << QPointF(midH + size / 3.0, midV) << QPointF(
                midH - size / 3.0, midV + size / 2.0);
        }
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::TabWidgetFrameClipLayout LC_SkinWidgetsLayoutResolver::resolveTabWidgetFrameClipLayout(
    const QStyleOption* option, const SkinScaledGeometries& geoms, const QWidget* widget, bool tabStripeAtBottom) {
    TabWidgetFrameClipLayout layout;
    if (tabStripeAtBottom && widget) {
        const QTabBar* bar = widget->findChild<const QTabBar*>();
        if (bar && bar->currentIndex() >= 0 && bar->isVisible()) {
            const QRect activeTabRect = bar->tabRect(bar->currentIndex());
            const QPoint localPos = widget->mapFromGlobal(bar->mapToGlobal(activeTabRect.topLeft()));
            const QRect mappedTabRect(localPos, activeTabRect.size());

            const int clipHeight = geoms.ints.scale4;
            if (option->rect.top() <= mappedTabRect.bottom() && option->rect.top() >= mappedTabRect.bottom() - clipHeight) {
                layout.clipRegion = QRegion(option->rect);
                const int offset1 = geoms.ints.scale1;
                const int offset2 = geoms.ints.scale2;
                const QRect overlapRect(mappedTabRect.left() + offset1, option->rect.top(), mappedTabRect.width() - offset2, offset2);
                layout.clipRegion -= overlapRect;
                layout.needClipping = true;
            }
        }
    }
    return layout;
}

LC_SkinWidgetsLayoutResolver::SpinBoxProgressBarLayout LC_SkinWidgetsLayoutResolver::resolveSpinBoxProgressBarLayout(
    const QStyle *style,
    const QRect &rect,
    const SkinScaledGeometries &geoms,
    qreal pct,
    bool hasFocus,
    const QWidget *widget,
    const QAbstractSpinBox *spinBox,
    bool useFocusedInputGlow) {

    SpinBoxProgressBarLayout layout;
    QRect editRect = rect;

    if (spinBox && widget) {
        QStyleOptionSpinBox spinBoxOpt;
        spinBoxOpt.initFrom(spinBox);
        QRect parentEditRect = style->subControlRect(QStyle::CC_SpinBox, &spinBoxOpt, QStyle::SC_SpinBoxEditField, spinBox);

        if (widget == spinBox) {
            editRect = parentEditRect;
        } else {
            QPoint localTopLeft = widget->mapFromParent(parentEditRect.topLeft());
            QPoint localBottomRight = widget->mapFromParent(parentEditRect.bottomRight());
            editRect = QRect(localTopLeft, localBottomRight);
        }
    }

    layout.editRect = editRect;

    const int verticalOffset = (hasFocus && useFocusedInputGlow) ? geoms.ints.scale2 : 0;
    const int horizontalOffset = (hasFocus && useFocusedInputGlow) ? geoms.ints.scale2 : 0;

    const int topMargin = verticalOffset;
    const int bottomMargin = verticalOffset;
    const int leftMargin = horizontalOffset;
    const int rightMargin = horizontalOffset;

    layout.bgRect = QRect(editRect.left() + leftMargin,
                          editRect.top() + topMargin,
                          editRect.width() - (leftMargin + rightMargin),
                          editRect.height() - (topMargin + bottomMargin));

    const int fillWidth = qRound(layout.bgRect.width() * pct);

    layout.fillRect = QRect(layout.bgRect.left(),
                            layout.bgRect.top(),
                            fillWidth,
                            layout.bgRect.height());

    const int cursorWidth = geoms.ints.scale2;
    layout.cursorRect = QRect(layout.fillRect.right() - cursorWidth, layout.fillRect.top(), cursorWidth, layout.fillRect.height());

    return layout;
}


LC_SkinWidgetsLayoutResolver::ScrollBarLayout LC_SkinWidgetsLayoutResolver::resolveScrollBarLayout(
    const QStyle *style,
    const QStyleOptionSlider *option,
    const SkinScaledGeometries &geoms,
    const QWidget *widget,
    bool transparentScrollbars) {

    ScrollBarLayout layout;
    layout.subPageRect = style->subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarSubPage, widget);
    layout.addPageRect = style->subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarAddPage, widget);
    layout.handleRect  = style->subControlRect(QStyle::CC_ScrollBar, option, QStyle::SC_ScrollBarSlider, widget);

    if (layout.handleRect.isValid()) {
        layout.handleValid = true;
        if (transparentScrollbars) {
            const int pad = geoms.scrollbar.paddingTransparent;
            layout.handleRadius = geoms.scrollbar.radiusTransparent;
            layout.adjustedHandleRect = QRectF(layout.handleRect).adjusted(pad, pad, -pad, -pad);
        } else {
            const int pad = geoms.scrollbar.paddingStandard;
            layout.handleRadius = geoms.scrollbar.radiusStandard;
            layout.adjustedHandleRect = QRectF(layout.handleRect).adjusted(pad, pad, -pad, -pad);
        }
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::ArrowLayout LC_SkinWidgetsLayoutResolver::resolveArrowLayout(
    QStyle::PrimitiveElement element,
    const QStyleOption *option,
    const SkinScaledGeometries &geoms,
    const QWidget *widget,
    bool customVectorIcons) {

    ArrowLayout layout;
    const QRect rect = option->rect;
    const qreal cx = rect.center().x();
    const qreal cy = rect.center().y();

    const bool isTabBarArrow = widget && widget->parentWidget() && qobject_cast<const QTabBar*>(widget->parentWidget());
    const bool isMenuArrow = widget && qobject_cast<const QMenu*>(widget);

    if (isMenuArrow && customVectorIcons) {
        if (element == QStyle::PE_IndicatorArrowRight) {
            layout.polygon << QPointF(cx - 1.5, cy - 3.0)
                           << QPointF(cx + 1.5, cy)
                           << QPointF(cx - 1.5, cy + 3.0);
            layout.isPolygon = true;
        } else if (element == QStyle::PE_IndicatorArrowLeft) {
            layout.polygon << QPointF(cx + 1.5, cy - 3.0)
                           << QPointF(cx - 1.5, cy)
                           << QPointF(cx + 1.5, cy + 3.0);
            layout.isPolygon = true;
        }
    } else if (isTabBarArrow) {
        layout.isLines = true;
        const int size = qMin(geoms.ints.scale5, rect.width() / 3);

        if (element == QStyle::PE_IndicatorArrowLeft) {
            layout.line1 = QLineF(cx + size / 2.0, cy - size, cx - size / 2.0, cy);
            layout.line2 = QLineF(cx - size / 2.0, cy, cx + size / 2.0, cy + size);
        } else if (element == QStyle::PE_IndicatorArrowRight) {
            layout.line1 = QLineF(cx - size / 2.0, cy - size, cx + size / 2.0, cy);
            layout.line2 = QLineF(cx + size / 2.0, cy, cx - size / 2.0, cy + size);
        } else if (element == QStyle::PE_IndicatorArrowUp) {
            layout.line1 = QLineF(cx - size, cy + size / 2.0, cx, cy - size / 2.0);
            layout.line2 = QLineF(cx, cy - size / 2.0, cx + size, cy + size / 2.0);
        } else if (element == QStyle::PE_IndicatorArrowDown) {
            layout.line1 = QLineF(cx - size, cy - size / 2.0, cx, cy + size / 2.0);
            layout.line2 = QLineF(cx, cy + size / 2.0, cx + size, cy - size / 2.0);
        }
    } else {
        layout.isLines = true;
        if (element == QStyle::PE_IndicatorArrowDown) {
            layout.line1 = QLineF(cx - 3.0, cy - 1.0, cx, cy + 2.0);
            layout.line2 = QLineF(cx, cy + 2.0, cx + 3.0, cy - 1.0);
        } else if (element == QStyle::PE_IndicatorArrowUp) {
            layout.line1 = QLineF(cx - 3.0, cy + 2.0, cx, cy - 1.0);
            layout.line2 = QLineF(cx, cy - 1.0, cx + 3.0, cy + 2.0);
        } else if (element == QStyle::PE_IndicatorArrowLeft) {
            layout.line1 = QLineF(cx + 2.0, cy - 3.0, cx - 1.0, cy);
            layout.line2 = QLineF(cx - 1.0, cy, cx + 2.0, cy + 3.0);
        } else if (element == QStyle::PE_IndicatorArrowRight) {
            layout.line1 = QLineF(cx - 2.0, cy - 3.0, cx + 1.0, cy);
            layout.line2 = QLineF(cx + 1.0, cy, cx - 2.0, cy + 3.0);
        }
    }

    return layout;
}

LC_SkinWidgetsLayoutResolver::TitleBarLayout LC_SkinWidgetsLayoutResolver::resolveTitleBarMetrics(const QRect &windowRect, const SkinScaledGeometries &geoms) {
    TitleBarLayout metrics;
    const int titleHeight = geoms.ints.scale24; // 24px title area
    metrics.titleBarRect = QRect(windowRect.left(), windowRect.top(), windowRect.width(), titleHeight);

    const int handleW = geoms.ints.scale12;
    const int handleH = geoms.ints.scale3;
    metrics.dragHandleRect = QRect(windowRect.center().x() - handleW / 2,
                                   windowRect.top() + (titleHeight - handleH) / 2,
                                   handleW, handleH);

    const int closeSize = geoms.ints.scale16; // Highly visible close button
    const int closeMargin = geoms.ints.scale6;
    metrics.closeButtonRect = QRect(windowRect.right() - closeSize - closeMargin,
                                    windowRect.top() + (titleHeight - closeSize) / 2,
                                    closeSize, closeSize);

    return metrics;
}


void LC_SkinWidgetsLayoutResolver::drawTitleBar(QPainter *painter,
                                                const TitleBarLayout &metrics,
                                                const SkinColors &desc,
                                                const SkinScaledGeometries &geoms,
                                                bool active,
                                                bool closeHovered,
                                                CloseButtonColorPolicy closePolicy,
                                                const QString &titleText) {
    Q_UNUSED(titleText);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 1. Paint centered drag handle (hidden on narrow windows to prevent text collision)
    const bool showHandle = (metrics.titleBarRect.width() >= geoms.ints.scale24 * 8);
    if (showHandle) {
        QColor gripColor = active ? desc.common.highlightColor : desc.splitter.splitterGripColorIdle;
        if (!active) {
            gripColor.setAlpha(115); // Focus-aware dimming: 45% opacity
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(gripColor);
        painter->drawRoundedRect(metrics.dragHandleRect, metrics.dragHandleRect.height() / 2.0, metrics.dragHandleRect.height() / 2.0);
    }

    // 2. Paint Close Button Vector Icon
    QColor closeColor = desc.splitter.splitterGripColorIdle;
    if (!active) {
        closeColor.setAlpha(115);
    }

    if (closeHovered) {
        switch (closePolicy) {
            case CloseButtonColorPolicy::AccentColor:
                closeColor = desc.common.highlightColor;
                break;
            case CloseButtonColorPolicy::MutedRed:
                closeColor = QColor(224, 108, 117);
                break;
            case CloseButtonColorPolicy::VibrantRed:
                closeColor = QColor(255, 59, 48);
                break;
            case CloseButtonColorPolicy::MutedNeutral:
                closeColor = desc.frame.borderHovered;
                break;
        }
    }

    painter->setPen(QPen(closeColor, 1.8, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    const int inset = geoms.ints.scale3; // Adjusted inset margin
    const QRect cRect = metrics.closeButtonRect.adjusted(inset, inset, -inset, -inset);
    painter->drawLine(cRect.topLeft(), cRect.bottomRight());
    painter->drawLine(cRect.topRight(), cRect.bottomLeft());

    painter->restore();
}

LC_SkinWidgetsLayoutResolver::TitleBarButtonLayout
LC_SkinWidgetsLayoutResolver::resolveTitleBarButtonLayout(
        const QRect &titleRect,
        const SkinScaledGeometries &geoms,
        QDockWidget::DockWidgetFeatures features,
        bool isVertical){
    TitleBarButtonLayout layout;

    // Resolve title bar thickness along its narrow layout axis
    const int thickness = isVertical ? titleRect.width() : titleRect.height();

    // Ensure button size never exceeds titlebar thickness, maintaining a minimum 2px margin on both sides
    const int maxAllowedBtnSize = qMax(geoms.ints.scale8, thickness - geoms.ints.scale4);
    const int btnSize = qMin(geoms.scaledMetrics.titleBarButtonSize, maxAllowedBtnSize);

    // Dynamic auto-centering margin
    const int margin  = (geoms.scaledMetrics.dockWidgetTitleBarButtonMargin >= 0 &&
                         (thickness - btnSize) > (geoms.scaledMetrics.dockWidgetTitleBarButtonMargin * 2))
                        ? geoms.scaledMetrics.dockWidgetTitleBarButtonMargin
                      : qMax(geoms.ints.scale1, (thickness - btnSize) / 2);

    const int spacing   = geoms.ints.scale2;

    if (isVertical) {
        // Vertical layout: stack buttons vertically at the top
        const int left = titleRect.left() + qMax(0, (titleRect.width() - btnSize) / 2);
        int top = titleRect.top() + margin;

        if (features & QDockWidget::DockWidgetClosable) {
            layout.closeRect = QRect(left, top, btnSize, btnSize);
            top += (btnSize + spacing);
        }
        if (features & QDockWidget::DockWidgetFloatable) {
            layout.floatRect = QRect(left, top, btnSize, btnSize);
            top += (btnSize + spacing);
        }

        const int leftSpacing = geoms.groupBox.titleLeftSpacing;
        layout.textRect = QRect(titleRect.left(), top + leftSpacing,
                                titleRect.width(), qMax(0, titleRect.bottom() - top - leftSpacing));
    } else {
        // Horizontal layout: stack buttons horizontally on the right
        const int top = titleRect.top() + margin;
        int right = titleRect.right() - margin;

        if (features & QDockWidget::DockWidgetClosable) {
            layout.closeRect = QRect(right - btnSize, top, btnSize, btnSize);
            right -= (btnSize + spacing);
        }
        if (features & QDockWidget::DockWidgetFloatable) {
            layout.floatRect = QRect(right - btnSize, top, btnSize, btnSize);
            right -= (btnSize + spacing);
        }

        const int leftSpacing = geoms.groupBox.titleLeftSpacing;
        layout.textRect = QRect(titleRect.left() + leftSpacing, titleRect.top(),
                                qMax(0, right - titleRect.left() - leftSpacing), titleRect.height());
    }

    return layout;
}
