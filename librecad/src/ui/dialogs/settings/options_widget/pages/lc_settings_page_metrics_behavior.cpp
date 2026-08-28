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

#include "lc_settings_page_metrics_behavior.h"
#include "ui_lc_settings_page_metrics_behavior.h"
#include "lc_preset_manager_metrics.h"
#include <QCheckBox>
#include <QComboBox>

LC_SettingsPageMetricsBehavior::LC_SettingsPageMetricsBehavior(QObject* parent)
    : LC_SettingsPageBase(tr("Behavior & Cursors"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageMetricsBehavior>()) {
    setSortWeight(50);
}

LC_SettingsPageMetricsBehavior::~LC_SettingsPageMetricsBehavior() = default;

void LC_SettingsPageMetricsBehavior::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMetrics*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerMetrics::configLoaded, this, [this](const StyleMetricsConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageMetricsBehavior::setupUi() {
    ui->setupUi(m_widget);

    ui->cbMnemonicUnderlineMode->addItem(tr("Hold ALT Key"), static_cast<int>(MnemonicUnderlineMode::PressAndHold));
    ui->cbMnemonicUnderlineMode->addItem(tr("Tap ALT to Toggle"), static_cast<int>(MnemonicUnderlineMode::StickyToggle));

    ui->cbDragCursorStyle->addItem(tr("Standard Arrow"), static_cast<int>(DragCursorStyle::StandardArrow));
    ui->cbDragCursorStyle->addItem(tr("Open Hand"), static_cast<int>(DragCursorStyle::OpenHand));
    ui->cbDragCursorStyle->addItem(tr("4-Way Move Arrows"), static_cast<int>(DragCursorStyle::SizeAll));
}

void LC_SettingsPageMetricsBehavior::setupBehavior() {
    const QList<QCheckBox*> checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (QCheckBox* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_SettingsPageMetricsBehavior::onControlChanged);
    }

    const QList<QComboBox*> comboBoxes = m_widget->findChildren<QComboBox*>();
    for (QComboBox* cb : comboBoxes) {
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageMetricsBehavior::onControlChanged);
    }
}

void LC_SettingsPageMetricsBehavior::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageMetricsBehavior::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageMetricsBehavior::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageMetricsBehavior::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageMetricsBehavior::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->chkHideShortcuts->setChecked(config.hideShortcutUnderlines);
    const int mnemonicIdx = ui->cbMnemonicUnderlineMode->findData(static_cast<int>(config.mnemonicUnderlineMode));
    if (mnemonicIdx >= 0) ui->cbMnemonicUnderlineMode->setCurrentIndex(mnemonicIdx);

    ui->chkFlatDisabledText->setChecked(config.flatDisabledText);
    ui->chkMenuIcons->setChecked(config.menuSupportsIcons);
    ui->chkMenuDisabledActive->setChecked(config.menuAllowActiveAndDisabled);

    const int cursorIdx = ui->cbDragCursorStyle->findData(static_cast<int>(config.dragCursorStyle));
    ui->cbDragCursorStyle->setCurrentIndex(cursorIdx != -1 ? cursorIdx : 2);

    m_blockSignals = false;
}

void LC_SettingsPageMetricsBehavior::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.hideShortcutUnderlines = ui->chkHideShortcuts->isChecked();
    config.mnemonicUnderlineMode = static_cast<MnemonicUnderlineMode>(ui->cbMnemonicUnderlineMode->currentData().toInt());
    config.flatDisabledText = ui->chkFlatDisabledText->isChecked();
    config.menuSupportsIcons = ui->chkMenuIcons->isChecked();
    config.menuAllowActiveAndDisabled = ui->chkMenuDisabledActive->isChecked();
    config.dragCursorStyle = static_cast<DragCursorStyle>(ui->cbDragCursorStyle->currentData().toInt());
}
