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

#include "lc_settings_page_snap_indicator.h"

#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_snap.h"
#include "ui_lc_settings_page_snap_indicator.h"

LC_SettingsPageSnapIndicator::LC_SettingsPageSnapIndicator(QObject* parent)
    : LC_SettingsPageBase(tr("Snap Indicator"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Snap::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSnapIndicator>()) {
    setSortWeight(30);
}

LC_SettingsPageSnapIndicator::~LC_SettingsPageSnapIndicator() = default;

void LC_SettingsPageSnapIndicator::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageSnapIndicator::setupBehavior() {
    enableWhenChecked(ui->cbDrawLines, ui->cbLinesType);
    enableWhenChecked(ui->cbDrawShape, ui->cbShapeType);
}

void LC_SettingsPageSnapIndicator::setupBindings() {
    using namespace CFG_Snap;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbDrawLines, o_IndicatorDrawLines },
        { ui->cbDrawShape, o_IndicatorDrawShape }
    });

    bindComboIndex({
        { ui->cbLinesType, o_IndicatorLinesType },
        { ui->cbShapeType, o_IndicatorShapeType }
    });

    bindColor({
        { ui->cbShapeColor, ui->pbShapeColor, o_SnapIndicator, tr("Select Snap Indicator Shape Color") },
        { ui->cbLinesColor, ui->pbLinesColor, o_SnapIndicatorLines, tr("Select Snap Indicator Lines Color") }
    });

    ui->wSnapLinesLineType->init(false, false, false);
    bindLineType(ui->wSnapLinesLineType, o_IndicatorLinesLineType);
}
