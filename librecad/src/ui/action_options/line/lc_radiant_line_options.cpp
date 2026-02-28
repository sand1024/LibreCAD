/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_radiant_line_options.h"

#include "lc_action_draw_line_radiant.h"
#include "lc_guarded_signals_blocker.h"
#include "rs_settings.h"
#include "ui_lc_radiant_line_options.h"

LC_RadiantLineOptions::LC_RadiantLineOptions()
    :  LC_ActionOptionsWidgetBase(RS2::ActionDrawLineRadiant, "Draw", "LineToFarPoint")
    , ui(new Ui::LC_RadiantLineOptions){
    ui->setupUi(this);
    connect(ui->cbLengthType, &QComboBox::currentIndexChanged, this, &LC_RadiantLineOptions::onLengthTypeIndexChanged);
    connect(ui->cbPointSelector, &QComboBox::currentIndexChanged, this, &LC_RadiantLineOptions::onActivePointIndexChanged);
    connect(ui->leX, &QLineEdit::editingFinished, this, &LC_RadiantLineOptions::onXEditingFinished);
    connect(ui->leY, &QLineEdit::editingFinished, this, &LC_RadiantLineOptions::onYEditingFinished);
    connect(ui->leLength, &QLineEdit::editingFinished, this, &LC_RadiantLineOptions::onLengthEditingFinished);

    connectInteractiveInputButton(ui->tbPickX, LC_ActionContext::InteractiveInputInfo::POINT_X, "pointX");
    ui->leX->setProperty("_tagHolder", "pointX");

    connectInteractiveInputButton(ui->tbPickY, LC_ActionContext::InteractiveInputInfo::POINT_Y, "pointY");
    ui->leY->setProperty("_tagHolder", "pointY");

    connectInteractiveInputButton(ui->tbPickLength, LC_ActionContext::InteractiveInputInfo::DISTANCE, "length");
    ui->leLength->setProperty("_tagHolder", "length");

    connectInteractiveInputButton(ui->tbPickPoint, LC_ActionContext::InteractiveInputInfo::POINT, "farPoint");
}

LC_RadiantLineOptions::~LC_RadiantLineOptions(){
    delete ui;
    m_action = nullptr;
}

void LC_RadiantLineOptions::saveRadiantPoint(LC_ActionDrawLineRadiant::RadiantIdx idx) {
    QString idxStr = QString::number(idx);
    const RS_Vector activePoint = m_action->getRadiantPoint(idx);
    save("FarPointX_" + idxStr,fromDouble(activePoint.x));
    save("FarPointY_"+idxStr, fromDouble(activePoint.y));
}

RS_Vector LC_RadiantLineOptions::loadRadiantPoint(LC_ActionDrawLineRadiant::RadiantIdx idx) {
    QString idxStr = QString::number(idx);
    const RS_Vector activePoint = m_action->getRadiantPoint(idx);
    const QString xStr = load("RadiantPointX_" + idxStr, fromDouble(activePoint.x));
    const QString yStr = load("RadiantPointY_"+idxStr, fromDouble(activePoint.y));

    double valueX;
    toDouble(xStr, valueX, 0.0, false);

    double valueY;
    toDouble(yStr, valueY, 0.0, false);

    const RS_Vector result(valueX, valueY);
    return result;
}

void LC_RadiantLineOptions::doSaveSettings() {
    const int activePointIndex = m_action->getActiveRadiantIndex();

    save("ActivePoint", activePointIndex);

    saveRadiantPoint(LC_ActionDrawLineRadiant::ONE);
    saveRadiantPoint(LC_ActionDrawLineRadiant::TWO);
    saveRadiantPoint(LC_ActionDrawLineRadiant::THREE);
    saveRadiantPoint(LC_ActionDrawLineRadiant::FOUR);

    save("Length", ui->leLength->text());
    save("LengthType", ui->cbLengthType->currentIndex());
}

void LC_RadiantLineOptions::doSetAction(RS_ActionInterface* a, bool update) {
    m_action = static_cast<LC_ActionDrawLineRadiant *>(a);

    int lengthType;
    QString length;
    RS_Vector activePoint;
    LC_ActionDrawLineRadiant::RadiantIdx pointId;

    if (update) {
        const double len = m_action->getLength();
        length = fromDouble(len);
        lengthType = m_action->getLenghType();
        pointId = m_action->getActiveRadiantIndex();
        activePoint = m_action->getActiveRadiant();
    }
    else {
        int pointIdx  = loadInt("ActivePoint", LC_ActionDrawLineRadiant::RadiantIdx::ONE);
        if (pointIdx < 0) {
            pointIdx = 0;
        }
        else if (pointIdx > LC_ActionDrawLineRadiant::RadiantIdx::LAST) {
            pointIdx = LC_ActionDrawLineRadiant::RadiantIdx::FOUR;
        }
        pointId = static_cast<LC_ActionDrawLineRadiant::RadiantIdx>(pointIdx);
        activePoint = loadRadiantPoint(pointId);
        m_action->setActiveRadiantIndex(pointId);
        m_action->setActiveRadiantPoint(activePoint);
        lengthType = loadInt("LengthType", LC_ActionDrawLineRadiant::LenghtType::LINE);
        length = load("Length", "100.0");
    }

    setLenghtTypeToActionAndView(lengthType);
    setLengthToActionAndView(length);

    setCurrentPointUI(pointId, activePoint);
}

void LC_RadiantLineOptions::onActivePointIndexChanged(int index) {
    setActivePointIndexToActionAndView(index);
}

void LC_RadiantLineOptions::onLengthTypeIndexChanged(int index) {
    setLenghtTypeToActionAndView(index);
}

void LC_RadiantLineOptions::onXEditingFinished() {
    const QString &expr = ui->leX->text();
    setPointXToActionAndView(expr);
}

void LC_RadiantLineOptions::onYEditingFinished() {
    const QString &expr = ui->leY->text();
    setPointYToActionAndView(expr);
}

void LC_RadiantLineOptions::onLengthEditingFinished() {
    const QString &expr = ui->leLength->text();
    setLengthToActionAndView(expr);
}

void LC_RadiantLineOptions::setLenghtTypeToActionAndView(int index) {
    const bool lenValueEnabled = index != 3;
    ui->leLength->setEnabled(lenValueEnabled);
    ui->tbPickLength->setEnabled(lenValueEnabled);
    const auto lengthType = static_cast<LC_ActionDrawLineRadiant::LenghtType>(index);
    m_action->setLengthType(lengthType);
    ui->cbLengthType->blockSignals(true);
    ui->cbLengthType->setCurrentIndex(index);
    ui->cbLengthType->blockSignals(false);
}

void LC_RadiantLineOptions::setPointXToActionAndView(const QString& val) {
    double value;
    if (toDouble(val, value, 0.0, false)){
        m_action->setActiveX(value);
        ui->leX->setText(fromDouble(value));
    }
}

void LC_RadiantLineOptions::setPointYToActionAndView(const QString& val) {
    double value;
    if (toDouble(val, value, 0.0, false)){
        m_action->setActiveY(value);
        ui->leY->setText(fromDouble(value));
    }
}

void LC_RadiantLineOptions::setLengthToActionAndView(const QString& val) {
    double value;
    if (toDouble(val, value, 1.0, false)){
        m_action->setLength(value);
        ui->leLength->setText(fromDouble(value));
    }
}

void LC_RadiantLineOptions::setActivePointIndexToActionAndView(int index) {
    LC_GROUP(getSettingsGroupName());
    saveRadiantPoint(m_action->getActiveRadiantIndex());
    const auto pointIdx = static_cast<LC_ActionDrawLineRadiant::RadiantIdx>(index);
    const RS_Vector loadedPoint = loadRadiantPoint(pointIdx);
    m_action->setActiveRadiantIndex(pointIdx);
    m_action->setActiveRadiantPoint(loadedPoint);
    LC_GROUP_END();

    setCurrentPointUI(pointIdx, loadedPoint);
}

void LC_RadiantLineOptions::setCurrentPointUI(LC_ActionDrawLineRadiant::RadiantIdx pointId, RS_Vector activePoint) {
    LC_GuardedSignalsBlocker({ui->leX, ui->leY, ui->cbLengthType});
    const QString pointX = fromDouble(activePoint.x);
    const QString pointY = fromDouble(activePoint.y);
    ui->leX->setText(pointX);
    ui->leY->setText(pointY);
    ui->cbPointSelector->setCurrentIndex(pointId);
}

void LC_RadiantLineOptions::languageChange() {
    ui->retranslateUi(this);
}
