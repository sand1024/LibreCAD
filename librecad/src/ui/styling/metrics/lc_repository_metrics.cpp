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

#include "lc_repository_metrics.h"

QJsonObject LC_RepositoryMetrics::configToJson(const StyleMetricsConfig& config) const {
    QJsonObject root;

    root["scroll_bar_width"] = config.scrollBarWidth;
    root["tree_indentation"] = config.treeIndentation;
    root["button_padding"] = config.buttonPadding;
    root["scroll_bar_min_length"] = config.scrollBarMinLength;
    root["dock_title_bar_height"] = config.dockTitleBarHeight;
    root["splitter_width"] = config.splitterWidth;
    root["item_view_row_padding"] = config.itemViewRowPadding;
    root["flat_disabled_text"] = config.flatDisabledText;
    root["hide_shortcut_underlines"] = config.hideShortcutUnderlines;

    root["tab_bar_tab_overlap"] = config.tabBarTabOverlap;
    root["tab_bar_tab_base_overlap"] = config.tabBarTabBaseOverlap;
    root["tab_bar_tab_h_space"] = config.tabBarTabHSpace;
    root["tab_bar_tab_v_space"] = config.tabBarTabVSpace;
    root["tab_active_extra_height"] = config.tabActiveExtraHeight;
    root["tab_close_indicator_size"] = config.tabCloseIndicatorSize;

    root["menu_vertical_padding"] = config.menuVerticalPadding;
    root["menu_horizontal_padding"] = config.menuHorizontalPadding;
    root["menu_border_width"] = config.menuBorderWidth;
    root["header_default_height"] = config.headerDefaultHeight;
    root["slider_handle_length"] = config.sliderHandleLength;

    root["dock_widget_title_bar_button_margin"] = config.dockWidgetTitleBarButtonMargin;
    root["slider_control_thickness"] = config.sliderControlThickness;
    root["focus_frame_h_margin"] = config.focusFrameHMargin;
    root["focus_frame_v_margin"] = config.focusFrameVMargin;
    root["layout_margin"] = config.layoutMargin;
    root["layout_spacing"] = config.layoutSpacing;
    root["toolbar_item_spacing"] = config.toolbarItemSpacing;
    root["toolbar_separator_width"] = config.toolbarSeparatorWidth;
    root["menu_bar_item_spacing"] = config.menuBarItemSpacing;
    root["menu_bar_vertical_margin"] = config.menuBarVerticalMargin;

    root["indicator_box_size"] = config.indicatorBoxSize;
    root["indicator_label_spacing"] = config.indicatorLabelSpacing;
    root["title_bar_button_size"] = config.titleBarButtonSize;
    root["sub_menu_overlap"] = config.subMenuOverlap;

    root["menu_supports_icons"] = config.menuSupportsIcons;
    root["menu_allow_active_and_disabled"] = config.menuAllowActiveAndDisabled;

    root["group_box_title_left_padding"] = config.groupBoxTitleLeftPadding;
    root["group_box_title_line_gap"] = config.groupBoxTitleLineGap;
    root["mnemonic_underline_mode"] = static_cast<int>(config.mnemonicUnderlineMode);

    root["tree_branch_indicator_size"] = config.treeBranchIndicatorSize;
    root["splitter_handle_length"] = config.splitterHandleLength;

    root["toolbar_popup_delay"] = config.toolbarPopupDelay;
    root["drag_cursor_style"] = static_cast<int>(config.dragCursorStyle);

    root["menu_bar_horizontal_margin"] = config.menuBarHorizontalMargin;
    root["text_cursor_width"] = config.textCursorWidth;
    root["tab_bar_close_button_position"] = static_cast<int>(config.tabBarCloseButtonPosition);
    root["toolbar_handle_extent"] = config.toolbarHandleExtent;
    root["tab_bar_scroll_button_width"] = config.tabBarScrollButtonWidth;

    root["menu_icon_size"] = config.menuIconSize;
    root["button_icon_size"] = config.buttonIconSize;
    root["toolbar_icon_size"] = config.toolBarIconSize;

    return root;
}

bool LC_RepositoryMetrics::configFromJson(const QJsonObject& json, StyleMetricsConfig& config) const {
    config.scrollBarWidth = json["scroll_bar_width"].toInt(12);
    config.treeIndentation = json["tree_indentation"].toInt(12);
    config.buttonPadding = json["button_padding"].toInt(5);
    config.scrollBarMinLength = json["scroll_bar_min_length"].toInt(25);
    config.dockTitleBarHeight = json["dock_title_bar_height"].toInt(18);
    config.splitterWidth = json["splitter_width"].toInt(4);
    config.itemViewRowPadding = json["item_view_row_padding"].toInt(4);
    config.flatDisabledText = json["flat_disabled_text"].toBool(true);
    config.hideShortcutUnderlines = json["hide_shortcut_underlines"].toBool(true);

    config.tabBarTabOverlap = json["tab_bar_tab_overlap"].toInt(1);
    config.tabBarTabBaseOverlap = json["tab_bar_tab_base_overlap"].toInt(1);
    config.tabBarTabHSpace = json["tab_bar_tab_h_space"].toInt(8);
    config.tabBarTabVSpace = json["tab_bar_tab_v_space"].toInt(4);
    config.tabActiveExtraHeight = json["tab_active_extra_height"].toInt(13);
    config.tabCloseIndicatorSize = json["tab_close_indicator_size"].toInt(14);

    config.menuVerticalPadding = json["menu_vertical_padding"].toInt(3);
    config.menuHorizontalPadding = json["menu_horizontal_padding"].toInt(4);
    config.menuBorderWidth = json["menu_border_width"].toInt(1);
    config.headerDefaultHeight = json["header_default_height"].toInt(22);
    config.sliderHandleLength = json["slider_handle_length"].toInt(15);

    config.dockWidgetTitleBarButtonMargin = json["dock_widget_title_bar_button_margin"].toInt(-1);
    config.sliderControlThickness = json["slider_control_thickness"].toInt(12);
    config.focusFrameHMargin = json["focus_frame_h_margin"].toInt(1);
    config.focusFrameVMargin = json["focus_frame_v_margin"].toInt(1);
    config.layoutMargin = json["layout_margin"].toInt(6);
    config.layoutSpacing = json["layout_spacing"].toInt(6);
    config.toolbarItemSpacing = json["toolbar_item_spacing"].toInt(3);
    config.toolbarSeparatorWidth = json["toolbar_separator_width"].toInt(4);
    config.menuBarItemSpacing = json["menu_bar_item_spacing"].toInt(6);
    config.menuBarVerticalMargin = json["menu_bar_vertical_margin"].toInt(2);

    config.indicatorBoxSize = json["indicator_box_size"].toInt(12);
    config.indicatorLabelSpacing = json["indicator_label_spacing"].toInt(4);
    config.titleBarButtonSize = json["title_bar_button_size"].toInt(16);
    config.subMenuOverlap = json["sub_menu_overlap"].toInt(-2);

    config.menuSupportsIcons = json["menu_supports_icons"].toBool(true);
    config.menuAllowActiveAndDisabled = json["menu_allow_active_and_disabled"].toBool(false);

    config.groupBoxTitleLeftPadding = json["group_box_title_left_padding"].toInt(8);
    config.groupBoxTitleLineGap = json["group_box_title_line_gap"].toInt(6);
    config.mnemonicUnderlineMode = static_cast<MnemonicUnderlineMode>(json["mnemonic_underline_mode"].toInt(
        static_cast<int>(MnemonicUnderlineMode::StickyToggle)));

    config.treeBranchIndicatorSize = json["tree_branch_indicator_size"].toInt(8);
    config.splitterHandleLength = json["splitter_handle_length"].toInt(30);

    config.toolbarPopupDelay = json["toolbar_popup_delay"].toInt(400);
    config.dragCursorStyle = static_cast<DragCursorStyle>(json["drag_cursor_style"].toInt(static_cast<int>(DragCursorStyle::SizeAll)));

    config.menuBarHorizontalMargin = json["menu_bar_horizontal_margin"].toInt(-1);
    config.textCursorWidth = json["text_cursor_width"].toInt(-1);
    config.tabBarCloseButtonPosition = static_cast<TabBarCloseButtonPosition>(
        json["tab_bar_close_button_position"].toInt(static_cast<int>(TabBarCloseButtonPosition::RightSide)));
    config.toolbarHandleExtent = json["toolbar_handle_extent"].toInt(-1);
    config.tabBarScrollButtonWidth = json["tab_bar_scroll_button_width"].toInt(-1);

    config.menuIconSize = json["menu_icon_size"].toInt(-1);
    config.buttonIconSize = json["button_icon_size"].toInt(-1);
    config.toolBarIconSize = json["toolbar_icon_size"].toInt(-1);

    return true;
}
