/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

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
 ******************************************************************************/

#include "lc_moveoptions.h"

#include "rs_actionmodifymove.h"
#include "ui_lc_moveoptions.h"

LC_MoveOptions::LC_MoveOptions()
    : LC_ActionOptionsWidgetBase(RS2::ActionModifyMove, "Modify", "Move")
      , ui(new Ui::LC_MoveOptions) {
    ui->setupUi(this);
    connect(ui->cbKeepOriginals, &QCheckBox::clicked, this, &LC_MoveOptions::cbKeepOriginalsClicked);
    connect(ui->cbMultipleCopies, &QCheckBox::clicked, this, &LC_MoveOptions::cbMultipleCopiesClicked);
    connect(ui->cbCurrentAttr, &QCheckBox::clicked, this, &LC_MoveOptions::cbUseCurrentAttributesClicked);
    connect(ui->cbCurrentLayer, &QCheckBox::clicked, this, &LC_MoveOptions::cbUseCurrentLayerClicked);
}

void LC_MoveOptions::doSaveSettings() {
    save("UseCurrentLayer", ui->cbCurrentLayer->isChecked());
    save("UseCurrentAttributes", ui->cbCurrentAttr->isChecked());
    save("KeepOriginals", ui->cbKeepOriginals->isChecked());
    save("MultipleCopies", ui->cbMultipleCopies->isChecked());
    save("Copies", ui->sbNumberOfCopies->value());
}

void LC_MoveOptions::doSetAction(RS_ActionInterface *a, const bool update) {
    m_action = static_cast<RS_ActionModifyMove *>(a);
    bool useMultipleCopies = false;
    bool keepOriginals = false;
    bool useCurrentLayer = false;
    bool useCurrentAttributes = false;
    int copiesNumber;
    if (update){
        useCurrentLayer = m_action->isUseCurrentLayer();
        useCurrentAttributes  = m_action->isUseCurrentAttributes();
        copiesNumber = m_action->getCopiesNumber();
        keepOriginals = m_action->isKeepOriginals();
        useMultipleCopies = m_action->isUseMultipleCopies();
    }
    else{
        useCurrentLayer = loadBool("UseCurrentLayer", false);
        useCurrentAttributes = loadBool("UseCurrentAttributes", false);
        keepOriginals = loadBool("KeepOriginals", false);
        useMultipleCopies = loadBool("MultipleCopies", false);
        copiesNumber = loadInt("Copies", 1);
    }
    setUseMultipleCopiesToActionAndView(useMultipleCopies);
    setCopiesNumberToActionAndView(copiesNumber);
    setUseCurrentLayerToActionAndView(useCurrentLayer);
    setUseCurrentAttributesToActionAndView(useCurrentAttributes);
    setKeepOriginalsToActionAndView(keepOriginals);
}

void LC_MoveOptions::languageChange() {
    ui->retranslateUi(this);
}

void LC_MoveOptions::setCopiesNumberToActionAndView(int number) const {
    number = std::max(number, 1);
    m_action->setCopiesNumber(number);
    ui->sbNumberOfCopies->setValue(number);
}

void LC_MoveOptions::setUseMultipleCopiesToActionAndView(const bool copies) const {
   m_action->setUseMultipleCopies(copies);
   ui->cbMultipleCopies->setChecked(copies);
   ui->sbNumberOfCopies->setEnabled(copies);
}

void LC_MoveOptions::setUseCurrentLayerToActionAndView(const bool val) const {
    m_action->setUseCurrentLayer(val);
    ui->cbCurrentLayer->setChecked(val);
}

void LC_MoveOptions::setUseCurrentAttributesToActionAndView(const bool val) const {
    m_action->setUseCurrentAttributes(val);
    ui->cbCurrentAttr->setChecked(val);
}

void LC_MoveOptions::setKeepOriginalsToActionAndView(const bool val) const {
    m_action->setKeepOriginals(val);
    ui->cbKeepOriginals->setChecked(val);
}

void LC_MoveOptions::cbKeepOriginalsClicked(const bool val) {
    setKeepOriginalsToActionAndView(val);
}

void LC_MoveOptions::cbMultipleCopiesClicked(const bool val) {
    setUseMultipleCopiesToActionAndView(val);
}

void LC_MoveOptions::cbUseCurrentAttributesClicked(const bool val) {
    setUseCurrentAttributesToActionAndView(val);
}

void LC_MoveOptions::cbUseCurrentLayerClicked(const bool val) {
    setUseCurrentLayerToActionAndView(val);
}

void LC_MoveOptions::on_sbNumberOfCopies_valueChanged(const int number) {
    setCopiesNumberToActionAndView(number);
}
