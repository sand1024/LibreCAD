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
#include "lc_styling_preview_controller.h"
#include "qc_applicationwindow.h"

LC_SettingsPageToolbarsAndDocks::LC_SettingsPageToolbarsAndDocks(QObject* parent)
    : LC_SettingsPageBase(tr("Toolbars and Docking"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageToolbarsAndDocks>()) {
    m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageToolbarsAndDocks::~LC_SettingsPageToolbarsAndDocks() = default;

void LC_SettingsPageToolbarsAndDocks::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerWidgets*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerWidgets::configLoaded, this, [this](const LC_WidgetsConfig&) {
            populateUiFromConfig();
        });
        populateUiFromConfig();
    }
}

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

void LC_SettingsPageToolbarsAndDocks::populateUiFromConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) {
        return;
    }

    m_blockSignals = true;
    const LC_WidgetsConfig& config = m_presetManager->workingConfig();

    ui->cbAllowToolbarIconSize->setChecked(config.toolbarAllowIconSize);
    ui->sbToolbarIconSize->setValue(config.toolbarIconSize);
    ui->cbFlatPickValuesButtons->setChecked(config.pickValueButtonsFlatIcons);

    ui->cbDockWidgetsFlatButtons->setChecked(config.dockWidgetsFlatButtons);
    ui->sbDockWidgetIconSize->setValue(config.dockWidgetsIconSize);
    ui->cmbDockTabDisplayMode->setCurrentIndex(config.dockTabDisplayMode);
    ui->cbDockTabOverrideIconSize->setChecked(config.dockTabOverrideIconSize);
    ui->sbDockTabIconSize->setValue(config.dockTabIconSize);
    ui->cbDockingVerticalTitleBar->setChecked(config.dockTitleBarVertical);
    ui->cbDockingVerticalTabs->setChecked(config.dockTabVertical);

    ui->cbLeftTBFlatButtons->setChecked(config.cadDockWidgetFlatButtons);
    ui->sbLeftTBIconSize->setValue(config.cadDockWidgetIconSize);
    ui->sbLeftTBColumnCount->setValue(config.cadDockWidgetColumnsCount);
    ui->cmbCadDockTabDisplayMode->setCurrentIndex(config.cadDockTabDisplayMode);
    ui->cbCadDockTabOverrideIconSize->setChecked(config.cadDockTabOverrideIconSize);
    ui->sbCadDockTabIconSize->setValue(config.cadDockTabIconSize);
    ui->cbCadDockingVerticalTitleBar->setChecked(config.cadDockTitleBarVertical);
    ui->cbCadDockingVerticalTabs->setChecked(config.cadDockVerticalTabs);

    ui->cbLeftTBAllFlatButtons->setChecked(config.cadToolsMatrixFlatButtons);
    ui->sbLeftTBAllIconSize->setValue(config.cadToolsMatrixIconSize);
    ui->sbLeftTBAllColumnCount->setValue(config.cadToolsMatrixColumnsCount);

    ui->cbDockingAllowNested->setChecked(config.dockAllowNested);

    ui->cbAllowStatusbarHeight->setChecked(config.allowStatusbarHeight);
    ui->sbStatusbarHeight->setValue(config.statusbarHeight);
    ui->cbAllowStatusbarFontSize->setChecked(config.allowStatusbarFontSize);
    ui->sbStatusbarFontSize->setValue(config.statusbarFontSize);

    ui->sbToolbarIconSize->setEnabled(config.toolbarIconSize);
    ui->sbStatusbarHeight->setEnabled(config.allowStatusbarHeight);
    ui->sbStatusbarFontSize->setEnabled(config.allowStatusbarFontSize);
    ui->sbDockTabIconSize->setEnabled(config.dockTabOverrideIconSize);
    ui->sbCadDockTabIconSize->setEnabled(config.cadDockTabOverrideIconSize);

    m_blockSignals = false;

    updateLivePreview();
}

void LC_SettingsPageToolbarsAndDocks::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) {
        return;
    }

    LC_WidgetsConfig& config = m_presetManager->workingConfig();

    config.toolbarAllowIconSize = ui->cbAllowToolbarIconSize->isChecked();
    config.toolbarIconSize = ui->sbToolbarIconSize->value();
    config.pickValueButtonsFlatIcons = ui->cbFlatPickValuesButtons->isChecked();

    config.dockWidgetsFlatButtons = ui->cbDockWidgetsFlatButtons->isChecked();
    config.dockWidgetsIconSize = ui->sbDockWidgetIconSize->value();
    config.dockTabDisplayMode = ui->cmbDockTabDisplayMode->currentIndex();
    config.dockTabOverrideIconSize = ui->cbDockTabOverrideIconSize->isChecked();
    config.dockTabIconSize = ui->sbDockTabIconSize->value();
    config.dockTitleBarVertical = ui->cbDockingVerticalTitleBar->isChecked();
    config.dockTabVertical = ui->cbDockingVerticalTabs->isChecked();

    config.cadDockWidgetFlatButtons = ui->cbLeftTBFlatButtons->isChecked();
    config.cadToolsMatrixIconSize = ui->sbLeftTBIconSize->value();
    config.cadToolsMatrixColumnsCount = ui->sbLeftTBColumnCount->value();
    config.cadDockTabDisplayMode = ui->cmbCadDockTabDisplayMode->currentIndex();
    config.cadDockTabOverrideIconSize = ui->cbCadDockTabOverrideIconSize->isChecked();
    config.cadDockTabIconSize = ui->sbCadDockTabIconSize->value();
    config.cadDockTitleBarVertical = ui->cbCadDockingVerticalTitleBar->isChecked();
    config.cadDockVerticalTabs = ui->cbCadDockingVerticalTabs->isChecked();

    config.cadToolsMatrixFlatButtons = ui->cbLeftTBAllFlatButtons->isChecked();
    config.cadToolsMatrixIconSize = ui->sbLeftTBAllIconSize->value();
    config.cadToolsMatrixColumnsCount = ui->sbLeftTBAllColumnCount->value();

    config.dockAllowNested = ui->cbDockingAllowNested->isChecked();

    config.allowStatusbarHeight = ui->cbAllowStatusbarHeight->isChecked();
    config.statusbarHeight = ui->sbStatusbarHeight->value();
    config.allowStatusbarFontSize = ui->cbAllowStatusbarFontSize->isChecked();
    config.statusbarFontSize = ui->sbStatusbarFontSize->value();
}


void LC_SettingsPageToolbarsAndDocks::doLoadSettings() {
    populateUiFromConfig();
}

bool LC_SettingsPageToolbarsAndDocks::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageToolbarsAndDocks::isModified() const {
    return (m_presetManager != nullptr) ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageToolbarsAndDocks::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
    updateLivePreview();
}

void LC_SettingsPageToolbarsAndDocks::updateLivePreview() {
    if (m_presetManager != nullptr) {
        syncUiToWorkingConfig();
    }
    LC_SettingsPageBase::updateLivePreview();
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewToolbarsAndDocks();
    }
}

void LC_SettingsPageToolbarsAndDocks::setPreviewController(LC_StylingPreviewController* controller) {
    m_previewController = controller;
    if (m_presetManager != nullptr) {
        m_presetManager->setPreviewController(controller);
    }
}

QWidget* LC_SettingsPageToolbarsAndDocks::getBottomWidget() {
    return (m_previewController != nullptr)
               ? m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck())
               : nullptr;
}
