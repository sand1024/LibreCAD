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

#include "lc_settings_page_cad_preferences.h"
#include "ui_lc_settings_page_cad_preferences.h"
#include "lc_settings_backend.h"
#include "lc_settings_modify.h"
#include "lc_settings_cad_preferences.h"

LC_SettingsPageCadPreferences::LC_SettingsPageCadPreferences(QObject* parent)
    : LC_SettingsPageBase(tr("CAD Preferences"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_CADPreferences::Group),
                           parent)  , ui(std::make_unique<Ui::LC_SettingsPageCadPreferences>()) {
    setSortWeight(40);
}

LC_SettingsPageCadPreferences::~LC_SettingsPageCadPreferences() = default;

void LC_SettingsPageCadPreferences::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageCadPreferences::setupBindings() {
    using namespace CFG_CADPreferences;
    bindBoolean({
       { ui->cbToActiveLayer, CFG_Modify::o_ModifyEntitiesToActiveLayer },
       { ui->cbKeepModifiedSelected, CFG_Modify::o_KeepModifiedSelected },
       { ui->cbAutoZoomDrawing, o_AutoZoomDrawing },
       { ui->cbAnglesInputInDecimalDegreesOnly, o_InputAnglesAsDecimalsOnly },
       { ui->cbFinishMovingByMouseUp, o_AdHockMovingEndsByMouseClick },
       { ui->cbSelectionWindowByMBClickAndUp, o_SelectionWindowBy2Clicks }
   });
}
