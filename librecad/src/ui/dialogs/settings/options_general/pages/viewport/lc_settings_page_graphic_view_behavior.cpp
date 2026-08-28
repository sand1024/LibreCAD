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

#include "lc_settings_page_graphic_view_behavior.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_graphic_view_behavior.h"
#include "lc_settings_backend.h"
#include "lc_settings_defaults.h"

LC_SettingsPageGraphicViewBehavior::LC_SettingsPageGraphicViewBehavior(QObject* parent)
    : LC_SettingsPageBase(tr("Behavior"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGraphicViewBehavior>()) {
    setSortWeight(10);
}

LC_SettingsPageGraphicViewBehavior::~LC_SettingsPageGraphicViewBehavior() = default;

void LC_SettingsPageGraphicViewBehavior::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGraphicViewBehavior::setupBindings() {
    using namespace CFG_Appearance;

    bindBoolean({
        { ui->scrollbars_check_box, o_ScrollBars },
        { ui->cb_antialiasing, o_Antialiasing },
        { ui->cbClassicRendering, o_ClassicRenderer },
        { ui->cb_autopanning, o_Autopanning },
        { ui->cbFirstTimeNoZoom, o_FirstTimeNoZoom },
        { ui->cbPanOnWheelZoom, o_PanOnZoom },
        { ui->cbWheelScrollInvertH, CFG_Defaults::o_WheelScrollInvertH },
        { ui->cbWheelScrollInvertV, CFG_Defaults::o_WheelScrollInvertV },
        { ui->cbInvertZoomDirection, CFG_Defaults::o_InvertZoomDirection }
    });

    // Symmetrical Overloaded bindCustom: accepts the o_ScrollZoomFactor descriptor directly [4.1]
    bindCustom<QDoubleSpinBox, int>(ui->sbDefaultZoomFactor, o_ScrollZoomFactor, false,
        &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 1000.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 1000.0); });
}
