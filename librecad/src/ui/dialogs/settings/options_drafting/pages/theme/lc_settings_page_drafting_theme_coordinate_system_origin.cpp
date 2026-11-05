/*******************************************************************************
 *
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

#include "lc_settings_page_drafting_theme_coordinate_system_origin.h"

#include "lc_settings_backend.h"
#include "lc_settings_coordinates_origin.h"
#include "ui_lc_settings_page_drafting_theme_coordinate_system_origin.h"

LC_SettingsPageDraftingThemeCoordinateSystemOrigin::LC_SettingsPageDraftingThemeCoordinateSystemOrigin(QObject* parent)
    : LC_SettingsPageBase(tr("Coordinates System"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_CoordinatesOrigin::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingThemeCoordinateSystemOrigin>()) {
}

LC_SettingsPageDraftingThemeCoordinateSystemOrigin::~LC_SettingsPageDraftingThemeCoordinateSystemOrigin() = default;

void LC_SettingsPageDraftingThemeCoordinateSystemOrigin::setupUi() {
    ui->setupUi(m_widget);
}


void LC_SettingsPageDraftingThemeCoordinateSystemOrigin::setupBehavior() {
    connect(ui->cbGridExtendAxisLines, &QCheckBox::toggled, this, [this](bool checked) {
        ui->cbXAxisAreas->setEnabled(checked);
        ui->cbYAxisAreas->setEnabled(checked);
    });
}

void LC_SettingsPageDraftingThemeCoordinateSystemOrigin::setupBindings() {
    using namespace CFG_CoordinatesOrigin;
    using namespace CFG_OverlayAnglesBaseMark;


    bindComboText({
        { ui->fcbUCSFont, o_ZeroMarkerFontName }
    });

    bindBoolean({
        { ui->cbGridExtendAxisLines, o_ExtendAxisLines }
    });

    bindComboIndex({
        { ui->cbXAxisAreas, o_ExtendModeXAxis },
        { ui->cbYAxisAreas, o_ExtendModeYAxis }
    });

    bindColor({
        { ui->cbAxisXColor, ui->pb_axis_X, o_ColorXAxis, tr("Select X-Axis Color") },
        { ui->cbAxisYColor, ui->pb_axis_Y, o_ColorYAxis, tr("Select Y-Axis Color") },
        { ui->cbAnglesMarkColorDirection, ui->pbAnglesMarkDirection, o_ColorAnglesBasisDirectionIndicator, tr("Select Arrow Direction Color") },
        { ui->cbAnglesMarkColorAngleRay, ui->pbAnglesMarkAngleRay, o_ColorAnglesBasisAngleRay, tr("Select Base Angle Ray Color") },
        { ui->cbRelativeZeroColor, ui->pb_relativeZeroColor, o_ColorRelativeZero, tr("Select Relative Zero Color") }
    });
}
