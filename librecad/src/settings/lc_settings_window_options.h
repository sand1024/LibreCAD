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


#ifndef LC_SETTINGS_WINDOW_OPTIONS_H
#define LC_SETTINGS_WINDOW_OPTIONS_H

#include "lc_setting.h"
#include "rs.h"

namespace CFG_WindowOptions {
    inline const LC_SettingsGroupBase Group("WindowOptions");

    inline const LC_Setting<RS2::TabShape> o_TabShape(&Group, "TabShape", RS2::Triangular);
    inline const LC_Setting<RS2::TabPosition> o_TabPosition(&Group, "TabPosition", RS2::West);
    inline const LC_Setting<RS2::SubWindowMode> o_SubWindowMode(&Group, "SubWindowMode", RS2::Maximized);
    inline const LC_Setting<bool> o_Maximized(&Group, "Maximized", true);
}
#endif
