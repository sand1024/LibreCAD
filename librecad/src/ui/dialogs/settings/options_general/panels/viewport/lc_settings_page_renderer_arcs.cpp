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

#include "lc_settings_page_renderer_arcs.h"
#include "ui_lc_settings_page_renderer_arcs.h"
#include "lc_settings_backend.h"
#include "lc_settings_render.h"

LC_SettingsPageRendererArcs::LC_SettingsPageRendererArcs(QObject* parent)
    : LC_SettingsPageBase(tr("Arcs Rendering"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Render::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageRendererArcs>()) {
    setSortWeight(20);
}

LC_SettingsPageRendererArcs::~LC_SettingsPageRendererArcs() = default;

void LC_SettingsPageRendererArcs::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageRendererArcs::setupBehavior() {
    enableWhenChecked(ui->rbRenderArcInterpolate, ui->groupBox_23);
    enableWhenChecked(ui->rbRenderArcInterpolate, ui->rbRenderCirclesAsArcs);
}

void LC_SettingsPageRendererArcs::setupBindings() {
    using namespace CFG_Render;

    bindBoolean({
        { ui->rbRenderCirclesAsArcs, o_CircleRenderAsArcs }
    });

    // Overloaded type-safe custom bindings [4.1]
    bindCustom<QRadioButton, bool>(ui->rbRenderArcInterpolate, o_ArcRenderInterpolate, false,
        &QRadioButton::toggled,
        [](QRadioButton* w) { return w->isChecked(); },
        [this](QRadioButton* w, bool v) {
            w->setChecked(v);
            ui->rbRenderArcQT->setChecked(!v);
        });

    bindCustom<QRadioButton, bool>(ui->rbRenderArcMethodFixed, o_ArcRenderInterpolateSegmentFixed, false,
        &QRadioButton::toggled,
        [](QRadioButton* w) { return w->isChecked(); },
        [this](QRadioButton* w, bool v) {
            w->setChecked(v);
            ui->rbRenderArcMethodSagitta->setChecked(!v);
        });

    // Custom double-scaled bindings (* 100 on disk storage)
    bindCustom<QSpinBox, int>(ui->sbRenderArcSegmentAngle, o_ArcRenderInterpolateSegmentAngle, false,
        &QSpinBox::valueChanged,
        [](QSpinBox* w) { return w->value() * 100; },
        [](QSpinBox* w, int v) { w->setValue(v / 100); });

    bindCustom<QDoubleSpinBox, int>(ui->sbRenderArcMaxSagitta, o_ArcRenderInterpolateSegmentSagitta, false,
        &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });
}
