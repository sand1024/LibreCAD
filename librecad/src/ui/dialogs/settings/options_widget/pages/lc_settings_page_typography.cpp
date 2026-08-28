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

#include "lc_settings_page_typography.h"
#include "ui_lc_settings_page_typography.h"
#include "lc_preset_manager_typography.h"
#include <QCheckBox>
#include <QFontComboBox>
#include <QSpinBox>

LC_SettingsPageTypography::LC_SettingsPageTypography(QObject* parent)
    : LC_SettingsPageBase(tr("Typography"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageTypography>()) {
    setSortWeight(30);
}

void LC_SettingsPageTypography::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerTypography*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerTypography::configLoaded, this, [this](const FontConfig&) {
            populateUiFromConfig();
        });
        populateUiFromConfig();
    }
}

void LC_SettingsPageTypography::setupUi() {
    ui->setupUi(m_widget);
    ui->fcTechnicalFont->setFontFilters(QFontComboBox::MonospacedFonts);
}

void LC_SettingsPageTypography::setupBehavior() {
    const QList<QFontComboBox*> fontCombos = m_widget->findChildren<QFontComboBox*>();
    for (QFontComboBox* fc : fontCombos) {
        connect(fc, &QFontComboBox::currentFontChanged, this, &LC_SettingsPageTypography::onControlChanged);
    }

    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageTypography::onControlChanged);
    }

    const QList<QCheckBox*> checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (QCheckBox* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_SettingsPageTypography::onControlChanged);
    }
}

void LC_SettingsPageTypography::loadSettings() {
    populateUiFromConfig();
}

bool LC_SettingsPageTypography::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageTypography::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageTypography::updateLivePreview() {
    if (m_presetManager != nullptr) {
        syncUiToWorkingConfig();
    }
    emit livePreviewRequested();
}

void LC_SettingsPageTypography::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
}

void LC_SettingsPageTypography::populateUiFromConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) {
        return;
    }

    m_blockSignals = true;
    const FontConfig& config = m_presetManager->workingConfig();

    ui->fcMainFont->setCurrentFont(QFont(config.mainFamily));
    ui->sbMainSize->setValue(config.mainSize);

    ui->sbHeadingOffset->setValue(config.headings.sizeOffset);
    ui->chkHeadingBold->setChecked(config.headings.bold);
    ui->chkHeadingItalic->setChecked(config.headings.italic);

    ui->sbMenuBarOffset->setValue(config.menuBar.sizeOffset);
    ui->chkMenuBarBold->setChecked(config.menuBar.bold);
    ui->chkMenuBarItalic->setChecked(config.menuBar.italic);

    ui->sbMenuOffset->setValue(config.menus.sizeOffset);
    ui->chkMenuBold->setChecked(config.menus.bold);
    ui->chkMenuItalic->setChecked(config.menus.italic);

    ui->sbButtonOffset->setValue(config.buttons.sizeOffset);
    ui->chkButtonBold->setChecked(config.buttons.bold);
    ui->chkButtonItalic->setChecked(config.buttons.italic);

    ui->sbInputOffset->setValue(config.inputs.sizeOffset);
    ui->chkInputBold->setChecked(config.inputs.bold);
    ui->chkInputItalic->setChecked(config.inputs.italic);

    ui->sbGenericDockOffset->setValue(config.genericDockTitle.sizeOffset);
    ui->chkGenericDockBold->setChecked(config.genericDockTitle.bold);
    ui->chkGenericDockItalic->setChecked(config.genericDockTitle.italic);

    ui->sbSpecialDockOffset->setValue(config.specialDockTitle.sizeOffset);
    ui->chkSpecialDockBold->setChecked(config.specialDockTitle.bold);
    ui->chkSpecialDockItalic->setChecked(config.specialDockTitle.italic);

    ui->fcTechnicalFont->setCurrentFont(QFont(config.techFamily));
    ui->sbTechnicalOffset->setValue(config.technical.sizeOffset);
    ui->chkTechnicalBold->setChecked(config.technical.bold);
    ui->chkTechnicalItalic->setChecked(config.technical.italic);

    m_blockSignals = false;
}

void LC_SettingsPageTypography::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) {
        return;
    }

    FontConfig& config = m_presetManager->workingConfig();

    config.mainFamily = ui->fcMainFont->currentFont().family();
    config.mainSize   = ui->sbMainSize->value();

    config.headings.sizeOffset = ui->sbHeadingOffset->value();
    config.headings.bold       = ui->chkHeadingBold->isChecked();
    config.headings.italic     = ui->chkHeadingItalic->isChecked();

    config.menuBar.sizeOffset = ui->sbMenuBarOffset->value();
    config.menuBar.bold       = ui->chkMenuBarBold->isChecked();
    config.menuBar.italic     = ui->chkMenuBarItalic->isChecked();

    config.menus.sizeOffset = ui->sbMenuOffset->value();
    config.menus.bold       = ui->chkMenuBold->isChecked();
    config.menus.italic     = ui->chkMenuItalic->isChecked();

    config.buttons.sizeOffset = ui->sbButtonOffset->value();
    config.buttons.bold       = ui->chkButtonBold->isChecked();
    config.buttons.italic     = ui->chkButtonItalic->isChecked();

    config.inputs.sizeOffset = ui->sbInputOffset->value();
    config.inputs.bold       = ui->chkInputBold->isChecked();
    config.inputs.italic     = ui->chkInputItalic->isChecked();

    config.genericDockTitle.sizeOffset = ui->sbGenericDockOffset->value();
    config.genericDockTitle.bold       = ui->chkGenericDockBold->isChecked();
    config.genericDockTitle.italic     = ui->chkGenericDockItalic->isChecked();

    config.specialDockTitle.sizeOffset = ui->sbSpecialDockOffset->value();
    config.specialDockTitle.bold       = ui->chkSpecialDockBold->isChecked();
    config.specialDockTitle.italic     = ui->chkSpecialDockItalic->isChecked();

    config.techFamily            = ui->fcTechnicalFont->currentFont().family();
    config.technical.sizeOffset  = ui->sbTechnicalOffset->value();
    config.technical.bold        = ui->chkTechnicalBold->isChecked();
    config.technical.italic      = ui->chkTechnicalItalic->isChecked();
}
