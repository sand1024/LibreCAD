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

#include "lc_settings_page_drafting_behavior_keyboard_and_mouse.h"

#include "lc_settings_backend.h"
#include "lc_settings_cad_preferences.h"
#include "lc_settings_drafting.h"
#include "lc_settings_drafting_keyboard.h"
#include "lc_settings_selection.h"
#include "ui_lc_settings_page_drafting_behavior_keyboard_and_mouse.h"

LC_SettingsPageDraftingBehaviorKeyboardAndMouse::LC_SettingsPageDraftingBehaviorKeyboardAndMouse(QObject* parent)
    : LC_SettingsPageBase(tr("Keyboard and Mouse"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_DraftingKeyboard::Group),
                           parent)  , ui(std::make_unique<Ui::LC_SettingsPageDraftingBehaviorKeyboardAndMouse>()) {
}

LC_SettingsPageDraftingBehaviorKeyboardAndMouse::~LC_SettingsPageDraftingBehaviorKeyboardAndMouse() = default;

void LC_SettingsPageDraftingBehaviorKeyboardAndMouse::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageDraftingBehaviorKeyboardAndMouse::setupBindings() {
    using namespace CFG_DraftingKeyboard;
    using namespace CFG_DraftingAreaStyling;
    using namespace CFG_DraftingMouseWheel;
    using namespace CFG_DraftingZoomAndPan;
    using namespace CFG_CADPreferences;
    using namespace CFG_Selection;


    bindBoolean({
        {ui->cbEvaluateOnSpace, o_EvaluateCommandOnSpace},
        {ui->cbToggleFreeSnapOnSpace, o_ToggleFreeSnapOnSpace},
        {ui->cbEnableKeyboardZoomAdjust, o_AllowScrollMoveAdjustByKeys},

        {ui->cbFirstTimeNoZoom, o_FirstTimeNoZoom},
        {ui->cbPanOnWheelZoom, o_PanOnZoom},

        {ui->cbWheelScrollInvertH, o_WheelScrollInvertH},
        {ui->cbWheelScrollInvertV, o_WheelScrollInvertV},
        {ui->cbInvertZoomDirection, o_InvertZoomDirection},
        {ui->cbNonLiniearZoom, o_NonLinearZoomFactor},

        { ui->cbFinishMovingByMouseUp, CFG_DraftingOperations::o_AdHockMovingEndsByMouseClick},
        { ui->cbAllowLassoSelection, o_AllowLassoSelectionByPressedMouseButton }
    });

    bindScaledDouble(ui->sbDefaultZoomFactor, o_ScrollZoomFactor, 1000.0, false);
}
