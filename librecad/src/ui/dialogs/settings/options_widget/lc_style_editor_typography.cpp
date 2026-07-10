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

#include "lc_style_editor_typography.h"
#include "ui_lc_style_editor_typography.h"
#include "support/lc_font_settings_widget.h"
#include "lc_ui_style_manager.h"
#include "lc_typography_repository.h"
#include "lc_typography_utils.h"

LC_StyleEditorTypography::LC_StyleEditorTypography(QWidget* parent, LC_UIStyleManager* styleManager)
    : LC_StyleEditorBase(parent, styleManager, styleManager->getTypographyRepository())
    , ui(new Ui::LC_StyleEditorTypography) {
    ui->setupUi(this);

    // Enforce monospaced filtering on the technical font drop-down
    ui->fcTechnicalFont->setFontFilters(QFontComboBox::MonospacedFonts);

    setupConnections();
}

LC_StyleEditorTypography::~LC_StyleEditorTypography() {
    delete ui;
}

void LC_StyleEditorTypography::setupConnections() {
    // Reflective Wiring: programmatically bind all input fields recursively
    const QList<QFontComboBox*> fontCombos = findChildren<QFontComboBox*>();
    for (QFontComboBox* fc : fontCombos) {
        connect(fc, &QFontComboBox::currentFontChanged, this, &LC_StyleEditorTypography::onControlChanged);
    }

    const QList<QSpinBox*> spinBoxes = findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorTypography::onControlChanged);
    }

    const QList<QCheckBox*> checkBoxes = findChildren<QCheckBox*>();
    for (QCheckBox* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_StyleEditorTypography::onControlChanged);
    }
}

void LC_StyleEditorTypography::onControlChanged() {
    if (m_blockSignals) return;
    notifyChanged(true);
}

bool LC_StyleEditorTypography::doLoadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY) {
        LC_TypographyUtils::initializeDefaultConfig(m_currentConfig);
    } else {
        m_styleManager->getTypographyRepository()->loadByKey(key, m_currentConfig);
    }

    m_blockSignals = true;
    loadConfigToUi(m_currentConfig);
    m_blockSignals = false;

    return true;
}

bool LC_StyleEditorTypography::doSavePresetAs(const QString& name, QString& key) {
    m_currentConfig = getConfigFromUi();
    m_currentConfig.name = name;
    return m_styleManager->getTypographyRepository()->save(name, m_currentConfig, key);
}


bool LC_StyleEditorTypography::saveCurrentPreset() {
    m_currentConfig = getConfigFromUi();
    QString outKey;
    return m_styleManager->getTypographyRepository()->save(m_currentConfig.name, m_currentConfig, outKey);
}

QString LC_StyleEditorTypography::getActivePresetKey() const {
    return m_styleManager->getActiveTypography();
}

QList<QPair<QString, QString>> LC_StyleEditorTypography::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Typography"), DEFAULT_THEME_KEY));
    choices.append(m_styleManager->getTypographyRepository()->getPresetChoices());
    return choices;
}

void LC_StyleEditorTypography::applyTransientState(QWidget* previewWindow) const {
    FontConfig tempFont = getConfigFromUi();

    // Compile local font properties of the transient edits
    const QFont mainFont(tempFont.mainFamily, tempFont.mainSize);

    QFont headingFont(tempFont.mainFamily, tempFont.mainSize + tempFont.headings.sizeOffset);
    headingFont.setBold(tempFont.headings.bold);
    headingFont.setItalic(tempFont.headings.italic);

    QFont menuBarFont(tempFont.mainFamily, tempFont.mainSize + tempFont.menuBar.sizeOffset);
    menuBarFont.setBold(tempFont.menuBar.bold);
    menuBarFont.setItalic(tempFont.menuBar.italic);

    QFont menuFont(tempFont.mainFamily, tempFont.mainSize + tempFont.menus.sizeOffset);
    menuFont.setBold(tempFont.menus.bold);
    menuFont.setItalic(tempFont.menus.italic);

    QFont buttonFont(tempFont.mainFamily, tempFont.mainSize + tempFont.buttons.sizeOffset);
    buttonFont.setBold(tempFont.buttons.bold);
    buttonFont.setItalic(tempFont.buttons.italic);

    QFont inputFont(tempFont.mainFamily, tempFont.mainSize + tempFont.inputs.sizeOffset);
    inputFont.setBold(tempFont.inputs.bold);
    inputFont.setItalic(tempFont.inputs.italic);

    QFont techFont(tempFont.techFamily, tempFont.mainSize + tempFont.technical.sizeOffset);
    techFont.setBold(tempFont.technical.bold);
    techFont.setItalic(tempFont.technical.italic);

    // Apply main frame font
    previewWindow->setFont(mainFont);

    // Recursively apply individual sizes to child groups inside the isolated preview window
    const QList<QWidget*> children = previewWindow->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->inherits("QHeaderView") || child->inherits("QTabBar")) {
            child->setFont(headingFont);
        } else if (child->inherits("QMenuBar")) {
            child->setFont(menuBarFont);
        } else if (child->inherits("QMenu")) {
            child->setFont(menuFont);
        } else if (child->inherits("QToolBar") || child->inherits("QToolButton") || child->inherits("QPushButton")) {
            child->setFont(buttonFont);
        } else if (child->inherits("QLineEdit") || child->inherits("QComboBox") || child->inherits("QAbstractSpinBox")) {
            child->setFont(inputFont);
        } else if (child->inherits("QTextEdit") || child->inherits("QPlainTextEdit") || child->inherits("QListWidget")) {
            child->setFont(techFont);
        } else {
            child->setFont(mainFont);
        }
        child->update();
    }
    previewWindow->update();
}

void LC_StyleEditorTypography::applyCurrentPreset() {
    m_styleManager->setActiveTypography(m_currentPresetKey);
    m_styleManager->applyActiveStyleAndTheme();
}

void LC_StyleEditorTypography::rollbackState() {
    // Symmetrical rollback discards current modifications
}

void LC_StyleEditorTypography::loadConfigToUi(const FontConfig& config) {
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

    ui->fcTechnicalFont->setCurrentFont(QFont(config.techFamily));
    ui->sbTechnicalOffset->setValue(config.technical.sizeOffset);
    ui->chkTechnicalBold->setChecked(config.technical.bold);
    ui->chkTechnicalItalic->setChecked(config.technical.italic);
}

FontConfig LC_StyleEditorTypography::getConfigFromUi() const {
    FontConfig config = m_currentConfig;

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

    config.techFamily            = ui->fcTechnicalFont->currentFont().family();
    config.technical.sizeOffset  = ui->sbTechnicalOffset->value();
    config.technical.bold        = ui->chkTechnicalBold->isChecked();
    config.technical.italic      = ui->chkTechnicalItalic->isChecked();

    return config;
}
