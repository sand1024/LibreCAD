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


#include "lc_preset_management_bar.h"
#include <QAbstractItemModel>
#include <QFileDialog>
#include <QInputDialog>

#include "lc_palette_editor_shared.h"
#include "ui_lc_preset_management_bar.h"

LC_PresetManagementBar::LC_PresetManagementBar(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LC_PresetManagementBar) {
    ui->setupUi(this);

    connect(ui->themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_PresetManagementBar::onComboIndexChanged);
}

LC_PresetManagementBar::~LC_PresetManagementBar() {
    delete ui;
}

void LC_PresetManagementBar::bindToManager(LC_PresetManagerInterface* manager) {
    m_manager = manager;
    if (!m_manager) {
        setVisible(false);
        return;
    }

    // 1. Customize UI labels and tooltips dynamically based on the active manager
    LC_PresetManagerUIStrings strings = m_manager->presetStrings();
    ui->lblSelect->setText(strings.labelText);
    ui->themeCombo->setToolTip(strings.selectToolTip);
    ui->btnSave->setToolTip(strings.saveToolTip);
    ui->btnSaveAs->setToolTip(strings.saveAsToolTip);
    ui->btnDelete->setToolTip(strings.deleteToolTip);
    ui->btnApply->setToolTip(strings.applyToolTip);
    ui->btnImport->setToolTip(strings.importToolTip);
    ui->btnExport->setToolTip(strings.exportToolTip);
    ui->btnRevert->setToolTip(strings.revertToolTip);

    ui->btnApply->setVisible(m_manager->supportsApply());
    ui->btnImport->setVisible(m_manager->supportsImportExport());
    ui->btnExport->setVisible(m_manager->supportsImportExport());

    disconnect(ui->btnSave, nullptr, nullptr, nullptr);
    disconnect(ui->btnSaveAs, nullptr, nullptr, nullptr);
    disconnect(ui->btnDelete, nullptr, nullptr, nullptr);
    disconnect(ui->btnImport, nullptr, nullptr, nullptr);
    disconnect(ui->btnExport, nullptr, nullptr, nullptr);
    disconnect(ui->btnApply, nullptr, nullptr, nullptr);

    connect(ui->btnSave, &QPushButton::clicked, this, [this]() {
        if (m_manager && m_manager->saveCurrentPreset()) {
            setDirty(false);
        }
    });

    // fixme - sand - rework and move out of there?
    connect(ui->btnSaveAs, &QPushButton::clicked, this, [this]() {
        if (m_manager) {
            bool ok;
            // QString name = LC_InputTextDialog::getText(this, tr("Save Preset"), tr("Enter unique preset name:"), existingPresetNames, true, "",
            //                                   &ok);
            QString name = QInputDialog::getText(this, tr("Save Preset As"),
                                                 tr("Enter unique preset name:"),
                                                 QLineEdit::Normal, QString(), &ok);
            if (ok && !name.trimmed().isEmpty()) {
            QString newKey;
                if (m_manager->savePresetAs(name.trimmed(), newKey)) {
                bindToManager(m_manager);
            }
        }
        }
    });

    connect(ui->btnDelete, &QPushButton::clicked, this, [this]() {
        if (m_manager && m_manager->deletePreset(currentPresetKey())) {
            bindToManager(m_manager);
        }
    });

    // Symmetrical File Import: prompts selector and delegates to manager [3.19]
    connect(ui->btnImport, &QPushButton::clicked, this, [this]() {
        if (m_manager) {
            QString filePath = QFileDialog::getOpenFileName(this, tr("Import Preset"), QString(), tr("Preset Files (*.theme.json *.json)"));
            if (!filePath.isEmpty()) {
                if (m_manager->importPresetFromFile(filePath, this)) {
                    bindToManager(m_manager); // Reload choices dynamically
                }
            }
        }
    });

    // Symmetrical File Export: prompts selector and delegates to manager [3.19]
    connect(ui->btnExport, &QPushButton::clicked, this, [this]() {
        if (m_manager) {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Export Preset"), QString(), tr("Preset Files (*.theme.json *.json)"));
            if (!filePath.isEmpty()) {
                m_manager->exportPresetToFile(currentPresetKey(), filePath, this);
            }
        }
    });

    // Wire the Apply button click (only enabled in clean states) [1.1.2]
    connect(ui->btnApply, &QPushButton::clicked, this, [this]() {
        if (m_manager) {
            m_manager->applyCurrentPreset();
            bindToManager(m_manager); // Force list refresh
        }
    });

    connect(ui->btnRevert, &QPushButton::clicked, this, [this]() {
       if (m_manager) {
           m_manager->rollbackState(); // Triggers reload/reset across the scope
           bindToManager(m_manager);   // Refresh list and button states
       }
   });

    QList<QPair<QString, QString>> presets = m_manager->getAvailablePresets();
    QString activeKey = m_manager->getActivePresetKey();
    populatePresets(presets, activeKey, activeKey);

    // 5. Setup dirty-state tracking callback
    m_manager->setChangedCallback([this](bool isDirty) {
        setDirty(isDirty);
    });

    updateButtons();
    setVisible(true);
}

void LC_PresetManagementBar::populatePresets(const QList<QPair<QString, QString>>& presets,
                                             const QString& activeKey,
                                             const QString& savedActiveKeyOnDisk) {
    m_blockSignals = true;
    ui->themeCombo->clear();

    for (const auto& preset : presets) {
        ui->themeCombo->addItem(preset.first, preset.second);
    }

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

    updateButtons();
    updateComboFonts(savedActiveKeyOnDisk);
}

QString LC_PresetManagementBar::currentPresetKey() const {
    return ui->themeCombo->currentData().toString();
}

QString LC_PresetManagementBar::currentPresetName() const {
    return ui->themeCombo->currentText();
}

void LC_PresetManagementBar::setCurrentPresetKey(const QString& key) {
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

void LC_PresetManagementBar::updateButtons() {
    if (!m_manager) return;

    QString key = currentPresetKey();
    bool isDefault = (key == DEFAULT_THEME_KEY);

    // 1. Visibilities (Configured dynamically based on manager properties) [2.3]
    ui->btnApply->setVisible(m_manager->supportsApply());
    ui->btnImport->setVisible(m_manager->supportsImportExport());
    ui->btnExport->setVisible(m_manager->supportsImportExport());

    // 2. Symmetrical Enabling states based on dirty status and read-only default constraints [1.1.2]
    ui->btnSave->setEnabled(m_isDirty && !isDefault);
    ui->btnDelete->setEnabled(!isDefault);
    ui->btnApply->setEnabled(!m_isDirty); // Enabled only if not modified

    ui->btnRevert->setEnabled(m_isDirty);
}

void LC_PresetManagementBar::updateComboFonts(const QString& activeItemKey) {
    QAbstractItemModel* model = ui->themeCombo->model();
    if (!model) {
        return;
    }

    QFont normalFont = ui->themeCombo->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    for (int i = 0; i < ui->themeCombo->count(); ++i) {
        QModelIndex itemIndex = model->index(i, 0);
        bool isApplied = (ui->themeCombo->itemData(i).toString() == activeItemKey);
        model->setData(itemIndex, isApplied ? boldFont : normalFont, Qt::FontRole);
    }
}

void LC_PresetManagementBar::onComboIndexChanged(int index) {
    if (m_blockSignals || index < 0) {
        return;
    }

    // Emit the stable non-localized key (itemData) to protect downstream logic
    emit presetSelected(ui->themeCombo->itemData(index).toString());
}

void LC_PresetManagementBar::setDirty(bool isDirty) {
    m_isDirty = isDirty;
    updateButtons(); // Centralized state update loop
}
