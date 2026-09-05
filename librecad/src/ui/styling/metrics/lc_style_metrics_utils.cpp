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


#include "lc_style_metrics_utils.h"
void LC_StyleMetricsUtils::initializeDefault(StyleMetricsConfig &config) {
    config.name = DEFAULT_THEME_NAME;
    applyDensityPreset(config, "standard");
}

void LC_StyleMetricsUtils::applyDensityPreset(StyleMetricsConfig &config, const QString &presetName) {
    if (presetName == "compact") {
        config.scrollBarWidth = 10;
        config.treeIndentation = 12;
        config.buttonPadding = 4;
        config.itemViewRowPadding = 2;
        config.scrollBarMinLength = 25;
        config.splitterWidth = 3;
        config.dockTitleBarHeight = 16;
        config.tabBarTabHSpace = 4;
        config.tabBarTabVSpace = 2;
        config.tabBarTabBaseOverlap = 0;
        config.dockWidgetTitleBarButtonMargin = 1;
        config.sliderControlThickness = 8;
        config.focusFrameHMargin = 0;
        config.focusFrameVMargin = 0;
        config.toolbarItemSpacing = 2;
        config.toolbarSeparatorWidth = 3;
        config.menuBarItemSpacing = 4;
        config.menuBarVerticalMargin = 1;
        config.indicatorBoxSize = 10;
        config.indicatorLabelSpacing = 3;
        config.titleBarButtonSize = 12;
        config.subMenuOverlap = -3;
        config.flatDisabledText = true;
        config.hideShortcutUnderlines = true;
        config.tabBarTabOverlap = 0;
        config.tabCloseIndicatorSize = 10;
        config.menuVerticalPadding = 2;
        config.menuHorizontalPadding = 3;
        config.menuBorderWidth = 1;
        config.headerDefaultHeight = 18;
        config.sliderHandleLength = 10;
        config.splitterHandleLength = 20;

        config.groupBoxTitleLeftPadding = 6;
        config.groupBoxTitleLineGap = 4;
        config.treeBranchIndicatorSize = 6;

        config.menuSupportsIcons = false;
        config.menuAllowActiveAndDisabled = false;
    }
    else if (presetName == "standard") {
        config.scrollBarWidth = 15;
        config.treeIndentation = 20;
        config.buttonPadding = 6;
        config.itemViewRowPadding = 4;
        config.scrollBarMinLength = 20;
        config.splitterWidth = 5;
        config.dockTitleBarHeight = 24;
        config.tabBarTabHSpace = 12;
        config.tabBarTabVSpace = 6;
        config.tabBarTabBaseOverlap = 2;
        config.dockWidgetTitleBarButtonMargin = 2;
        config.sliderControlThickness = 15;
        config.focusFrameHMargin = 2;
        config.focusFrameVMargin = 2;
        config.toolbarItemSpacing = 6;
        config.toolbarSeparatorWidth = 6;
        config.menuBarItemSpacing = 8;
        config.menuBarVerticalMargin = 4;
        config.indicatorBoxSize = 12;
        config.indicatorLabelSpacing = 6;
        config.titleBarButtonSize = 16;
        config.subMenuOverlap = -2;
        config.flatDisabledText = false;
        config.hideShortcutUnderlines = false;
        config.tabBarTabOverlap = 1;
        config.tabCloseIndicatorSize = 14;
        config.menuVerticalPadding = 3;
        config.menuHorizontalPadding = 5;
        config.menuBorderWidth = 1;
        config.headerDefaultHeight = 22;
        config.sliderHandleLength = 15;
        config.splitterHandleLength = 30;

        config.treeBranchIndicatorSize = 8;
        config.groupBoxTitleLeftPadding = 8;
        config.groupBoxTitleLineGap = 6;

        config.menuSupportsIcons = true;
        config.menuAllowActiveAndDisabled = false;
    }
    else if (presetName == "spacious") {
        config.scrollBarWidth = 18;
        config.treeIndentation = 26;
        config.buttonPadding = 8;
        config.itemViewRowPadding = 6;
        config.scrollBarMinLength = 30;
        config.splitterWidth = 6;
        config.dockTitleBarHeight = 28;
        config.tabBarTabHSpace = 16;
        config.tabBarTabVSpace = 8;
        config.tabBarTabBaseOverlap = 3;
        config.dockWidgetTitleBarButtonMargin = 3;
        config.sliderControlThickness = 20;
        config.focusFrameHMargin = 3;
        config.focusFrameVMargin = 3;
        config.toolbarItemSpacing = 8;
        config.toolbarSeparatorWidth = 8;
        config.menuBarItemSpacing = 10;
        config.menuBarVerticalMargin = 6;
        config.indicatorBoxSize = 16;
        config.indicatorLabelSpacing = 8;
        config.titleBarButtonSize = 20;
        config.subMenuOverlap = 0;
        config.flatDisabledText = false;
        config.hideShortcutUnderlines = false;
        config.tabBarTabOverlap = 3;
        config.tabCloseIndicatorSize = 16;
        config.menuVerticalPadding = 5;
        config.menuHorizontalPadding = 8;
        config.menuBorderWidth = 2;
        config.headerDefaultHeight = 28;
        config.sliderHandleLength = 20;
        config.splitterHandleLength = 40;

        config.groupBoxTitleLeftPadding = 12;
        config.groupBoxTitleLineGap = 8;
        config.treeBranchIndicatorSize = 10;

        config.menuSupportsIcons = true;
        config.menuAllowActiveAndDisabled = true;
    }
}
