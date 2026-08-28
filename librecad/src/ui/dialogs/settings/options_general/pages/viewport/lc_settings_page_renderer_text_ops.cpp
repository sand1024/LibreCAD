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

#include "lc_settings_page_renderer_text_ops.h"
#include "ui_lc_settings_page_renderer_text_ops.h"
#include "lc_settings_backend.h"
#include "lc_settings_render.h"

LC_SettingsPageRendererTextOps::LC_SettingsPageRendererTextOps(QObject* parent)
    : LC_SettingsPageBase(tr("General"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Render::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageRendererTextOps>()) {
    setSortWeight(110);
}

LC_SettingsPageRendererTextOps::~LC_SettingsPageRendererTextOps() = default;

void LC_SettingsPageRendererTextOps::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageRendererTextOps::setupBindings() {
    using namespace CFG_Render;

    bindBoolean({
        { ui->cbTextDraftOnPanning, o_DrawTextsAsDraftInPanning },
        { ui->cbTextDraftInPreview, o_DrawTextsAsDraftInPreview }
    });


    bindInt({
        { ui->sbFontLettersColumnCount, o_FontLettersColumnsCount }
    });
}
