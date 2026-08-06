/*
 * **************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * *********************************************************************
 */
#include "qg_dlgimageoptions.h"

#include "rs_math.h"
#include "lc_settings_export_to_image.h"

/*
 *  Constructs a QG_ImageOptionsDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_ImageOptionsDialog::QG_ImageOptionsDialog(QWidget* parent)
    : LC_Dialog(parent, "ImageOptions"){
    setupUi(this);
    init();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_ImageOptionsDialog::languageChange()
{
    retranslateUi(this);
}

void QG_ImageOptionsDialog::init() {
    m_graphicSize = RS_Vector(0.0,0.0);
    m_updateEnabled = false;
    m_useResolution = true;
    {
        using namespace CFG_ExportToImage;
        if (o_UseResolution) {
            cbResolution->setCurrentIndex(cbResolution->findText(QString("%1").arg(o_Resolution)));
        } else {
            leWidth->setText(o_Width);
            leHeight->setText(o_Height);
        }
        if (o_BlackBackground) {
            rbBlack->setChecked(true);
            rbWhite->setChecked(false);
        } else {
            rbBlack->setChecked(false);
            rbWhite->setChecked(true);
        }
        if (o_BlackWhite) {
            rbBlackWhite->setChecked(true);
            rbColoured->setChecked(false);
        } else {
            rbBlackWhite->setChecked(false);
            rbColoured->setChecked(true);
        }
        leLeftRight->setText(o_BorderLeftRight);
        leTopBottom->setText(o_BorderTopBottom);
        if (o_BorderSameSize) {
            cbSameBorders->setChecked(true);
            sameBordersChanged();
        }
    }

    m_updateEnabled = true;
}

void QG_ImageOptionsDialog::setGraphicSize(const RS_Vector& s) {
    m_graphicSize = s;
    if(!m_useResolution){
        sizeChanged();
    }
    else {
        resolutionChanged();
    }
}

void QG_ImageOptionsDialog::ok() {
    using namespace CFG_ExportToImage;

    o_UseResolution = m_useResolution;
    o_Resolution = cbResolution->currentText();
    o_Width = leWidth->text();
    o_Height = leHeight->text();
    o_BorderLeftRight = leLeftRight->text();
    o_BorderTopBottom = leTopBottom->text();
    o_BorderSameSize = cbSameBorders->isChecked();
    o_BlackBackground = rbBlack->isChecked();
    o_BlackWhite = rbBlackWhite->isChecked();

    accept();
}

void QG_ImageOptionsDialog::sameBordersChanged() const {
    if(cbSameBorders->isChecked()) {
        leTopBottom->setText(leLeftRight->text());
        leTopBottom->setDisabled(true);
    }
    else {
        leTopBottom->setEnabled(true);
    }
}

void QG_ImageOptionsDialog::borderChanged() const {
    if(cbSameBorders->isChecked()) {
        leTopBottom->setText(leLeftRight->text());
    }
}

void QG_ImageOptionsDialog::sizeChanged() {
    if (m_updateEnabled) {
        m_updateEnabled = false;
        m_useResolution = false;
        cbResolution->setCurrentIndex(cbResolution->findText("auto"));
        m_updateEnabled = true;
    }
}

void  QG_ImageOptionsDialog::resolutionChanged() {
    if (m_updateEnabled) {
        m_updateEnabled = false;
        bool ok = false;
        double res = RS_Math::eval(cbResolution->currentText(), &ok);
        if (!ok) {
            res = 1.0;
        }
        const int w = RS_Math::round(res * m_graphicSize.x);
        const int h = RS_Math::round(res * m_graphicSize.y);
        m_useResolution = true;
        leWidth->setText(QString("%1").arg(w));
        leHeight->setText(QString("%1").arg(h));
        m_updateEnabled = true;
    }
}

QSize QG_ImageOptionsDialog::getSize() const {
    return QSize(RS_Math::round(RS_Math::eval(leWidth->text())),
                    RS_Math::round(RS_Math::eval(leHeight->text())));
}

QSize QG_ImageOptionsDialog::getBorders() const {
    return QSize(RS_Math::round(RS_Math::eval(leLeftRight->text())),
                   RS_Math::round(RS_Math::eval(leTopBottom->text())));
}

bool QG_ImageOptionsDialog::isBackgroundBlack() const {
    return rbBlack->isChecked();
}

bool QG_ImageOptionsDialog::isBlackWhite() const {
    return rbBlackWhite->isChecked();
}
