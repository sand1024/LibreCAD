/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_dlgquickselection.h"

#include "lc_entity_meta_utils.h"
#include "rs_selection.h"
#include "ui_lc_dlgquickselection.h"

LC_DlgQuickSelection::LC_DlgQuickSelection(QWidget* parent, RS_Document* container, LC_GraphicViewport* viewport)
    : LC_Dialog(parent, "QuickSelection"), ui(new Ui::LC_DlgQuickSelection),
    m_document{container}, m_viewport{viewport}{
    ui->setupUi(this);

    auto selection = RS_Selection(m_document, m_viewport);
    selection.collectCurrentSelectionState(m_selectionState);

    bool hasSelection = m_selectionState.hasSelection();

    ui->cbApplyTo->addItem(tr("Entire Drawing"));
    if (hasSelection) {
        ui->cbApplyTo->addItem(tr("Current Selection"));
    }

    connect(ui->cbApplyTo, &QComboBox::currentIndexChanged, this, &LC_DlgQuickSelection::onApplyToCurrentIndexChanged);
    connect(ui->cbEntityType, &QComboBox::currentIndexChanged, this, &LC_DlgQuickSelection::onEntityTypeIndexChanged);
    connect(ui->lvProperties, &QListWidget::currentRowChanged, this, &LC_DlgQuickSelection::onPropertyListRowChanged);
    connect(ui->tbManualSelect, &QToolButton::clicked, this, &LC_DlgQuickSelection::onManualSelectionClicked);

    onApplyToCurrentIndexChanged(0);
    onEntityTypeIndexChanged(0);
}

LC_DlgQuickSelection::~LC_DlgQuickSelection() {
    delete ui;
}


void LC_DlgQuickSelection::onApplyToCurrentIndexChanged(int index) const {
    ui->cbEntityType->blockSignals(true);
    ui->cbEntityType->clear();
    ui->cbEntityType->addItem(tr("Multiple"), RS2::EntityType::EntityUnknown);
    if (index == 0) {
        setupEntitiesTypesList(m_selectionState.documentEntityTypes);
    }
    else {
        setupEntitiesTypesList(m_selectionState.selectedEntityTypes);
    }
    ui->cbEntityType->blockSignals(false);
    ui->cbEntityType->setCurrentIndex(0);
}

void LC_DlgQuickSelection::onEntityTypeIndexChanged(int index) {
}

void LC_DlgQuickSelection::onPropertyListRowChanged(int currentRow) {

}

void LC_DlgQuickSelection::onManualSelectionClicked(bool checked) {
}

void LC_DlgQuickSelection::setupEntitiesTypesList(const QSet<RS2::EntityType>& set) const {
    auto entityTypeCombobox = ui->cbEntityType;
    LC_EntityMetaUtils::setupEntitiesTypesList(entityTypeCombobox, set);
}
