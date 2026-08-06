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

#include "lc_settings_page_autosave.h"
#include "ui_lc_settings_page_autosave.h"
#include "lc_settings_backend.h"
#include "lc_settings_defaults.h"

LC_SettingsPageAutosave::LC_SettingsPageAutosave(QObject* parent)
    : LC_SettingsPageBase(tr("Auto-save & Backup"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Defaults::Group),
                           parent)  , ui(std::make_unique<Ui::LC_SettingsPageAutosave>()){
    setSortWeight(30);
}

LC_SettingsPageAutosave::~LC_SettingsPageAutosave() = default;

void LC_SettingsPageAutosave::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageAutosave::setupBehavior() {
    enableWhenChecked(ui->cbAutoBackup, ui->cbAutoSaveTime);
}


void LC_SettingsPageAutosave::setupBindings() {
    using namespace CFG_Defaults;

    bindBoolean({
        { ui->cbAutoBackup, o_AutoBackupDocument }
    });

    bindInt({
        { ui->cbAutoSaveTime, o_AutoSaveTime }
    });

    bindComboText({
        { ui->cbAutoSaveFileNamePrefix, o_AutosaveFilePrefix },
        { ui->cbBackupFileSuffix, o_BackupFileSuffix }
    });
}
