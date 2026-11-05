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

#include "lc_settings_page_drafting_behavior_general.h"

#include "lc_settings_backend.h"
#include "lc_settings_drafting.h"
#include "lc_settings_draft_mode.h"
#include "lc_settings_grid.h"
#include "lc_settings_preview.h"
#include "ui_lc_settings_page_drafting_behavior_general.h"

LC_SettingsPageDraftingBehaviorGeneral::LC_SettingsPageDraftingBehaviorGeneral(QObject* parent)
    : LC_SettingsPageBase(tr("General"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_DraftingAreaStyling::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingBehaviorGeneral>()) {
}

LC_SettingsPageDraftingBehaviorGeneral::~LC_SettingsPageDraftingBehaviorGeneral() = default;

void LC_SettingsPageDraftingBehaviorGeneral::setupBehavior() {
    LC_SettingsPageBase::setupBehavior();
    enableWhenChecked(ui->cbShowScrollbars, ui->cbScrollBarsDrawingExtent);
    enableWhenChecked(ui->cbShowScrollbars, ui->cbScrollBarsDrawingExtentTooltip);

    enableWhenChecked(ui->cbVisualizeHovering, ui->cbHighlightWIthLinewidthInDraft);
    enableWhenChecked(ui->cbVisualizeHovering, ui->cbShowRefPointsOnHovering);
}

void LC_SettingsPageDraftingBehaviorGeneral::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingBehaviorGeneral::setupBindings() {
    using namespace CFG_DraftingAreaStyling;
    using namespace CFG_DraftingOperations;
    using namespace CFG_Grid;
    using namespace CFG_DraftMode;
    using namespace CFG_DraftingPreview;

    bindBoolean({
        // Canvas Chrome & Navigation
        { ui->cbShowScrollbars, o_ScrollBarsVisibleForDraftingArea },
        { ui->cbScrollBarsDrawingExtent, o_ScrollBarsContentBand },
        { ui->cbScrollBarsDrawingExtentTooltip, o_ScrollBarsContentToolTip },
        { ui->cb_antialiasing, o_Antialiasing },
        { ui->cbClassicRendering, o_ClassicRenderer },
        {ui->cb_autopanning, CFG_DraftingZoomAndPan::o_Autopanning},
        { ui->cbVisualizeHovering, o_HighlightEntitiesOnHover },
        { ui->cbHighlightWIthLinewidthInDraft, o_IgnoreDraftForHighlight },
        { ui->cbShowDraftModeMarker, o_ShowDraftModeMarker },
        { ui->cbDisplayRefPoints, o_ShowRefEntitiesOnPreview} ,
        { ui->cbShowRefPointsOnHovering, o_HighlightEntitiesRefPointsOnHover},
        { ui->cbToActiveLayer, o_ModifyEntitiesToActiveLayer},
        { ui->cbKeepModifiedSelected, o_KeepModifiedEntitiesSelected},
        { ui->cbAutoZoomDrawing, o_AutoZoomDrawingAfterOpening },
    });

    bind(ui->sbMaxPreview, o_MaxEntitiesNumberInPreview, false);
}
