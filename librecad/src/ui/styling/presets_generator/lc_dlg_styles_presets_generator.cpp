/*******************************************************************************
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

#include "lc_dlg_styles_presets_generator.h"

#include <QFileDialog>
#include <QMessageBox>

#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_style_preset_generator.h"
#include "lc_ui_style_manager.h"

#include "ui_lc_dlg_styles_presets_generator.h"

LC_DlgStylesPresetsGenerator::LC_DlgStylesPresetsGenerator(QWidget* parent, LC_UIStyleManager* styleManager) :LC_Dialog(parent, "Styling Preset Generator")
    , ui(new Ui::LC_DlgStylesPresetsGenerator)
    , m_styleManager{styleManager}{
    ui->setupUi(this);

    connect(ui->pbSelectIconStylesDir, &QToolButton::clicked, [this](bool)-> void {
        const QString folder = selectFolder(tr("Select Icon Styles Directory"));
        if (folder != nullptr) {
            ui->leStylesDir->setText(QDir::toNativeSeparators(folder));
        }
    });

    connect(ui->pbSelectSkinsDir, &QToolButton::clicked, [this](bool)-> void {
       const QString folder = selectFolder(tr("Select Fusion Skins Directory"));
       if (folder != nullptr) {
           ui->leSkinsDir->setText(QDir::toNativeSeparators(folder));
       }
   });

    ui->leStylesDir->setText(m_styleManager->getIconsStyleRepository()->configDir());
    ui->leSkinsDir->setText(m_styleManager->getSkinsRepository()->configDir());

    connect(ui->pbGenerateSkins, &QPushButton::clicked, this, &LC_DlgStylesPresetsGenerator::generateSkins);
    connect(ui->pbGenerateStyles, &QPushButton::clicked, this, &LC_DlgStylesPresetsGenerator::generateIconStyles);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &LC_DlgStylesPresetsGenerator::accept);

    connect(ui->chIconStyleShortBases, &QCheckBox::toggled, [this](bool toggled)->void {
        int maxVariants = toggled ? 72 : 270;
        ui->sbStylesCount->setMaximum(maxVariants);
    });
}

QString LC_DlgStylesPresetsGenerator::selectFolder(const QString &title){
    QString folder = nullptr;
    QFileDialog dlg(this);
    if (title != nullptr) {
        const QString dlgTitle = title;
        dlg.setWindowTitle(dlgTitle);
    }
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly);

    if (dlg.exec() != 0) {
        folder = dlg.selectedFiles().at(0);
    }
    return folder;
}

void LC_DlgStylesPresetsGenerator::generateSkins(bool checked) {
    LC_StylePresetGenerator generator(ui->leSkinsDir->text(), ui->leStylesDir->text());
    int generatedSkins = generator.generateSkins(ui->sbSkinsCount->value());
    QMessageBox::information(this, "Skins Generator", QString("Skins were generated. Amount: %1.").arg(generatedSkins), QMessageBox::Ok);
    m_styleManager->getIconsStyleRepository()->initializeIndex();
}

void LC_DlgStylesPresetsGenerator::generateIconStyles(bool checked) {
    LC_StylePresetGenerator generator(ui->leSkinsDir->text(), ui->leStylesDir->text());
    int generatedIconStyles = generator.generateIconStyles(ui->sbStylesCount->value(), ui->chIconStyleShortBases->isChecked());
    QMessageBox::information(this, "Styles Generator", QString("Icon styles were generated. Amount: %1.").arg(generatedIconStyles), QMessageBox::Ok);
}

LC_DlgStylesPresetsGenerator::~LC_DlgStylesPresetsGenerator() {
    delete ui;
}
