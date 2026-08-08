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

#include "lc_settings_page_snap_visual_behavior.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_snap_visual.h"
#include "ui_lc_settings_page_snap_visual_behavior.h"

LC_SettingsPageSnapVisualBehavior::LC_SettingsPageSnapVisualBehavior(QObject* parent)
    : LC_SettingsPageBase(tr("Behavior"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_VisualSnap::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSnapVisualBehavior>()) {
    setSortWeight(20);
}

LC_SettingsPageSnapVisualBehavior::~LC_SettingsPageSnapVisualBehavior() = default;

void LC_SettingsPageSnapVisualBehavior::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageSnapVisualBehavior::setupBehavior() {
    enableWhenChecked(ui->cbVisualSnapAutoAddSnapPoint, ui->cbVSAutoAddLastSnapOnly);
}

void LC_SettingsPageSnapVisualBehavior::setupBindings() {
    using namespace CFG_VisualSnap;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbVisualSnapAutoAddSnapPoint, o_VSSnapAutoAddSnapPoint },
        { ui->cbVSAutoAddLastSnapOnly, o_VSSnapAutoAddLastSnapPointOnly },
        { ui->cbVisualSnapManualAddingWithCTRL, o_VSSnapManualAddingWithCTRL },
        { ui->cbVisualSnapShowNotSnappableGuides, o_VSShowNotSnappableGuides },
        { ui->cbVisualSnapClearSolutionByRMB, o_VSClearSolutionByRMB },
        { ui->cbVisualSnapRaysAngleSnapVertexes, o_VSAngleSnapStepRaysVertexes },
        { ui->cbVisualSnapRaysAngleSnapRelative, o_VSAngleSnapStepRaysRelative },
        { ui->cbVisualSnapVertexVertexDistanceCircles, o_VSVertexVertexDistanceCircles },
        { ui->cbVisualSnapVertexVertexDistancesTangential, o_VSVertexVertexDistanceTangents }
    });

    bindInt({
        { ui->sbVSCatchDistance, o_VSGuidingEntitiesCatchDistance },
    });
}
