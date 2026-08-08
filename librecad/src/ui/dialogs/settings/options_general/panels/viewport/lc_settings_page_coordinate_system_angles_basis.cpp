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


#include "lc_settings_page_coordinate_system_angles_basis.h"

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "ui_lc_settings_page_coordinate_system_angles_basis.h"


LC_SettingsPageCoordinateSystemAnglesBasis::LC_SettingsPageCoordinateSystemAnglesBasis(QObject* parent)
    : LC_SettingsPageBase(tr("Angles Basis"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageCoordinateSystemAnglesBasis>()) {
    setSortWeight(40);
}

LC_SettingsPageCoordinateSystemAnglesBasis::~LC_SettingsPageCoordinateSystemAnglesBasis() = default;

void LC_SettingsPageCoordinateSystemAnglesBasis::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageCoordinateSystemAnglesBasis::setupBehavior() {
    enableWhenChecked(ui->cbAnglesMarkVisible, ui->cbAnglesBaseShowPolicy);
    enableWhenChecked(ui->cbAnglesMarkVisible, ui->cbAnglesMarkColorDirection);
    enableWhenChecked(ui->cbAnglesMarkVisible, ui->cbAnglesMarkColorAngleRay);
    enableWhenChecked(ui->cbAnglesMarkVisible, ui->pbAnglesMarkDirection);
    enableWhenChecked(ui->cbAnglesMarkVisible, ui->pbAnglesMarkAngleRay);
}

void LC_SettingsPageCoordinateSystemAnglesBasis::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbAnglesMarkVisible, o_AnglesBasisMarkEnabled }
    });

    bindComboIndex({
        { ui->cbAnglesBaseShowPolicy, o_AnglesBasisMarkPolicy }
    });

    bindColor({
        { ui->cbAnglesMarkColorDirection, ui->pbAnglesMarkDirection, o_AnglesBasisDirectionIndicatorColor, tr("Select Arrow Direction Color") },
        { ui->cbAnglesMarkColorAngleRay, ui->pbAnglesMarkAngleRay, o_AnglesBasisAngleRayColor, tr("Select Base Angle Ray Color") }
    });
}
