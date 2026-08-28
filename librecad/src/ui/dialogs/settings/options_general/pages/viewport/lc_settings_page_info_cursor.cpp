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
// context around LC_SettingsPageInfoCursor methods inside lc_settings_page_info_cursor.cpp:
#include "lc_settings_page_info_cursor.h"
#include "ui_lc_settings_page_info_cursor.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "lc_settings_info_overlay_cursor.h"
#include "rs_settings.h"

LC_SettingsPageInfoCursor::LC_SettingsPageInfoCursor(QObject* parent)
    : LC_SettingsPageBase(tr("Informational Cursor"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_InfoOverlayCursor::Group), parent) , ui(std::make_unique<Ui::LC_SettingsPageInfoCursor>()) {
    // setSortWeight(40);
}

LC_SettingsPageInfoCursor::~LC_SettingsPageInfoCursor() = default;

void LC_SettingsPageInfoCursor::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageInfoCursor::setupBehavior() {
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_17);
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_18);
    enableWhenChecked(ui->cbInfoOverlayEnable, ui->groupBox_16);
    enableWhenChecked(ui->cbInfoOverlayRelative, ui->cbInfoOverlayRelativeDeltas);
    enableWhenChecked(ui->cbInfoOverlayAbsolutePosition, ui->cbShowWorldCoordinates);
    enableWhenChecked(ui->cbInfoOverlayCommandPrompt, ui->cbInfoOverlayCommandName);
}

void LC_SettingsPageInfoCursor::setupBindings() {
    using namespace CFG_InfoOverlayCursor;

    bindBoolean({
        { ui->cbInfoOverlayEnable, o_InfoCursorEnabled },
        { ui->cbInfoOverlayRelative, o_ShowRelativeDA },
        { ui->cbInfoOverlayRelativeDeltas, o_ShowRelativeDD },
        { ui->cbInfoOverlayCommandPrompt, o_ShowPrompt },
        { ui->cbInfoOverlayCommandName, o_ShowActionName },
        { ui->cbInfoOverlayAbsolutePosition, o_ShowAbsolute },
        { ui->cbShowWorldCoordinates, o_ShowAbsoluteWCS },
        { ui->cbInfoOverlaySnap, o_ShowSnapInfo },
        { ui->cbInfoOverlaySnapEntityInfo, o_ShowPropertiesCatched },
        { ui->cbInfoOverlayPreviewCreatingEntity, o_ShowPropertiesCreating },
        { ui->cbInfoOverlayPreviewEditingEntity, o_ShowPropertiesEdit },
        { ui->cbInfoOverlayShowLabels, o_ShowLabels },
        { ui->cbInfoOverlayInOneLine, o_SingleLine }
    });

    bindInt({
        { ui->sbInfoOverlayFontSize, o_FontSize },
        { ui->sbInfoOverlayOffset, o_OffsetFromCursor }
    });

    bindComboText({
           { ui->fcbInfoOverlayFont, o_FontName }
       });

    bindColor({
        { ui->cbInfoOverlayRelativeColor, ui->pbInfoOverlayRelativeColor, CFG_Colors::o_InfoOverlayRelativeColor, tr("Select Relative Coordinates Color") },
        { ui->cbInfoOverlayCommandPromptColor, ui->pbInfoOverlayCommandPromptColor, CFG_Colors::o_InfoOverlayPromptColor, tr("Select Command Prompt Color") },
        { ui->cbInfoOverlayAbsolutePositionColor, ui->pbcbInfoOverlayAbsolutePositionColor, CFG_Colors::o_InfoOverlayAbsoluteCoordinatesColor, tr("Select Absolute Coordinates Color") },
        { ui->cbInfoOverlaySnapColor, ui->pbInfoOverlaySnapColor, CFG_Colors::o_InfoOverlaySnapInfoColor, tr("Select Snap Marker Color") }
    });
}
