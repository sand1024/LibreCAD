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

#include "lc_settings_page_drafting_theme_selection.h"


#include "ui_lc_settings_page_drafting_theme_selection.h"
#include "lc_settings_backend.h"
#include "lc_settings_drafting.h"
#include "lc_settings_selection_overlay.h"
#include "lc_settings_snap_indicator.h"

LC_SettingsPageDraftingThemeSelection::LC_SettingsPageDraftingThemeSelection(QObject* parent)
    : LC_SettingsPageBase(tr("Selection"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_DraftingAreaStyling::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingThemeSelection>()) {
}

LC_SettingsPageDraftingThemeSelection::~LC_SettingsPageDraftingThemeSelection() = default;

void LC_SettingsPageDraftingThemeSelection::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingThemeSelection::setupBehavior() {
}

void LC_SettingsPageDraftingThemeSelection::setupBindings() {
    using namespace CFG_DraftingAreaStyling;
    using namespace CFG_EntitiesPickBox;
    using namespace CFG_OverlaySelection;

    bindInt({{ui->sbOverlayBoxTransparency, o_SelectionOverlayTransparency}});

    bindColor({
            { ui->cbStartHandleColor, ui->pb_start, o_ColorEntityHandleStart, tr("Select Start Handle Color") },
            { ui->cbHandleColor, ui->pb_handle, o_ColorEntityHandleInternal, tr("Select Handle Color") },
            { ui->cbEndHandleColor, ui->pb_end, o_ColorEntityHandleEnd, tr("Select End Handle Color") },
            {ui->cbSelectedColor, ui->pb_selected, o_ColorSelectedEntity, tr("Select Selected Entity Color")},
            {ui->cbOverlayBoxLine, ui->pbOverlayBoxLine, o_ColorSelectionOverlayLine, tr("Select Overlay Line Color")},
            {ui->cbOverlayBoxFill, ui->pbOverlayBoxFill, o_ColorSelectionOverlayFill, tr("Select Overlay Fill Color")},
            {ui->cbOverlayBoxLineInverted, ui->pbOverlayBoxLineInverted, o_ColorSelectionOverlayInvertedLine, tr("Select Inverted Line Color")},
            {ui->cbOverlayBoxFillInverted, ui->pbOverlayBoxFillInverted, o_ColorSelectionOverlayInvertedFill, tr("Select Overlay Inverted Fill Color")},
            {ui->cbPickBox, ui->pbPickBoxColor, o_ColorSelectionPickBox, tr("Select Pick Box Color")}
    });

    bindComboIndex({
        { ui->cbPickBoxLinesSize, o_SelectionPickBoxLinesSizeType }
    });

    ui->wOverlaySelectionLineType->init(false, false, false);
    bindLineType(ui->wOverlaySelectionLineType, o_SelectionOverlayLineType);

    ui->wSelectedEntityLineType->init(false, false, false);
    bindLineType(ui->wSelectedEntityLineType, o_SelectedEntityLineType);

    ui->wOverlaySelectionInvertedLineType->init(false, false, false);
    bindLineType(ui->wOverlaySelectionInvertedLineType, o_SelectionOverlayInvertedLineType);
}
