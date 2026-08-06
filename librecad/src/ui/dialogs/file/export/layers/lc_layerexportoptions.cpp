/*
 * **************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * *********************************************************************
 */

#include "lc_layerexportoptions.h"

#include "lc_layersexporter.h"
#include "lc_setting.h"
#include "lc_settings_export_layers.h"
#include "rs_settings.h"
#include "ui_lc_layerexportoptions.h"

LC_LayerExportOptionsWidget::LC_LayerExportOptionsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LC_LayerExportOptionsWidget){

    ui->setupUi(this);
    connect(ui->cbCreateSeparateDrawingForLayer, &QCheckBox::toggled, this,
            &LC_LayerExportOptionsWidget::onCreateSeparateDrawingToggled);
    loadFromOptions();
}

LC_LayerExportOptionsWidget::~LC_LayerExportOptionsWidget(){
    delete ui;
}

void LC_LayerExportOptionsWidget::onCreateSeparateDrawingToggled([[maybe_unused]] bool enable) const {
    ui->cbStoreEntitiesInOriginalLayer->setEnabled(ui->cbCreateSeparateDrawingForLayer->isChecked());
}

void LC_LayerExportOptionsWidget::fillLayerExportOptions(LC_LayersExportOptions* options) const {
    options->exportNamedViews = ui->cbExportNamedViews->isChecked();
    options->exportUcSs = ui->cbExportUCSs->isChecked();
    options->createSeparateDocumentPerLayer  = ui->cbCreateSeparateDrawingForLayer->isChecked();
    options->putEntitiesToOriginalLayer = ui->cbStoreEntitiesInOriginalLayer->isChecked();
    saveToOptions();
}

void LC_LayerExportOptionsWidget::loadFromOptions() const {
    using namespace CFG_ExportLayers;

    ui->cbExportUCSs->setChecked(o_ExportUCS);
    ui->cbExportNamedViews->setChecked(o_ExportViews);
    const bool separateDocForLayer = o_DocumentPerLayer;
    ui->cbCreateSeparateDrawingForLayer->setChecked(separateDocForLayer);
    ui->cbStoreEntitiesInOriginalLayer->setChecked(o_EntitiesInOriginalLayer);
    ui->cbStoreEntitiesInOriginalLayer->setEnabled(separateDocForLayer);
}

void LC_LayerExportOptionsWidget::saveToOptions() const {
    using namespace CFG_ExportLayers;

    o_ExportUCS = ui->cbExportUCSs->isChecked();
    o_ExportViews = ui->cbExportNamedViews->isChecked();
    o_DocumentPerLayer = ui->cbCreateSeparateDrawingForLayer->isChecked();
    o_EntitiesInOriginalLayer = ui->cbStoreEntitiesInOriginalLayer->isChecked();
}
