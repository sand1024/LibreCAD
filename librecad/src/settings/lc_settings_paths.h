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


#ifndef LC_SETTINGS_PATHS_H
#define LC_SETTINGS_PATHS_H

#include <QDir>

#include "lc_setting.h"
#include "rs_system.h"

namespace CFG_Paths {
    inline const LC_SettingsGroupBase Group("Paths");

    // fixme - sand - settings - review default there. Probably we should rely on app dir!

    inline const LC_Setting<QString> o_Translations(&Group, "Translations", "");
    inline const LC_Setting<QString> o_Patterns(&Group, "Patterns", "");
    inline const LC_Setting<QString> o_Fonts(&Group, "Fonts", "");
    inline const LC_Setting<QString> o_Library(&Group, "Library", "");
    inline const LC_Setting<QString> o_Template(&Group, "Template", "");
    inline const LC_Setting<QString> o_VariableFile(&Group, "VariableFile", "");
    inline const LC_Setting<QString> o_OtherSettingsDir(&Group, "OtherSettingsDir", RS_System::instance()->getAppDataDir().trimmed()); // fixme - rework, one may be not intitialized?
    inline const LC_Setting<QString> o_ExportSettingsDir(&Group, "ExportSettingsDir", RS_SYSTEM->getHomeDir());


    inline const LC_Setting<QString> o_Save(&Group, "Save", QDir::toNativeSeparators(QDir::homePath()));
    inline const LC_Setting<QString> o_SaveDrawingFilter(&Group, "SaveDrawingFilter",  "Drawing Exchange DXF 2007 (*.dxf)");
    inline const LC_Setting<QString> o_Open(&Group, "Open", QDir::toNativeSeparators(QDir::homePath()));
}


#endif
