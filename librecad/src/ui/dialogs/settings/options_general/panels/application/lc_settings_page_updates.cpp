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

#include "lc_settings_page_updates.h"
#include "ui_lc_settings_page_updates.h"
#include "lc_settings_backend.h"
#include "lc_settings_startup.h"

LC_SettingsPageUpdates::LC_SettingsPageUpdates(QObject* parent)
    : LC_SettingsPageBase(tr("Updates & Version"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Startup::Group),
                           parent)  , ui(std::make_unique<Ui::LC_SettingsPageUpdates>()) {
    setSortWeight(80);
}

LC_SettingsPageUpdates::~LC_SettingsPageUpdates() = default;

void LC_SettingsPageUpdates::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageUpdates::setupBehavior() {
    enableWhenChecked(ui->cbCheckNewVersion, ui->cbCheckNewVersionIgnorePreRelease);
}

void LC_SettingsPageUpdates::setupBindings() {
    using namespace CFG_Startup;

    bindBoolean({
        { ui->cbCheckNewVersion, o_CheckForNewVersions },
        { ui->cbCheckNewVersionIgnorePreRelease, o_IgnorePreReleaseVersions }
    });
}
