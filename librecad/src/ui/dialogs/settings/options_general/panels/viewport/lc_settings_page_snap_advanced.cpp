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

#include "lc_settings_page_snap_general.h"
#include "ui_lc_settings_page_snap_general.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_defaults.h"
#include "lc_settings_snap.h"
#include "qg_linetypebox.h"
#include "rs_settings.h"

LC_SettingsPageSnapGeneral::LC_SettingsPageSnapGeneral(QObject* parent)
    : LC_SettingsPageBase(tr("General Snap"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Snap::Group), parent), ui(std::make_unique<Ui::LC_SettingsPageSnapGeneral>())  {
    setSortWeight(10);
}

LC_SettingsPageSnapGeneral::~LC_SettingsPageSnapGeneral() = default;

void LC_SettingsPageSnapGeneral::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageSnapGeneral::setupBehavior() {
    enableWhenChecked(ui->indicator_lines_checkbox, ui->indicator_lines_combobox);
    enableWhenChecked(ui->indicator_shape_checkbox, ui->indicator_shape_combobox);
}

void LC_SettingsPageSnapGeneral::setupBindings() {
     using namespace CFG_Snap;

    bindBoolean({
        { ui->indicator_lines_checkbox, o_IndicatorDrawLines },
        { ui->indicator_shape_checkbox, o_IndicatorDrawShape },
        { ui->cursor_hiding_checkbox, CFG_Appearance::o_CursorHidingWhenSnapping },
        { ui->cbShowSnapOptionsInSnapBar, CFG_Appearance::o_showSnapOptionsInSnapToolbar },
        { ui->cbDontSnapToInvisibleGrid, CFG_Appearance::o_SnapGridIgnoreIfNoGrid },
        { ui->cbAngleSnapToLinesIfGrid, o_AngleSnapToLinesIfGrid }
    });

    bindInt({
         {ui->sbShapeSize, o_IndicatorShapeSize},
        {ui->sbSnapLinesLineWidth, o_IndicatorScreenLinesLineWidth},
        { ui->sbAngleSnapMarkRadius, CFG_Appearance::o_AngleSnapMarkerSize },
        { ui->sbCatchEntitySnapDistance, o_AdvSnapEntityCatchRange }
    });

    bindComboIndex({
        { ui->indicator_lines_combobox, o_IndicatorLinesType },
        { ui->indicator_shape_combobox, o_IndicatorShapeType },
        { ui->cbAngleSnapStep, CFG_Defaults::o_AngleSnapStep }
    });

    // Custom double-scaled bindings (* 100 on disk storage)
    bindCustom<QDoubleSpinBox, int>(ui->sbFreeSnapSwitchDistance, o_AdvSnapOnEntitySwitchToFreeDistance, false,
        &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindCustom<QDoubleSpinBox, int>(ui->sbMinGridCellSnapFactor, o_AdvSnapGridCellSnapFactor, false,
        &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindColor({
        { ui->cb_snap_lines_color, ui->pb_snap_lines_color, CFG_Colors::o_SnapIndicatorLines, tr("Select Snap Indicator Lines Color") },
        { ui->cb_snap_color, ui->pb_snap_color, CFG_Colors::o_SnapIndicator, tr("Select Snap Indicator Shape Color") }
    });

    ui->wSnapLinesLineType->init(false, false, false);

    bindLineType(ui->wSnapLinesLineType, o_IndicatorLinesLineType);
}
