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

#include "lc_metrics_density_header_bar.h"
#include "ui_lc_metrics_density_header_bar.h"

LC_MetricsDensityHeaderBar::LC_MetricsDensityHeaderBar(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_MetricsDensityHeaderBar>()) {
    ui->setupUi(this);

    ui->cbWorkspaceDensity->addItem(tr("Compact (CAD Optimal)"), "compact");
    ui->cbWorkspaceDensity->addItem(tr("Standard (Default)"), "standard");
    ui->cbWorkspaceDensity->addItem(tr("Spacious"), "spacious");
    ui->cbWorkspaceDensity->addItem(tr("Custom"), "custom");

    connect(ui->cbWorkspaceDensity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!m_blockSignals && index >= 0) {
            emit densityPresetSelected(ui->cbWorkspaceDensity->itemData(index).toString());
        }
    });
}

LC_MetricsDensityHeaderBar::~LC_MetricsDensityHeaderBar() = default;

void LC_MetricsDensityHeaderBar::setDensityPreset(const QString& presetKey) {
    m_blockSignals = true;
    int idx = ui->cbWorkspaceDensity->findData(presetKey);
    if (idx >= 0) {
        ui->cbWorkspaceDensity->setCurrentIndex(idx);
    }
    else {
        setDensityToCustom();
    }
    m_blockSignals = false;
}

void LC_MetricsDensityHeaderBar::setDensityToCustom() {
    m_blockSignals = true;
    int idx = ui->cbWorkspaceDensity->findData("custom");
    if (idx >= 0) {
        ui->cbWorkspaceDensity->setCurrentIndex(idx);
    }
    m_blockSignals = false;
}

QString LC_MetricsDensityHeaderBar::currentDensityPreset() const {
    return ui->cbWorkspaceDensity->currentData().toString();
}
