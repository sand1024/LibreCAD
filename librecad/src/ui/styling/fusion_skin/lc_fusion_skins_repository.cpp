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

#include "lc_fusion_skins_repository.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>

#include "lc_proxy_style.h"
#include "rs_system.h"

QJsonObject LC_FusionSkinsRepository::configToJson(const SkinConfig& config) const {
    QJsonObject root;
    root["style_archetype"] = static_cast<int>(config.styleArchetype);
    root["box_decoration"]  = static_cast<int>(config.boxDecoration);
    root["custom_dock_title_bar"] = config.customDockTitleBar;
    root["dock_title_style"]  = static_cast<int>(config.dockTitleBarStyle);
    root["accented_scrollbars"] = config.accentedScrollbars;
    root["transparent_scrollbars"] = config.transparentScrollbars;
    root["linked_icon_style"] = config.linkedIconStyleName;
    root["use_theme_default_icons"] = config.useThemeDefaultIcons;
    root["custom_group_box_bar"] = config.customGroupBoxBar;
    root["group_box_header_style"]  = static_cast<int>(config.groupBoxHeaderStyle);
    root["group_box_boundary_style"] = static_cast<int>(config.groupBoxBoundaryStyle);
    root["group_box_use_accent"]    = config.groupBoxUseAccent;
    root["show_active_row_spotlight"] = config.showActiveRowSpotlight;
    root["show_item_view_hover"] = config.showItemViewHover;

    root["branch_indicator_style"]    = static_cast<int>(config.branchIndicatorStyle);
    root["show_tree_connecting_lines"] = config.showTreeConnectingLines;
    root["custom_splitter_grip"]       = config.customSplitterGrip;
    root["splitter_grip_style"]        = static_cast<int>(config.splitterGripStyle);
    root["highlight_splitter_on_drag"] = config.highlightSplitterOnDrag;
    root["show_grip_background_well"]  = config.showGripBackgroundWell;
    root["accent_grips"]       = config.accentGrips;
    root["persistent_dock_splitter"]   = config.persistentDockSplitter;

    root["use_focused_input_glow"]     = config.useFocusedInputGlow;

    root["use_tool_button_underline"]     = config.useToolButtonUnderline;
    root["tool_button_indicator_style"]   = static_cast<int>(config.toolButtonIndicatorStyle);
    root["custom_tool_tip_card"]       = config.customToolTipCard;
    root["custom_dock_buttons"]         = config.customVectorIconsInButtons;
    root["tab_stripe_at_bottom"]       = config.tabStripeAtBottom;

    root["custom_toolbar_overflow_grip"] = config.customToolbarOverflowGrip;
    root["auto_popup_toolbar_overflow"]  = config.autoPopupToolbarOverflow;
    root["auto_popup_instant_buttons"] = config.autoPopupInstantButtons;

    root["auto_popup_menu_bar"]          = config.autoPopupMenuBar;
    root["use_menu_bar_hover_card"] = config.useMenuBarHoverCard;
    root["show_menu_command_aliases"] = config.showMenuCommandAliases;

    root["use_segmented_tool_buttons"] = config.useSegmentedToolButtons;
    root["segmented_separation_style"] = static_cast<int>(config.segmentedSeparationStyle);
    root["segmented_color_policy"]      = static_cast<int>(config.segmentedColorPolicy);

    root["use_spin_box_progress_bar"] = config.useSpinBoxProgressBar;
    root["use_status_pill_chips"]      = config.useStatusPillChips;
    root["use_floating_hud"]           = config.useFloatingHUD;
    root["close_button_color_policy"]  = static_cast<int>(config.closeButtonColorPolicy);
    root["custom_menu_tear_off"]       = config.customMenuTearOff;
    root["sync_checked_menu_state"]    = config.syncCheckedMenuState;

    root["show_generic_dock_icons"]    = config.showGenericDockIcons; // Serialize
    root["show_special_dock_icons"]    = config.showSpecialDockIcons; // Serialize

    root["custom_dialog_title_bar"] = config.customDialogTitleBar;

    auto serializeScheme = [](const ColorSchemeData &scheme) {
        QJsonObject obj;
        obj["qss"] = scheme.qss;
        obj["contrast_policy"] = static_cast<int>(scheme.contrastPolicy);
        obj["contrast_weight"] = static_cast<int>(scheme.contrastWeight);
        obj["auto_calculate_3d_helpers"] = scheme.autoCalculate3DHelpers;
        obj["bevel_seed_role"] = static_cast<int>(scheme.bevelSeedRole);

        QJsonObject paletteObj;
        forEachRoleState([&](const PaletteRoleMapping& row, const PaletteStateMapping& state) {
            QJsonObject roleObj = paletteObj[row.name].toObject();
            roleObj[state.name] = scheme.palette[row.name][state.name].name(QColor::HexArgb);
            paletteObj[row.name] = roleObj;
        });
        obj["palette"] = paletteObj;
        return obj;
    };

    root["light_scheme"] = serializeScheme(config.light);
    root["dark_scheme"] = serializeScheme(config.dark);

    return root;
}

bool LC_FusionSkinsRepository::configFromJson(const QJsonObject& json, SkinConfig& config) const {
    config.styleArchetype = static_cast<StyleArchetype>(json["style_archetype"].toInt(static_cast<int>(StyleArchetype::ClassicFusion)));
    config.boxDecoration  = static_cast<BoxDecoration>(json["box_decoration"].toInt(static_cast<int>(BoxDecoration::DividingHairline)));
    config.customDockTitleBar = json["custom_dock_title_bar"].toBool(false);
    config.dockTitleBarStyle  = static_cast<DockTitleBarStyle>(json["dock_title_style"].toInt(static_cast<int>(DockTitleBarStyle::Native)));
    config.accentedScrollbars = json["accented_scrollbars"].toBool(false);
    config.transparentScrollbars = json["transparent_scrollbars"].toBool(false);
    config.linkedIconStyleName  = json["linked_icon_style"].toString("Default");
    config.useThemeDefaultIcons = json["use_theme_default_icons"].toBool(true);
    config.customGroupBoxBar   = json["custom_group_box_bar"].toBool(false);
    config.groupBoxHeaderStyle  = static_cast<GroupBoxHeaderStyle>(json["group_box_header_style"].toInt(static_cast<int>(GroupBoxHeaderStyle::Plain)));
    config.groupBoxBoundaryStyle = static_cast<GroupBoxBoundaryStyle>(json["group_box_boundary_style"].toInt(static_cast<int>(GroupBoxBoundaryStyle::Full)));
    config.groupBoxUseAccent    = json["group_box_use_accent"].toBool(false);
    config.showActiveRowSpotlight = json["show_active_row_spotlight"].toBool(false);
    config.showItemViewHover = json["show_item_view_hover"].toBool(true);
    config.branchIndicatorStyle = static_cast<BranchIndicatorStyle>(
       json["branch_indicator_style"].toInt(static_cast<int>(BranchIndicatorStyle::MutedChevrons))
   );
    config.showTreeConnectingLines = json["show_tree_connecting_lines"].toBool(false);

    config.customSplitterGrip       = json["custom_splitter_grip"].toBool(false);
    config.splitterGripStyle        = static_cast<SplitterGripStyle>(json["splitter_grip_style"].toInt(static_cast<int>(SplitterGripStyle::MutedDots)));
    config.highlightSplitterOnDrag = json["highlight_splitter_on_drag"].toBool(false);
    config.showGripBackgroundWell  = json["show_grip_background_well"].toBool(false);
    config.accentGrips       = json["accent_grips"].toBool(false);
    config.persistentDockSplitter   = json["persistent_dock_splitter"].toBool(true);
    config.useFocusedInputGlow     = json["use_focused_input_glow"].toBool(false);

    config.useToolButtonUnderline     = json["use_tool_button_underline"].toBool(false);
    config.toolButtonIndicatorStyle   = static_cast<ToolButtonIndicatorStyle>(
        json["tool_button_indicator_style"].toInt(static_cast<int>(ToolButtonIndicatorStyle::ContextStripe))
    );
    config.customToolTipCard       = json["custom_tool_tip_card"].toBool(false);
    config.customVectorIconsInButtons           = json["custom_dock_buttons"].toBool(false);
    config.tabStripeAtBottom       = json["tab_stripe_at_bottom"].toBool(false);

    config.customToolbarOverflowGrip = json["custom_toolbar_overflow_grip"].toBool(false);
    config.autoPopupToolbarOverflow  = json["auto_popup_toolbar_overflow"].toBool(false);
    config.autoPopupInstantButtons = json["auto_popup_instant_buttons"].toBool(false);

    config.autoPopupMenuBar          = json["auto_popup_menu_bar"].toBool(false);

    config.useMenuBarHoverCard = json["use_menu_bar_hover_card"].toBool(false);
    config.showMenuCommandAliases = json["show_menu_command_aliases"].toBool(false);

    config.useSegmentedToolButtons = json["use_segmented_tool_buttons"].toBool(false);
    config.segmentedSeparationStyle = static_cast<SegmentedSeparationStyle>(
        json["segmented_separation_style"].toInt(static_cast<int>(SegmentedSeparationStyle::ContinuousCard))
    );

    config.segmentedColorPolicy = static_cast<SegmentedColorPolicy>(
           json["segmented_color_policy"].toInt(static_cast<int>(SegmentedColorPolicy::HarmonizedAnalogous))
    );

    config.useSpinBoxProgressBar = json["use_spin_box_progress_bar"].toBool(false);
    config.useStatusPillChips      = json["use_status_pill_chips"].toBool(false);
    config.useFloatingHUD        = json["use_floating_hud"].toBool(false);
    config.closeButtonColorPolicy = static_cast<CloseButtonColorPolicy>(
        json["close_button_color_policy"].toInt(static_cast<int>(CloseButtonColorPolicy::AccentColor))
    );

    config.customMenuTearOff = json["custom_menu_tear_off"].toBool(false);
    config.syncCheckedMenuState = json["sync_checked_menu_state"].toBool(false);

    config.showGenericDockIcons = json["show_generic_dock_icons"].toBool(true); // Deserialize
    config.showSpecialDockIcons = json["show_special_dock_icons"].toBool(true); // Deserialize

    config.customDialogTitleBar = json["custom_dialog_title_bar"].toBool(false); // Deserialize

    auto deserializeScheme = [](const QJsonObject &obj, ColorSchemeData &scheme) {
        scheme.qss = obj["qss"].toString();
        scheme.contrastPolicy = static_cast<ContrastPolicy>(obj["contrast_policy"].toInt(static_cast<int>(ContrastPolicy::Standard)));
        scheme.contrastWeight = static_cast<ContrastWeight>(obj["contrast_weight"].toInt(static_cast<int>(ContrastWeight::Balanced)));
        scheme.autoCalculate3DHelpers = obj["auto_calculate_3d_helpers"].toBool(true);
        scheme.bevelSeedRole = static_cast<QPalette::ColorRole>(obj["bevel_seed_role"].toInt(static_cast<int>(QPalette::Button)));

        QJsonObject paletteObj = obj["palette"].toObject();
        forEachRoleState([&](const PaletteRoleMapping& row, const PaletteStateMapping& state) {
            QJsonObject roleObj = paletteObj[row.name].toObject();
            QString hexColor = roleObj[state.name].toString();
            auto color = !hexColor.isEmpty() ? QColor(hexColor) : Qt::white;
            scheme.palette[row.name][state.name] = color;
        });
    };

    deserializeScheme(json["light_scheme"].toObject(), config.light);
    deserializeScheme(json["dark_scheme"].toObject(), config.dark);
    return true;
}
