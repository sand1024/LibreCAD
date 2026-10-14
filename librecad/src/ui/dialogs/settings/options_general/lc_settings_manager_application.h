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

#ifndef LC_SETTINGS_MANAGER_GENERIC_OPTIONS_H
#define LC_SETTINGS_MANAGER_GENERIC_OPTIONS_H

#include "lc_settings_manager_base.h"
#include "lc_settings_registry.h"

class QC_ApplicationWindow;

namespace LC_SettingsPagesApplication {
    inline const QString DLG_GENERAL_PREFERENCES                    = "general_preferences";

    inline const QString PAGE_APP                                   = "app";
    inline const QString PAGE_APP_PROGRAM_DEFAULTS                  = "app.program_defaults";
    inline const QString PAGE_APP_DEFAULTS                          = "app.defaults";
    inline const QString PAGE_APP_AUTOSAVE                          = "app.autosave";
    inline const QString PAGE_APP_CAD_PREFERENCES                   = "app.cad_preferences";
    inline const QString PAGE_APP_KEYBOARD                          = "app.keyboard";
    inline const QString PAGE_APP_COMMAND_LINE                     = "app.command_line";
    inline const QString PAGE_APP_PATHS                             = "app.paths";
    inline const QString PAGE_APP_UPDATES                           = "app.updates";
    inline const QString PAGE_APP_MAINTENANCE                       = "app.maintenance";
    inline const QString PAGE_APP_STARTUP                           = "app.startup";
    inline const QString PAGE_APP_LANGUAGE                          = "app.language";
}


class LC_SettingsManagerApplication: public LC_SettingsManagerBase {
public:
    static QWidget* createGraphicViewPreview(QWidget* parent);
    static void initialize(QC_ApplicationWindow* applicationWindow);

    static bool showOptionsApplication(QWidget* parent, int* exitCode) {
        const auto settingsRegistry = LC_SettingsRegistry::instance();
        const bool accepted = settingsRegistry->showDialog(LC_SettingsPagesApplication::DLG_GENERAL_PREFERENCES, "",
                                                           parent, nullptr, nullptr, exitCode);

        if (accepted) {
            parent->update();
        }
        return accepted;
    }
};

#endif
