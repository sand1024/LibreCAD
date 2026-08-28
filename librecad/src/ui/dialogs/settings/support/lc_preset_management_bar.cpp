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
#include <QMessageBox>

#include "lc_filenameselectionservice.h"
#include "lc_inputtextdialog.h"
#include "lc_palette_editor_shared.h"
#include "lc_preset_manager_interface.h"
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

bool LC_PresetManagementBar::savePresetAs() {
    if (m_manager == nullptr) {
        return false;
    }
    if (m_manager->promptSavePresetAs(this)) {
        bindToManager(m_manager); // Reload choices and select newly created preset
        return true;
    }
    return false;
}

bool LC_PresetManagementBar::obtainFileName(QString& fileName, const bool forRead, const LC_PresetManagerUIStrings& strings) {
    const QString& dir = ""; // fixme - complete file settings!!!
    const QString defFileName = dir + "/shortcuts.lcvs";
    return LC_FileNameSelectionService::doObtainFileName(this, fileName, forRead, "lcvs",
        defFileName, strings.importDialogTitle,
        strings.exportDialogTitle, strings.presetFileFilter);
}

void LC_PresetManagementBar::bindToManager(LC_PresetManagerInterface* manager) {
    m_manager = manager;
    if (m_manager == nullptr) {
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
        if (m_manager != nullptr && m_manager->saveCurrentPreset()) {
            setDirty(false);
        }
    });

    connect(ui->btnSaveAs, &QPushButton::clicked, this, [this]()->void {
        if (savePresetAs()) {
            return;
        }
    });

    connect(ui->btnDelete, &QPushButton::clicked, this, [this, strings]() {
        if (m_manager == nullptr) {
            return;
        }

        const QString name = currentPresetName();
         const auto result = QMessageBox::question(this,
                                             strings.deleteConfirmTitle,
                                             strings.deleteConfirmLabel.arg(name),
                                             QMessageBox::Yes | QMessageBox::No);

         if (result == QMessageBox::Yes) {
             if (m_manager->deletePreset(currentPresetKey())) {
                 bindToManager(m_manager);
             }
         }
    });

    // fixme - sand - use LC_DimStylesExporter::obtainFileName for file!!!

    // Symmetrical File Import: prompts selector and delegates to manager
    connect(ui->btnImport, &QPushButton::clicked, this, [this, strings]() {
        if (m_manager != nullptr) {
            const QString filePath = QFileDialog::getOpenFileName(this, strings.importDialogTitle, QString(), strings.presetFileFilter);
            if (!filePath.isEmpty()) {
                if (m_manager->importPresetFromFile(filePath, this)) {
                    bindToManager(m_manager); // Reload choices dynamically
                }
            }
        }
    });

    // Symmetrical File Export: prompts selector and delegates to manager
    connect(ui->btnExport, &QPushButton::clicked, this, [this, strings]() {
        if (m_manager != nullptr) {
            const QString filePath = QFileDialog::getSaveFileName(this, strings.exportDialogTitle, QString(), strings.presetFileFilter);
            if (!filePath.isEmpty()) {
                m_manager->exportPresetToFile(currentPresetKey(), filePath, this);
            }
        }
    });

    // Wire the Apply button click (only enabled in clean states) [1.1.2]
    connect(ui->btnApply, &QPushButton::clicked, this, [this]() {
        if (m_manager != nullptr) {
            m_manager->applyCurrentPreset();
            bindToManager(m_manager); // Force list refresh
        }
    });

    connect(ui->btnRevert, &QPushButton::clicked, this, [this]() {
       if (m_manager != nullptr) {
           m_manager->rollbackState(); // Triggers reload/reset across the scope
           bindToManager(m_manager);   // Refresh list and button states
       }
   });

    const QList<QPair<QString, QString>> presets = m_manager->getAvailablePresets();
    const QString selectedKey = m_manager->getActivePresetKey();
    const QString appliedKey = m_manager->getAppliedPresetKey();
    populatePresets(presets, selectedKey, appliedKey);

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
    QString name = ui->themeCombo->currentText();
    if (name.endsWith(" *")) {
        name.chop(2);
    }
    return name;
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

void LC_PresetManagementBar::updateButtons() const {
    if (m_manager == nullptr) {
        return;
    }

    const QString key = currentPresetKey();
    const bool isDefault = (key == DEFAULT_THEME_KEY);
    // 1. Visibilities (Configured dynamically based on manager properties) [2.3]

    ui->btnApply->setVisible(m_manager->supportsApply());
    ui->btnImport->setVisible(m_manager->supportsImportExport());
    ui->btnExport->setVisible(m_manager->supportsImportExport());

    // 2. Symmetrical Enabling states based on dirty status and read-only default constraints [1.1.2]
    const bool anyChange = m_isDirty || m_manager->isPresetModified();
    ui->btnSave->setEnabled(anyChange && !isDefault);
    ui->btnDelete->setEnabled(!isDefault);
    ui->btnApply->setEnabled(!anyChange);
    ui->btnRevert->setEnabled(anyChange);

    updateActiveTabText(anyChange);
    updateComboFonts(m_manager->getAppliedPresetKey());
}

void LC_PresetManagementBar::updateActiveTabText(bool modified) const {
    const int currentIdx = ui->themeCombo->currentIndex();
    const int count = ui->themeCombo->count();

    for (int i = 0; i < count; ++i) {
        QString text = ui->themeCombo->itemText(i);
        const bool hasAsterisk = text.endsWith(" *");

        if (i == currentIdx) {
            if (modified && !hasAsterisk) {
                ui->themeCombo->setItemText(i, text + " *");
            }
            else if (!modified && hasAsterisk) {
                text.chop(2);
                ui->themeCombo->setItemText(i, text);
            }
        }
        else if (hasAsterisk) {
            text.chop(2);
            ui->themeCombo->setItemText(i, text);
        }
    }
}

void LC_PresetManagementBar::updateComboFonts(const QString& activeItemKey) const {
    QAbstractItemModel* model = ui->themeCombo->model();
    if (model == nullptr) {
        return;
    }

    const QFont normalFont = ui->themeCombo->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    for (int i = 0; i < ui->themeCombo->count(); ++i) {
        QModelIndex itemIndex = model->index(i, 0);
        const bool isApplied = (ui->themeCombo->itemData(i).toString() == activeItemKey);
        model->setData(itemIndex, isApplied ? boldFont : normalFont, Qt::FontRole);
    }
}

void LC_PresetManagementBar::onComboIndexChanged(int index) {
    if (m_blockSignals || index < 0) {
        return;
    }

    emit presetSelected(ui->themeCombo->itemData(index).toString());
}

void LC_PresetManagementBar::setDirty(bool isDirty) {
    m_isDirty = isDirty;
    updateButtons(); // Centralized state update loop
}
