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

#include "lc_settings_page_general_user_interface.h"

#include "lc_settings_app_styling.h"
#include "lc_settings_cad_preferences.h"
#include "lc_settings_defaults.h"
#include "lc_settings_drafting.h"
#include "lc_settings_startup.h"
#include "lc_settings_window_options.h"
#include "ui_lc_settings_page_general_user_interface.h"

LC_SettingsPageGeneralUserInterface::LC_SettingsPageGeneralUserInterface(QObject* parent)
    : LC_SettingsPageBase(tr("User Interface"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_DraftingAreaStyling::Group),
                           parent), ui(std::make_unique<Ui::LC_SettingsPageGeneralUserInterface>()) {
}

LC_SettingsPageGeneralUserInterface::~LC_SettingsPageGeneralUserInterface() = default;

void LC_SettingsPageGeneralUserInterface::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGeneralUserInterface::setupBehavior() {
    enableWhenChecked(ui->cbPersistentDialogs, ui->cbPersistentDialogSizeOnly);
    enableWhenChecked(ui->cbTabCloseButton, ui->cbTabCloseButtonMode);
}

void LC_SettingsPageGeneralUserInterface::setupBindings() {
    using namespace CFG_NewDrawingDefaults;
    using namespace CFG_DraftingOperations;
    using namespace CFG_Startup;
    using namespace CFG_Dialogs;
    using namespace CFG_AppStyling;
    using namespace CFG_DrawingDocumentsMDIOptions;

    bindBoolean({
        { ui->cbUseQtFileOpenDialog, o_UseQtFileOpenDialog },
        { ui->cbUseQtColorPickerDialog, o_UseQtColorPickerDialog }, // fixme - sand - review usage and picking color via non-OS color picker all over the codebase
        { ui->cbPersistentDialogs, o_PersistDialogPositions },
        { ui->cbPersistentDialogSizeOnly, o_PersistDialogRestoreSizeOnly },
        { ui->cbTabCloseButton, o_ShowCloseButtonOnDrawingTab },
        { ui->cbShowCurrentActionIconInOptions, o_ShowActionIconInToolOptionsToolbar },
        { ui->cbShowEntityIDs, o_ShowEntityIDs },
        { ui->cbChangingViewOnlyModifiesDrawing, o_ModifyDocumentOnViewOnlyChange },
        { ui->cbInteractiveInputInActionToolbarEnabled, o_InteractiveInputEnabled },
        {ui->cbAllowMenusDetaching, o_AllowMenusTearOff, true },
        {ui->cbMainMenuShowIcons, o_MainMenuTopLevelIconsOnly },
        {ui->cbShowToolbarTooltips, o_ShowToolbarsTooltip},
        {ui->cbShowKeyboardShortcutsInToolTips, o_ShowKeyboardShortcutsInTooltips},
        { ui->cbAnglesInputInDecimalDegreesOnly, CFG_CADPreferences::o_InputAnglesAsDecimalsOnly }
    });

    bindComboIndex({
       { ui->cbMenuType, o_MainMenuType }
   });

    bindCustom<QComboBox, bool>(ui->cbTabCloseButtonMode, o_ShowCloseButtonOnActiveDrawingTabOnly, false,
    &QComboBox::currentTextChanged,
        [](QComboBox* w) { return w->currentIndex() == 1; },
        [](QComboBox* w, bool v) { w->setCurrentIndex(v ? 1 : 0); });
}
