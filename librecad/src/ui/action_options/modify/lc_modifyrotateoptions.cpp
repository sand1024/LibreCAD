/****************************************************************************
**
* Options widget for ModifyRotate action

Copyright (C) 2024 LibreCAD.org
Copyright (C) 2024 sand1024

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/
#include "lc_modifyrotateoptions.h"

#include "rs_actionmodifyrotate.h"
#include "ui_lc_modifyrotateoptions.h"

LC_ModifyRotateOptions::LC_ModifyRotateOptions()
    : LC_ActionOptionsWidgetBase(RS2::ActionModifyRotate, "Modify", "Rotate")
    , ui(new Ui::LC_ModifyRotateOptions){
    ui->setupUi(this);

    connect(ui->cbKeepOriginals, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbKeepOriginalsClicked);
    connect(ui->cbMultipleCopies, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbMultipleCopiesClicked);
    connect(ui->cbCurrentAttr, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbUseCurrentAttributesClicked);
    connect(ui->cbCurrentLayer, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbUseCurrentLayerClicked);

    connect(ui->cbFreeAngle, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbFreeAngleClicked);
    connect(ui->cbRelativeAngle, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbRelativeAngleClicked);
    connect(ui->cbFreeRefAngle, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbFreeRefAngleClicked);
    connect(ui->cbCenterPointFirst, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::cbCenterPointFirstClicked);
    connect(ui->cbTwoRotations, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::onTwoRotationsClicked);
    connect(ui->cbAbsoluteRefAngle, &QCheckBox::clicked, this, &LC_ModifyRotateOptions::onAbsoluteRefAngleClicked);

    connect(ui->sbNumberOfCopies, &QSpinBox::valueChanged, this, &LC_ModifyRotateOptions::onCopiesNumberValueChanged);

    connect(ui->leAngle, &QLineEdit::editingFinished, this, &LC_ModifyRotateOptions::onAngleEditingFinished);
    connect(ui->leAngle2, &QLineEdit::editingFinished, this,&LC_ModifyRotateOptions::onRefPointAngleEditingFinished);

    pickAngleSetup("angle", ui->tbPickAngle, ui->leAngle);
    pickAngleSetup("angle2", ui->tbPickAngle2, ui->leAngle2);
}

LC_ModifyRotateOptions::~LC_ModifyRotateOptions(){
    delete ui;
}

void LC_ModifyRotateOptions::updateUI(const int mode) {
    switch (mode){
        case UPDATE_ANGLE: {  // update on SetTargetPoint
            const QString angle = fromDouble(m_action->getCurrentAngleDegrees());

            ui->leAngle->blockSignals(true);
            ui->leAngle->setText(angle);
            ui->leAngle->blockSignals(false);

            ui->leAngle->update();
            break;
        }
        case DISABLE_SECOND_ROTATION:{
            allowSecondRotationUI(false);
            break;
        }
        case ENABLE_SECOND_ROTATION:{
            allowSecondRotationUI(true);
            break;
        }
        case UPDATE_ANGLE2: {  // update on SetTargetPoint
            const QString angle2 = fromDouble(m_action->getCurrentAngle2Degrees());

            ui->leAngle2->blockSignals(true);
            ui->leAngle2->setText(angle2);
            ui->leAngle2->blockSignals(false);

            ui->leAngle2->update();
            break;
        }
        default:
            break;
    }
}

void LC_ModifyRotateOptions::allowSecondRotationUI(const bool enable) const {
    const bool enableSecondAngle = enable && !ui->cbFreeRefAngle->isChecked();
    ui->leAngle2->setEnabled(enableSecondAngle);
    ui->tbPickAngle2->setEnabled(enableSecondAngle);
    ui->cbTwoRotations->setEnabled(enable);
    ui->cbFreeRefAngle->setEnabled(enable);
    ui->cbAbsoluteRefAngle->setEnabled(enable);

}

void LC_ModifyRotateOptions::doSaveSettings() {
    save("KeepOriginals", ui->cbKeepOriginals->isChecked());
    save("MultipleCopies", ui->cbMultipleCopies->isChecked());
    save("Copies", ui->sbNumberOfCopies->value());
    save("UseCurrentLayer",ui->cbCurrentLayer->isChecked());
    save("UseCurrentAttributes",ui->cbCurrentAttr->isChecked());
    save("Angle", ui->leAngle->text());
    save("AngleIsFree", ui->cbFreeAngle->isChecked());
    save("AngleIsRelative", ui->cbFreeAngle->isChecked());
    save("CenterPointFirst", ui->cbCenterPointFirst->isChecked());
    save("TwoRotations",     ui->cbTwoRotations->isChecked());
    save("AngleRef", ui->leAngle2->text());
    save("AngleRefIsFree", ui->cbFreeRefAngle->isChecked());
    save("AngleRefIsAbsolute", ui->cbAbsoluteRefAngle->isChecked());
}

void LC_ModifyRotateOptions::doSetAction(RS_ActionInterface *a, const bool update) {
    m_action = static_cast<RS_ActionModifyRotate *>(a);
    QString angle;
    QString angle2;

    bool useMultipleCopies;
    bool keepOriginals;
    bool useCurrentLayer;
    bool useCurrentAttributes;
    int copiesNumber;

    bool freeAngle;
    bool relativeAngle;
    bool freeRefAngle;
    bool absoluteRefAngle;
    bool twoRotations;
    bool centerPointFirst;

    if (update){
        useCurrentLayer = m_action->isUseCurrentLayer();
        useCurrentAttributes = m_action->isUseCurrentAttributes();
        keepOriginals = m_action->isKeepOriginals();
        useMultipleCopies = m_action->isUseMultipleCopies();
        copiesNumber = m_action->getCopiesNumber();

        twoRotations = m_action->isRotateAlsoAroundReferencePoint();
        freeAngle = m_action->isFreeAngle();
        relativeAngle = m_action->isRelativeAngle();
        centerPointFirst = m_action->isCenterPointFirst();
        freeRefAngle = m_action->isFreeRefPointAngle();
        absoluteRefAngle = m_action->isRefPointAngleAbsolute();
        angle = fromDouble(RS_Math::rad2deg(m_action->getAngle()));
        angle2 = fromDouble(RS_Math::rad2deg(m_action->getRefPointAngle()));
    }
    else{
        useCurrentLayer = loadBool("UseCurrentLayer", true);
        useCurrentAttributes = loadBool("UseCurrentAttributes", true);
        keepOriginals = loadBool("KeepOriginals", false);
        useMultipleCopies = loadBool("MultipleCopies", false);
        copiesNumber = loadInt("Copies", 1);

        twoRotations = loadBool("TwoRotations", false);
        freeAngle = loadBool("AngleIsFree", true);
        relativeAngle = loadBool("AngleIsRelative", true);
        centerPointFirst = loadBool("CenterPointFirst", false);
        freeRefAngle = loadBool("AngleRefIsFree", false);
        absoluteRefAngle = loadBool("AngleRefIsAbsolute", false);
        angle = load("Angle", "0.0");
        angle2= load("AngleRef", "0.0");
    }
    setUseMultipleCopiesToActionAndView(useMultipleCopies);
    setCopiesNumberToActionAndView(copiesNumber);
    setUseCurrentLayerToActionAndView(useCurrentLayer);
    setUseCurrentAttributesToActionAndView(useCurrentAttributes);
    setKeepOriginalsToActionAndView(keepOriginals);

    setAngleToActionAndView(angle);
    setFreeAngleToActionAndView(freeAngle);
    setRelativeAngleToActionAndView(relativeAngle);
    setCenterPointFirstToActionAndView(centerPointFirst);

    setFreeRefAngleToActionAndView(freeRefAngle);
    setRefPointAngleToActionAndView(angle2);
    setAbsoluteRefAngleToActionAndView(absoluteRefAngle);
    setTwoRotationsToActionAndView(twoRotations);
}

void LC_ModifyRotateOptions::setUseMultipleCopiesToActionAndView(const bool copies) const {
    m_action->setUseMultipleCopies(copies);
    ui->cbMultipleCopies->setChecked(copies);
    ui->sbNumberOfCopies->setEnabled(copies);
}

void LC_ModifyRotateOptions::setUseCurrentLayerToActionAndView(const bool val) const {
    m_action->setUseCurrentLayer(val);
    ui->cbCurrentLayer->setChecked(val);
}

void LC_ModifyRotateOptions::setUseCurrentAttributesToActionAndView(const bool val) const {
    m_action->setUseCurrentAttributes(val);
    ui->cbCurrentAttr->setChecked(val);
}

void LC_ModifyRotateOptions::setKeepOriginalsToActionAndView(const bool val) const {
    m_action->setKeepOriginals(val);
    ui->cbKeepOriginals->setChecked(val);
}

void LC_ModifyRotateOptions::setCopiesNumberToActionAndView(int number) const {
    if (number < 1){
        number = 1;
    }
    m_action->setCopiesNumber(number);
    ui->sbNumberOfCopies->setValue(number);
}

void LC_ModifyRotateOptions::setTwoRotationsToActionAndView(const bool val) {
    allowSecondRotationUI(val);
    ui->cbTwoRotations->setEnabled(true);
    ui->cbTwoRotations->setChecked(val);
    m_action->setRotateAlsoAroundReferencePoint(val);
}

void LC_ModifyRotateOptions::setFreeAngleToActionAndView(const bool val) const {
    ui->cbFreeAngle->setChecked(val);
    m_action->setFreeAngle(val);
    if (val){
        ui->leAngle->setEnabled(false);
        ui->tbPickAngle->setEnabled(false);
    }
    else{
        ui->leAngle->setEnabled(true);
        ui->tbPickAngle->setEnabled(true);
    }
    ui->cbRelativeAngle->setEnabled(val);
}

void LC_ModifyRotateOptions::setRelativeAngleToActionAndView(bool val) {
    ui->cbRelativeAngle->setChecked(val);
    m_action->setRelativeAngle(val);
}

void LC_ModifyRotateOptions::setCenterPointFirstToActionAndView(bool val) {
    ui->cbCenterPointFirst->setChecked(val);
    m_action->setCenterPointFirst(val);
}

void LC_ModifyRotateOptions::setAbsoluteRefAngleToActionAndView(const bool checked) const {
    ui->cbAbsoluteRefAngle->setChecked(checked);
    m_action->setRefPointAngleAbsolute(checked);
}

void LC_ModifyRotateOptions::setFreeRefAngleToActionAndView(const bool checked) const {
    ui->cbFreeRefAngle->setChecked(checked);
    if (ui->cbTwoRotations->isChecked()) {
        ui->leAngle2->setEnabled(!checked);
        ui->tbPickAngle2->setEnabled(!checked);
    }
    m_action->setFreeRefPointAngle(checked);
}

void LC_ModifyRotateOptions::setAngleToActionAndView(const QString& val) {
    double angle;
    if (toDoubleAngleDegrees(val, angle, 0.0, false)) {
        const QString &factorStr = fromDouble(angle);
        ui->leAngle->setText(factorStr);
        m_action->setAngle(RS_Math::deg2rad(angle));
    }
}

void LC_ModifyRotateOptions::setRefPointAngleToActionAndView(const QString& val) {
    double angle;
    if (toDoubleAngleDegrees(val, angle, 0.0, false)) {
        const QString &factorStr = fromDouble(angle);
        ui->leAngle2->setText(factorStr);
        m_action->setRefPointAngle(RS_Math::deg2rad(angle));
    }
}

void LC_ModifyRotateOptions::cbKeepOriginalsClicked(const bool val) {
    setKeepOriginalsToActionAndView(val);
}

void LC_ModifyRotateOptions::cbMultipleCopiesClicked(const bool val) {
    setUseMultipleCopiesToActionAndView(val);
}

void LC_ModifyRotateOptions::cbUseCurrentAttributesClicked(const bool val) {
    setUseCurrentAttributesToActionAndView(val);
}

void LC_ModifyRotateOptions::cbUseCurrentLayerClicked(const bool val) {
    setUseCurrentLayerToActionAndView(val);
}

void LC_ModifyRotateOptions::cbFreeAngleClicked(const bool val) {
    setFreeAngleToActionAndView(val);
}

void LC_ModifyRotateOptions::cbRelativeAngleClicked(const bool val) {
    setRelativeAngleToActionAndView(val);
}

void LC_ModifyRotateOptions::cbCenterPointFirstClicked(const bool val) {
    setCenterPointFirstToActionAndView(val);
}

void LC_ModifyRotateOptions::cbFreeRefAngleClicked(const bool val) {
    setFreeRefAngleToActionAndView(val);
}

void LC_ModifyRotateOptions::onAbsoluteRefAngleClicked(const bool val){
    setAbsoluteRefAngleToActionAndView(val);
}

void LC_ModifyRotateOptions::onTwoRotationsClicked(const bool val) {
    setTwoRotationsToActionAndView(val);
}

void LC_ModifyRotateOptions::onAngleEditingFinished() {
    setAngleToActionAndView(ui->leAngle->text());
}

void LC_ModifyRotateOptions::onRefPointAngleEditingFinished() {
    setRefPointAngleToActionAndView(ui->leAngle2->text());
}

void LC_ModifyRotateOptions::onCopiesNumberValueChanged(const int value) {
    setCopiesNumberToActionAndView(value);
}

void LC_ModifyRotateOptions::languageChange() {
    ui->retranslateUi(this);
}
