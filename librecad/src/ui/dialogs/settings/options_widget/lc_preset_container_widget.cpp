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

#include "lc_preset_container_widget.h"
#include "ui_lc_preset_container_widget.h"
#include "lc_preset_manager_bar.h"
#include "lc_style_editor_interface.h"
#include "lc_ui_style_manager.h"
#include "lc_skin_preview_window.h"
#include "lc_inputtextdialog.h"
#include <QMessageBox>
#include <QTabWidget>
#include <QTimer>

#include "lc_editor_utils.h"
#include "lc_palette_color_utils.h"

LC_PresetContainerWidget::LC_PresetContainerWidget(QWidget* parent,
                                                   LC_UIStyleManager* styleManager,
                                                   LC_StyleEditorInterface* editor)
    : QWidget(parent)
    , ui(new Ui::LC_PresetContainerWidget)
    , m_styleManager(styleManager)
    , m_editor(editor) {
    ui->setupUi(this);

    // 1. Programmatically instantiate and nest the preset bar into the top placeholder layout
    m_presetBar = new LC_PresetManagerBar(this);
    ui->presetBarLayout->addWidget(m_presetBar);

    // 2. Programmatically nest the active custom editor panel into the central placeholder layout
    QWidget* editorWidget = m_editor->getWidget();
    ui->editorLayout->addWidget(editorWidget);

    // 3. Connect Preset Bar signals
    connect(m_presetBar, &LC_PresetManagerBar::presetSelected, this, &LC_PresetContainerWidget::onPresetSelected);
    connect(m_presetBar, &LC_PresetManagerBar::saveRequested, this, &LC_PresetContainerWidget::onSaveRequested);
    connect(m_presetBar, &LC_PresetManagerBar::saveAsRequested, this, &LC_PresetContainerWidget::onSaveAsRequested);
    connect(m_presetBar, &LC_PresetManagerBar::deleteRequested, this, &LC_PresetContainerWidget::onDeleteRequested);
    connect(m_presetBar, &LC_PresetManagerBar::applyRequested, this, &LC_PresetContainerWidget::onApplyRequested);

    // 4. Connect centralized visual preview controls
    connect(ui->chkShowPreview, &QCheckBox::toggled, this, &LC_PresetContainerWidget::onShowPreviewToggled);
    connect(ui->chkDisablePreview, &QCheckBox::toggled, this, &LC_PresetContainerWidget::onDisablePreviewToggled);
    connect(ui->cbCvdSimulator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_PresetContainerWidget::onCvdSimulatorChanged);

    // 5. Connect standard database preset actions
    connect(ui->btnRevert, &QPushButton::clicked, this, &LC_PresetContainerWidget::onRevertClicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &LC_PresetContainerWidget::onResetClicked);

    // 6. Bind the editor's change callback to track dirty states reactively
    m_editor->setChangedCallback([this](bool dirty) {
        setDirty(dirty);
        updateLivePreview();
    });

    // 7. Populate the CVD Accessibility Simulator combobox
    ui->cbCvdSimulator->blockSignals(true);
    ui->cbCvdSimulator->addItem(tr("Normal Vision"), static_cast<int>(LC_PaletteColorUtils::CVDType::Normal));
    ui->cbCvdSimulator->addItem(tr("Protanopia (Red-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Protanopia));
    ui->cbCvdSimulator->addItem(tr("Deuteranopia (Green-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Deuteranopia));
    ui->cbCvdSimulator->addItem(tr("Tritanopia (Blue-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Tritanopia));
    ui->cbCvdSimulator->blockSignals(false);

    // 8. Fine-Grained Capability Handshake: configure individual visibilities on-the-fly
    bool supportsPreview = m_editor->supportsLivePreview();
    bool supportsCvd     = m_editor->supportsAccessibilityCheck();

    ui->chkShowPreview->setVisible(supportsPreview);
    ui->chkDisablePreview->setVisible(supportsPreview);
    ui->lblCvd->setVisible(supportsCvd);
    ui->cbCvdSimulator->setVisible(supportsCvd);

    // Hide the group box entirely only if BOTH are unsupported (e.g. non-color, non-preview editors)
    ui->previewControlGroup->setVisible(supportsPreview || supportsCvd);

    if (m_editor->supportsLivePreview() && ui->chkShowPreview->isChecked()) {
        // We wait 30ms to let the main dialog geometry and positioning settle on screen
        QTimer::singleShot(30, this, [this]() {
            setPreviewVisible(true);
        });
    }
}

LC_PresetContainerWidget::~LC_PresetContainerWidget() {
    if (m_previewWindow) {
        m_previewWindow->close();
        m_previewWindow->deleteLater();
    }
    delete ui;
}

void LC_PresetContainerWidget::initialize(const QString& initialKey) {
    m_editor->onInitialize();
    m_originalActiveKey = m_editor->getActivePresetKey();
    m_currentKey = (!initialKey.isEmpty()) ? initialKey : m_originalActiveKey;

    m_editor->loadPreset(m_currentKey);
    setDirty(false);
    populatePresetBar();
}

void LC_PresetContainerWidget::populatePresetBar() {
    m_blockSignals = true;
    QList<QPair<QString, QString>> choices = m_editor->getAvailablePresets();
    m_presetBar->populatePresets(choices, m_currentKey, m_originalActiveKey);
    m_blockSignals = false;
    updatePresetBarStates();
}

void LC_PresetContainerWidget::updatePresetBarStates() {
    m_blockSignals = true;
    m_presetBar->setCurrentPresetKey(m_currentKey);
    m_presetBar->updateComboFonts(m_originalActiveKey); // Highlight active-on-disk preset in bold
    m_blockSignals = false;

    bool isDefault = (m_currentKey == DEFAULT_THEME_KEY);
    bool isActive  = (m_currentKey == m_originalActiveKey);
    m_presetBar->setDeleteButtonEnabled(!isDefault && !isActive);
    m_presetBar->setSaveButtonEnabled(m_isDirty && !isDefault);
    m_presetBar->setApplyButtonEnabled(!m_isDirty);

    ui->btnRevert->setEnabled(m_isDirty);
    ui->btnReset->setEnabled(!isDefault);

    emit activePresetChanged(m_presetBar->currentPresetName());
    updateLivePreview();
}


void LC_PresetContainerWidget::onPresetSelected(const QString& key) {
    if (m_blockSignals) return;

    if (m_isDirty) {
        auto reply = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("You have unsaved changes on the current preset. Do you want to discard them?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            m_presetBar->setCurrentPresetKey(m_currentKey);
            return;
        }
    }

    m_currentKey = key;
    m_editor->loadPreset(key);
    setDirty(false);
    updatePresetBarStates();
}

void LC_PresetContainerWidget::onSaveRequested() {
    if (m_editor->saveCurrentPreset()) {
        setDirty(false);
    }
}

void LC_PresetContainerWidget::onSaveAsRequested() {
    QStringList existingPresetNames;
    m_presetBar->fillNamesList(existingPresetNames);

    bool ok;

    QString name = LC_InputTextDialog::getText(this, tr("Save Preset"), tr("Enter unique preset name:"), existingPresetNames, true, "",
                                               &ok);
    if (ok && !name.trimmed().isEmpty()) {
        QString cleanName = name.trimmed();
        QString newKey;
        // Delegate save-as to repository to sanitize the filename and return the new stable key
        if (m_editor->savePresetAs(cleanName, newKey)) {
            m_currentKey = newKey; // Select the newly generated file key instantly
            setDirty(false);
            populatePresetBar();
        }
    }
}

void LC_PresetContainerWidget::onDeleteRequested() {
    if (m_currentKey == m_originalActiveKey) {
        QMessageBox::warning(this, tr("Delete Blocked"),
            tr("The selected preset is currently active and applied globally. "
               "Please apply a different preset before attempting to delete this one."));
        return;
    }
    auto reply = QMessageBox::question(this, tr("Confirm Deletion"),
        tr("Are you sure you want to permanently delete this preset from disk?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_editor->deletePreset(m_currentKey)) {
            m_currentKey = DEFAULT_THEME_KEY;
            m_editor->loadPreset(m_currentKey);
            setDirty(false);
            populatePresetBar();
        }
    }
}

void LC_PresetContainerWidget::onApplyRequested() {
    applyCurrentPreset();
}

void LC_PresetContainerWidget::onRevertClicked() {
    auto reply = QMessageBox::question(this, tr("Confirm Revert"),
        tr("Discard all active modifications and reload the preset as saved on disk?"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_editor->loadPreset(m_currentKey);
        setDirty(false);
        updatePresetBarStates();
    }
}

void LC_PresetContainerWidget::onResetClicked() {
    auto reply = QMessageBox::question(this, tr("Confirm Reset"),
        tr("Wipe the active configuration and restore standard factory defaults?"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_editor->loadPreset(DEFAULT_THEME_KEY);
        setDirty(true);
        updatePresetBarStates();
    }
}

void LC_PresetContainerWidget::onShowPreviewToggled(bool checked) {
    setPreviewVisible(checked);
}

void LC_PresetContainerWidget::onDisablePreviewToggled(bool checked) {
    if (m_previewWindow) {
        const auto tabs = m_previewWindow->findChild<QTabWidget*>();
        if (tabs) {
            tabs->setEnabled(!checked);
        }
    }
}

void LC_PresetContainerWidget::onCvdSimulatorChanged(int index) {
    Q_UNUSED(index);
    updateLivePreview();
}

void LC_PresetContainerWidget::applyCurrentPreset() {
    m_editor->applyCurrentPreset();
    m_originalActiveKey = m_currentKey;
    setDirty(false);
    updatePresetBarStates();
}

void LC_PresetContainerWidget::rollback() {
    m_editor->rollbackState();
    m_currentKey = m_originalActiveKey;
    m_editor->loadPreset(m_currentKey);
    setDirty(false);
    updatePresetBarStates();
}

void LC_PresetContainerWidget::setDirty(bool dirty) {
    m_isDirty = dirty;
    m_presetBar->setSaveButtonEnabled(m_isDirty && m_currentKey != DEFAULT_THEME_KEY);
    m_presetBar->setApplyButtonEnabled(!m_isDirty);
    ui->btnRevert->setEnabled(m_isDirty);
    emit dirtinessChanged(m_isDirty);
}

void LC_PresetContainerWidget::setPreviewVisible(bool visible) {
    if (!m_editor->supportsLivePreview()) return;

    if (visible) {
        if (!m_previewWindow) {
            m_previewWindow = new LC_SkinPreviewWindow(this);
            m_previewWindow->move(this->window()->geometry().right() + 10, this->window()->geometry().top());
        }
        m_previewWindow->show();
        updateLivePreview();
    } else if (m_previewWindow) {
        m_previewWindow->hide();
    }
}

bool LC_PresetContainerWidget::isPreviewVisible() const {
    return m_previewWindow && m_previewWindow->isVisible();
}

LC_PaletteColorUtils::CVDType LC_PresetContainerWidget::activeCvdType() const {
    int cvdIdx = ui->cbCvdSimulator->currentIndex();
    if (cvdIdx >= 0) {
        return static_cast<LC_PaletteColorUtils::CVDType>(ui->cbCvdSimulator->itemData(cvdIdx).toInt());
    }
    return LC_PaletteColorUtils::CVDType::Normal;
}

void LC_PresetContainerWidget::updateLivePreview() const {
    if (m_editor->supportsLivePreview()) {
        if (m_previewWindow && m_previewWindow->isVisible()) {
            m_editor->applyTransientState(m_previewWindow);
        }
    } else {
        m_editor->applyTransientState(nullptr);
        parentWidget()->update();
    }
}
