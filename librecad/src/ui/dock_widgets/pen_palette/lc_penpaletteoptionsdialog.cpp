/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2024 sand1024
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
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

#include "lc_penpaletteoptionsdialog.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "lc_dialog.h"
#include "lc_peninforegistry.h"
#include "lc_penpaletteoptions.h"


class LC_PenPaletteOptions;

LC_PenPaletteOptionsDialog::LC_PenPaletteOptionsDialog(QWidget *parent, LC_PenPaletteOptions* options, [[maybe_unused]]bool focusOnFile) :
    LC_Dialog(parent, "PenPaletteOptions"), m_options{options}{
    setupUi(this);

    chkShowColorIcon->setChecked(m_options->showColorIcon);
    chkShowColorName->setChecked(m_options->showColorName);
    chkShowLineTypeIcon->setChecked(m_options->showTypeIcon);
    chkShowLineTypeName->setChecked(m_options->showTypeName);
    chkShowWidthIcon->setChecked(m_options->showWidthIcon);
    chkShowWidthName->setChecked(m_options->showWidthName);
    cbShowMessageForNoSelection->setChecked(m_options->showNoSelectionMessage);
    cbFilterCaseInsensitive->setChecked(m_options->ignoreCaseOnMatch);
    cbShowGrid->setChecked(m_options->showGrid);

    cbDoubleClickMode->addItem(tr("Do nothing"));
    cbDoubleClickMode->addItem(tr("Select entities by attributes pen"));
    cbDoubleClickMode->addItem(tr("Select entities by drawing pen"));

    cbDoubleClickMode->setCurrentIndex(m_options->doubleClickOnTableMode);

    cbShowTooltip->setChecked(m_options->showToolTip);
    cbAllRowBold->setChecked(m_options->showEntireRowBold);

    switch (m_options->colorNameDisplayMode){
        case LC_PenInfoRegistry::ColorNameDisplayMode::RGB:
            rbRGB->setChecked(true);
            break;
        case LC_PenInfoRegistry::ColorNameDisplayMode::HEX:
            rbHEX->setChecked(true);
            break;
        case LC_PenInfoRegistry::ColorNameDisplayMode::NATURAL:
            rbNatural->setChecked(true);
            break;
        default:
            break;
    }


    connect(buttonBox, &QDialogButtonBox::accepted, this, &LC_PenPaletteOptionsDialog::validate);
}

LC_PenPaletteOptionsDialog::~LC_PenPaletteOptionsDialog()= default;

void LC_PenPaletteOptionsDialog::languageChange(){
    retranslateUi(this);
}


/**
 * Validation of entered data on button closing
 */
void LC_PenPaletteOptionsDialog::validate(){
    bool doAccept = true;
    LC_PenInfoRegistry::ColorNameDisplayMode colorMode = LC_PenInfoRegistry::ColorNameDisplayMode::RGB;

    if (rbRGB->isChecked()){
        colorMode = LC_PenInfoRegistry::ColorNameDisplayMode::RGB;
    } else if (rbHEX->isChecked()){
        colorMode = LC_PenInfoRegistry::ColorNameDisplayMode::HEX;
    } else if (rbNatural->isChecked()){
      colorMode = LC_PenInfoRegistry::ColorNameDisplayMode::NATURAL;
    }


    // all fine, store user's input to options
    if (doAccept){
        m_options->showGrid =  cbShowGrid->isChecked();

        const bool showToolTip = cbShowTooltip->isChecked();
        const bool showColorIcon = chkShowColorIcon->isChecked();
        const bool showColorName = chkShowColorName->isChecked();
        const bool showLineTypeIcon = chkShowLineTypeIcon->isChecked();
        const bool showLineTypeName = chkShowLineTypeName->isChecked();
        const bool showLineWidthIcon = chkShowWidthIcon->isChecked();
        const bool showLineWidthName = chkShowWidthName->isChecked();
        const bool showNoSelectionMessage = cbShowMessageForNoSelection->isChecked();
        const bool ignoreCaseOnMatch = cbFilterCaseInsensitive->isChecked();

        const bool allRowBold = cbAllRowBold->isChecked();
        m_options->showEntireRowBold = allRowBold;
        m_options->showToolTip = showToolTip;
        m_options->showWidthIcon = showLineWidthIcon;
        m_options->showWidthName = showLineWidthName;
        m_options->showTypeIcon = showLineTypeIcon;
        m_options->showTypeName = showLineTypeName;
        m_options->showColorIcon = showColorIcon;
        m_options->showColorName = showColorName;
        m_options->ignoreCaseOnMatch = ignoreCaseOnMatch;
        m_options->showNoSelectionMessage = showNoSelectionMessage;

        m_options->colorNameDisplayMode = colorMode;

        const int doubleClickMode = cbDoubleClickMode->currentIndex();
        m_options->doubleClickOnTableMode = static_cast<PenPaletteDoubleClickMode>(doubleClickMode);
        accept();
    }
}
