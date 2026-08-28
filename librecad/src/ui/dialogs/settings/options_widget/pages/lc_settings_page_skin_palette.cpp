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

#include "lc_settings_page_skin_palette.h"
#include "ui_lc_settings_page_skin_palette.h"
#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>
#include <QTableWidget>
#include "lc_color_button.h"
#include "lc_icons_style_repository.h"
#include "lc_palette_color_utils.h"
#include "lc_preset_manager_fusion_skin.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

LC_SettingsPageSkinPalette::LC_SettingsPageSkinPalette(QObject* parent)
    : LC_SettingsPageBase(tr("Color Palette & Bevels"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSkinPalette>()) {
    setSortWeight(10);
}

LC_SettingsPageSkinPalette::~LC_SettingsPageSkinPalette() = default;

void LC_SettingsPageSkinPalette::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerFusionSkin*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerFusionSkin::configLoaded, this, [this](const SkinConfig&) {
            populateTablesFromConfig();
        });
        connect(m_presetManager, &LC_PresetManagerFusionSkin::variantChanged, this, [this](bool isDark) {
            m_blockSignals = true;
            ui->rbDarkMode->setChecked(isDark);
            ui->rbLightMode->setChecked(!isDark);
            m_blockSignals = false;
            populateTablesFromConfig();
        });
        populateTablesFromConfig();
    }
}

void LC_SettingsPageSkinPalette::setupUi() {
    ui->setupUi(m_widget);

    const bool isDark = LC_PaletteColorUtils::isSystemInDarkMode();
    ui->rbDarkMode->setChecked(isDark);
    ui->rbLightMode->setChecked(!isDark);

    setupComboboxes();
    setupGeneratorMenu();
    setupTablesStructure();
}

void LC_SettingsPageSkinPalette::setupComboboxes() {
    ui->cbContrastPolicy->clear();
    ui->cbContrastPolicy->addItem(tr("Not Affected (Keep Baseline)"), static_cast<int>(ContrastPolicy::Standard));
    ui->cbContrastPolicy->addItem(tr("Recessed Window (Low Eye-Strain)"), static_cast<int>(ContrastPolicy::RecessedWindow));
    ui->cbContrastPolicy->addItem(tr("Elevated Window (Polished Panels)"), static_cast<int>(ContrastPolicy::ElevatedWindow));
    ui->cbContrastPolicy->addItem(tr("Accent Tinted Background"), static_cast<int>(ContrastPolicy::AccentTinted));

    ui->cbBevelSeed->clear();
    ui->cbBevelSeed->addItem(tr("Button Background (Standard)"), static_cast<int>(QPalette::Button));
    ui->cbBevelSeed->addItem(tr("Window Background (Recessed)"), static_cast<int>(QPalette::Window));
    ui->cbBevelSeed->addItem(tr("Active Highlight (Tinted Glow)"), static_cast<int>(QPalette::Highlight));

    ui->cbContrastWeight->clear();
    ui->cbContrastWeight->addItem(tr("None (Flat Edges)"), static_cast<int>(ContrastWeight::None));
    ui->cbContrastWeight->addItem(tr("Soft Contrast"), static_cast<int>(ContrastWeight::Soft));
    ui->cbContrastWeight->addItem(tr("Balanced (Default)"), static_cast<int>(ContrastWeight::Balanced));
    ui->cbContrastWeight->addItem(tr("Hard Contrast"), static_cast<int>(ContrastWeight::Hard));

    auto* styleMgr = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
    if (styleMgr && styleMgr->getIconsStyleRepository()) {
        ui->cbLinkedIconStyle->clear();
        ui->cbLinkedIconStyle->addItem(tr("Default (Classic)"), DEFAULT_THEME_KEY);
        ui->cbLinkedIconStyle->addItems(styleMgr->getIconsStyleRepository()->getAvailableNames());
    }
}

void LC_SettingsPageSkinPalette::setupGeneratorMenu() {
    ui->btnGenerate->setPopupMode(QToolButton::InstantPopup);
    auto* menu = new QMenu(ui->btnGenerate);

    QAction* actHarmonized = menu->addAction(QIcon(":/icons/palette_1_point.lci"), tr("1-Color Theme (Monochromatic)..."));
    QAction* actTwoColor = menu->addAction(QIcon(":/icons/palette_2_points.lci"), tr("2-Color Theme (Surface + Accent)..."));
    QAction* actHighContrast = menu->addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("High-Contrast Theme..."));

    ui->btnGenerate->setMenu(menu);

    connect(actHarmonized, &QAction::triggered, this, &LC_SettingsPageSkinPalette::onGenerateHarmonizedTheme);
    connect(actTwoColor, &QAction::triggered, this, &LC_SettingsPageSkinPalette::onGenerateTwoColorTheme);
    connect(actHighContrast, &QAction::triggered, this, &LC_SettingsPageSkinPalette::onGenerateHighContrastTheme);
}

void LC_SettingsPageSkinPalette::setupTablesStructure() {
    auto configureTable = [this](QTableWidget* table, const QList<PaletteRoleMapping>& rolesList, bool isBevelTable) {
        table->setColumnCount(4);
        table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Role")));
        table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Active")));
        table->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Inactive")));
        table->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Disabled")));

        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setDefaultSectionSize(24);

        table->setRowCount(rolesList.size());
        for (int row = 0; row < table->rowCount(); ++row) {
            const QString roleName = rolesList[row].name;
            const QPalette::ColorRole role = rolesList[row].role;
            const QString tooltipText = getRoleTooltip(role);

            auto* item = new QTableWidgetItem(tr(roleName.toUtf8().constData()));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setToolTip(tooltipText);
            table->setItem(row, 0, item);

            for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
                const QString stateName = PALETTE_STATES[colIdx - 1].name;
                auto* btn = new LC_ColorButton(table);
                btn->setToolTip(tooltipText);

                if (isBevelTable) {
                    btn->setLockedToolTip(tooltipText + "\n" + tr("[Procedurally calculated: Uncheck 'Procedural 3D Bevels' to edit manually]"));
                }

                btn->setDialogTitle(tr("Select %1 Color (%2)").arg(tr(roleName.toUtf8().constData()), tr(stateName.toUtf8().constData())));
                connect(btn, &LC_ColorButton::colorChanged, this, &LC_SettingsPageSkinPalette::onControlChanged);
                table->setCellWidget(row, colIdx, btn);
            }
        }
    };

    configureTable(ui->tableInterface, BASE_INTERFACE_ROLES, false);
    configureTable(ui->tableBevel, BEVEL_HELPER_ROLES, true);
}

void LC_SettingsPageSkinPalette::setupBehavior() {
    connect(ui->rbLightMode, &QRadioButton::toggled, this, &LC_SettingsPageSkinPalette::onVariantToggled);
    connect(ui->rbDarkMode, &QRadioButton::toggled, this, &LC_SettingsPageSkinPalette::onVariantToggled);
    connect(ui->chkAutoCalc3D, &QCheckBox::toggled, this, &LC_SettingsPageSkinPalette::onAutoCalc3DToggled);
    connect(ui->cbBevelSeed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinPalette::onBevelSeedChanged);
    connect(ui->cbContrastWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinPalette::onContrastWeightChanged);
    connect(ui->cbContrastPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinPalette::onContrastPolicyChanged);
    connect(ui->chkUseThemeIcons, &QCheckBox::toggled, this, &LC_SettingsPageSkinPalette::onUseThemeIconsToggled);
    connect(ui->cbLinkedIconStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinPalette::onControlChanged);
}

void LC_SettingsPageSkinPalette::loadSettings() {
    populateTablesFromConfig();
}

bool LC_SettingsPageSkinPalette::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageSkinPalette::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageSkinPalette::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageSkinPalette::onVariantToggled(bool checked) {
    if (m_blockSignals || !checked || m_presetManager == nullptr) return;

    syncUiToWorkingConfig();
    const bool darkSelected = ui->rbDarkMode->isChecked();
    m_presetManager->setCurrentVariantDark(darkSelected);
}

void LC_SettingsPageSkinPalette::onAutoCalc3DToggled(bool checked) {
    if (m_blockSignals || m_presetManager == nullptr) return;

    ui->cbBevelSeed->setEnabled(checked);
    ui->cbContrastWeight->setEnabled(checked);

    syncUiToWorkingConfig();
    const bool isDark = ui->rbDarkMode->isChecked();
    ColorSchemeData& scheme = isDark ? m_presetManager->workingConfig().dark : m_presetManager->workingConfig().light;
    scheme.autoCalculate3DHelpers = checked;

    if (checked) {
        m_presetManager->calculateProceduralBevels(isDark);
    }
    else {
        for (int row = 0; row < ui->tableBevel->rowCount(); ++row) {
            for (int col = 1; col <= 3; ++col) {
                if (auto* btn = qobject_cast<LC_ColorButton*>(ui->tableBevel->cellWidget(row, col))) {
                    btn->setLocked(false);
                }
            }
        }
    }
    m_presetManager->onSubPageControlChanged();
}

void LC_SettingsPageSkinPalette::onBevelSeedChanged(int) {
    if (m_blockSignals || m_presetManager == nullptr) return;
    syncUiToWorkingConfig();
    if (ui->chkAutoCalc3D->isChecked()) {
        m_presetManager->calculateProceduralBevels(ui->rbDarkMode->isChecked());
    }
    m_presetManager->onSubPageControlChanged();
}

void LC_SettingsPageSkinPalette::onContrastWeightChanged(int) {
    if (m_blockSignals || m_presetManager == nullptr) return;
    syncUiToWorkingConfig();
    if (ui->chkAutoCalc3D->isChecked()) {
        m_presetManager->calculateProceduralBevels(ui->rbDarkMode->isChecked());
    }
    m_presetManager->onSubPageControlChanged();
}

void LC_SettingsPageSkinPalette::onContrastPolicyChanged(int) {
    onControlChanged();
}

void LC_SettingsPageSkinPalette::onUseThemeIconsToggled(bool checked) {
    ui->cbLinkedIconStyle->setEnabled(checked);
    onControlChanged();
}

void LC_SettingsPageSkinPalette::onGenerateHarmonizedTheme() {
    if (m_presetManager == nullptr) return;
    QColor baseColor = QColorDialog::getColor(Qt::blue, getEditingWidget(), tr("Select Base Harmony Color"));
    if (!baseColor.isValid()) return;
    syncUiToWorkingConfig();
    m_presetManager->generateHarmonizedTheme(baseColor);
}

void LC_SettingsPageSkinPalette::onGenerateTwoColorTheme() {
    if (m_presetManager == nullptr) return;
    QColor surface = QColorDialog::getColor(QColor(45, 45, 45), getEditingWidget(), tr("Select Primary Surface Color"));
    if (!surface.isValid()) return;
    QColor accent = QColorDialog::getColor(QColor(42, 130, 218), getEditingWidget(), tr("Select Accent Highlight Color"));
    if (!accent.isValid()) return;
    syncUiToWorkingConfig();
    m_presetManager->generateTwoColorTheme(surface, accent);
}

void LC_SettingsPageSkinPalette::onGenerateHighContrastTheme() {
    if (m_presetManager == nullptr) return;
    QColor baseColor = QColorDialog::getColor(Qt::blue, getEditingWidget(), tr("Select Base Contrast Color"));
    if (!baseColor.isValid()) return;
    syncUiToWorkingConfig();
    m_presetManager->generateHighContrastTheme(baseColor);
}

void LC_SettingsPageSkinPalette::populateTablesFromConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const bool isDark = ui->rbDarkMode->isChecked();
    const auto& skin = m_presetManager->workingConfig();
    const ColorSchemeData& scheme = isDark ? skin.dark : skin.light;

    auto loadTableColors = [&](const QList<PaletteRoleMapping>& rolesList, QTableWidget* table, bool shouldLock) {
        for (int row = 0; row < rolesList.size(); ++row) {
            const QString roleName = rolesList[row].name;
            for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
                if (auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, colIdx))) {
                    const QString stateName = PALETTE_STATES[colIdx - 1].name;
                    const QColor col = scheme.palette.value(roleName).value(stateName, Qt::white);
                    btn->setLocked(shouldLock);
                    btn->setColor(col);
                }
            }
        }
    };

    loadTableColors(BASE_INTERFACE_ROLES, ui->tableInterface, false);
    loadTableColors(BEVEL_HELPER_ROLES, ui->tableBevel, scheme.autoCalculate3DHelpers);

    ui->chkAutoCalc3D->setChecked(scheme.autoCalculate3DHelpers);
    ui->cbBevelSeed->setEnabled(scheme.autoCalculate3DHelpers);
    ui->cbContrastWeight->setEnabled(scheme.autoCalculate3DHelpers);

    const int seedIdx = ui->cbBevelSeed->findData(static_cast<int>(scheme.bevelSeedRole));
    if (seedIdx >= 0) ui->cbBevelSeed->setCurrentIndex(seedIdx);

    const int weightIdx = ui->cbContrastWeight->findData(static_cast<int>(scheme.contrastWeight));
    if (weightIdx >= 0) ui->cbContrastWeight->setCurrentIndex(weightIdx);

    const int policyIdx = ui->cbContrastPolicy->findData(static_cast<int>(scheme.contrastPolicy));
    if (policyIdx >= 0) ui->cbContrastPolicy->setCurrentIndex(policyIdx);

    ui->chkUseThemeIcons->setChecked(skin.useThemeDefaultIcons);
    ui->cbLinkedIconStyle->setEnabled(skin.useThemeDefaultIcons);
    const int iconIdx = ui->cbLinkedIconStyle->findText(skin.linkedIconStyleName);
    if (iconIdx >= 0) ui->cbLinkedIconStyle->setCurrentIndex(iconIdx);

    m_blockSignals = false;
}

void LC_SettingsPageSkinPalette::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    const bool isDark = ui->rbDarkMode->isChecked();
    auto& skin = m_presetManager->workingConfig();
    ColorSchemeData& scheme = isDark ? skin.dark : skin.light;

    scheme.autoCalculate3DHelpers = ui->chkAutoCalc3D->isChecked();
    scheme.bevelSeedRole = static_cast<QPalette::ColorRole>(ui->cbBevelSeed->currentData().toInt());
    scheme.contrastWeight = static_cast<ContrastWeight>(ui->cbContrastWeight->currentData().toInt());
    scheme.contrastPolicy = static_cast<ContrastPolicy>(ui->cbContrastPolicy->currentData().toInt());

    skin.useThemeDefaultIcons = ui->chkUseThemeIcons->isChecked();
    skin.linkedIconStyleName = ui->cbLinkedIconStyle->currentText();

    auto saveTableColors = [&](const QList<PaletteRoleMapping>& rolesList, QTableWidget* table) {
        for (int row = 0; row < rolesList.size(); ++row) {
            const QString roleName = rolesList[row].name;
            for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
                if (const auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, colIdx))) {
                    const QString stateName = PALETTE_STATES[colIdx - 1].name;
                    scheme.palette[roleName][stateName] = btn->color();
                }
            }
        }
    };

    saveTableColors(BASE_INTERFACE_ROLES, ui->tableInterface);
    saveTableColors(BEVEL_HELPER_ROLES, ui->tableBevel);
}

QString LC_SettingsPageSkinPalette::getRoleTooltip(QPalette::ColorRole role) const {
    switch (role) {
        case QPalette::Window:            return tr("Background color for main windows, dialog boxes, and group containers.");
        case QPalette::WindowText:        return tr("Standard text color for labels and static headings.");
        case QPalette::Base:              return tr("Background color for text fields, tree views, and drop-down list boxes.");
        case QPalette::AlternateBase:     return tr("Background color used for alternating rows in tabular grids.");
        case QPalette::ToolTipBase:       return tr("Background color of popup tooltips.");
        case QPalette::ToolTipText:       return tr("Text color of popup tooltips.");
        case QPalette::Text:              return tr("Standard text color for user inputs and text editors.");
        case QPalette::Button:            return tr("Background color for buttons, tabs, toolbars, and scrollbar arrows.");
        case QPalette::ButtonText:        return tr("Text and icon foreground color for buttons and action controls.");
        case QPalette::BrightText:        return tr("Contrasting text color used for highlights or error messages against dark backings.");
        case QPalette::Link:              return tr("Color of interactive text hyperlinks.");
        case QPalette::Highlight:         return tr("Background color for selected items and active focus outlines.");
        case QPalette::HighlightedText:   return tr("Text color for selected items.");
        case QPalette::PlaceholderText:   return tr("Muted text color shown in empty fields prior to user typing.");
        case QPalette::Light:             return tr("Bevel color used for the brightest highlights and chiseled edges.");
        case QPalette::Midlight:          return tr("Bevel color used for medium-bright lines and dividers.");
        case QPalette::Dark:              return tr("Bevel color used for shadow lines and structural margins.");
        case QPalette::Mid:               return tr("Bevel color used for medium shadows and standard boundaries.");
        case QPalette::Shadow:            return tr("Bevel color used for deep outer 3D shadows.");
        default: break;
    }
    return QString();
}
