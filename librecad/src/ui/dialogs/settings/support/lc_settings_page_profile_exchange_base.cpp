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

#include "lc_settings_page_profile_exchange_base.h"

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "lc_inputtextdialog.h"
#include "lc_preset_manager_interface.h"

LC_SettingsPageProfileExchangeBase::LC_SettingsPageProfileExchangeBase(const QString& displayName, QObject* parent)
    : LC_SettingsPageBase(displayName, nullptr, parent) {
}

void LC_SettingsPageProfileExchangeBase::setupExchangeUi(QWidget* parentWidget, QWidget* extraWidget) {
    auto* mainLayout = new QVBoxLayout(parentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    auto* gbProfiles = new QGroupBox(profileBoxTitle(), parentWidget);
    auto* gridLayout = new QVBoxLayout(gbProfiles);
    gridLayout->setSpacing(10);

    auto* lblDesc = new QLabel(profileDescriptionText(), gbProfiles);
    lblDesc->setWordWrap(true);
    gridLayout->addWidget(lblDesc);

    auto* btnRow = new QHBoxLayout();
    auto* pbImport = new QPushButton(tr("Import Profile..."), gbProfiles);
    pbImport->setIcon(QIcon(":/icons/fileopen.lci"));
    pbImport->setToolTip(tr("Import a consolidated profile package from disk."));

    auto* pbExport = new QPushButton(tr("Export Profile..."), gbProfiles);
    pbExport->setIcon(QIcon(":/icons/save.lci"));
    pbExport->setToolTip(tr("Export active configurations into a consolidated profile package."));

    connect(pbImport, &QPushButton::clicked, this, &LC_SettingsPageProfileExchangeBase::onImportProfileClicked);
    connect(pbExport, &QPushButton::clicked, this, &LC_SettingsPageProfileExchangeBase::onExportProfileClicked);

    btnRow->addWidget(pbImport);
    btnRow->addWidget(pbExport);
    btnRow->addStretch();
    gridLayout->addLayout(btnRow);

    mainLayout->addWidget(gbProfiles);

    if (extraWidget != nullptr) {
        mainLayout->addWidget(extraWidget);
    }

    mainLayout->addStretch();
}

void LC_SettingsPageProfileExchangeBase::onExportProfileClicked() {
    QList<LC_ChecklistChoice> exportChoices = getExportChoices();
    const QString promptMsg = tr("Specify the profile name and select which active components to package:");

    QString profileName;

    // Dialog 1: Combined Profile Name + Component Checklist
    const bool dialogAccepted = LC_DlgChecklistSelection::selectChoicesWithInput(
        getEditingWidget(),
        tr("Export Profile Options"),
        promptMsg,
        tr("Profile Name:"),
        profileDefaultName(),
        profileName,
        exportChoices);

    if (!dialogAccepted || profileName.isEmpty()) {
        return;
    }

    // Auto-generate sanitized default filename from the entered profile name
    QString defaultFilename = profileName.toLower().trimmed();
    defaultFilename.replace(" ", "_");
    defaultFilename.remove(QRegularExpression(R"([^a-z0-9_\-\.])"));
    defaultFilename += profileExtension();

    // Dialog 2: File Chooser pre-populated with sanitized filename
    const QString filePath = QFileDialog::getSaveFileName(
        getEditingWidget(), tr("Export Profile"), defaultFilename, profileFileFilter());
    if (filePath.isEmpty()) {
        return;
    }

    if (executeExport(filePath, profileName, exportChoices)) {
        QMessageBox::information(getEditingWidget(), tr("Profile Exported"),
                                 tr("Profile '%1' has been successfully exported.").arg(profileName));
    }
    else {
        QMessageBox::critical(getEditingWidget(), tr("Export Error"),
                              tr("An error occurred while exporting the profile package."));
    }
}

void LC_SettingsPageProfileExchangeBase::onImportProfileClicked() {
    // Dialog 1: File Chooser
    const QString filePath = QFileDialog::getOpenFileName(
        getEditingWidget(), tr("Import Profile"), QString(), profileFileFilter());
    if (filePath.isEmpty()) {
        return;
    }

    QString profileName;
    QList<LC_ChecklistChoice> availableChoices;
    if (!inspectProfileFile(filePath, profileName, availableChoices)) {
        QMessageBox::critical(getEditingWidget(), tr("Import Error"),
                              tr("Could not read or parse the selected profile package."));
        return;
    }

    const QString promptMsg = tr("Profile '%1' contains the following components. Select which to import and activate:")
                                  .arg(profileName);

    // Dialog 2: Pure Checklist of components present in the file
    if (!LC_DlgChecklistSelection::selectChoices(getEditingWidget(), tr("Import Profile Components"), promptMsg, availableChoices)) {
        return;
    }

    QString importedName;
    if (executeImport(filePath, availableChoices, importedName)) {
        activateImportedProfile();
        QMessageBox::information(getEditingWidget(), tr("Profile Imported"),
                                 tr("Profile '%1' has been successfully imported and activated.").arg(importedName));
    }
    else {
        QMessageBox::critical(getEditingWidget(), tr("Import Error"),
                              tr("An error occurred while importing the profile components."));
    }
}
