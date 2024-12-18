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
#include "qg_linepolygonoptions.h"

#include "ui_qg_linepolygonoptions.h"
#include "rs_debug.h"
#include "lc_actiondrawlinepolygon4.h"

QG_LinePolygonOptions::QG_LinePolygonOptions()
    : LC_ActionOptionsWidgetBase(RS2::ActionNone, "Draw", "LinePolygon")
    , ui(new Ui::Ui_LinePolygonOptions{}){
    ui->setupUi(this);
    connect(ui->sbNumber, &QSpinBox::valueChanged, this, &QG_LinePolygonOptions::onNumberValueChanged);
    connect(ui->cbPolyline, &QCheckBox::toggled, this, &QG_LinePolygonOptions::onPolylineToggled);
    connect(ui->cbRadius, &QCheckBox::toggled, this, &QG_LinePolygonOptions::onRadiusToggled);
    connect(ui->cbVertexToVertex, &QCheckBox::toggled, this, &QG_LinePolygonOptions::onVertexToggled);
    connect(ui->leRadius, &QLineEdit::editingFinished, this, &QG_LinePolygonOptions::onRadiusEditingFinished);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_LinePolygonOptions::~QG_LinePolygonOptions() = default;

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_LinePolygonOptions::languageChange(){
    ui->retranslateUi(this);
}

bool QG_LinePolygonOptions::checkActionRttiValid(RS2::ActionType actionType){
    sideSideAction = actionType == RS2::ActionDrawLinePolygonSideSide;
    return actionType == RS2::ActionDrawLinePolygonCenCor ||
           actionType == RS2::ActionDrawLinePolygonCenTan ||
           actionType == RS2::ActionDrawLinePolygonCorCor ||
           sideSideAction;
}

void QG_LinePolygonOptions::doSaveSettings(){
    save("Number", ui->sbNumber->text());
    save("Polyline", ui->cbPolyline->isChecked());
    save("Rounded", ui->cbRadius->isChecked());
    save("Radius", ui->leRadius->text());
    if (sideSideAction){
        save("VertexVertex", ui->cbVertexToVertex->isChecked());
    }
}

QString QG_LinePolygonOptions::getSettingsOptionNamePrefix(){
    switch (action->rtti()){
        case RS2::ActionDrawLinePolygonCenCor:
            return "LinePolygon";
        case RS2::ActionDrawLinePolygonCenTan:
            return "LinePolygon3";
        case RS2::ActionDrawLinePolygonCorCor:
            return "LinePolygon2";
        case RS2::ActionDrawLinePolygonSideSide:
            return "LinePolygonSS";
        default:
            return "LinePolygon";
    }
}

void QG_LinePolygonOptions::doSetAction(RS_ActionInterface *a, bool update){
    action = dynamic_cast<LC_ActionDrawLinePolygonBase*>(a);

    int number;
    bool polyline;
    bool rounded;
    QString radius;
    bool vertextVertex;
    if (update){
        number = action->getNumber();
        polyline = action->isPolyline();
        rounded = action->isCornersRounded();
        radius = fromDouble(action->getRoundingRadius());
        if (sideSideAction){
            auto* specificAction = dynamic_cast<LC_ActionDrawLinePolygon4 *>(a);
            vertextVertex = specificAction->isVertexVertexMode();
        }
    } else {
        number = loadInt("Number", 3);
        polyline = loadBool("Polyline", false);
        rounded = loadBool("Rounded", false);
        radius = load("Radius", "0.0");
        if (sideSideAction) {
            vertextVertex = loadBool("VertexVertex", false);
        }
    }

    ui->cbVertexToVertex->setVisible(sideSideAction);
    setNumberToActionAndView(number);
    setPolylineToActionAndView(polyline);
    setRoundedToActionAndView(rounded);
    setRadiusToActionAndView(radius);
    if (sideSideAction){
        setVertexVertexToActionAndView(vertextVertex);
    }
}

void QG_LinePolygonOptions::setPolylineToActionAndView(bool val) {
    action->setPolyline(val);
    ui->cbPolyline->setChecked(val);
}

void QG_LinePolygonOptions::setRoundedToActionAndView(bool val) {
    action->setCornersRounded(val);
    ui->cbRadius->setChecked(val);
}

void QG_LinePolygonOptions::setVertexVertexToActionAndView(bool val) {
    if (sideSideAction){
        auto* specificAction = dynamic_cast<LC_ActionDrawLinePolygon4 *>(action);
        specificAction->setVertexVertexMode(val);
    }
    ui->cbVertexToVertex->setChecked(val);
}

void QG_LinePolygonOptions::setRadiusToActionAndView(const QString &val) {
    double value;
    if (toDouble(val, value, 0.0, true)){
        action->setRoundingRadius(value);
        ui->leRadius->setText(fromDouble(value));
    }
}

void QG_LinePolygonOptions::setNumberToActionAndView(int number){
    action->setNumber(number);
    ui->sbNumber->setValue(number);
}

void QG_LinePolygonOptions::onNumberValueChanged( [[maybe_unused]]int number){
    setNumberToActionAndView(ui->sbNumber->value());
}

void QG_LinePolygonOptions::onPolylineToggled([[maybe_unused]]bool value) {
    setPolylineToActionAndView(ui->cbPolyline->isChecked());
}

void QG_LinePolygonOptions::onRadiusToggled([[maybe_unused]]bool val) {
    setRoundedToActionAndView(ui->cbRadius->isChecked());
}

void QG_LinePolygonOptions::onVertexToggled([[maybe_unused]]bool val) {
    setVertexVertexToActionAndView(ui->cbVertexToVertex->isChecked());
}

void QG_LinePolygonOptions::onRadiusEditingFinished() {
    setRadiusToActionAndView(ui->leRadius->text());
}