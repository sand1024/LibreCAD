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

#ifndef LC_SETTINGS_DEFAULTS_H
#define LC_SETTINGS_DEFAULTS_H

#include "lc_setting.h"
#include "rs.h"

namespace CFG_Defaults {
    inline const LC_SettingsGroupBase Group("Defaults");

    inline const LC_Setting<QString> o_Unit(&Group, "Unit", "Millimeter");
    inline const LC_Setting<int> o_AutoSaveTime(&Group, "AutoSaveTime", 5);
    inline const LC_Setting<bool> o_AutoBackupDocument(&Group, "AutoBackupDocument", true);
    inline const LC_Setting<QString> o_AutosaveFilePrefix(&Group, "AutosaveFilePrefix", "#");
    inline const LC_Setting<QString> o_BackupFileSuffix(&Group, "BackupFileSuffix", "~");
    inline const LC_Setting<bool> o_UseQtFileOpenDialog(&Group, "UseQtFileOpenDialog", true);
    inline const LC_Setting<bool> o_UseQtColorPickerDialog(&Group, "UseQtColorPickerDialog", true);
    inline const LC_Setting<bool> o_WheelScrollInvertH(&Group, "WheelScrollInvertH", false);
    inline const LC_Setting<bool> o_WheelScrollInvertV(&Group, "WheelScrollInvertV", false);
    inline const LC_Setting<bool> o_InvertZoomDirection(&Group, "InvertZoomDirection", false);
    inline const LC_Setting<int> o_AngleSnapStep(&Group, "AngleSnapStep", 3);
    inline const LC_Setting<bool> o_GridOffForNewDrawing(&Group, "GridOffForNewDrawing", false);
    inline const LC_Setting<bool> o_IsometricGrid(&Group, "IsometricGrid", false);
    inline const LC_Setting<RS2::IsoGridViewType> o_IsoGridView(&Group, "IsoGridView", RS2::IsoGridViewType::IsoLeft);
    inline const LC_Setting<QString> o_AnglesBaseAngle(&Group, "AnglesBaseAngle", "0.0");
    inline const LC_Setting<bool> o_AnglesCounterClockwise(&Group, "AnglesCounterClockwise", true);
    inline const LC_Setting<bool> o_InteractiveInputEnabled(&Group, "InteractiveInputEnabled", true); // fixme - move to appearance?
}

#endif
