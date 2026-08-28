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

#include "lc_settings_page_coordinate_system_relative_zero.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_coordinate_system_relative_zero.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"


LC_SettingsPageCoordinateSystemRelativeZero::LC_SettingsPageCoordinateSystemRelativeZero(QObject* parent)
    : LC_SettingsPageBase(tr("Relative Zero"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageCoordinateSystemRelativeZero>()) {
    setSortWeight(20);
}

LC_SettingsPageCoordinateSystemRelativeZero::~LC_SettingsPageCoordinateSystemRelativeZero() = default;

void LC_SettingsPageCoordinateSystemRelativeZero::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageCoordinateSystemRelativeZero::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbHideRelativeZero, o_HideRelativeZero }
    });

    bindInt({
        { ui->sbRelZeroRadius, o_RelZeroMarkerRadius }
    });

    bindColor({
        { ui->cbRelativeZeroColor, ui->pb_relativeZeroColor, o_RelativeZeroColor, tr("Select Relative Zero Color") }
    });
}
