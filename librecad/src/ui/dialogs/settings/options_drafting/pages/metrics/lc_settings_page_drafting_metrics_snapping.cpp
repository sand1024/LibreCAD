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

#include "lc_settings_page_drafting_metrics_snapping.h"

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "ui_lc_settings_page_drafting_metrics_snapping.h"

LC_SettingsPageDraftingMetricsSnapping::LC_SettingsPageDraftingMetricsSnapping(QObject* parent)
    : LC_SettingsPageBase(tr("Snapping"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Snap::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingMetricsSnapping>()) {
    setSortWeight(20);
}

LC_SettingsPageDraftingMetricsSnapping::~LC_SettingsPageDraftingMetricsSnapping() = default;

void LC_SettingsPageDraftingMetricsSnapping::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingMetricsSnapping::setupBindings() {
    using namespace CFG_Snap;
    using namespace CFG_VisualSnap;
    using namespace CFG_Appearance;

    bindInt({
        { ui->sbIndicatorShapeSize, o_IndicatorShapeSize },
        { ui->sbIndicatorLinesLineWidth, o_IndicatorScreenLinesLineWidth },
        { ui->sbVSProjectedSnapSize, o_VSProjectedSnapSize },
        { ui->sbVSHighlightedVertexSize, o_VSHighlightedVertexSize },
        { ui->sbVSVertexSize, o_VSVertexSize },
        { ui->sbVSGuidingEntitiesLineWidth, o_VSGuidingEntitiesLineWidth },
        { ui->sbVSGuidingPointsLineWidth, o_VSGuidingPointsLineWidth },
        { ui->sbVSGuidingEntityLabelFontSize, o_VSGuidingEntityLabelFontSize },
        { ui->sbAngleSnapMarkerSize, o_AngleSnapMarkerSize },
        { ui->sbVSGuidingLabelOffsetPx, o_VSGuidingLabelOffsetPx }
    });
}
