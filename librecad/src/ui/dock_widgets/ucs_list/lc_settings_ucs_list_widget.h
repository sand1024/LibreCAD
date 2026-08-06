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

#ifndef LC_SETTINGS_UCS_LIST_WIDGET_H
#define LC_SETTINGS_UCS_LIST_WIDGET_H
#include "lc_setting.h"
#include "lc_ucslistoptions.h"

namespace CFG_WidgetUCSList {
    inline const LC_SettingsGroupBase Group("Widget.UCSList");

    inline const LC_Setting<bool> o_ShowTooltip(&Group, "ShowTooltip", true);
    inline const LC_Setting<bool> o_ShowColumnTypeIcon(&Group, "ShowColumnTypeIcon", false);
    inline const LC_Setting<bool> o_ShowColumnGridType(&Group, "ShowColumnGridType", false);
    inline const LC_Setting<bool> o_ShowColumnPositionAndAngle(&Group, "ShowColumnPositionAndAngle", false);
    inline const LC_Setting<bool> o_ConfirmDelete(&Group, "ConfirmDelete", true);
    inline const LC_Setting<bool> o_SingleClickRestore(&Group, "SingleClickRestore", false);
    inline const LC_Setting<LC_UCSListOptions::DoubleClickPolicy> o_DoubleClickPolicy(&Group, "DoubleClickPolicy", LC_UCSListOptions::APPLY_UCS);
    inline const LC_Setting<bool> o_ShowGrid(&Group, "ShowGrid", true);
}

#endif
