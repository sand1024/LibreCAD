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

#include "lc_settings_page_drafting_theme_info_cursor.h"

#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "ui_lc_settings_page_drafting_theme_info_cursor.h"

LC_SettingsPageDraftingThemeInfoCursor::LC_SettingsPageDraftingThemeInfoCursor(QObject* parent)
    : LC_SettingsPageBase(tr("Info Cursor & Input Assistant"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Colors::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageDraftingThemeInfoCursor>()) {
    setSortWeight(45);
}

LC_SettingsPageDraftingThemeInfoCursor::~LC_SettingsPageDraftingThemeInfoCursor() = default;

void LC_SettingsPageDraftingThemeInfoCursor::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingThemeInfoCursor::setupBindings() {
    using namespace CFG_InfoCursor;
    using namespace CFG_RelativePositionAssistant;
    using namespace CFG_Colors;

    bindComboText({
        { ui->fcbInfoFont, o_InfoCursorFontName },
        { ui->fcbAssistantFont, o_AssistantFontName }
    });

    bindColor({
        { ui->cbAbsColor, ui->pbAbsColor, o_InfoOverlayAbsoluteCoordinatesColor, tr("Select Absolute Coordinates Color") },
        { ui->cbRelColor, ui->pbRelColor, o_InfoOverlayRelativeColor, tr("Select Relative Coordinates Color") },
        { ui->cbPromptColor, ui->pbPromptColor, o_InfoOverlayPromptColor, tr("Select Command Prompt Color") },
        { ui->cbSnapColor, ui->pbSnapColor, o_InfoOverlaySnapInfoColor, tr("Select Snap Indicator Text Color") },
        { ui->cbAssistantBgColor, ui->pbAssistantBgColor, o_RelativePositionAssistantBackgroundColor, tr("Select Assistant Background Color") },
        { ui->cbAssistantTextColor, ui->pbAssistantTextColor, o_RelativePositionAssistantTextColor, tr("Select Assistant Text Color") }
    });
}
