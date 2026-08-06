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

#ifndef LC_SETTINGS_EXPORT_LAYERS_H
#define LC_SETTINGS_EXPORT_LAYERS_H
#include <QDir>

#include "lc_setting.h"

namespace CFG_ExportLayers {
    inline const LC_SettingsGroupBase Group("Export.Layers");

    inline const LC_Setting<bool> o_ExportUCS(&Group, "ExportUCS", true);
    inline const LC_Setting<bool> o_ExportViews(&Group, "ExportViews", true);
    inline const LC_Setting<bool> o_DocumentPerLayer(&Group, "DocumentPerLayer", false);
    inline const LC_Setting<bool> o_EntitiesInOriginalLayer(&Group, "EntitiesInOriginalLayer", false);

    inline const LC_Setting<QString> o_DirPath(&Group, "DirPath", QDir::toNativeSeparators(QDir::homePath()));
    inline const LC_Setting<QString> o_FileFilter(&Group, "FileFilter", QDir::toNativeSeparators(QDir::homePath()));


}
#endif
