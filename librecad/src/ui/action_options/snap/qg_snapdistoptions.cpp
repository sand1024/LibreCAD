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

#include <QVariant>
#include "qg_snapdistoptions.h"
#include "rs_math.h"
#include "ui_qg_snapdistoptions.h"

/*
 *  Constructs a QG_SnapDistOptions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
QG_SnapDistOptions::QG_SnapDistOptions(QWidget* parent)
    : QWidget(parent)
	, ui(new Ui::Ui_SnapDistOptions{}){
	ui->setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_SnapDistOptions::~QG_SnapDistOptions() = default;

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_SnapDistOptions::languageChange(){
	ui->retranslateUi(this);
}

void QG_SnapDistOptions::saveSettings() {
    RS_SETTINGS->beginGroup("/Snap");
    RS_SETTINGS->writeEntry("/Distance", ui->leDist->text());
    RS_SETTINGS->endGroup();
}

void QG_SnapDistOptions::useSnapDistanceValue(double* d) {
    dist = d;
    RS_SETTINGS->beginGroup("/Snap");
    QString distance = RS_SETTINGS->readEntry("/Distance", "1.0");
    RS_SETTINGS->endGroup();

    *dist= RS_Math::eval(distance, 1.0);
    QString value = QString::number(*dist, 'g', 6);
    ui->leDist->setText(value);
}

void QG_SnapDistOptions::on_leDist_editingFinished() {
    if (dist) {
        QString value = ui->leDist->text();
        *dist = RS_Math::eval(value, 1.0);
        value = QString::number(*dist, 'g', 6);
        ui->leDist->setText(value);
        saveSettings();
    }
}

void QG_SnapDistOptions::doShow() {
    bool requestFocus = !isVisible();
    show();
    if (requestFocus){
        ui->leDist->setFocus();
    }
}

double* QG_SnapDistOptions::getDistanceValue() {
    return dist;
}