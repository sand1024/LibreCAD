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

#include "lc_settings_page_drafting_metrics_markers.h"

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "ui_lc_settings_page_drafting_metrics_markers.h"

LC_SettingsPageDraftingMetricsMarkers::LC_SettingsPageDraftingMetricsMarkers(QObject* parent)
    : LC_SettingsPageBase(tr("Markers and Handles"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageDraftingMetricsMarkers>()) {
    setSortWeight(10);
}

LC_SettingsPageDraftingMetricsMarkers::~LC_SettingsPageDraftingMetricsMarkers() = default;

void LC_SettingsPageDraftingMetricsMarkers::setupUi() {
    ui->setupUi(m_widget);
    ui->sbHighlightedEntityScreenLineWidth->setSpecialValueText("Auto");
    ui->sbSelectedEntityScreenLineWidth->setSpecialValueText("Auto");
}

void LC_SettingsPageDraftingMetricsMarkers::setupBindings() {
    using namespace CFG_Appearance;

    bindInt({
        {ui->sbHandleSize, o_EntityHandleSize},
        {ui->sbDraftMarkerFontSize, o_DraftMarkerFontSize},
        {ui->sbPreviewRefPointsSize, o_PreviewRefPointSize}
    });

    bindDouble({
        {ui->sbOverlaysLineWidth, o_OverlaysScreenLineWidth},
        {ui->sbDraftModeScreenLineWidth, o_DraftModeScreenLineWidth},
        {ui->sbPreviewScreenLineWidth, o_PreviewScreenLineWidth},
        {ui->sbHighlightedEntityScreenLineWidth, o_HighlightedEntityScreenLineWidth},
        {ui->sbSelectedEntityScreenLineWidth, o_SelectedEntityScreenLineWidth}
    });
}
