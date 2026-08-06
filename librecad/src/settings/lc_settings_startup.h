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


#ifndef LC_SETTINGS_STARTUP_H
#define LC_SETTINGS_STARTUP_H

#include "lc_setting.h"

namespace CFG_Startup {
    inline const LC_SettingsGroupBase Group("Startup");
    inline const LC_Setting<bool> o_ShowSplash(&Group, "ShowSplash", true);
    inline const LC_Setting<bool> o_TabMode(&Group, "TabMode", false);
    inline const LC_Setting<bool> o_Maximize(&Group, "Maximize", false);
    inline const LC_Setting<bool> o_EnableLeftSidebar(&Group, "EnableLeftSidebar", true);
    inline const LC_Setting<bool> o_CADSideBarUngrouped(&Group, "CADSideBarUngrouped", false);
    inline const LC_Setting<bool> o_EnableCADToolbars(&Group, "EnableCADToolbars", true);
    inline const LC_Setting<bool> o_OpenLastOpenedFiles(&Group, "OpenLastOpenedFiles", true);
    inline const LC_Setting<bool> o_UseClassicStatusBar(&Group, "UseClassicStatusBar", false);
    inline const LC_Setting<bool> o_ShowCommandPromptInStatusBar(&Group, "ShowCommandPromptInStatusBar", true);
    inline const LC_Setting<bool> o_CheckForNewVersions(&Group, "CheckForNewVersions", true);
    inline const LC_Setting<bool> o_ExpandedToolsMenu(&Group, "ExpandedToolsMenu", false);
    inline const LC_Setting<bool> o_ExpandedToolsMenuTillEntity(&Group, "ExpandedToolsMenuTillEntity", false);
    inline const LC_Setting<bool> o_ShowToolbarsTooltip(&Group, "ShowToolbarsTooltip", true);
    inline const LC_Setting<bool> o_ShowVersionInTitle(&Group, "ShowVersionInTitle", true);

    inline const LC_Setting<QString> o_LastOpenFilesList(&Group, "LastOpenFilesList", "");
    inline const LC_Setting<QString> o_LastOpenFilesActive(&Group, "LastOpenFilesActive", "");

    inline const LC_Setting<bool> o_IgnorePreReleaseVersions(&Group, "IgnorePreReleaseVersions", true);
    inline const LC_Setting<QString> o_IgnoredRelease(&Group, "IgnoredRelease", "0.0.0.0");
    inline const LC_Setting<QString> o_IgnoredPreRelease(&Group, "IgnoredPreRelease", "0.0.0.0");

    inline const LC_Setting<bool> o_FirstLoad(&Group, "FirstLoad", true);
}

#endif
