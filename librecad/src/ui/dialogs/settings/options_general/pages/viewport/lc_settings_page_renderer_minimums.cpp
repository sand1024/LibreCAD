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

#include "lc_settings_page_renderer_minimums.h"
#include "ui_lc_settings_page_renderer_minimums.h"
#include "lc_settings_backend.h"
#include "lc_settings_render.h"

LC_SettingsPageRendererMinimums::LC_SettingsPageRendererMinimums(QObject* parent)
    : LC_SettingsPageBase(tr("Rendering Minimums"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Render::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageRendererMinimums>()) {
    setSortWeight(30);
}

LC_SettingsPageRendererMinimums::~LC_SettingsPageRendererMinimums() = default;

void LC_SettingsPageRendererMinimums::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageRendererMinimums::setupBindings() {
    using namespace CFG_Render;

    bindInt({
        { ui->sbTextMinHeight, o_MinRenderableTextHeightPx }
    });

    // Symmetrical Double-scaled custom binders (* 100 on save)
    bindCustom<QDoubleSpinBox, int>(ui->sbRenderMinLineLen, o_MinLineLen, false,
    &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindCustom<QDoubleSpinBox, int>(ui->sbRenderMinEllipseMajor, o_MinEllipseMajor, false,
    &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindCustom<QDoubleSpinBox, int>(ui->sbRenderMinArcRadius, o_MinArcRadius, false,
    &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindCustom<QDoubleSpinBox, int>(ui->sbRenderMinCircleRadius, o_MinCircleRadius, false,
    &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });

    bindCustom<QDoubleSpinBox, int>(ui->sbRenderMinEllipseMinor, o_MinEllipseMinor, false,
    &QDoubleSpinBox::valueChanged,
        [](QDoubleSpinBox* w) { return static_cast<int>(w->value() * 100.0); },
        [](QDoubleSpinBox* w, int v) { w->setValue(v / 100.0); });
}
