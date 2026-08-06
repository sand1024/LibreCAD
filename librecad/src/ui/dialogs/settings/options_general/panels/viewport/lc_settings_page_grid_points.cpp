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

#include "lc_settings_page_grid_points.h"
#include "ui_lc_settings_page_grid_points.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_grid.h"
#include "qg_linetypebox.h"
#include "rs_settings.h"

LC_SettingsPageGridPoints::LC_SettingsPageGridPoints(QObject* parent)
    : LC_SettingsPageBase(tr("Points Grid"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Grid::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageGridPoints>()) {
    setSortWeight(20);
}

LC_SettingsPageGridPoints::~LC_SettingsPageGridPoints() = default;

void LC_SettingsPageGridPoints::setupUi() {
    ui->setupUi(m_widget);
}



void LC_SettingsPageGridPoints::setupBindings() {
    using namespace CFG_Grid;

    bindInt({
        { ui->sbMetaGridPointsWidth, o_metaGridPointsLineWidth},
        { ui->sbPointSize, o_GridPointsPointSize}

    });

    bindColor({
        { ui->cbGridPointsColor, ui->pb_gridPoints, CFG_Colors::o_Grid, tr("Select Grid Points Color") },
        { ui->cbMetaGridPointsColor, ui->pb_metaPoints, CFG_Colors::o_MetaGrid, tr("Select Meta-grid Points Color") }
    });

    ui->wMetaGridPointsLineType->init(false, false, false);
    bindLineType(ui->wMetaGridPointsLineType, o_metaGridPointsLineType);
}
