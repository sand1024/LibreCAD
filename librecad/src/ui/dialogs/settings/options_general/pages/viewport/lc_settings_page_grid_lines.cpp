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

#include "lc_settings_page_grid_lines.h"
#include "ui_lc_settings_page_grid_lines.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_grid.h"

LC_SettingsPageGridLines::LC_SettingsPageGridLines(QObject* parent)
    : LC_SettingsPageBase(tr("Lines Grid"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Grid::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGridLines>()) {
    setSortWeight(30);
}

LC_SettingsPageGridLines::~LC_SettingsPageGridLines() = default;

void LC_SettingsPageGridLines::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGridLines::setupBindings() {
    using namespace CFG_Grid;
    using namespace CFG_Colors;

    bindInt({
        { ui->sbGridLinesLineWidth, o_GridLinesLineWidth },
        { ui->sbMetaGridLinesWidth, o_metaGridLinesLineWidth }
    });

    bindColor({
        { ui->cbGridLinesColor, ui->pb_gridLines, o_GridLinesColor, tr("Select Grid Lines Color") },
        { ui->cbMetaGridLinesColor, ui->pb_metaLines, o_MetaGridLinesColor, tr("Select Meta-grid Lines Color") }
    });

    ui->wGridLinesLineType->init(false, false, false);
    bindLineType(ui->wGridLinesLineType, o_GridLinesLineType);

    ui->wMetaGridLinesLineType->init(false, false, false);
    bindLineType(ui->wMetaGridLinesLineType, o_metaGridLinesLineType);
}
