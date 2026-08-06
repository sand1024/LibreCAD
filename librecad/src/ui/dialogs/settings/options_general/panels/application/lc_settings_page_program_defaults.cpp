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

#include "lc_settings_page_program_defaults.h"

#include "lc_settings_appearance.h"
#include "lc_settings_defaults.h"
#include "lc_settings_startup.h"
#include "ui_lc_settings_page_program_defaults.h"

LC_SettingsPageProgramDefaults::LC_SettingsPageProgramDefaults(QObject* parent)
    : LC_SettingsPageBase(tr("User Interface"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group),
                           parent), ui(std::make_unique<Ui::LC_SettingsPageProgramDefaults>()) {
    // Dynamic Sorting: positions the page first inside the Application tree block
    setSortWeight(10);
}


LC_SettingsPageProgramDefaults::~LC_SettingsPageProgramDefaults() = default;


void LC_SettingsPageProgramDefaults::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageProgramDefaults::setupBehavior() {
    enableWhenChecked(ui->cbPersistentDialogs, ui->cbPersistentDialogSizeOnly);
    enableWhenChecked(ui->cbTabCloseButton, ui->cbTabCloseButtonMode);
    enableWhenChecked(ui->cbExpandToolsMenu, ui->cbExpandToolsMenuTillEntity);
}

void LC_SettingsPageProgramDefaults::setupBindings() {
    using namespace CFG_Defaults;
    using namespace CFG_Appearance;
    using namespace CFG_Startup;

    bindBoolean({
        { ui->cbUseQtFileOpenDialog, o_UseQtFileOpenDialog },
        { ui->cbUseQtColorPickerDialog, o_UseQtFileOpenDialog },
        { ui->cbShowKeyboardShortcutsInToolTips, o_ShowKeyboardShortcutsInTooltips },
        { ui->cbPersistentDialogs, o_PersistDialogPositions },
        { ui->cbPersistentDialogSizeOnly, o_PersistDialogRestoreSizeOnly },
        { ui->cbTabCloseButton, o_ShowCloseButton },
        { ui->cbShowCurrentActionIconInOptions, o_ShowActionIconInOptions },
        { ui->cbShowEntityIDs, o_ShowEntityIDs },
        { ui->cbChangingViewOnlyModifiesDrawing, o_ModifyOnViewChange },
        { ui->cbInteractiveInputInActionToolbarEnabled, o_InteractiveInputEnabled },
        { ui->cbExpandToolsMenu, o_ExpandedToolsMenu },
        { ui->cbExpandToolsMenuTillEntity, o_ExpandedToolsMenuTillEntity }
    });

    bindCustom<QComboBox, bool>(ui->cbTabCloseButtonMode, o_ShowCloseButtonActiveOnly, false,
    &QComboBox::currentTextChanged,
        [](QComboBox* w) { return w->currentIndex() == 1; },
        [](QComboBox* w, bool v) { w->setCurrentIndex(v ? 1 : 0); });
}
