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

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_drafting_theme_selection.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "rs_settings.h"

LC_SettingsPageDraftingThemeSelection::LC_SettingsPageDraftingThemeSelection(QObject* parent)
    : LC_SettingsPageBase(tr("Selection"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingThemeSelection>()) {
    setSortWeight(40);
}

LC_SettingsPageDraftingThemeSelection::~LC_SettingsPageDraftingThemeSelection() = default;

void LC_SettingsPageDraftingThemeSelection::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingThemeSelection::setupBehavior() {
}

void LC_SettingsPageDraftingThemeSelection::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;


    bindInt({{ui->sbOverlayBoxTransparency, o_OverlayBoxTransparency}});

    bindColor({
            { ui->cbStartHandleColor, ui->pb_start, o_EntityHandleStartColor, tr("Select Start Handle Color") },
            { ui->cbHandleColor, ui->pb_handle, o_EntityHandleInternalColor, tr("Select Handle Color") },
            { ui->cbEndHandleColor, ui->pb_end, o_EntityHandleEndColor, tr("Select End Handle Color") },
            {ui->cbSelectedColor, ui->pb_selected, o_SelectedEntityColor, tr("Select Selected Entity Color")},
            {ui->cbOverlayBoxLine, ui->pbOverlayBoxLine, o_OverlayBoxLineColor, tr("Select Overlay Line Color")},
            {ui->cbOverlayBoxFill, ui->pbOverlayBoxFill, o_OverlayBoxFillColor, tr("Select Overlay Fill Color")},
            {ui->cbOverlayBoxLineInverted, ui->pbOverlayBoxLineInverted, o_OverlayBoxLineInvertedColor, tr("Select Inverted Line Color")},
            {ui->cbOverlayBoxFillInverted, ui->pbOverlayBoxFillInverted, o_OverlayBoxFillInvertedColor, tr("Select Inverted Fill Color")}
    });

    ui->wOverlaySelectionLineType->init(false, false, false);

    bindLineType(ui->wOverlaySelectionLineType, o_SelectionOverlayLineType);

    ui->wOverlaySelectionInvertedLineType->init(false, false, false);
    bindLineType(ui->wOverlaySelectionInvertedLineType, o_SelectionOverlayInvertedLineType);
}
