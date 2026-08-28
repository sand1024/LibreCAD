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

#include "lc_settings_page_graphic_view_colors.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_graphic_view_colors.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "qg_linetypebox.h"


LC_SettingsPageGraphicViewColors::LC_SettingsPageGraphicViewColors(QObject* parent)
    : LC_SettingsPageBase(tr("Main Colors"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Colors::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageGraphicViewColors>()) {
    setSortWeight(30);
}

LC_SettingsPageGraphicViewColors::~LC_SettingsPageGraphicViewColors() = default;

void LC_SettingsPageGraphicViewColors::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGraphicViewColors::setupBindings() {
    using namespace CFG_Colors;
    using namespace CFG_Appearance;

    bindInt({{ui->sbOverlaysLineWidth, o_OverlaysScreenLineWidth}});

    bindColor({
        {ui->cbBackgroundColor, ui->pb_background, o_BackgroundColor, tr("Select Background Color")},
        {ui->cbForegroundColor, ui->pb_selected, o_ForegroundBWColor, tr("Select Foreground Entity Color")}
    });
}
