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

#include "lc_settings_page_graphic_view_handles.h"

#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_graphic_view_handles.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "rs_settings.h"

LC_SettingsPageGraphicViewHandles::LC_SettingsPageGraphicViewHandles(QObject* parent)
    : LC_SettingsPageBase(tr("Entity Handles"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGraphicViewHandles>()) {
    setSortWeight(40);
}

LC_SettingsPageGraphicViewHandles::~LC_SettingsPageGraphicViewHandles() = default;

void LC_SettingsPageGraphicViewHandles::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGraphicViewHandles::setupBehavior() {
    enableWhenChecked(ui->cbShowRefPointsOnHovering, ui->sbHandleSize);
}

void LC_SettingsPageGraphicViewHandles::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbShowRefPointsOnHovering, o_VisualizeHoveringRefPoints }
    });

    bindInt({
        { ui->sbHandleSize, o_EntityHandleSize }
    });

    bindColor({
        { ui->cbStartHandleColor, ui->pb_start, o_EntityHandleStartColor, tr("Select Start Handle Color") },
        { ui->cbHandleColor, ui->pb_handle, o_EntityHandleInternalColor, tr("Select Handle Color") },
        { ui->cbEndHandleColor, ui->pb_end, o_EntityHandleEndColor, tr("Select End Handle Color") }
    });
}
