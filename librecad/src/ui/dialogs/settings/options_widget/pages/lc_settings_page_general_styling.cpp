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

#include "lc_settings_page_general_styling.h"
#include "ui_lc_settings_page_general_styling.h"
#include <QDir>
#include <QFileDialog>
#include <QStyleFactory>
#include "lc_settings_app_styling.h"
#include "lc_settings_backend.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_SettingsPageGeneralStyling::LC_SettingsPageGeneralStyling(QObject* parent)
    : LC_SettingsPageBase(tr("General Styling"),
                          std::make_unique<LC_LibreCADSettingsBackend>(CFG_AppStyling::Group),
                          parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGeneralStyling>()) {
    m_styleManager = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
}

LC_SettingsPageGeneralStyling::~LC_SettingsPageGeneralStyling() = default;

void LC_SettingsPageGeneralStyling::setupUi() {
    ui->setupUi(m_widget);

    setupStyleCombobox();
    setupThemeModeCombobox();
    updateStyleDependencyStates();
}

void LC_SettingsPageGeneralStyling::setupStyleCombobox() const {
    ui->cbStyle->clear();
    ui->cbStyle->addItems(QStyleFactory::keys());
}

void LC_SettingsPageGeneralStyling::setupThemeModeCombobox() const {
    ui->cbThemeModeOverride->clear();
    ui->cbThemeModeOverride->addItem(tr("Follow System Settings"), static_cast<int>(ThemeModeOverride::FollowSystem));
    ui->cbThemeModeOverride->addItem(tr("Force Light Mode"), static_cast<int>(ThemeModeOverride::ForceLight));
    ui->cbThemeModeOverride->addItem(tr("Force Dark Mode"), static_cast<int>(ThemeModeOverride::ForceDark));
}

void LC_SettingsPageGeneralStyling::setupBehavior() {
    connect(ui->cbAllowStyle, &QCheckBox::toggled, this, &LC_SettingsPageGeneralStyling::updateStyleDependencyStates);
    connect(ui->cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageGeneralStyling::updateStyleDependencyStates);

    connect(ui->tbSelectStylesheet, &QToolButton::clicked, this, &LC_SettingsPageGeneralStyling::chooseStyleSheet);

    connect(ui->cbAllowStyle, &QCheckBox::toggled, this, [this](bool) { updateLivePreview(); });
    connect(ui->cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { updateLivePreview(); });
    connect(ui->cbThemeModeOverride, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { updateLivePreview(); });
    connect(ui->cbIgnoreIconStylingInTheme, &QCheckBox::toggled, this, [this](bool) { updateLivePreview(); });
    connect(ui->leStylesheet, &QLineEdit::editingFinished, this, [this]() { updateLivePreview(); });
}

void LC_SettingsPageGeneralStyling::setupBindings() {
    using namespace CFG_AppStyling;

    bindBoolean({
        { ui->cbAllowStyle, o_AllowStyle }
    });

    bindString({
        { ui->leStylesheet, o_StyleSheet }
    });

    bindCustom<QComboBox, QString>(
        ui->cbStyle, o_Style, false,
        &QComboBox::currentTextChanged,
        [](QComboBox* w) { return w->currentText(); },
        [](QComboBox* w, const QString& val) {
            QString styleToFind = val.isEmpty() ? QStringLiteral("Fusion") : val;
            int idx = w->findText(styleToFind, Qt::MatchFixedString);
            if (idx >= 0) {
                w->setCurrentIndex(idx);
            }
            else if (w->count() > 0) {
                w->setCurrentIndex(0);
            }
        });

    bindCustom<QComboBox, int>(
            ui->cbThemeModeOverride, o_ThemeModeOverride.fullKey(), static_cast<int>(o_ThemeModeOverride.defaultValue()), false,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [](QComboBox* w) { return w->currentData().toInt(); },
            [](QComboBox* w, int val) {
                int idx = w->findData(val);
                if (idx >= 0) w->setCurrentIndex(idx);
            });

    bindCustom<QCheckBox, bool>(
        ui->cbIgnoreIconStylingInTheme, "Widgets/IgnoreIconStylingInTheme", false, false,
        &QCheckBox::toggled,
        [](QCheckBox* w) { return w->isChecked(); },
        [](QCheckBox* w, bool val) { w->setChecked(val); });
}

void LC_SettingsPageGeneralStyling::updateStyleDependencyStates() {
    const bool allowStyle = ui->cbAllowStyle->isChecked();
    ui->cbStyle->setEnabled(allowStyle);

    const bool isFusion = allowStyle && (ui->cbStyle->currentText() == "Fusion");
    ui->lblThemeMode->setEnabled(isFusion);
    ui->cbThemeModeOverride->setEnabled(isFusion);
    ui->cbIgnoreIconStylingInTheme->setEnabled(isFusion);

    ui->lblStylesheet->setEnabled(!isFusion);
    ui->leStylesheet->setEnabled(!isFusion);
    ui->tbSelectStylesheet->setEnabled(!isFusion);
}

void LC_SettingsPageGeneralStyling::chooseStyleSheet() {
    const QString path = QFileDialog::getOpenFileName(
        getEditingWidget(), tr("Select Stylesheet File"), QString(), tr("Qt Stylesheets (*.qss *.css);;All Files (*.*)"));
    if (!path.isEmpty()) {
        ui->leStylesheet->setText(QDir::toNativeSeparators(path));
        updateLivePreview();
    }
}

bool LC_SettingsPageGeneralStyling::saveSettings() {
    const bool success = LC_SettingsPageBase::saveSettings();
    if (success && m_styleManager != nullptr) {
        m_styleManager->setStyleAllowed(ui->cbAllowStyle->isChecked());
        m_styleManager->setActiveStyle(ui->cbStyle->currentText());
        m_styleManager->setThemeModeOverride(static_cast<ThemeModeOverride>(ui->cbThemeModeOverride->currentData().toInt()));
        m_styleManager->setActiveStyleSheet(ui->leStylesheet->text());
        m_styleManager->setIgnoreIconStylingInTheme(ui->cbIgnoreIconStylingInTheme->isChecked());
        m_styleManager->applyActiveStyleAndTheme();
    }
    return success;
}
