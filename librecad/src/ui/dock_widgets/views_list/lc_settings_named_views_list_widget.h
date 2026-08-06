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

#ifndef LC_SETTINGS_NAMED_VIEWS_LIST_WIDGET_H
#define LC_SETTINGS_NAMED_VIEWS_LIST_WIDGET_H
#include "lc_namedviewslistoptions.h"
#include "lc_setting.h"

namespace CFG_WidgetNamedViewsList {
    inline const LC_SettingsGroupBase Group("Widget.ViewsList");

    inline const LC_Setting<bool> o_ShowTooltip(&Group, "ShowTooltip", true);
    inline const LC_Setting<bool> o_ShowIconType(&Group, "ShowIconType", true);
    inline const LC_Setting<bool> o_ShowColumnIconGrid(&Group, "ShowColumnIconGrid", true);
    inline const LC_Setting<bool> o_ShowColumnIconUCSType(&Group, "ShowColumnIconUCSType", true);
    inline const LC_Setting<bool> o_ShowColumnViewDetails(&Group, "ShowColumnViewDetails", true);
    inline const LC_Setting<bool> o_ShowColumnUCSDetails(&Group, "ShowColumnUCSDetails", true);
    inline const LC_Setting<bool> o_ConfirmDelete(&Group, "ConfirmDelete", true);
    inline const LC_Setting<bool> o_ReplaceDuplicateSilently(&Group, "ReplaceDuplicateSilently", false);
    inline const LC_Setting<LC_NamedViewsListOptions::DoubleClickPolicy> o_DoubleClickPolicy(
        &Group, "DoubleClickPolicy", LC_NamedViewsListOptions::RENAME);
    inline const LC_Setting<bool> o_RestoreViewBySingleClick(&Group, "RestoreViewBySingleClick", true);
    inline const LC_Setting<bool> o_ShowGrid(&Group, "ShowGrid", true);
}

#endif
