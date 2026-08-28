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

#include "lc_settings_page_command_line.h"
#include "ui_lc_settings_page_command_line.h"
#include "lc_settings_backend.h"
#include "lc_settings_commands_promotion.h"

LC_SettingsPageCommandLine::LC_SettingsPageCommandLine(QObject* parent)
    : LC_SettingsPageBase(tr("Command Line Informer"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_CommandsPromotion::Group),
                           parent) , ui(std::make_unique<Ui::LC_SettingsPageCommandLine>())  {
    setSortWeight(60);
}

LC_SettingsPageCommandLine::~LC_SettingsPageCommandLine() = default;

void LC_SettingsPageCommandLine::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageCommandLine::setupBindings() {
    using namespace CFG_CommandsPromotion;

    bindBoolean({
        { ui->cbShowCommandInMenu, o_ShowCommandInMenu },
        { ui->cbPromoteCommandInCmd, o_PromoteCommands }
    });
}
