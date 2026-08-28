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

#include "lc_settings_page_paths.h"
#include "ui_lc_settings_page_paths.h"
#include "lc_settings_backend.h"
#include "lc_settings_paths.h"
#include "rs_system.h"

LC_SettingsPagePaths::LC_SettingsPagePaths(QObject* parent)
    : LC_SettingsPageBase(tr("Paths"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Paths::Group),
                           parent)  , ui(std::make_unique<Ui::LC_SettingsPagePaths>()){
    // Priority 70 inside the Application parent index block
    setSortWeight(70);
}

LC_SettingsPagePaths::~LC_SettingsPagePaths() = default;

void LC_SettingsPagePaths::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPagePaths::setupBindings() {
    using namespace CFG_Paths;

    bindString({
        { ui->lePathTranslations, o_Translations },
        { ui->lePathHatch, o_Patterns },
        { ui->lePathFonts, o_Fonts },
        { ui->leTemplate, o_Template },
        { ui->variablefile_field, o_VariableFile },
        { ui->leOtherSettingsDirectory, o_OtherSettingsDir },
        { ui->lePathLibrary, o_Library, true /*requiresRestart*/ }
    });

    bindDirectoryChooser(ui->fonts_button, ui->lePathFonts, tr("Select Fonts Folder"));
    bindDirectoryChooser(ui->translation_button, ui->lePathTranslations, tr("Select Translations Folder"));
    bindDirectoryChooser(ui->hatchpatterns_button, ui->lePathHatch, tr("Select Hatch Patterns Folder"));
    bindDirectoryChooser(ui->tbOtherSettings, ui->leOtherSettingsDirectory, tr("Select Other Settings Folder"));
    bindDirectoryChooser(ui->btPathLibrary, ui->lePathLibrary, tr("Select Parts Library Folder"));
    bindFileChooser(ui->btTemplate, ui->leTemplate, tr("Select Template File"), "*.dxf");
    bindFileChooser(ui->variablefile_button, ui->variablefile_field, tr("Select Variable File"), "*.*");
}

bool LC_SettingsPagePaths::saveSettings() {
    QString currentLib = ui->lePathLibrary->text().trimmed();
    if (m_originalLibraryPath != currentLib) {
        setRestartRequired(true);
    }
    return LC_SettingsPageBase::saveSettings();
}
