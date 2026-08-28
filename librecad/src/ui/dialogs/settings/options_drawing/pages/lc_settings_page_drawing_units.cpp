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

#include "lc_settings_page_drawing_units.h"
#include "ui_lc_settings_page_drawing_units.h"
#include "lc_document_settings_backend.h"
#include "rs_units.h"
#include "rs_math.h"
#include "rs_filterdxfrw.h"
#include <QMessageBox>

LC_SettingsPageDrawingUnits::LC_SettingsPageDrawingUnits(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Units"), nullptr, parent) // Bypassing binder safely
    , m_graphic(graphic) {
}

LC_SettingsPageDrawingUnits::~LC_SettingsPageDrawingUnits() = default;

void LC_SettingsPageDrawingUnits::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingUnits>();
    ui->setupUi(m_widget);

    // Main drawing units - populated sequentially (None = 0, Inch = 1, etc.)
    for (int i = RS2::None; i < RS2::LastUnit; i++) {
        ui->cbUnit->addItem(RS_Units::unitToString(static_cast<RS2::Unit>(i)));
    }

    // Length Formats - Explicitly bound to exact DXF standard values (1 to 6)
    ui->cbLengthFormat->addItem(tr("Scientific"), 1);
    ui->cbLengthFormat->addItem(tr("Decimal"), 2);
    ui->cbLengthFormat->addItem(tr("Engineering"), 3);
    ui->cbLengthFormat->addItem(tr("Architectural"), 4);
    ui->cbLengthFormat->addItem(tr("Fractional"), 5);
    ui->cbLengthFormat->addItem(tr("Architectural (metric)"), 6);

    // Angle Formats - Simple indices 0 to 4 match DXF standard exactly
    ui->cbAngleFormat->addItem(tr("Decimal Degrees"));
    ui->cbAngleFormat->addItem(tr("Deg/min/sec"));
    ui->cbAngleFormat->addItem(tr("Gradians"));
    ui->cbAngleFormat->addItem(tr("Radians"));
    ui->cbAngleFormat->addItem(tr("Surveyor's units"));
}

void LC_SettingsPageDrawingUnits::loadSettings() {
    if (!m_graphic) return;

    // Load Main Unit
    int insunits = m_graphic->getVariableInt("$INSUNITS", 0);
    ui->cbUnit->setCurrentIndex(ui->cbUnit->findText(RS_Units::unitToString(RS_FilterDXFRW::numberToUnit(insunits))));

    // Load Length Format (Direct DXF matching)
    int lunits = m_graphic->getVariableInt("$LUNITS", 2);
    ui->cbLengthFormat->setCurrentIndex(ui->cbLengthFormat->findData(lunits));
    onLengthFormatChanged(); // Populate precision list and apply constraint rules
    ui->cbLengthPrecision->setCurrentIndex(m_graphic->getVariableInt("$LUPREC", 4));

    // Load Angle Format (Using index directly to avoid QVariant(0) ambiguity)
    int aunits = m_graphic->getAngleFormat();
    ui->cbAngleFormat->setCurrentIndex(aunits);
    onAngleFormatChanged(); // Populate precision list
    ui->cbAnglePrecision->setCurrentIndex(m_graphic->getVariableInt("$AUPREC", 2));

    // Load Basis
    double baseAngle = m_graphic->getAnglesBase();
    ui->leAngleBaseZero->setText(QString::number(RS_Math::rad2deg(baseAngle)));

    bool ccw = m_graphic->areAnglesCounterClockWise();
    if (ccw) ui->rbAngleBasePositive->setChecked(true);
    else ui->rbAngleBaseNegative->setChecked(true);

    updateUnitsPreview();
}

bool LC_SettingsPageDrawingUnits::saveSettings() {
    if (!m_graphic) return false;

    // Constraint Validation
    int fVal = ui->cbLengthFormat->currentData().toInt();
    RS2::Unit u = static_cast<RS2::Unit>(ui->cbUnit->currentIndex());

    // engineering = 3, architectural = 4 in DXF
    if ((fVal == 3 || fVal == 4) && u != RS2::Inch) {
        QMessageBox::warning(m_widget, tr("Options"), tr("For the length formats 'Engineering' and 'Architectural', the unit must be set to Inch."));
        return false;
    }
    // architectural metric = 6 in DXF
    if (fVal == 6 && u != RS2::Meter) {
        QMessageBox::warning(m_widget, tr("Options"), tr("For the length format 'Architectural (metric)', the unit must be set to Meter."));
        return false;
    }

    m_graphic->setUnit(u);
    RS_Units::setCurrentDrawingUnits(u); // Update global unit configuration

    m_graphic->addVariable("$LUNITS", fVal, 70);
    m_graphic->addVariable("$LUPREC", ui->cbLengthPrecision->currentIndex(), 70);
    m_graphic->addVariable("$AUNITS", ui->cbAngleFormat->currentIndex(), 70); // Saved using index
    m_graphic->addVariable("$AUPREC", ui->cbAnglePrecision->currentIndex(), 70);

    bool ok;
    double baseDeg = RS_Math::eval(ui->leAngleBaseZero->text(), &ok);
    if (ok) m_graphic->setAnglesBase(RS_Math::deg2rad(baseDeg));
    m_graphic->setAnglesCounterClockwise(ui->rbAngleBasePositive->isChecked());

    return true;
}

bool LC_SettingsPageDrawingUnits::isModified() const {
    if (!m_graphic) return false;

    // Compare Main Unit
    if (static_cast<RS2::Unit>(ui->cbUnit->currentIndex()) != m_graphic->getUnit()) {
        return true;
    }

    // Compare Length Format (Strict DXF variable matching)
    if (ui->cbLengthFormat->currentData().toInt() != m_graphic->getVariableInt("$LUNITS", 2)) {
        return true;
    }

    // Compare Length Precision
    if (ui->cbLengthPrecision->currentIndex() != m_graphic->getVariableInt("$LUPREC", 4)) {
        return true;
    }

    // Compare Angle Format (Using stable index comparison)
    if (ui->cbAngleFormat->currentIndex() != m_graphic->getAngleFormat()) {
        return true;
    }

    // Compare Angle Precision
    if (ui->cbAnglePrecision->currentIndex() != m_graphic->getVariableInt("$AUPREC", 2)) {
        return true;
    }

    // Compare Angle Base Zero
    bool ok;
    double currentBaseAngle = RS_Math::deg2rad(RS_Math::eval(ui->leAngleBaseZero->text(), &ok));
    if (ok) {
        if (std::abs(currentBaseAngle - m_graphic->getAnglesBase()) > 1e-9) {
            return true;
        }
    }

    // Compare Direction
    if (ui->rbAngleBasePositive->isChecked() != m_graphic->areAnglesCounterClockWise()) {
        return true;
    }

    return false;
}

void LC_SettingsPageDrawingUnits::setupBehavior() {
    connect(ui->cbLengthFormat, QOverload<int>::of(&QComboBox::activated), this, &LC_SettingsPageDrawingUnits::onLengthFormatChanged);
    connect(ui->cbAngleFormat, QOverload<int>::of(&QComboBox::activated), this, &LC_SettingsPageDrawingUnits::onAngleFormatChanged);

    auto update = [this] { updateUnitsPreview(); };
    connect(ui->cbUnit, QOverload<int>::of(&QComboBox::activated), update);
    connect(ui->cbLengthPrecision, QOverload<int>::of(&QComboBox::activated), update);
    connect(ui->cbAnglePrecision, QOverload<int>::of(&QComboBox::activated), update);
}

void LC_SettingsPageDrawingUnits::onLengthFormatChanged() {
    int oldIndex = ui->cbLengthPrecision->currentIndex(); // FIX: Preserve active index
    int dxfFormatVal = ui->cbLengthFormat->currentData().toInt();

    ui->cbLengthPrecision->clear();

    // 1 = Scientific, 2 = Decimal, 6 = Architectural Metric
    if (dxfFormatVal == 1 || dxfFormatVal == 2 || dxfFormatVal == 6) {
        for (int i = 0; i <= 8; i++) ui->cbLengthPrecision->addItem(QString("%1").arg(0.0, 0, 'f', i));
    } else {
        ui->cbLengthPrecision->addItems({"0", "0 1/2", "0 1/4", "0 1/8", "0 1/16", "0 1/32", "0 1/64", "0 1/128"});
    }

    // FIX: Safely restore active precision index, otherwise fallback to standard default (index 2)
    if (oldIndex >= 0 && oldIndex < ui->cbLengthPrecision->count()) {
        ui->cbLengthPrecision->setCurrentIndex(oldIndex);
    } else {
        ui->cbLengthPrecision->setCurrentIndex(qMin(2, ui->cbLengthPrecision->count() - 1));
    }

    // LEGACY CONSTRAINTS: Map DXF value to RS2 enum for automatic unit selection
    // RS2::Engineering = 2, RS2::Architectural = 3, RS2::ArchitecturalMetric = 5
    RS2::LinearFormat format = static_cast<RS2::LinearFormat>(dxfFormatVal - 1);
    switch (format) {
        case RS2::Engineering:
        case RS2::Architectural: {
            ui->cbUnit->setCurrentIndex(RS2::Inch);
            break;
        }
        case RS2::ArchitecturalMetric: {
            ui->cbUnit->setCurrentIndex(RS2::Meter);
            break;
        }
        case RS2::Decimal: {
            const auto unit = static_cast<RS2::Unit>(ui->cbUnit->currentIndex());
            if (unit == RS2::Foot || unit == RS2::Inch || unit == RS2::Microinch ||
                unit == RS2::Mil || unit == RS2::Mile || unit == RS2::Yard) {
                ui->cbUnit->setCurrentIndex(RS2::Millimeter);
            }
            break;
        }
        default:
            break;
    }

    updateUnitsPreview();
}

void LC_SettingsPageDrawingUnits::onAngleFormatChanged() {
    int oldIndex = ui->cbAnglePrecision->currentIndex(); // FIX: Preserve active index
    int dxfAngleVal = ui->cbAngleFormat->currentIndex(); // Safely reading index directly

    ui->cbAnglePrecision->clear();

    RS2::AngleFormat format = static_cast<RS2::AngleFormat>(dxfAngleVal);
    switch(format) {
        case RS2::DegreesDecimal:
            for (int i = 0; i <= 8; i++) ui->cbAnglePrecision->addItem(QString("%1").arg(0.0, 0, 'f', i));
            break;
        case RS2::DegreesMinutesSeconds:
            ui->cbAnglePrecision->addItem(QString("0%1").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'00\"").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'00.0\"").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'00.00\"").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'00.000\"").arg(QChar(0xB0)));
            ui->cbAnglePrecision->addItem(QString("0%100'00.0000\"").arg(QChar(0xB0)));
            break;
        case RS2::Gradians:
            ui->cbAnglePrecision->addItems({"0g", "0.0g", "0.00g", "0.000g", "0.0000g", "0.00000g", "0.000000g", "0.0000000g", "0.00000000g"});
            break;
        case RS2::Radians:
            ui->cbAnglePrecision->addItems({"0r", "0.0r", "0.00r", "0.000r", "0.0000r", "0.00000r", "0.000000r", "0.0000000r", "0.00000000r"});
            break;
        case RS2::Surveyors:
            ui->cbAnglePrecision->addItems({"N 0d E", "N 0d00' E", "N 0d00'00\" E", "N 0d00'00.0\" E", "N 0d00'00.00\" E", "N 0d00'00.000\" E", "N 0d00'00.0000\" E"});
            break;
        default:
            ui->cbAnglePrecision->addItem("0");
            break;
    }

    // FIX: Safely restore active precision index, otherwise fallback to standard default (index 2)
    if (oldIndex >= 0 && oldIndex < ui->cbAnglePrecision->count()) {
        ui->cbAnglePrecision->setCurrentIndex(oldIndex);
    } else {
        ui->cbAnglePrecision->setCurrentIndex(qMin(2, ui->cbAnglePrecision->count() - 1));
    }

    updateUnitsPreview();
}

void LC_SettingsPageDrawingUnits::updateUnitsPreview() {
    int dxfFormatVal = ui->cbLengthFormat->currentData().toInt();
    QString lin = RS_Units::formatLinear(14.43112351,
                                        static_cast<RS2::Unit>(ui->cbUnit->currentIndex()),
                                        static_cast<RS2::LinearFormat>(dxfFormatVal - 1),
                                        ui->cbLengthPrecision->currentIndex());
    ui->lLinear->setText(lin);

    int dxfAngleVal = ui->cbAngleFormat->currentIndex();
    QString ang = RS_Units::formatAngle(0.5327714,
                                       static_cast<RS2::AngleFormat>(dxfAngleVal),
                                       ui->cbAnglePrecision->currentIndex());
    ui->lAngular->setText(ang);
}
