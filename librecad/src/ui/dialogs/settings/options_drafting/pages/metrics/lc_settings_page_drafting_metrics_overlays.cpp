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

#include "lc_settings_page_drafting_metrics_overlays.h"

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "ui_lc_settings_page_drafting_metrics_overlays.h"

LC_SettingsPageDraftingMetricsOverlays::LC_SettingsPageDraftingMetricsOverlays(QObject* parent)
    : LC_SettingsPageBase(tr("Grid and Tools"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingMetricsOverlays>()) {
    setSortWeight(30);
}

LC_SettingsPageDraftingMetricsOverlays::~LC_SettingsPageDraftingMetricsOverlays() = default;

void LC_SettingsPageDraftingMetricsOverlays::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingMetricsOverlays::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Grid;
    using namespace CFG_InfoOverlayCursor;
    using namespace CFG_RelativePositionAssistant;

    bindInt({
        { ui->sbHandleSize, o_EntityHandleSize },
        { ui->sbGridLinesLineWidth, o_GridLinesLineWidth },
        { ui->sbMetaGridLinesLineWidth, o_metaGridLinesLineWidth },
        { ui->sbMetaGridPointsLineWidth, o_metaGridPointsLineWidth },
        { ui->sbGridPointsPointSize, o_GridPointsPointSize },
        { ui->sbInfoOverlayFontSize, o_FontSize },
        { ui->sbOffsetFromCursor, o_OffsetFromCursor },
        { ui->sbAssistantFontSize, o_AssistantFontSize }
    });
}
