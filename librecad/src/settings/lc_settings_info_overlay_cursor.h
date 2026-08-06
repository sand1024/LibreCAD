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

#ifndef CFG_INFOOVERLAYCURSOR_H
#define CFG_INFOOVERLAYCURSOR_H

#include "lc_setting.h"

namespace CFG_InfoOverlayCursor {
    inline const LC_SettingsGroupBase Group("InfoOverlayCursor");

    inline const LC_Setting<bool> o_InfoCursorEnabled(&Group, "Enabled", true);
    inline const LC_Setting<bool> o_ShowAbsolute(&Group, "ShowAbsolute", true);
    inline const LC_Setting<bool> o_ShowAbsoluteWCS(&Group, "ShowAbsoluteWCS", false);
    inline const LC_Setting<bool> o_ShowRelativeDA(&Group, "ShowRelativeDA", true);
    inline const LC_Setting<bool> o_ShowRelativeDD(&Group, "ShowRelativeDD", true);
    inline const LC_Setting<bool> o_ShowSnapInfo(&Group, "ShowSnapInfo", true);
    inline const LC_Setting<bool> o_ShowPrompt(&Group, "ShowPrompt", true);
    inline const LC_Setting<bool> o_ShowActionName(&Group, "ShowActionName", true);
    inline const LC_Setting<bool> o_ShowLabels(&Group, "ShowLabels", false);
    inline const LC_Setting<bool> o_SingleLine(&Group, "SingleLine", true);
    inline const LC_Setting<int> o_FontSize(&Group, "FontSize", 10);
    inline const LC_Setting<QString> o_FontName(&Group, "FontName", "Verdana");
    inline const LC_Setting<int> o_OffsetFromCursor(&Group, "OffsetFromCursor", 15);
    inline const LC_Setting<bool> o_ShowPropertiesCatched(&Group, "ShowPropertiesCatched", true);
    inline const LC_Setting<bool> o_ShowPropertiesEdit(&Group, "ShowPropertiesEdit", true);
    inline const LC_Setting<bool> o_ShowPropertiesCreating(&Group, "ShowPropertiesCreating", true);
}

#endif 
