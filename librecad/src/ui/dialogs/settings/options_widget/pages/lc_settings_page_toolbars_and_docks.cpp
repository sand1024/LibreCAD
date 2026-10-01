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

#include "lc_settings_page_toolbars_and_docks.h"

#include <QStatusBar>

#include "ui_lc_settings_page_toolbars_and_docks.h"
#include "lc_settings_backend.h"
#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "lc_styling_preview_controller.h"
#include "lc_widget_factory.h"
#include "qc_applicationwindow.h"

LC_SettingsPageToolbarsAndDocks::LC_SettingsPageToolbarsAndDocks(QObject* parent)
    : LC_SettingsPageBase(tr("Toolbars and Docking"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Widgets::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageToolbarsAndDocks>()) {
    m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageToolbarsAndDocks::~LC_SettingsPageToolbarsAndDocks() = default;

void LC_SettingsPageToolbarsAndDocks::setupUi() {
    ui->setupUi(m_widget);
    const bool useClassicalStatusBar = CFG_Startup::o_UseClassicStatusBar;
    ui->gbStatusBar->setEnabled(useClassicalStatusBar);
}

void LC_SettingsPageToolbarsAndDocks::setupBehavior() {
    enableWhenChecked(ui->cbAllowToolbarIconSize, ui->sbToolbarIconSize);
    enableWhenChecked(ui->cbAllowStatusbarHeight, ui->sbStatusbarHeight);
    enableWhenChecked(ui->cbAllowStatusbarFontSize, ui->sbStatusbarFontSize);
    enableWhenChecked(ui->cbDockTabOverrideIconSize, ui->sbDockTabIconSize);
    enableWhenChecked(ui->cbCadDockTabOverrideIconSize, ui->sbCadDockTabIconSize);

    const QList<QCheckBox*> checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (auto* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_SettingsPageToolbarsAndDocks::onControlChanged);
    }

    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (auto* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageToolbarsAndDocks::onControlChanged);
    }

    const QList<QComboBox*> comboBoxes = m_widget->findChildren<QComboBox*>();
    for (auto* cmb : comboBoxes) {
        connect(cmb, &QComboBox::currentIndexChanged, this, &LC_SettingsPageToolbarsAndDocks::onControlChanged);
    }
}

void LC_SettingsPageToolbarsAndDocks::setupBindings() {
    using namespace CFG_Widgets;

    bindBoolean({
        {ui->cbAllowToolbarIconSize, o_AllowToolbarIconSize},
        {ui->cbFlatPickValuesButtons, o_PickValueButtonsFlatIcons},
        {ui->cbAllowStatusbarHeight, o_AllowStatusbarHeight},
        {ui->cbAllowStatusbarFontSize, o_AllowStatusbarFontSize},
        {ui->cbDockingAllowNested, o_DockAllowNested},
        {ui->cbLeftTBFlatButtons, o_CadToolsFlatIcons},
        {ui->cbLeftTBAllFlatButtons, o_CadToolsMatrixFlatIcons},
        {ui->cbDockWidgetsFlatButtons, o_DockWidgetsFlatIcons},
        {ui->cbDockingVerticalTitleBar, o_DockTitleBarVertical},
        {ui->cbCadDockingVerticalTitleBar, o_CadDockTitleBarVertical},
        {ui->cbDockingVerticalTabs, o_DockVerticalTabs},
        {ui->cbCadDockingVerticalTitleBar, o_CadDockVerticalTabs},
        {ui->cbDockTabOverrideIconSize, o_DockTabOverrideIconSize},
        {ui->cbCadDockTabOverrideIconSize, o_CadDockTabOverrideIconSize}
    });

    bindInt({
        {ui->sbToolbarIconSize, o_ToolbarIconSize},
        {ui->sbLeftTBIconSize, o_LeftToolbarIconSize},
        {ui->sbLeftTBColumnCount, o_LeftToolbarColumnsCount},
        {ui->sbLeftTBAllIconSize, o_LeftToolbarAllIconSize},
        {ui->sbLeftTBAllColumnCount, o_LeftToolbarAllColumnsCount},
        {ui->sbDockWidgetIconSize, o_DockWidgetsIconSize},
        {ui->sbStatusbarHeight, o_StatusbarHeight},
        {ui->sbStatusbarFontSize, o_StatusbarFontSize},
        {ui->sbDockTabIconSize, o_DockTabIconSize},
        {ui->sbCadDockTabIconSize, o_CadDockTabIconSize}
    });

    bindComboIndex({{ui->cmbDockTabDisplayMode, o_DockTabDisplayMode}, {ui->cmbCadDockTabDisplayMode, o_CadDockTabDisplayMode}});
}

void LC_SettingsPageToolbarsAndDocks::loadSettings() {
    m_blockSignals = true;
    LC_SettingsPageBase::loadSettings();
    m_blockSignals = false;

    // Explicitly synchronize dependent controls after binder load (since signals were blocked)
    ui->sbToolbarIconSize->setEnabled(ui->cbAllowToolbarIconSize->isChecked());
    ui->sbStatusbarHeight->setEnabled(ui->cbAllowStatusbarHeight->isChecked());
    ui->sbStatusbarFontSize->setEnabled(ui->cbAllowStatusbarFontSize->isChecked());

    ui->sbDockTabIconSize->setEnabled(CFG_Widgets::o_DockTabOverrideIconSize);
    ui->sbCadDockTabIconSize->setEnabled(CFG_Widgets::o_CadDockTabOverrideIconSize);

    updateLivePreview();
}

bool LC_SettingsPageToolbarsAndDocks::saveSettings() {
    const bool success = LC_SettingsPageBase::saveSettings();
    if (success) {
        const auto appWindow = QC_ApplicationWindow::getAppWindow();
        if (appWindow != nullptr) {
            appWindow->updateToolbarsIconSize(ui->cbAllowToolbarIconSize->isChecked(), ui->sbToolbarIconSize->value());

            if (ui->cbAllowStatusbarFontSize->isChecked()) {
                QFont font;
                font.setPointSize(ui->sbStatusbarFontSize->value());
                appWindow->statusBar()->setFont(font);
            }
            if (ui->cbAllowStatusbarHeight->isChecked()) {
                appWindow->statusBar()->setMinimumHeight(ui->sbStatusbarHeight->value());
            }

            LC_WidgetFactory::updateDockOptions(appWindow, ui->cbDockingAllowNested->isChecked(), ui->cbCadDockingVerticalTabs->isChecked(),
                                                ui->cbDockingVerticalTabs->isChecked());

            LC_WidgetFactory::updateDockWidgetsTitleBarType(appWindow, ui->cbCadDockingVerticalTitleBar->isChecked(),
                                                            ui->cbDockingVerticalTitleBar->isChecked());

            appWindow->fireWidgetSettingsChanged();
        }
    }
    return success;
}

void LC_SettingsPageToolbarsAndDocks::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    updateLivePreview();
}

void LC_SettingsPageToolbarsAndDocks::updateLivePreview() {
    LC_SettingsPageBase::updateLivePreview();
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewToolbarsAndDocks();
    }
}

void LC_SettingsPageToolbarsAndDocks::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
}

QWidget* LC_SettingsPageToolbarsAndDocks::getBottomWidget() {
    return (m_previewController != nullptr)
               ? m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck())
               : nullptr;
}
