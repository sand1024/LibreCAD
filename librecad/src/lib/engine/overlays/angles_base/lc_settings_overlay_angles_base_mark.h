/*******************************************************************************
 *
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

#ifndef LC_SETTINGS_OVERLAY_ANGLES_BASE_MARK_H
#define LC_SETTINGS_OVERLAY_ANGLES_BASE_MARK_H
#include "lc_setting.h"

namespace CFG_OverlayAnglesBaseMark {
    inline const LC_SettingsGroupBase Group("OverlayAnglesBaseMark");


    inline const LC_Setting<bool> o_AnglesBasisMarkEnabled(&Group, "AnglesBasisMarkEnabled", true);

    enum ShowAnglesBaseMarkType{
        SHOW_ALWAYS = 0,
        SHOW_IF_NON_DEFAULTS
    };

    inline const LC_Setting<int> o_AnglesBasisMarkPolicy(&Group, "AnglesBasisMarkPolicy", SHOW_ALWAYS);

    // Angle Basis Ray
    inline const LC_Setting<QColor> o_ColorAnglesBasisDirectionIndicator(&Group, "ColorDirection", QColor("#017CFF"));
    inline const LC_Setting<QColor> o_ColorAnglesBasisAngleRay(&Group, "ColorAngleRay", QColor("#00FFFF"));
}

#endif
