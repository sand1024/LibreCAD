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

#include "lc_settings_page_renderer_advanced.h"

#include "lc_settings_backend.h"
#include "lc_settings_render.h"
#include "ui_lc_settings_page_renderer_advanced.h"

LC_SettingsPageRendererAdvanced::LC_SettingsPageRendererAdvanced(QObject* parent)
    : LC_SettingsPageBase(tr("Advanced"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Render::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageRendererAdvanced>()) {
    setSortWeight(20);
}

LC_SettingsPageRendererAdvanced::~LC_SettingsPageRendererAdvanced() = default;

void LC_SettingsPageRendererAdvanced::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageRendererAdvanced::setupBehavior() {
    enableWhenChecked(ui->rbRenderArcInterpolate, ui->gbInterpolateMethod);
    enableWhenChecked(ui->rbRenderArcInterpolate, ui->cbRenderCirclesAsArcs);
}

void LC_SettingsPageRendererAdvanced::setupBindings() {
    using namespace CFG_Render;

    // Circles as Arcs
    bindBoolean({
        { ui->cbRenderCirclesAsArcs, o_CircleRenderAsArcs }
    });

    // Native vs Tessellated Radio
    bindCustom<QRadioButton, bool>(ui->rbRenderArcInterpolate, o_ArcRenderInterpolate, false,
        &QRadioButton::toggled,
        [](QRadioButton* w) { return w != nullptr ? w->isChecked() : false; },
        [this](QRadioButton* w, bool v) {
            if (w != nullptr) {
                w->setChecked(v);
                ui->rbRenderArcQT->setChecked(!v);
            }
        });

    // Fixed Angle vs Sagitta Method Radio
    bindCustom<QRadioButton, bool>(ui->rbRenderArcMethodFixed, o_ArcRenderInterpolateSegmentFixed, false,
        &QRadioButton::toggled,
        [](QRadioButton* w) { return w != nullptr ? w->isChecked() : false; },
        [this](QRadioButton* w, bool v) {
            if (w != nullptr) {
                w->setChecked(v);
                ui->rbRenderArcMethodSagitta->setChecked(!v);
            }
        });

    // Angle step (scaled * 100 on disk)
    bindScaledDouble(ui->sbRenderArcSegmentAngle, o_ArcRenderInterpolateSegmentAngle);

    // Sagitta tolerance (scaled * 100 on disk)
    bindScaledDouble(ui->sbRenderArcMaxSagitta, o_ArcRenderInterpolateSegmentSagitta);

    // Culling Minimums
    bindInt({
        { ui->sbTextMinHeight, o_MinRenderableTextHeightPx }
    });

    bindScaledDouble(ui->sbRenderMinLineLen, o_MinLineLen);
    bindScaledDouble(ui->sbRenderMinArcRadius, o_MinArcRadius);
    bindScaledDouble(ui->sbRenderMinCircleRadius, o_MinCircleRadius);
    bindScaledDouble(ui->sbRenderMinEllipseMajor, o_MinEllipseMajor);
    bindScaledDouble(ui->sbRenderMinEllipseMinor, o_MinEllipseMinor);
}
