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

#include "lc_settings_page_snap_angle.h"
#include "ui_lc_settings_page_snap_angle.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_defaults.h"
#include "lc_settings_snap.h"
#include "qg_linetypebox.h"
#include "rs_settings.h"

LC_SettingsPageSnapAngle::LC_SettingsPageSnapAngle(QObject* parent)
    : LC_SettingsPageBase(tr("Angle Snap"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Snap::Group), parent), ui(std::make_unique<Ui::LC_SettingsPageSnapAngle>())  {
}

LC_SettingsPageSnapAngle::~LC_SettingsPageSnapAngle() = default;

void LC_SettingsPageSnapAngle::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageSnapAngle::setupBehavior() {
}

void LC_SettingsPageSnapAngle::setupBindings() {
     using namespace CFG_Snap;

    bindBoolean({
        { ui->cbAngleSnapToLinesIfGrid, o_AngleSnapToLinesIfGrid }
    });

    bindInt({
        { ui->sbAngleSnapMarkRadius, CFG_Appearance::o_AngleSnapMarkerSize }
    });

    bindComboIndex({
        { ui->cbAngleSnapStep, CFG_Defaults::o_AngleSnapStep }
    });
}
