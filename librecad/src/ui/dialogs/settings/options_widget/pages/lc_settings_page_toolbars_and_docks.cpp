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
#include "lc_widgetfactory.h"
#include "qc_applicationwindow.h"

LC_SettingsPageToolbarsAndDocks::LC_SettingsPageToolbarsAndDocks(QObject* parent)
    : LC_SettingsPageBase(tr("Toolbars and Docking"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_Widgets::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageToolbarsAndDocks>()) {
    m_styleManager=  QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageToolbarsAndDocks::~LC_SettingsPageToolbarsAndDocks() = default;

void LC_SettingsPageToolbarsAndDocks::setupUi() {
    ui->setupUi(m_widget);

    const bool cadSidebarUngrouped = CFG_Startup::o_CADSideBarUngrouped;
    ui->gbCADWidgets->setEnabled(!cadSidebarUngrouped);
    ui->gbCADWidgetsUngrouped->setEnabled(cadSidebarUngrouped);

    const bool useClassicalStatusBar = CFG_Startup::o_UseClassicStatusBar;
    ui->gbStatusBar->setEnabled(useClassicalStatusBar);
}

void LC_SettingsPageToolbarsAndDocks::setupBehavior() {
    enableWhenChecked(ui->cbAllowToolbarIconSize, ui->sbToolbarIconSize);
    enableWhenChecked(ui->cbAllowStatusbarHeight, ui->sbStatusbarHeight);
    enableWhenChecked(ui->cbAllowStatusbarFontSize, ui->sbStatusbarFontSize);

    const QList<QCheckBox*> checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (auto* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_SettingsPageToolbarsAndDocks::onControlChanged);
    }

    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (auto* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageToolbarsAndDocks::onControlChanged);
    }
}

void LC_SettingsPageToolbarsAndDocks::setupBindings() {
    using namespace CFG_Widgets;

    bindBoolean({
        { ui->cbAllowToolbarIconSize, o_AllowToolbarIconSize },
        { ui->cbFlatPickValuesButtons, o_PickValueButtonsFlatIcons },
        { ui->cbLeftTBFlatButtons, o_LeftToolbarFlatIcons },
        { ui->cbLeftTBAllFlatButtons, o_LeftToolbarAllFlatIcons },
        { ui->cbDockWidgetsFlatButtons, o_DockWidgetsFlatIcons },
        { ui->cbDockingAllowNested, o_DockAllowNested },
        { ui->cbDockingVerticalTitleBar, o_DockTitleBarVertical },
        { ui->cbDockingVerticalTabs, o_DockVerticalTabs },
        { ui->cbAllowStatusbarHeight, o_AllowStatusbarHeight },
        { ui->cbAllowStatusbarFontSize, o_AllowStatusbarFontSize }
    });

    bindInt({
        { ui->sbToolbarIconSize, o_ToolbarIconSize },
        { ui->sbLeftTBIconSize, o_LeftToolbarIconSize },
        { ui->sbLeftTBColumnCount, o_LeftToolbarColumnsCount },
        { ui->sbLeftTBAllIconSize, o_LeftToolbarAllIconSize },
        { ui->sbLeftTBAllColumnCount, o_LeftToolbarAllColumnsCount },
        { ui->sbDockWidgetIconSize, o_DockWidgetsIconSize },
        { ui->sbStatusbarHeight, o_StatusbarHeight },
        { ui->sbStatusbarFontSize, o_StatusbarFontSize }
    });
}

void LC_SettingsPageToolbarsAndDocks::loadSettings() {
    m_blockSignals = true;
    LC_SettingsPageBase::loadSettings();
    m_blockSignals = false;

    updateLivePreview();
}

bool LC_SettingsPageToolbarsAndDocks::saveSettings() {
    const bool success = LC_SettingsPageBase::saveSettings();
    if (success) {
        const auto& appWindow = QC_ApplicationWindow::getAppWindow();
        if (appWindow != nullptr) {
            if (ui->cbAllowToolbarIconSize->isChecked()) {
                const int size = ui->sbToolbarIconSize->value();
                appWindow->setIconSize(QSize(size, size));
            }
            if (ui->cbAllowStatusbarFontSize->isChecked()) {
                QFont font;
                font.setPointSize(ui->sbStatusbarFontSize->value());
                appWindow->statusBar()->setFont(font);
            }
            if (ui->cbAllowStatusbarHeight->isChecked()) {
                appWindow->statusBar()->setMinimumHeight(ui->sbStatusbarHeight->value());
            }

            LC_WidgetFactory::updateDockOptions(appWindow.get(),
                                                ui->cbDockingAllowNested->isChecked(),
                                                ui->cbDockingVerticalTabs->isChecked());
            LC_WidgetFactory::updateDockWidgetsTitleBarType(appWindow.get(),
                                                           ui->cbDockingVerticalTitleBar->isChecked());
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
               ? m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck()): nullptr;
}
