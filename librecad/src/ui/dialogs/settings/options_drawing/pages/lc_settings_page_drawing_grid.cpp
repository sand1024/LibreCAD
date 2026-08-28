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

#include "lc_settings_page_drawing_grid.h"
#include "ui_lc_settings_page_drawing_grid.h"
#include "lc_document_settings_backend.h"
#include "lc_settings_appearance.h"
#include "rs_graphic.h"



LC_SettingsPageDrawingGrid::LC_SettingsPageDrawingGrid(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Grid"), nullptr, parent) // Pass nullptr to base safely
    , m_graphic(graphic) {
}

LC_SettingsPageDrawingGrid::~LC_SettingsPageDrawingGrid() = default;

void LC_SettingsPageDrawingGrid::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingGrid>();
    ui->setupUi(m_widget);
}

void LC_SettingsPageDrawingGrid::loadSettings() {
    if (!m_graphic) return;

    ui->cbGridOn->setChecked(m_graphic->isGridOn());

    const bool isometric = m_graphic->isIsometricGrid();
    if (isometric) {
        ui->rbIsometricGrid->setChecked(true);
        const RS2::IsoGridViewType isoType = m_graphic->getIsoView();
        switch (isoType) {
            case RS2::IsoLeft:  ui->rbIsoLeft->setChecked(true); break;
            case RS2::IsoTop:   ui->rbIsoTop->setChecked(true); break;
            case RS2::IsoRight: ui->rbIsoRight->setChecked(true); break;
        }
    } else {
        ui->rbOrthogonalGrid->setChecked(true);
    }

    RS_Vector spacing = m_graphic->getUserGridSpacing();
    ui->cbXSpacing->setEditText(spacing.x == 0.0 ? tr("auto") : QString::number(spacing.x));
    ui->cbYSpacing->setEditText(spacing.y == 0.0 ? tr("auto") : QString::number(spacing.y));

    // Load read-only info from Application Settings
    using namespace CFG_Appearance;
    auto stateStr = [](bool s) { return s ? tr("ON") : tr("OFF"); };
    ui->lGridStateScaling->setText(stateStr(o_ScaleGrid));
    ui->lGridStateUnitless->setText(stateStr(o_UnitlessGrid));
    ui->lGridStateDrawGrid->setText(stateStr(o_GridDraw));
    ui->lGridStateDrawMetaGrid->setText(stateStr(o_metaGridDraw));

    updateSpacingEnabledState();
}

bool LC_SettingsPageDrawingGrid::saveSettings() {
    if (!m_graphic) return false;

    m_graphic->setGridOn(ui->cbGridOn->isChecked());

    const bool isometric = ui->rbIsometricGrid->isChecked();
    m_graphic->setIsometricGrid(isometric);

    if (isometric) {
        if (ui->rbIsoLeft->isChecked()) m_graphic->setIsoView(RS2::IsoLeft);
        else if (ui->rbIsoTop->isChecked()) m_graphic->setIsoView(RS2::IsoTop);
        else m_graphic->setIsoView(RS2::IsoRight);
    }

    const double xVal = (ui->cbXSpacing->currentText() == tr("auto") || ui->cbXSpacing->currentText().isEmpty()) ? 0.0 : ui->cbXSpacing->currentText().toDouble();
    const double yVal = (ui->cbYSpacing->currentText() == tr("auto") || ui->cbYSpacing->currentText().isEmpty()) ? 0.0 : ui->cbYSpacing->currentText().toDouble();

    RS_Vector spacing(xVal, yVal);
    m_graphic->setUserGridSpacing(spacing);

    return true;
}

bool LC_SettingsPageDrawingGrid::isModified() const {
    if (!m_graphic) return false;

    // 1. Compare Grid Visibility State
    if (ui->cbGridOn->isChecked() != m_graphic->isGridOn()) {
        return true;
    }

    // 2. Compare Orthogonal vs Isometric Toggles
    const bool uiIsometric = ui->rbIsometricGrid->isChecked();
    if (uiIsometric != m_graphic->isIsometricGrid()) {
        return true;
    }

    // 3. If Isometric is checked, compare current View Type
    if (uiIsometric) {
        RS2::IsoGridViewType uiIsoView = RS2::IsoTop;
        if (ui->rbIsoLeft->isChecked()) uiIsoView = RS2::IsoLeft;
        else if (ui->rbIsoRight->isChecked()) uiIsoView = RS2::IsoRight;

        if (uiIsoView != m_graphic->getIsoView()) {
            return true;
        }
    }

    // 4. Compare Custom Grid Spacing (Handling "auto" as 0.0 with epsilon float safety)
    double uiX = (ui->cbXSpacing->currentText() == tr("auto") || ui->cbXSpacing->currentText().isEmpty()) ? 0.0 : ui->cbXSpacing->currentText().toDouble();
    double uiY = (ui->cbYSpacing->currentText() == tr("auto") || ui->cbYSpacing->currentText().isEmpty()) ? 0.0 : ui->cbYSpacing->currentText().toDouble();

    RS_Vector docSpacing = m_graphic->getUserGridSpacing();

    if (std::abs(uiX - docSpacing.x) > 1e-9 || std::abs(uiY - docSpacing.y) > 1e-9) {
        return true;
    }

    return false;
}

void LC_SettingsPageDrawingGrid::setupBehavior() {
    connect(ui->cbGridOn, &QCheckBox::toggled, this, &LC_SettingsPageDrawingGrid::updateSpacingEnabledState);
    connect(ui->rbOrthogonalGrid, &QRadioButton::toggled, this, &LC_SettingsPageDrawingGrid::updateSpacingEnabledState);
    connect(ui->rbIsometricGrid, &QRadioButton::toggled, this, &LC_SettingsPageDrawingGrid::updateSpacingEnabledState);
}

void LC_SettingsPageDrawingGrid::updateSpacingEnabledState() {
    const bool gridOn = ui->cbGridOn->isChecked();
    const bool ortho = ui->rbOrthogonalGrid->isChecked();

    ui->rbOrthogonalGrid->setEnabled(gridOn);
    ui->rbIsometricGrid->setEnabled(gridOn);
    ui->gbIsoType->setEnabled(gridOn && !ortho);

    ui->cbXSpacing->setEnabled(gridOn && ortho);
    ui->cbYSpacing->setEnabled(gridOn);
}

void LC_SettingsPageDrawingGrid::onAboutToHide() {
    LC_SettingsPageBase::onAboutToHide();
}
