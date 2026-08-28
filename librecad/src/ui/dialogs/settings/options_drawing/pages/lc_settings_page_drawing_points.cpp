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

#include "lc_settings_page_drawing_points.h"
#include "ui_lc_settings_page_drawing_points.h"
#include "lc_document_settings_backend.h"
#include "dxf_format.h"
#include "rs_math.h"
#include "lc_defaults.h"

LC_SettingsPageDrawingPoints::LC_SettingsPageDrawingPoints(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Points"), nullptr, parent), m_graphic(graphic) {
}

LC_SettingsPageDrawingPoints::~LC_SettingsPageDrawingPoints() = default;

void LC_SettingsPageDrawingPoints::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingPoints>();
    ui->setupUi(m_widget);

    m_ptGroup = new QButtonGroup(m_widget);
    m_ptGroup->setExclusive(true);

    // Row 1: Standard
    m_ptGroup->addButton(ui->bDot, DXF_FORMAT_PDMode_CentreDot);
    m_ptGroup->addButton(ui->bBlank, DXF_FORMAT_PDMode_CentreBlank);
    m_ptGroup->addButton(ui->bPlus, DXF_FORMAT_PDMode_CentrePlus);
    m_ptGroup->addButton(ui->bCross, DXF_FORMAT_PDMode_CentreCross);
    m_ptGroup->addButton(ui->bTick, DXF_FORMAT_PDMode_CentreTick);

    // Row 2: Circle
    m_ptGroup->addButton(ui->bDotCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreDot));
    m_ptGroup->addButton(ui->bBlankCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreBlank));
    m_ptGroup->addButton(ui->bPlusCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentrePlus));
    m_ptGroup->addButton(ui->bCrossCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreCross));
    m_ptGroup->addButton(ui->bTickCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreTick));

    // Row 3: Square
    m_ptGroup->addButton(ui->bDotSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
    m_ptGroup->addButton(ui->bBlankSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreBlank));
    m_ptGroup->addButton(ui->bPlusSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentrePlus));
    m_ptGroup->addButton(ui->bCrossSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreCross));
    m_ptGroup->addButton(ui->bTickSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreTick));

    // Row 4: Circle + Square
    m_ptGroup->addButton(ui->bDotCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreDot));
    m_ptGroup->addButton(ui->bBlankCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreBlank));
    m_ptGroup->addButton(ui->bPlusCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentrePlus));
    m_ptGroup->addButton(ui->bCrossCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreCross));
    m_ptGroup->addButton(ui->bTickCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreTick));
}

void LC_SettingsPageDrawingPoints::setupBehavior() {
    connect(ui->rbRelSize, &QRadioButton::toggled, this, &LC_SettingsPageDrawingPoints::updatePointSizeUnitsLabel);
}

void LC_SettingsPageDrawingPoints::loadSettings() {
    if (!m_graphic) {
        return;
    }

    const int pdmode = m_graphic->getVariableInt("$PDMODE", LC_DEFAULTS_PDMode);
    if (auto* btn = m_ptGroup->button(pdmode)) {
        btn->setChecked(true);
    }

    const double pdsize = m_graphic->getVariableDouble("$PDSIZE", LC_DEFAULTS_PDSize);
    ui->rbRelSize->setChecked(pdsize <= 0.0);
    ui->rbAbsSize->setChecked(pdsize > 0.0);
    ui->lePointSize->setText(QString::number(std::abs(pdsize), 'g', 6));

    updatePointSizeUnitsLabel();
}

bool LC_SettingsPageDrawingPoints::saveSettings() {
    if (!m_graphic) {
        return false;
    }

    m_graphic->addVariable("$PDMODE", m_ptGroup->checkedId(), 70);

    bool ok;
    double val = RS_Math::eval(ui->lePointSize->text(), &ok);
    if (!ok)
        val = LC_DEFAULTS_PDSize;

    // DXF Logic: Negative = Screen %, Positive = Dwg Units
    if (ui->rbRelSize->isChecked()) {
        val = -std::abs(val);
    }
    else {
        val = std::abs(val);
    }

    m_graphic->addVariable("$PDSIZE", val, 40);
    return true;
}

bool LC_SettingsPageDrawingPoints::isModified() const {
    if (!m_graphic) {
        return false;
    }

    if (m_ptGroup->checkedId() != m_graphic->getVariableInt("$PDMODE", LC_DEFAULTS_PDMode)) {
        return true;
    }

    bool ok;
    double uiSize = RS_Math::eval(ui->lePointSize->text(), &ok);
    if (!ok) {
        uiSize = LC_DEFAULTS_PDSize;
    }

    if (ui->rbRelSize->isChecked()) {
        uiSize = -std::abs(uiSize);
    }
    else {
        uiSize = std::abs(uiSize);
    }

    const double docSize = m_graphic->getVariableDouble("$PDSIZE", LC_DEFAULTS_PDSize);

    // Float comparison using a reliable precision threshold
    if (std::abs(uiSize - docSize) > 1e-9) {
        return true;
    }

    return false;
}

void LC_SettingsPageDrawingPoints::updatePointSizeUnitsLabel() {
    ui->lPtSzUnits->setText(ui->rbRelSize->isChecked() ? tr("Screen %") : tr("Dwg Units"));
}
