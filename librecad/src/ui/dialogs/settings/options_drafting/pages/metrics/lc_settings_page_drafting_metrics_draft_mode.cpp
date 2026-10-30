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

#include "lc_settings_page_drafting_metrics_draft_mode.h"

#include "lc_settings_appearance.h"
#include "lc_settings_backend.h"
#include "ui_lc_settings_page_drafting_metrics_draft_mode.h"

LC_SettingsPageDraftingMetricsDraftMode::LC_SettingsPageDraftingMetricsDraftMode(QObject* parent)
    : LC_SettingsPageBase(tr("Draft Mode"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageDraftingMetricsDraftMode>()) {
    setSortWeight(10);
}

LC_SettingsPageDraftingMetricsDraftMode::~LC_SettingsPageDraftingMetricsDraftMode() = default;

void LC_SettingsPageDraftingMetricsDraftMode::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingMetricsDraftMode::setupBindings() {
    using namespace CFG_Appearance;

    bindInt({
        {ui->sbDraftMarkerFontSize, o_DraftMarkerFontSize}
    });

    bindDouble({
        {ui->sbDraftModeScreenLineWidth, o_DraftModeScreenLineWidth}
    });
}
