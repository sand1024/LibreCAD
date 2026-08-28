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

#include "lc_settings_page_maintenance.h"
#include "ui_lc_settings_page_maintenance.h"
#include "lc_settings_backend.h"
#include "lc_settingsexporter.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"
#include <QMessageBox>

#include "lc_settings_appearance.h"

LC_SettingsPageMaintenance::LC_SettingsPageMaintenance(QObject* parent)
    : LC_SettingsPageBase(tr("Maintenance"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group),
                           parent), ui(std::make_unique<Ui::LC_SettingsPageMaintenance>()) {
    setSortWeight(90);
}

LC_SettingsPageMaintenance::~LC_SettingsPageMaintenance() = default;

void LC_SettingsPageMaintenance::setupUi() {
    ui->setupUi(m_widget);

    connect(ui->pbExportSettings, &QPushButton::clicked, this, &LC_SettingsPageMaintenance::exportSettings);
    connect(ui->pbImportSettings, &QPushButton::clicked, this, &LC_SettingsPageMaintenance::importSettings);
    connect(ui->pb_clear_geometry, &QPushButton::clicked, this, &LC_SettingsPageMaintenance::clearGeometry);
    connect(ui->pb_clear_all, &QPushButton::clicked, this, &LC_SettingsPageMaintenance::clearAllSettings);
}

void LC_SettingsPageMaintenance::exportSettings() {
    LC_SettingsExporter exporter;
    exporter.exportSettings(getEditingWidget());
}

void LC_SettingsPageMaintenance::importSettings() {
    LC_SettingsExporter importer;
    if (importer.importSettings(getEditingWidget())) {
        QC_ApplicationWindow::getAppWindow()->initSettings(false);
    }
}

void LC_SettingsPageMaintenance::clearGeometry() {
    RS_Settings::instance()->clearGeometry();
    QMessageBox::information(getEditingWidget(), "info", tr("You must restart LibreCAD to see the changes."));
}

void LC_SettingsPageMaintenance::clearAllSettings() {
    const QMessageBox::StandardButton reply = QMessageBox::question(getEditingWidget(), tr("Clear settings"),
                                                              tr("This will also include custom menus and toolbars. Continue?"),
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        RS_Settings::instance()->clearAll();
        QMessageBox::information(getEditingWidget(), "info", tr("You must restart LibreCAD to see the changes."));
    }
}
