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


#include "lc_metrics_repository.h"


QJsonObject LC_MetricsRepository::configToJson(const StyleMetricsConfig& config) const {
    QJsonObject root;


    root["scrollBarWidth"] = config.scrollBarWidth;
    root["treeIndentation"] = config.treeIndentation;
    root["buttonPadding"] = config.buttonPadding;
    root["scrollBarMinLength"] = config.scrollBarMinLength;
    root["dockTitleBarHeight"] = config.dockTitleBarHeight;
    root["splitterWidth"] = config.splitterWidth;
    root["itemViewRowPadding"] = config.itemViewRowPadding;
    root["flatDisabledText"] = config.flatDisabledText;
    root["hideShortcutUnderlines"] = config.hideShortcutUnderlines;

    root["tabBarTabOverlap"] = config.tabBarTabOverlap;
    root["tabBarTabBaseOverlap"] = config.tabBarTabBaseOverlap;
    root["tabBarTabHSpace"] = config.tabBarTabHSpace;
    root["tabBarTabVSpace"] = config.tabBarTabVSpace;
    root["tabActiveExtraHeight"] = config.tabActiveExtraHeight;
    root["tabCloseIndicatorSize"] = config.tabCloseIndicatorSize;

    root["menuVerticalPadding"] = config.menuVerticalPadding;
    root["menuHorizontalPadding"] = config.menuHorizontalPadding;
    root["menuBorderWidth"] = config.menuBorderWidth;
    root["headerDefaultHeight"] = config.headerDefaultHeight;
    root["sliderHandleLength"] = config.sliderHandleLength;

    root["dockWidgetTitleBarButtonMargin"] = config.dockWidgetTitleBarButtonMargin;
    root["sliderControlThickness"] = config.sliderControlThickness;
    root["focusFrameHMargin"] = config.focusFrameHMargin;
    root["focusFrameVMargin"] = config.focusFrameVMargin;
    root["layoutMargin"] = config.layoutMargin;
    root["layoutSpacing"] = config.layoutSpacing;
    root["toolbarItemSpacing"] = config.toolbarItemSpacing;
    root["toolbarSeparatorWidth"] = config.toolbarSeparatorWidth;
    root["menuBarItemSpacing"] = config.menuBarItemSpacing;
    root["menuBarVerticalMargin"] = config.menuBarVerticalMargin;

    root["indicatorBoxSize"] = config.indicatorBoxSize;
    root["indicatorLabelSpacing"] = config.indicatorLabelSpacing;
    root["titleBarButtonSize"] = config.titleBarButtonSize;
    root["subMenuOverlap"] = config.subMenuOverlap;

    root["menuSupportsIcons"] = config.menuSupportsIcons;
    root["menuAllowActiveAndDisabled"] = config.menuAllowActiveAndDisabled;

    root["group_box_title_left_padding"] = config.groupBoxTitleLeftPadding;
    root["group_box_title_line_gap"]     = config.groupBoxTitleLineGap;
    root["mnemonic_underline_mode"] = static_cast<int>(config.mnemonicUnderlineMode);

    root["tree_branch_indicator_size"] = config.treeBranchIndicatorSize;
    root["splitter_handle_length"]       = config.splitterHandleLength;

    root["toolbar_popup_delay"]        = config.toolbarPopupDelay;
    root["drag_cursor_style"] = static_cast<int>(config.dragCursorStyle);

    return root;
}

bool LC_MetricsRepository::configFromJson(const QJsonObject& json, StyleMetricsConfig& config) const {

    config.scrollBarWidth = json["scrollBarWidth"].toInt(12);
    config.treeIndentation = json["treeIndentation"].toInt(12);
    config.buttonPadding = json["buttonPadding"].toInt(5);
    config.scrollBarMinLength = json["scrollBarMinLength"].toInt(25);
    config.dockTitleBarHeight = json["dockTitleBarHeight"].toInt(18);
    config.splitterWidth = json["splitterWidth"].toInt(4);
    config.itemViewRowPadding = json["itemViewRowPadding"].toInt(4);
    config.flatDisabledText = json["flatDisabledText"].toBool(true);
    config.hideShortcutUnderlines = json["hideShortcutUnderlines"].toBool(true);

    config.tabBarTabOverlap = json["tabBarTabOverlap"].toInt(1);
    config.tabBarTabBaseOverlap = json["tabBarTabBaseOverlap"].toInt(1);
    config.tabBarTabHSpace = json["tabBarTabHSpace"].toInt(8);
    config.tabBarTabVSpace = json["tabBarTabVSpace"].toInt(4);
    config.tabActiveExtraHeight = json["tabActiveExtraHeight"].toInt(13);
    config.tabCloseIndicatorSize = json["tabCloseIndicatorSize"].toInt(14);

    config.menuVerticalPadding = json["menuVerticalPadding"].toInt(3);
    config.menuHorizontalPadding = json["menuHorizontalPadding"].toInt(4);
    config.menuBorderWidth = json["menuBorderWidth"].toInt(1);
    config.headerDefaultHeight = json["headerDefaultHeight"].toInt(22);
    config.sliderHandleLength = json["sliderHandleLength"].toInt(15);

    config.dockWidgetTitleBarButtonMargin = json["dockWidgetTitleBarButtonMargin"].toInt(-1);
    config.sliderControlThickness = json["sliderControlThickness"].toInt(12);
    config.focusFrameHMargin = json["focusFrameHMargin"].toInt(1);
    config.focusFrameVMargin = json["focusFrameVMargin"].toInt(1);
    config.layoutMargin = json["layoutMargin"].toInt(6);
    config.layoutSpacing = json["layoutSpacing"].toInt(6);
    config.toolbarItemSpacing = json["toolbarItemSpacing"].toInt(3);
    config.toolbarSeparatorWidth = json["toolbarSeparatorWidth"].toInt(4);
    config.menuBarItemSpacing = json["menuBarItemSpacing"].toInt(6);
    config.menuBarVerticalMargin = json["menuBarVerticalMargin"].toInt(2);

    config.indicatorBoxSize = json["indicatorBoxSize"].toInt(12);
    config.indicatorLabelSpacing = json["indicatorLabelSpacing"].toInt(4);
    config.titleBarButtonSize = json["titleBarButtonSize"].toInt(16);
    config.subMenuOverlap = json["subMenuOverlap"].toInt(-2);

    config.menuSupportsIcons = json["menuSupportsIcons"].toBool(true);
    config.menuAllowActiveAndDisabled = json["menuAllowActiveAndDisabled"].toBool(false);

    config.groupBoxTitleLeftPadding = json["group_box_title_left_padding"].toInt(8);
    config.groupBoxTitleLineGap     = json["group_box_title_line_gap"].toInt(6);
    config.mnemonicUnderlineMode = static_cast<MnemonicUnderlineMode>(json["mnemonic_underline_mode"].toInt(static_cast<int>(MnemonicUnderlineMode::StickyToggle)));

    config.treeBranchIndicatorSize = json["tree_branch_indicator_size"].toInt(8);
    config.splitterHandleLength      = json["splitter_handle_length"].toInt(30);

    config.toolbarPopupDelay       = json["toolbar_popup_delay"].toInt(400);
    config.dragCursorStyle = static_cast<DragCursorStyle>(json["drag_cursor_style"].toInt(static_cast<int>(DragCursorStyle::SizeAll)));

    return true;
}
