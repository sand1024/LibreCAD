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

#include "lc_settings_page_drafting_theme_snap_visual.h"
#include "ui_lc_settings_page_drafting_theme_snap_visual.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_snap_visual.h"

LC_SettingsPageDraftingThemeSnapVisual::LC_SettingsPageDraftingThemeSnapVisual(QObject* parent)
    : LC_SettingsPageBase(tr("Visual Snap"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_VisualSnap::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingThemeSnapVisual>()) {
    setSortWeight(20);
}

LC_SettingsPageDraftingThemeSnapVisual::~LC_SettingsPageDraftingThemeSnapVisual() = default;

void LC_SettingsPageDraftingThemeSnapVisual::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingThemeSnapVisual::setupBehavior() {
}

void LC_SettingsPageDraftingThemeSnapVisual::setupBindings() {
    using namespace CFG_VisualSnap;
    using namespace CFG_Colors;


    ui->wGuidingEntityLineType->init(false, false, false);
    bindLineType(ui->wGuidingEntityLineType, o_VSGuidingEntitiesLineType);
    ui->wActiveGuidingEntityLineType->init(false, false, false);
    bindLineType(ui->wActiveGuidingEntityLineType, o_VSActiveGuidingEntityLineType);
    ui->wDocGuidingEntityLineType->init(false, false, false);
    bindLineType(ui->wDocGuidingEntityLineType, o_VSDocGuidingEntitiesLineType);

    bindColor({
        { ui->cbColorVisualSnapGuideEntities, ui->pbVisualSnapGuideEntitiesColor, o_VisualSnapGuideEntitiesColor, tr("Select Visual Snap Guide Color") },
        { ui->cbColorVisualSnapVertexes, ui->pbVisualSnapVertexesColor, o_VisualSnapVertexesColor, tr("Select Visual Snap Point Color") },
        { ui->cbColorVisualSnapProjectedSnap, ui->pbVisualSnapProjectedSnapColor, o_VisualSnapProjectedSnapColor, tr("Select Projected Snap Color") },
        { ui->cbColorVisualSnapDocumentEntity, ui->pbVisualSnapDocEntitiesColor, o_VisualSnapDocumentEntitiesColor, tr("Select Involved Document Entity Color") }
    });
}
