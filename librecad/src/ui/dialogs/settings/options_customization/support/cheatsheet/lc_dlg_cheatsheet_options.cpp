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

#include "lc_dlg_cheatsheet_options.h"
#include "ui_lc_dlg_cheatsheet_options.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>

#include "lc_filenameselectionservice.h"

const QString LC_DlgCheatsheetOptions::getDefaultFileName() {
    // Clean scheme name of any filesystem-illegal characters (\ / : * ? " < > |)
    QString cleanScheme = !m_options.schemeName.isEmpty() ? m_options.schemeName : tr("Default");
    cleanScheme.replace(QRegularExpression(R"([\\/:*?"<>|])"), " ");
    cleanScheme = cleanScheme.simplified();

    const QString typeStr = (m_options.type == CheatsheetType::Keymap)
                                ? tr("Keymap")
                                : tr("Commands");

    const QString defaultFileName = tr("LibreCAD Cheatsheet - %1 (%2).pdf").arg(typeStr, cleanScheme);
    return defaultFileName;
}

LC_DlgCheatsheetOptions::LC_DlgCheatsheetOptions(QWidget* parent, LC_CheatsheetOptions& options)
    : LC_Dialog(parent, "CheatsheetOptions")
    , ui(std::make_unique<Ui::LC_DlgCheatsheetOptions>())
    , m_options(options) {
    ui->setupUi(this);

    const QString defaultDocs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    const QString defaultFileName = getDefaultFileName();
    ui->lePdfPath->setText(defaultDocs + "/" + defaultFileName);

    ui->cbOnlyAssigned->setVisible(m_options.type == CheatsheetType::Keymap);
    ui->cbIncludeKeywords->setVisible(m_options.type == CheatsheetType::CommandAliases);

    connect(ui->rbPdf, &QRadioButton::toggled, this, &LC_DlgCheatsheetOptions::onDestinationChanged);
    connect(ui->rbPrinter, &QRadioButton::toggled, this, &LC_DlgCheatsheetOptions::onDestinationChanged);
    connect(ui->tbBrowsePdf, &QToolButton::clicked, this, &LC_DlgCheatsheetOptions::onBrowsePdfClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LC_DlgCheatsheetOptions::validateAndAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    const bool isImperial = (QLocale().measurementSystem() == QLocale::ImperialSystem);
    m_options.paperSize = isImperial ? CheatsheetPaperSize::Letter : CheatsheetPaperSize::A4;
    ui->rbLetter->setChecked(isImperial);
    ui->rbA4->setChecked(!isImperial);

    ui->cbIncludeDescriptions->setChecked(m_options.includeDescriptions);
    connect(ui->rbPdf, &QRadioButton::toggled, this, &LC_DlgCheatsheetOptions::onDestinationChanged);

    const QString filterLabel = (m_options.type == CheatsheetType::Keymap)
       ? tr("Include filtered shortcuts only")
       : tr("Include filtered commands only");

    ui->cbOnlyFiltered->setText(filterLabel);
    ui->cbOnlyFiltered->setVisible(m_options.isModelFiltered);
    ui->cbOnlyFiltered->setChecked(m_options.isModelFiltered && m_options.onlyFilteredItems);

    ui->gbFlatMode->setChecked(!m_options.groupByCategory);

    onDestinationChanged();
}

LC_DlgCheatsheetOptions::~LC_DlgCheatsheetOptions() = default;

void LC_DlgCheatsheetOptions::onDestinationChanged() {
    const bool isPdf = ui->rbPdf->isChecked();
    ui->lePdfPath->setEnabled(isPdf);
    ui->tbBrowsePdf->setEnabled(isPdf);

    auto* okBtn = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn != nullptr) {
        okBtn->setText(isPdf ? tr("Export PDF") : tr("Print..."));
    }
}

void LC_DlgCheatsheetOptions::onBrowsePdfClicked() {
    const QString filter = tr("PDF Documents (*.pdf);;All Files (*.*)");

    QString path = ui->lePdfPath->text();
    LC_FileNameSelectionService::doObtainFileName(this, path, false, "pdf", getDefaultFileName(), "",
                                                        tr("Save Cheatsheet PDF"), filter);
    // const QString path = QFileDialog::getSaveFileName(this, tr("Save Cheatsheet PDF"), , filter);
    if (!path.isEmpty()) {
        ui->lePdfPath->setText(path);
    }
}

void LC_DlgCheatsheetOptions::validateAndAccept() {
    if (ui->rbPdf->isChecked() && ui->lePdfPath->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Destination"), tr("Please choose a destination file path for the PDF document."));
        return;
    }

    m_options.destination = ui->rbPdf->isChecked() ? CheatsheetDestination::PdfFile : CheatsheetDestination::Printer;
    m_options.paperSize = ui->rbLetter->isChecked() ? CheatsheetPaperSize::Letter : CheatsheetPaperSize::A4;
    m_options.pdfFilePath = ui->lePdfPath->text().trimmed();
    m_options.onlyFilteredItems = m_options.isModelFiltered && ui->cbOnlyFiltered->isChecked();
    m_options.onlyAssignedShortcuts = ui->cbOnlyAssigned->isChecked();
    m_options.includeKeywords = ui->cbIncludeKeywords->isChecked();
    m_options.includeDescriptions = ui->cbIncludeDescriptions->isChecked();
    m_options.isLandscape = ui->rbLandscape->isChecked();
    m_options.groupByCategory = !ui->gbFlatMode->isChecked();

    accept();
}
