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

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "lc_settings_defaults.h"
#include "lc_settings_grid.h"
#include "ui_lc_settings_page_drafting_behavior_general.h"

LC_SettingsPageDraftingBehaviorGeneral::LC_SettingsPageDraftingBehaviorGeneral(QObject* parent)
    : LC_SettingsPageBase(tr("General"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingBehaviorGeneral>()) {
    setSortWeight(10);
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
    using namespace CFG_Appearance;
    using namespace CFG_Grid;

    bindBoolean({
        // Canvas Chrome & Navigation
        { ui->cbShowScrollbars, o_ScrollBars },
        { ui->cbScrollBarsDrawingExtent, o_ScrollBarsContentBand },
        { ui->cbScrollBarsDrawingExtentTooltip, o_ScrollBarsContentToolTip },
        { ui->cb_antialiasing, o_Antialiasing },
        { ui->cbClassicRendering, o_ClassicRenderer },
        {ui->cb_autopanning, CFG_ZoomAndPan::o_Autopanning},
        {ui->cbDrawGrid, o_GridDraw},
        {ui->cbDrawMetaGrid, o_metaGridDraw},
        {ui->cbDrawVerticalForIsoTop, o_GridDrawIsoVerticalForTop},
        {ui->cbScaleGrid, o_ScaleGrid},
        {ui->cbUnitlessGrid, o_UnitlessGrid},
        { ui->cbVisualizeHovering, o_VisualizeHovering },
        { ui->cbHighlightWIthLinewidthInDraft, o_IgnoreDraftForHighlight },
        { ui->cbShowDraftModeMarker, o_ShowDraftModeMarker },
        { ui->cbDisplayRefPoints, o_VisualizePreviewRefPoints} ,
        { ui->cbShowRefPointsOnHovering, o_VisualizeHoveringRefPoints},
        { ui->cbShowUCSZeroMarker, o_ShowUCSZeroMarker },
        { ui->cbShowWCSZeroMarker, o_ShowWCSZeroMarker },
        { ui->cbHideRelativeZero, o_HideRelativeZero},
        { ui->cbAnglesMarkVisible, o_AnglesBasisMarkEnabled }
    });

    bindComboIndex({
        { ui->cbAnglesBaseShowPolicy, o_AnglesBasisMarkPolicy }
    });


}
