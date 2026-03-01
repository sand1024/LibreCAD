/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/
#include "qg_lineangleoptions.h"

#include "rs_actiondrawlineangle.h"
#include "ui_qg_lineangleoptions.h"

/*
 *  Constructs a QG_LineAngleOptions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
QG_LineAngleOptions::QG_LineAngleOptions()
    : LC_ActionOptionsWidgetBase( RS2::ActionNone, "Draw", "LineAngle")
    , ui(std::make_unique<Ui::Ui_LineAngleOptions>()){
    ui->setupUi(this);

    connect(ui->leAngle, &QLineEdit::editingFinished, this, &QG_LineAngleOptions::onAngleEditingFinished);
    connect(ui->leLength, &QLineEdit::editingFinished, this, &QG_LineAngleOptions::onLengthEditingFinished);
    connect(ui->cbSnapPoint, &QComboBox::currentIndexChanged, this, &QG_LineAngleOptions::onSnapPointCurrentIndexChanged);
    connect(ui->cbForAnglesBasis, &QCheckBox::toggled, this, &QG_LineAngleOptions::onAnglesBasisToggled);
    connect(ui->cbLengthType,&QComboBox::currentIndexChanged, this, &QG_LineAngleOptions::onLengthTypeCurrentIndexChanged);

    pickAngleSetup("angle", ui->tbPickAngle, ui->leAngle);
    pickDistanceSetup("length", ui->tbPickLength, ui->leLength);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_LineAngleOptions::~QG_LineAngleOptions() = default;

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_LineAngleOptions::languageChange(){
    ui->retranslateUi(this);
}

bool QG_LineAngleOptions::checkActionRttiValid(const RS2::ActionType actionType){
    return actionType == RS2::ActionDrawLineAngle ||
           actionType == RS2::ActionDrawLineHorizontal ||
           actionType == RS2::ActionDrawLineVertical;
}

void QG_LineAngleOptions::setupLengthType() {
    ui->cbLengthType->blockSignals(true);
    ui->cbLengthType->clear();
    ui->cbLengthType->addItem(tr("Line"), RS_ActionDrawLineAngle::LengthType::LINE);
    if (m_action->rtti() == RS2::ActionDrawLineAngle) {
        ui->cbLengthType->addItem(tr("By X"), RS_ActionDrawLineAngle::LengthType::BY_X);
        ui->cbLengthType->addItem(tr("By Y"), RS_ActionDrawLineAngle::LengthType::BY_Y);
    }
    ui->cbLengthType->addItem(tr("Free"), RS_ActionDrawLineAngle::LengthType::FREE);
    ui->cbLengthType->blockSignals(false);
}

void QG_LineAngleOptions::doSetAction(RS_ActionInterface *a, const bool update){
    m_action = static_cast<RS_ActionDrawLineAngle*>(a);
    const bool angleIsFixed = m_action->hasFixedAngle();

    setupLengthType();
    QString angle;
    QString length;
    int snapPoint;
    bool inAngleBasis = false;

    int lenType;

    // settings from action:
    if (update) {
        if (angleIsFixed) {
            inAngleBasis = m_action->isInAngleBasis();
        } else {
            angle = fromDouble(m_action->getUcsAngleDegrees());
        }
        length = fromDouble(m_action->getLength());
        snapPoint = m_action->getSnapPoint();
        lenType = m_action->getLengthType();
    } else {
        if (angleIsFixed) {
            inAngleBasis = loadBool("InAnglesBasis", false);
        } else {
            angle = load("Angle", "30.0");
        }
        lenType = loadInt("LengthType", 0);
        length = load("Length", "10.0");
        snapPoint = loadInt("SnapPoint", 0);
    }

    ui->leAngle->setVisible(!angleIsFixed);
    ui->tbPickAngle->setVisible(!angleIsFixed && m_interactiveInputControlsVisible);
    ui->lAngle->setVisible(!angleIsFixed);

    if (angleIsFixed) {
        const bool hasCustomAnglesBasis = m_action->hasNonDefaultAnglesBasis();
        ui->cbForAnglesBasis->setVisible(hasCustomAnglesBasis);
        setToAngleBasis(inAngleBasis);
    } else {
        setAngleToActionAndView(angle);
        ui->cbForAnglesBasis->setVisible(false);
    }
    setSnapPointToActionAndView(snapPoint);
    setLengthToActionAndView(length);
    setupLengthType();

    ui->leAngle->setText(angle);
    ui->leLength->setText(length);

    ui->cbLengthType->blockSignals(true);
    const int idx = ui->cbLengthType->findData(lenType);
    if (idx != -1) {
        ui->cbLengthType->setCurrentIndex(idx);
        const bool notFreeLength = lenType != RS_ActionDrawLineAngle::FREE;
        ui->leLength->setEnabled(notFreeLength);
    }
    auto type = static_cast<RS_ActionDrawLineAngle::LengthType>(lenType);
    m_action->setLengthType(type, false);
    ui->cbLengthType->blockSignals(false);
}

void QG_LineAngleOptions::doSaveSettings() {
    if (m_action->hasFixedAngle()) {
        save("InAnglesBasis", ui->cbForAnglesBasis->isChecked());
    } else {
        save("Angle", ui->leAngle->text());
    }

    save("Length", ui->leLength->text());
    save("SnapPoint", ui->cbSnapPoint->currentIndex());
    save("LengthType", ui->cbLengthType->currentData().toInt());
}

void QG_LineAngleOptions::onSnapPointCurrentIndexChanged(const int number){
    setSnapPointToActionAndView(number);
}

void QG_LineAngleOptions::onLengthTypeCurrentIndexChanged(int number) {
    int lt = ui->cbLengthType->itemData(number).toInt();
    const auto type = static_cast<RS_ActionDrawLineAngle::LengthType>(lt);
    setLengthTypeToActionAndView(type);
}

void QG_LineAngleOptions::setLengthTypeToActionAndView(RS_ActionDrawLineAngle::LengthType lenType) {
    m_action->setLengthType(lenType, true);
}

void QG_LineAngleOptions::onLengthEditingFinished(){
    setLengthToActionAndView(ui->leLength->text());
}
void QG_LineAngleOptions::onAngleEditingFinished(){
    setAngleToActionAndView(ui->leAngle->text());
}

void QG_LineAngleOptions::onAnglesBasisToggled(const bool val) {
    setToAngleBasis(val);
}

void QG_LineAngleOptions::setAngleToActionAndView(const QString& val){
    double angle = 0.;
    if (toDoubleAngleDegrees(val, angle, 0.0, false)){
        m_action->setUcsAngleDegrees(angle);
        ui->leAngle->setText(fromDouble(angle));
    }
}

void QG_LineAngleOptions::setSnapPointToActionAndView(const int val) const {
    m_action->setSnapPoint(val);
    ui->cbSnapPoint->setCurrentIndex(val);
}

void QG_LineAngleOptions::setLengthToActionAndView(const QString& val){
    double len = 0.;
    if (toDouble(val, len, 1.0, false)){
        m_action->setLength(len);
        ui->leLength->setText(fromDouble(len));
    }
}

void QG_LineAngleOptions::setToAngleBasis(const bool val) const {
    ui->cbForAnglesBasis->setChecked(val);
    m_action->setInAngleBasis(val);
}
