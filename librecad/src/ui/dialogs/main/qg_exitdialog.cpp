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
#include "qg_exitdialog.h"

#include <QMessageBox>
#include <QPushButton>

#include "ui_qg_exitdialog.h"

/*
 *  Constructs a QG_ExitDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_ExitDialog::QG_ExitDialog(QWidget* parent)
	: LC_Dialog(parent, "DlgExit")
    , ui(std::make_unique<Ui::QG_ExitDialog>()){
	ui->setupUi(this);
    init();
}

QG_ExitDialog::~QG_ExitDialog() = default;

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_ExitDialog::languageChange(){
	ui->retranslateUi(this);
}

void QG_ExitDialog::init(){
    QG_ExitDialog::setShowOptionsForAll(false);
	//set dlg icon
    QMessageBox mb({}, "", "");
	ui->l_icon->setPixmap( mb.iconPixmap());
}

void QG_ExitDialog::clicked(QAbstractButton * button){
	QDialogButtonBox::StandardButton bt = ui->buttonBox->standardButton ( button );
    switch (bt){
    case QDialogButtonBox::No:
        done(DontSave);
        break;
    case QDialogButtonBox::NoToAll:
        done(DontSaveAll);
        break;
    case QDialogButtonBox::Yes:
        done(Save);
        break;
    case QDialogButtonBox::YesToAll:
        done(SaveAll);
        break;
    default:
        done(Cancel);
    };
}

void QG_ExitDialog::setText(const QString& text) {
	ui->lQuestion->setText(text);
}

void QG_ExitDialog::setTitle(const QString& text) {
    setWindowTitle(text);
}

void QG_ExitDialog::setForce(bool force) {
	 QPushButton * bCancel = ui->buttonBox->button ( QDialogButtonBox::Cancel );
     bCancel->setDisabled(force);
}

void QG_ExitDialog::setShowOptionsForAll(bool show){

    auto yesToAll = ui->buttonBox->button(QDialogButtonBox::YesToAll);
    yesToAll->setVisible(show);
    ui->buttonBox->button(QDialogButtonBox::NoToAll)->setVisible(show);

    QPushButton* defaultButton;
    if (show) {
        defaultButton = yesToAll;
    }
    else {
        defaultButton = ui->buttonBox->button(QDialogButtonBox::Yes);
    }

    defaultButton->setAutoDefault(true);
    defaultButton->setDefault(true);
    defaultButton->setFocus();
}
