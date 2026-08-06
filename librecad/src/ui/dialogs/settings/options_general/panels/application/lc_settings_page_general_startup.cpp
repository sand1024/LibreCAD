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

#include "lc_settings_page_general_startup.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_general_startup.h"
#include "lc_settings_backend.h"
#include "lc_settings_startup.h"

LC_SettingsPageGeneralStartup::LC_SettingsPageGeneralStartup(QObject* parent)
    : LC_SettingsPageBase(tr("Startup"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Startup::Group),
                           parent) , ui(std::make_unique<Ui::LC_SettingsPageGeneralStartup>()){
    setSortWeight(10);
}

LC_SettingsPageGeneralStartup::~LC_SettingsPageGeneralStartup() = default;

void LC_SettingsPageGeneralStartup::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGeneralStartup::setupBindings() {
    using namespace CFG_Startup;

    bindBoolean({
        {ui->cbSplash, o_ShowSplash},
        {ui->tab_mode_check_box, o_TabMode},
        {ui->maximize_checkbox, o_Maximize},
        {ui->cbOpenLastFiles, o_OpenLastOpenedFiles},
        {ui->cbEnableCADDockWidgets, o_EnableLeftSidebar, true},
        {ui->cbCADWidgetsUngroupped, o_CADSideBarUngrouped, true},
        {ui->cad_toolbars_checkbox, o_EnableCADToolbars, true},
        {ui->cbAllowMenusDetaching, CFG_Appearance::o_AllowMenusTearOff, true },
        {ui->cbClassicStatusBar, o_UseClassicStatusBar, true},
        {ui->cbDuplicateActionsPromptsInStatusBar, o_ShowCommandPromptInStatusBar},
        {ui->cbStartupTBTooltips, o_ShowToolbarsTooltip, true},
        {ui->cbShowVersionInWinTitle, o_ShowVersionInTitle}
    });
}
