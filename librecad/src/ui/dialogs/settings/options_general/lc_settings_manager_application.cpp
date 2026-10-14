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

#include "lc_settings_manager_application.h"

#include "lc_graphic_view_preview_widget.h"

#include "lc_settings_page_autosave.h"
#include "lc_settings_page_cad_preferences.h"
#include "lc_settings_page_command_line.h"
#include "lc_settings_page_drawing_defaults.h"
#include "lc_settings_page_general_startup.h"
#include "lc_settings_page_keyboard.h"
#include "lc_settings_page_language.h"
#include "lc_settings_page_maintenance.h"
#include "lc_settings_page_paths.h"
#include "lc_settings_page_program_defaults.h"
#include "lc_settings_page_updates.h"
#include "lc_repository_drafting_theme.h"
#include "lc_visual_snap_data.h"
#include "qc_applicationwindow.h"

class LC_OverlayDrawablesContainer;

QWidget* LC_SettingsManagerApplication::createGraphicViewPreview(QWidget* parent) {
    QWidget* result = new LC_GraphicViewPreviewWidget(parent);
    return result;
}

void LC_SettingsManagerApplication::initialize(QC_ApplicationWindow* appWindow) {
    using namespace LC_SettingsPagesApplication;

    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = DLG_GENERAL_PREFERENCES;

    reg->configureDialog(targetDialog, {QObject::tr("Application Preferences"), true, false, false,":/icons/settings.lci"});

    // Pages Registration
    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {
        { PAGE_APP, "", index(QObject::tr("Application"),
            QObject::tr("Configure general application settings, defaults, workspace "
                        "profiles, paths, and update preferences.")) },
        { PAGE_APP_PROGRAM_DEFAULTS, PAGE_APP, page<LC_SettingsPageProgramDefaults>() },
        { PAGE_APP_DEFAULTS, PAGE_APP, page<LC_SettingsPageDrawingDefaults>() },
        { PAGE_APP_AUTOSAVE, PAGE_APP, page<LC_SettingsPageAutosave>() },
        { PAGE_APP_CAD_PREFERENCES, PAGE_APP, page<LC_SettingsPageCadPreferences>() },
        { PAGE_APP_KEYBOARD, PAGE_APP, page<LC_SettingsPageKeyboard>() },
        { PAGE_APP_COMMAND_LINE, PAGE_APP, page<LC_SettingsPageCommandLine>() },
        { PAGE_APP_PATHS, PAGE_APP, page<LC_SettingsPagePaths>() },
        { PAGE_APP_UPDATES, PAGE_APP, page<LC_SettingsPageUpdates>() },
        { PAGE_APP_MAINTENANCE, PAGE_APP, page<LC_SettingsPageMaintenance>() },
        { PAGE_APP_STARTUP, PAGE_APP, page<LC_SettingsPageGeneralStartup>() },
        { PAGE_APP_LANGUAGE, PAGE_APP, page<LC_SettingsPageLanguage>() }
    };

    reg->registerPages(targetDialog, pages);
}
