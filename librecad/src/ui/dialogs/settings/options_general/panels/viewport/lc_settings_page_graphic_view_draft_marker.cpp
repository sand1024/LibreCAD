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

#include "lc_settings_page_graphic_view_draft_marker.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_graphic_view_draft_marker.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "rs_settings.h"

LC_SettingsPageGraphicViewDraftMarker::LC_SettingsPageGraphicViewDraftMarker(QObject* parent)
    : LC_SettingsPageBase(tr("Draft Mode Marker"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGraphicViewDraftMarker>()) {
    setSortWeight(50);
}

LC_SettingsPageGraphicViewDraftMarker::~LC_SettingsPageGraphicViewDraftMarker() = default;

void LC_SettingsPageGraphicViewDraftMarker::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGraphicViewDraftMarker::setupBehavior() {
    enableWhenChecked(ui->cbShowDraftModeMarker, ui->cbDraftModeMarkerColor);
    enableWhenChecked(ui->cbShowDraftModeMarker, ui->pbDraftModeColor);
    enableWhenChecked(ui->cbShowDraftModeMarker, ui->fcbDraftModeFont);
    enableWhenChecked(ui->cbShowDraftModeMarker, ui->sbDraftModeFontSize);
}

void LC_SettingsPageGraphicViewDraftMarker::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbShowDraftModeMarker, o_ShowDraftModeMarker }
    });

    bindInt({
        { ui->sbDraftModeFontSize, o_DraftMarkerFontSize }
    });

    bindComboText({
         { ui->fcbDraftModeFont, o_DraftMarkerFontName }
     });

    bindColor({
        { ui->cbDraftModeMarkerColor, ui->pbDraftModeColor, o_DraftModeMarker, tr("Select Draft Marker Color") }
    });
}
