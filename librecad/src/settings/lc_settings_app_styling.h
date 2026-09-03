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


#ifndef LC_SETTINGS_APP_STYLING_H
#define LC_SETTINGS_APP_STYLING_H

#include "lc_setting.h"
#include "lc_palette_editor_shared.h" // fixme - sand - bad dependency!!
namespace CFG_AppStyling {
    inline const LC_SettingsGroupBase Group("AppStyling");

    inline const LC_Setting<bool> o_AllowTheme(&Group, "AllowTheme", false);
    inline const LC_Setting<bool> o_AllowStyle(&Group, "AllowStyle", false);
    inline const LC_Setting<QString> o_Style(&Group, "Style", "");
    inline const LC_Setting<QString> o_StyleSheet(&Group, "StyleSheet", "");
    inline const LC_Setting<QString> o_ActiveTypographyName(&Group, "ActiveTypographyName", "Default");
    inline const LC_Setting<QString> o_ActiveSkinName(&Group, "ActiveSkinName", "Default");
    inline const LC_Setting<QString> o_ActiveMetricsName(&Group, "ActiveMetricsName", "Default");
    inline const LC_Setting<QString> o_ActiveGraphicViewScheme(&Group, "ActiveGraphicViewScheme", "Default");
    inline const LC_Setting<QString> o_ActivePalette(&Group, "ActivePaletteName", "Default");
    inline const LC_Setting<ThemeModeOverride> o_ThemeModeOverride(&Group, "ThemeModeOverride", ThemeModeOverride::FollowSystem);

    inline const LC_Setting<QColor> o_ColorFilteredItem(&Group, "ColorSearchHighlight", QColor("#2a82da")); // Blue
    inline const LC_Setting<QColor> o_ColorConflictingItem(&Group, "ColorConflictingItem", QColor("#d9534f")); // Red
    inline const LC_Setting<QColor> o_ColorSearchResultItem(&Group, "ColorSearchResultItem", QColor("#bd6313")); // Amber
}
#endif
