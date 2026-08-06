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


#ifndef LC_SETTINGS_RELATIVE_POSITION_ASSISTANT_H
#define LC_SETTINGS_RELATIVE_POSITION_ASSISTANT_H

#include "lc_setting.h"

namespace CFG_RelativePositionAssistant {
    inline const LC_SettingsGroupBase Group("RelativePositionAssistant");

    inline const LC_Setting<int> o_AssistantFontSize(&Group, "FontSize", 10);
    inline const LC_Setting<QString> o_AssistantFontName(&Group, "FontName", "Verdana");
    inline const LC_Setting<bool> o_RememberCoordinatesMode(&Group, "RememberCoordinatesMode", false);
    inline const LC_Setting<bool> o_LastInvocationOffsetMode(&Group,"LastInvocationOffsetMode", true);
    inline const LC_Setting<bool> o_StartInOffsetMode(&Group, "StartInOffsetMode", true);
}

#endif
