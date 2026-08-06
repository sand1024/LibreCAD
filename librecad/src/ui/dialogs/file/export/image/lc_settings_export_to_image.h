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

#ifndef LC_SETTINGS_EXPORT_TO_IMAGE_H
#define LC_SETTINGS_EXPORT_TO_IMAGE_H

#include "lc_setting.h"

namespace CFG_ExportToImage {
    inline const LC_SettingsGroupBase Group("Export.Image");
    inline const LC_Setting<bool> o_Device(&Group, "UseResolution", true);
    inline const LC_Setting<bool> o_BlackBackground(&Group, "BlackBackground", true);
    inline const LC_Setting<bool> o_BlackWhite(&Group, "BlackWhite", true);
    inline const LC_Setting<bool> o_BorderSameSize(&Group, "BorderSameSize", true);
    inline const LC_Setting<bool> o_UseResolution(&Group, "UseResolution", true);
    inline const LC_Setting<QString> o_Resolution(&Group, "Resolution", "1");
    inline const LC_Setting<QString> o_Width(&Group, "Width", "640");
    inline const LC_Setting<QString> o_Height(&Group, "Height", "480");
    inline const LC_Setting<QString> o_BorderLeftRight(&Group, "BorderLeftRight", "5");
    inline const LC_Setting<QString> o_BorderTopBottom(&Group, "BorderTopBottom", "5");
}

#endif
