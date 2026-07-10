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

#include "lc_preset_manager_bar.h"
#include "ui_lc_preset_manager_bar.h"
#include <QAbstractItemModel>
#include <QFont>

#include "lc_editor_utils.h"

LC_PresetManagerBar::LC_PresetManagerBar(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LC_PresetManagerBar) {
    ui->setupUi(this);

    connect(ui->themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_PresetManagerBar::onComboIndexChanged);
    connect(ui->btnSave, &QPushButton::clicked, this, &LC_PresetManagerBar::saveRequested);
    connect(ui->btnSaveAs, &QPushButton::clicked, this, &LC_PresetManagerBar::saveAsRequested);
    connect(ui->btnDelete, &QPushButton::clicked, this, &LC_PresetManagerBar::deleteRequested);
    connect(ui->btnApply, &QPushButton::clicked, this, &LC_PresetManagerBar::applyRequested);
}

LC_PresetManagerBar::~LC_PresetManagerBar() {
    delete ui;
}

void LC_PresetManagerBar::populatePresets(const QList<QPair<QString, QString>>& presets,
                                         const QString& activeKey,
                                         const QString& savedActiveKeyOnDisk) {
    m_blockSignals = true;
    ui->themeCombo->clear();

    // Map localized first elements to text, and stable second elements to itemData
    for (const auto& preset : presets) {
        ui->themeCombo->addItem(preset.first, preset.second);
    }

    // Match index strictly using user-role itemData keys
    int idx = -1;
    for (int i = 0; i < ui->themeCombo->count(); ++i) {
        if (ui->themeCombo->itemData(i).toString() == activeKey) {
            idx = i;
            break;
        }
    }

    if (idx >= 0) {
        ui->themeCombo->setCurrentIndex(idx);
        m_previousIndex = idx;
    } else if (ui->themeCombo->count() > 0) {
        ui->themeCombo->setCurrentIndex(0);
        m_previousIndex = 0;
    }
    m_blockSignals = false;

    // Apply the bold visual helper based on activeItemKey
    updateComboFonts(savedActiveKeyOnDisk);
}


QString LC_PresetManagerBar::currentPresetKey() const {
    return ui->themeCombo->currentData().toString();
}

QString LC_PresetManagerBar::currentPresetName() const {
    return ui->themeCombo->currentText();
}

void LC_PresetManagerBar::setCurrentPresetKey(const QString& key) {
    int idx = -1;
    for (int i = 0; i < ui->themeCombo->count(); ++i) {
        if (ui->themeCombo->itemData(i).toString() == key) {
            idx = i;
            break;
        }
    }
    if (idx >= 0) {
        m_blockSignals = true;
        ui->themeCombo->setCurrentIndex(idx);
        m_previousIndex = idx;
        m_blockSignals = false;
    }
}

void LC_PresetManagerBar::setLabelText(const QString& text) {
    ui->lblSelect->setText(text);
}

void LC_PresetManagerBar::setSaveButtonToolTip(const QString& tooltip) {
    ui->btnSave->setToolTip(tooltip);
}

void LC_PresetManagerBar::setSaveAsButtonToolTip(const QString& tooltip) {
    ui->btnSaveAs->setToolTip(tooltip);
}

void LC_PresetManagerBar::setDeleteButtonToolTip(const QString& tooltip) {
    ui->btnDelete->setToolTip(tooltip);
}

void LC_PresetManagerBar::setApplyButtonToolTip(const QString& tooltip) {
    ui->btnApply->setToolTip(tooltip);
}

void LC_PresetManagerBar::setSaveButtonEnabled(bool enabled) {
    ui->btnSave->setEnabled(enabled);
}

void LC_PresetManagerBar::setDeleteButtonEnabled(bool enabled) {
    ui->btnDelete->setEnabled(enabled);
}

void LC_PresetManagerBar::setApplyButtonEnabled(bool enabled) {
    ui->btnApply->setEnabled(enabled);
}

void LC_PresetManagerBar::updateComboFonts(const QString& activeItemKey) const {
    QAbstractItemModel* model = ui->themeCombo->model();
    if (!model) return;

    QFont normalFont = ui->themeCombo->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    for (int i = 0; i < ui->themeCombo->count(); ++i) {
        QModelIndex itemIndex = model->index(i, 0);
        // Verify key matches using itemData instead of currentText()
        bool isApplied = (ui->themeCombo->itemData(i).toString() == activeItemKey);
        model->setData(itemIndex, isApplied ? boldFont : normalFont, Qt::FontRole);
    }
}

void LC_PresetManagerBar::fillNamesList(QStringList& namesList) const {
    LC_EditorUtils::fillNamesFromCombobox(ui->themeCombo, namesList);
}

void LC_PresetManagerBar::updateComboFonts(const QString& activeItemKey) {
      LC_EditorUtils::updatePresetComboFonts(ui->themeCombo, activeItemKey);
}

void LC_PresetManagerBar::onComboIndexChanged(int index) {
    if (m_blockSignals || index < 0) return;
    // Emit the stable key from itemData to protect downstream code from translations
    emit presetSelected(ui->themeCombo->itemData(index).toString());
}
