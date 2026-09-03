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

#include "lc_semantic_colors_ui_helper.h"
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
    m_blockSignals = true;
    ui->setupUi(m_widget);

    setupStyleCombobox();
    LC_SemanticColorsUiHelper::setupTable(ui->tableSemantic);
    updateStyleDependencyStates();
    m_blockSignals = false;
}

void LC_SettingsPageGeneralStyling::setupStyleCombobox() {
    ui->cbStyle->clear();
    ui->cbStyle->addItems(QStyleFactory::keys());
}

void LC_SettingsPageGeneralStyling::setupBehavior() {
    connect(ui->cbAllowStyle, &QCheckBox::toggled, this, &LC_SettingsPageGeneralStyling::onControlChanged);
    connect(ui->cbStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageGeneralStyling::onControlChanged);
    connect(ui->leStylesheet, &QLineEdit::editingFinished, this, &LC_SettingsPageGeneralStyling::onControlChanged);

    connect(ui->tbSelectStylesheet, &QToolButton::clicked, this, &LC_SettingsPageGeneralStyling::chooseStyleSheet);

    connect(ui->pbGoToFusionTheme, &QPushButton::clicked, this, [this]() {
        emit navigateToPage(LC_SettingsPagesStyling::PAGE_STYLING_FUSION);
    });

    LC_SemanticColorsUiHelper::connectChanged(ui->tableSemantic, this, [this]() {
        onControlChanged();
    });


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
}

void LC_SettingsPageGeneralStyling::updateStyleDependencyStates() {
    const bool allowStyle = ui->cbAllowStyle->isChecked();
    ui->cbStyle->setEnabled(allowStyle);

    const bool isFusion = allowStyle && (ui->cbStyle->currentText() == "Fusion");
    ui->lblStylesheet->setEnabled(!isFusion);
    ui->leStylesheet->setEnabled(!isFusion);
    ui->tbSelectStylesheet->setEnabled(!isFusion);

    ui->gbSemanticColors->setVisible(!isFusion);
    ui->gbFusionLink->setVisible(isFusion);
}

void LC_SettingsPageGeneralStyling::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    updateStyleDependencyStates();
    applyTransientStyle();
    updateLivePreview();
}

void LC_SettingsPageGeneralStyling::applyTransientStyle() {
    if (m_styleManager == nullptr) {
        return;
    }

    const bool allowStyle = ui->cbAllowStyle->isChecked();
    const QString styleName = ui->cbStyle->currentText();
    const auto themeModeOverride = m_styleManager->getThemeModeOverride();
    const QString stylesheetPath = ui->leStylesheet->text().trimmed();

    CFG_AppStyling::o_AllowStyle.set(allowStyle);
    CFG_AppStyling::o_Style.set(styleName);
    CFG_AppStyling::o_StyleSheet.set(stylesheetPath);

    const QString paletteKey = m_styleManager->getActivePalette();
    const QString skinKey = m_styleManager->getActiveSkin();
    const QString metricsKey = m_styleManager->getActiveMetrics();
    const QString typographyKey = m_styleManager->getActiveTypography();
    const QString iconStyleKey = m_styleManager->getActiveIconStyle();

    m_styleManager->applyTransientTheme(allowStyle, styleName,
                                        paletteKey, skinKey, metricsKey,
                                        typographyKey, iconStyleKey,
                                        themeModeOverride);

    if (allowStyle && styleName != "Fusion" && !stylesheetPath.isEmpty()) {
        m_styleManager->loadStyleSheet(stylesheetPath);
    }
}

void LC_SettingsPageGeneralStyling::chooseStyleSheet() {
    const QString path = QFileDialog::getOpenFileName(
        getEditingWidget(), tr("Select Stylesheet File"), QString(), tr("Qt Stylesheets (*.qss *.css);;All Files (*.*)"));
    if (!path.isEmpty()) {
        ui->leStylesheet->setText(QDir::toNativeSeparators(path));
        onControlChanged();
    }
}

void LC_SettingsPageGeneralStyling::loadSettings() {
    m_blockSignals = true;
    LC_SettingsPageBase::loadSettings();
    updateStyleDependencyStates();

    QMap<QString, QColor> baselineColors;
    baselineColors[SEMANTIC_COLOR_KEY_FILTERED_ITEM]  = CFG_AppStyling::o_ColorFilteredItem.get();
    baselineColors[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]   = CFG_AppStyling::o_ColorConflictingItem.get();
    baselineColors[SEMANTIC_COLOR_KEY_SEARCH_RESULT]  = CFG_AppStyling::o_ColorSearchResultItem.get();

    LC_SemanticColorsUiHelper::populateTable(ui->tableSemantic, baselineColors, false, false);
    m_blockSignals = false;
}

bool LC_SettingsPageGeneralStyling::saveSettings() {
    const bool success = LC_SettingsPageBase::saveSettings();
    if (success) {
        QMap<QString, QColor> extracted;
        LC_SemanticColorsUiHelper::extractColors(ui->tableSemantic, extracted);

        if (extracted.contains(SEMANTIC_COLOR_KEY_FILTERED_ITEM)) {
            CFG_AppStyling::o_ColorFilteredItem.set(extracted[SEMANTIC_COLOR_KEY_FILTERED_ITEM]);
        }
        if (extracted.contains(SEMANTIC_COLOR_KEY_CONFLICTING_ITEM)) {
            CFG_AppStyling::o_ColorConflictingItem.set(extracted[SEMANTIC_COLOR_KEY_CONFLICTING_ITEM]);
        }
        if (extracted.contains(SEMANTIC_COLOR_KEY_SEARCH_RESULT)) {
            CFG_AppStyling::o_ColorSearchResultItem.set(extracted[SEMANTIC_COLOR_KEY_SEARCH_RESULT]);
        }

        auto* styleMgr = QC_ApplicationWindow::getAppWindow() ? QC_ApplicationWindow::getAppWindow()->getUiStyleManager() : nullptr;
        if (styleMgr != nullptr) {
            styleMgr->updateSemanticColorsCache();
        }
    }
    return success;
}
