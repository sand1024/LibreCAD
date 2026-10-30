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

#include "lc_settings_page_drafting_behavior_info_cursor.h"

#include "lc_settings_backend.h"
#include "lc_settings_info_overlay_cursor.h"
#include "ui_lc_settings_page_drafting_behavior_info_cursor.h"

LC_SettingsPageDraftingBehaviorInfoCursor::LC_SettingsPageDraftingBehaviorInfoCursor(QObject* parent)
    : LC_SettingsPageBase(tr("Informational Cursor"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_InfoCursor::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingBehaviorInfoCursor>()) {
    setSortWeight(40);
}

LC_SettingsPageDraftingBehaviorInfoCursor::~LC_SettingsPageDraftingBehaviorInfoCursor() = default;

void LC_SettingsPageDraftingBehaviorInfoCursor::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingBehaviorInfoCursor::setupBehavior() {
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_2);
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_18);
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_16);
    enableWhenChecked(ui->cbInfoOverlayRelative, ui->cbInfoOverlayRelativeDeltas);
    enableWhenChecked(ui->cbInfoOverlayAbsolutePosition, ui->cbShowWorldCoordinates);
    enableWhenChecked(ui->cbInfoOverlayCommandPrompt, ui->cbInfoOverlayCommandName);
}

void LC_SettingsPageDraftingBehaviorInfoCursor::setupBindings() {
    using namespace CFG_InfoCursor;

    bindBoolean({
        { ui->cbInfoOverlayEnable, o_InfoCursorEnabled },
        { ui->cbInfoOverlayRelative, o_ShowRelativeDA },
        { ui->cbInfoOverlayRelativeDeltas, o_ShowRelativeDD },
        { ui->cbInfoOverlayCommandPrompt, o_ShowPrompt },
        { ui->cbInfoOverlayCommandName, o_ShowActionName },
        { ui->cbInfoOverlayAbsolutePosition, o_ShowAbsolute },
        { ui->cbShowWorldCoordinates, o_ShowAbsoluteWCS },
        { ui->cbInfoOverlaySnap, o_ShowSnapInfo },
        { ui->cbInfoOverlaySnapEntityInfo, o_ShowEntityCaughtProperties },
        { ui->cbInfoOverlayPreviewCreatingEntity, o_ShowEntityCreatingProperties },
        { ui->cbInfoOverlayPreviewEditingEntity, o_ShowEntityEditingProperties },
        { ui->cbInfoOverlayShowLabels, o_ShowLabels },
        { ui->cbInfoOverlayInOneLine, o_SingleLine }
    });
}
