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

#include "lc_style_editor_fusion_skin.h"

#include "ui_lc_style_editor_fusion_skin.h"
#include "lc_ui_style_manager.h"
#include "lc_preset_container_widget.h"
#include "lc_color_button.h"
#include "lc_palette_color_utils.h"
#include "lc_proxy_style.h"
#include "lc_icons_style_repository.h"
#include <QStyleFactory>
#include <QHeaderView>
#include <QMenu>
#include <QColorDialog>

#include "lc_fusion_skins_repository.h"
#include "rs_debug.h"

LC_StyleEditorFusionSkin::LC_StyleEditorFusionSkin(QWidget* parent, LC_UIStyleManager* styleManager)
    : LC_StyleEditorBase(parent, styleManager, styleManager->getSkinsRepository())
    , ui(new Ui::LC_StyleEditorFusionSkin) {
    ui->setupUi(this);

    m_currentVariantDark = LC_PaletteColorUtils::isSystemInDarkMode();
    ui->rbDarkMode->setChecked(m_currentVariantDark);
    ui->rbLightMode->setChecked(!m_currentVariantDark);

    setupComboboxes();
    setupTablesStructure();
    setupGeneratorMenu();
    setupConnections();
}

LC_StyleEditorFusionSkin::~LC_StyleEditorFusionSkin() {
    delete ui;
}

void LC_StyleEditorFusionSkin::setupComboboxes() {
    ui->cbStyleArchetype->clear();
    ui->cbStyleArchetype->addItem(tr("Classic Fusion (Native UI)"), static_cast<int>(StyleArchetype::ClassicFusion));
    ui->cbStyleArchetype->addItem(tr("Flat Modern"), static_cast<int>(StyleArchetype::FlatModern));
    ui->cbStyleArchetype->addItem(tr("Soft Satin"), static_cast<int>(StyleArchetype::SoftSatin));
    ui->cbStyleArchetype->addItem(tr("Glassy Gloss (Vista Style)"), static_cast<int>(StyleArchetype::GlassyGloss));
    ui->cbStyleArchetype->addItem(tr("Accent Outline"), static_cast<int>(StyleArchetype::AccentOutline));

    ui->cbBoxDecoration->clear();
    ui->cbBoxDecoration->addItem(tr("Frameless (Clean Flat)"), static_cast<int>(BoxDecoration::Frameless));
    ui->cbBoxDecoration->addItem(tr("Dividing Hairline"), static_cast<int>(BoxDecoration::DividingHairline));
    ui->cbBoxDecoration->addItem(tr("Box Outline Frame"), static_cast<int>(BoxDecoration::BoxOutline));
    ui->cbBoxDecoration->addItem(tr("Recessed Well"), static_cast<int>(BoxDecoration::RecessedWell));
    ui->cbBoxDecoration->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(BoxDecoration::LeftAccentBar));
    ui->cbBoxDecoration->addItem(tr("Active Accent Frame"), static_cast<int>(BoxDecoration::ActiveAccentFrame));

    ui->cbBevelSeed->clear();
    ui->cbBevelSeed->addItem(tr("Button Background (Standard)"), static_cast<int>(QPalette::Button));
    ui->cbBevelSeed->addItem(tr("Window Background (Recessed)"), static_cast<int>(QPalette::Window));
    ui->cbBevelSeed->addItem(tr("Active Highlight (Tinted Glow)"), static_cast<int>(QPalette::Highlight));

    ui->cbContrastPolicy->clear();
    ui->cbContrastPolicy->addItem(tr("Not Affected (Keep Baseline)"), static_cast<int>(ContrastPolicy::Standard));
    ui->cbContrastPolicy->addItem(tr("Recessed Window (Low Eye-Strain)"), static_cast<int>(ContrastPolicy::RecessedWindow));
    ui->cbContrastPolicy->addItem(tr("Elevated Window (Polished Panels)"), static_cast<int>(ContrastPolicy::ElevatedWindow));
    ui->cbContrastPolicy->addItem(tr("Accent Tinted Background"), static_cast<int>(ContrastPolicy::AccentTinted));

    ui->cbContrastWeight->clear();
    ui->cbContrastWeight->addItem(tr("None (Flat Edges)"), static_cast<int>(ContrastWeight::None));
    ui->cbContrastWeight->addItem(tr("Soft Contrast"), static_cast<int>(ContrastWeight::Soft));
    ui->cbContrastWeight->addItem(tr("Balanced (Default)"), static_cast<int>(ContrastWeight::Balanced));
    ui->cbContrastWeight->addItem(tr("Hard Contrast"), static_cast<int>(ContrastWeight::Hard));

    ui->cbDockTitleStyle->clear();
    ui->cbDockTitleStyle->addItem(tr("Native (Fusion Default)"), static_cast<int>(DockTitleBarStyle::Native));
    ui->cbDockTitleStyle->addItem(tr("Solid Well"), static_cast<int>(DockTitleBarStyle::CustomSolid));
    ui->cbDockTitleStyle->addItem(tr("Soft Satin"), static_cast<int>(DockTitleBarStyle::CustomSatin));
    ui->cbDockTitleStyle->addItem(tr("Glassy Gloss"), static_cast<int>(DockTitleBarStyle::CustomGlassy));
    ui->cbDockTitleStyle->addItem(tr("Accent Outline"), static_cast<int>(DockTitleBarStyle::CustomAccentOutline));
    ui->cbDockTitleStyle->addItem(tr("Accent Line (Sidebar Style)"), static_cast<int>(DockTitleBarStyle::CustomAccentLine));

    ui->cbGroupBoxHeaderStyle->clear();
    ui->cbGroupBoxHeaderStyle->addItem(tr("Plain (Minimal text)"), static_cast<int>(GroupBoxHeaderStyle::Plain));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Overlapping"), static_cast<int>(GroupBoxHeaderStyle::Overlapping));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Clean Break"), static_cast<int>(GroupBoxHeaderStyle::BreakBorder));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Header Underline Divider"), static_cast<int>(GroupBoxHeaderStyle::HeaderUnderline));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Header Banner Card block"), static_cast<int>(GroupBoxHeaderStyle::HeaderBanner));

    ui->cbGroupBoxBoundaryStyle->clear();
    ui->cbGroupBoxBoundaryStyle->addItem(tr("None (Frameless)"), static_cast<int>(GroupBoxBoundaryStyle::None));
    ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
    ui->cbGroupBoxBoundaryStyle->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(GroupBoxBoundaryStyle::LeftStripe));
    ui->cbGroupBoxBoundaryStyle->addItem(tr("Top Line Only"), static_cast<int>(GroupBoxBoundaryStyle::TopLine));

    ui->cbBranchIndicatorStyle->clear();
    ui->cbBranchIndicatorStyle->addItem(tr("Sleek Chevrons (Modern IDE)"), static_cast<int>(BranchIndicatorStyle::MutedChevrons));
    ui->cbBranchIndicatorStyle->addItem(tr("Plus/Minus Square Boxes"), static_cast<int>(BranchIndicatorStyle::PlusMinusBoxes));
    ui->cbBranchIndicatorStyle->addItem(tr("Tactile Circle Badges"), static_cast<int>(BranchIndicatorStyle::TactileCircles));
    ui->cbBranchIndicatorStyle->addItem(tr("Guidelines Only (No Icons)"), static_cast<int>(BranchIndicatorStyle::ClassicLinesOnly));
    ui->cbBranchIndicatorStyle->addItem(tr("None (Open Indented Layout)"), static_cast<int>(BranchIndicatorStyle::None));

    ui->cbSplitterGripStyle->clear();
    ui->cbSplitterGripStyle->addItem(tr("Sleek 3-Dot Grip"), static_cast<int>(SplitterGripStyle::MutedDots));
    ui->cbSplitterGripStyle->addItem(tr("Rounded Capsule Pill"), static_cast<int>(SplitterGripStyle::RoundedPill));
    ui->cbSplitterGripStyle->addItem(tr("Short 3-Ridge Ribbed Grip"), static_cast<int>(SplitterGripStyle::RibbedGrip));
    ui->cbSplitterGripStyle->addItem(tr("Double Full-Length Ridges"), static_cast<int>(SplitterGripStyle::DoubleRidges));
    ui->cbSplitterGripStyle->addItem(tr("Razor Hairline Divider"), static_cast<int>(SplitterGripStyle::BorderHairline));

    ui->cbToolButtonIndicatorStyle->clear();
    ui->cbToolButtonIndicatorStyle->addItem(tr("Context-Aware Stripe (Adaptive)"), static_cast<int>(ToolButtonIndicatorStyle::ContextStripe));
    ui->cbToolButtonIndicatorStyle->addItem(tr("Centered 4px Accent Dot"), static_cast<int>(ToolButtonIndicatorStyle::AccentDot));
    ui->cbToolButtonIndicatorStyle->addItem(tr("Soft Outline Active Frame"), static_cast<int>(ToolButtonIndicatorStyle::AccentFrame));

    ui->cbSegmentedSeparationStyle->clear();
    ui->cbSegmentedSeparationStyle->addItem(tr("Continuous Card Well (Blender)"), static_cast<int>(SegmentedSeparationStyle::ContinuousCard));
    ui->cbSegmentedSeparationStyle->addItem(tr("Minimal Outer Border Contour"), static_cast<int>(SegmentedSeparationStyle::MinimalBorder));
    ui->cbSegmentedSeparationStyle->addItem(tr("Transparent Idle (Hover Highlight Only)"), static_cast<int>(SegmentedSeparationStyle::TransparentIdle));

    ui->cbSegmentedColorPolicy->clear();
    ui->cbSegmentedColorPolicy->addItem(tr("Muted Neutral Alternating"), static_cast<int>(SegmentedColorPolicy::NeutralAlternating));
    ui->cbSegmentedColorPolicy->addItem(tr("Palette-Matched Analogous (Recommended)"), static_cast<int>(SegmentedColorPolicy::HarmonizedAnalogous));
    ui->cbSegmentedColorPolicy->addItem(tr("Vibrant Full Pastel Spectrum"), static_cast<int>(SegmentedColorPolicy::VibrantSpectrum));
    ui->cbSegmentedColorPolicy->addItem(tr("Anchored Analogous Hue-Shift"), static_cast<int>(SegmentedColorPolicy::AnalogousHueShift));

    ui->cbCloseColorPolicy->clear();
    ui->cbCloseColorPolicy->addItem(tr("Active Accent Highlight"), static_cast<int>(CloseButtonColorPolicy::AccentColor));
    ui->cbCloseColorPolicy->addItem(tr("Muted Red Warning"), static_cast<int>(CloseButtonColorPolicy::MutedRed));
    ui->cbCloseColorPolicy->addItem(tr("Vibrant Pure Red"), static_cast<int>(CloseButtonColorPolicy::VibrantRed));
    ui->cbCloseColorPolicy->addItem(tr("Muted Neutral Border"), static_cast<int>(CloseButtonColorPolicy::MutedNeutral));
}

void LC_StyleEditorFusionSkin::setupGeneratorMenu() {
    ui->btnGenerate->setPopupMode(QToolButton::InstantPopup);
    auto* generateMenu = new QMenu(ui->btnGenerate);

    const QAction* actHarmonized   = generateMenu->addAction(QIcon(":/icons/palette_1_point.lci"), tr("1-Color Theme (Monochromatic)..."));
    const QAction* actTwoColor     = generateMenu->addAction(QIcon(":/icons/palette_2_points.lci"), tr("2-Color Theme (Surface + Accent)..."));
    const QAction* actHighContrast = generateMenu->addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("High-Contrast Theme..."));

    ui->btnGenerate->setMenu(generateMenu);

    connect(actHarmonized,   &QAction::triggered, this, &LC_StyleEditorFusionSkin::onGenerateHarmonizedTheme);
    connect(actTwoColor,     &QAction::triggered, this, &LC_StyleEditorFusionSkin::onGenerateTwoColorTheme);
    connect(actHighContrast, &QAction::triggered, this, &LC_StyleEditorFusionSkin::onGenerateHighContrastTheme);
}

void LC_StyleEditorFusionSkin::setupTablesStructure() {
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
                connect(btn, &LC_ColorButton::colorChanged, this, &LC_StyleEditorFusionSkin::onControlChanged);
                table->setCellWidget(row, colIdx, btn);
            }
        }
    };

    configureTable(ui->tableInterface, BASE_INTERFACE_ROLES, false);
    configureTable(ui->tableBevel, BEVEL_HELPER_ROLES, true);
}

void LC_StyleEditorFusionSkin::setupConnections() {
    connect(ui->rbLightMode, &QRadioButton::toggled, this, &LC_StyleEditorFusionSkin::onVariantTabChanged);
    connect(ui->rbDarkMode, &QRadioButton::toggled, this, &LC_StyleEditorFusionSkin::onVariantTabChanged);

    connect(ui->cbStyleArchetype, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onArchetypeIndexChanged);
    connect(ui->cbBoxDecoration, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onDecorationIndexChanged);
    connect(ui->cbBevelSeed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onBevelSeedChanged);
    connect(ui->cbContrastPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->cbContrastWeight, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->cbDockTitleStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkAutoCalc3D, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onAutoCalc3DToggled);
    connect(ui->chkCustomDockTitle, &QCheckBox::toggled, this, [this](bool checked) {
        ui->cbDockTitleStyle->setEnabled(checked);
        onControlChanged();
    });

    connect(ui->chkCustomGroupBox, &QCheckBox::toggled, this, [this](bool checked) {
        updateGroupBoxUiState();
        onControlChanged();
    });
    connect(ui->cbGroupBoxHeaderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        updateGroupBoxUiState();
        onControlChanged();
    });
    connect(ui->cbGroupBoxBoundaryStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkGroupBoxUseAccent, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkActiveRowSpotlight, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkShowItemViewHover, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->cbBranchIndicatorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkShowTreeConnectingLines, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkCustomSplitterGrip, &QCheckBox::toggled, this, [this](bool checked) {
       updateSplitterUiState();
       onControlChanged();
   });

    connect(ui->chkUseSegmentedButtons, &QCheckBox::toggled, this, [this](bool checked) {
       updateSegmentedButtonsUiState();
       onControlChanged();
   });
    connect(ui->cbSegmentedSeparationStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->cbSegmentedColorPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkCustomToolbarOverflow, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkAutoPopupToolbarOverflow, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkAutoPopupInstantButtons, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkAutoPopupMenuBar, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkMenuBarHoverCard, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkShowMenuCommandAliases, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->cbSplitterGripStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkHighlightSplitterOnDrag, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkShowGripBackgroundWell, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkAccentGrips, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkPersistentDockSplitter, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkFocusedInputGlow, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkUseSpinBoxProgressBar, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkToolButtonUnderline, &QCheckBox::toggled, this, [this](bool checked) {
        updateToolButtonUiState();
        onControlChanged();
    });
    connect(ui->cbToolButtonIndicatorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkCustomToolTipCard, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkCustomVectorIcons, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkTabStripeAtBottom, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkCustomMenuForTearOff, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkSyncCheckedMenuState, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkHighContrastScrollbars, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkTransparentScrollbars, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkUseThemeIcons, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onUseThemeIconsToggled);
    connect(ui->cbLinkedIconStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkUseFloatingHUD, &QCheckBox::toggled, this, [this](bool checked) {
        ui->cbCloseColorPolicy->setEnabled(checked);
        onControlChanged();
    });
    connect(ui->cbCloseColorPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkUseStatusPillChips, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->qssEdit, &QTextEdit::textChanged, this, &LC_StyleEditorFusionSkin::onControlChanged);

    connect(ui->chkShowGenericDockIcons, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkShowSpecialDockIcons, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
    connect(ui->chkCustomDialogTitleBar, &QCheckBox::toggled, this, &LC_StyleEditorFusionSkin::onControlChanged);
}

void LC_StyleEditorFusionSkin::onControlChanged() {
    if (m_blockSignals) return;
    notifyChanged(true);
}

void LC_StyleEditorFusionSkin::onVariantTabChanged(bool checked) {
    if (m_blockSignals || !checked) return;

    const bool darkScheme = ui->rbDarkMode->isChecked();
    saveUiToConfig(!darkScheme); // Save the variant we just left
    m_currentVariantDark = darkScheme;

    loadConfigToUi(m_currentVariantDark); // Load the variant we just entered
}

bool LC_StyleEditorFusionSkin::doLoadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY) {
        LC_PaletteColorUtils::initializeDefaultConfig(m_currentConfig);
    } else {
        m_styleManager->getSkinsRepository()->loadByKey(key, m_currentConfig);
    }

    m_blockSignals = true;

    ui->cbStyleArchetype->setCurrentIndex(ui->cbStyleArchetype->findData(static_cast<int>(m_currentConfig.styleArchetype)));
    ui->cbBoxDecoration->setCurrentIndex(ui->cbBoxDecoration->findData(static_cast<int>(m_currentConfig.boxDecoration)));
    ui->chkCustomDockTitle->setChecked(m_currentConfig.customDockTitleBar);
    ui->cbDockTitleStyle->setCurrentIndex(ui->cbDockTitleStyle->findData(static_cast<int>(m_currentConfig.dockTitleBarStyle)));
    ui->cbDockTitleStyle->setEnabled(m_currentConfig.customDockTitleBar);

    ui->chkCustomGroupBox->setChecked(m_currentConfig.customGroupBoxBar);
    ui->cbGroupBoxHeaderStyle->setCurrentIndex(ui->cbGroupBoxHeaderStyle->findData(static_cast<int>(m_currentConfig.groupBoxHeaderStyle)));
    ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(m_currentConfig.groupBoxBoundaryStyle)));
    ui->chkGroupBoxUseAccent->setChecked(m_currentConfig.groupBoxUseAccent);

    ui->chkActiveRowSpotlight->setChecked(m_currentConfig.showActiveRowSpotlight);
    ui->chkShowItemViewHover->setChecked(m_currentConfig.showItemViewHover);

    ui->cbBranchIndicatorStyle->setCurrentIndex(
        ui->cbBranchIndicatorStyle->findData(static_cast<int>(m_currentConfig.branchIndicatorStyle)));
    ui->chkShowTreeConnectingLines->setChecked(m_currentConfig.showTreeConnectingLines);

    ui->chkCustomToolbarOverflow->setChecked(m_currentConfig.customToolbarOverflowGrip);
    ui->chkAutoPopupToolbarOverflow->setChecked(m_currentConfig.autoPopupToolbarOverflow);
    ui->chkAutoPopupInstantButtons->setChecked(m_currentConfig.autoPopupInstantButtons);
    ui->chkAutoPopupMenuBar->setChecked(m_currentConfig.autoPopupMenuBar);
    ui->chkMenuBarHoverCard->setChecked(m_currentConfig.useMenuBarHoverCard);
    ui->chkShowMenuCommandAliases->setChecked(m_currentConfig.showMenuCommandAliases);
    ui->chkCustomMenuForTearOff->setChecked(m_currentConfig.customMenuTearOff);


    ui->chkSyncCheckedMenuState->setChecked(m_currentConfig.syncCheckedMenuState);

    ui->chkCustomSplitterGrip->setChecked(m_currentConfig.customSplitterGrip);
    ui->cbSplitterGripStyle->setCurrentIndex(ui->cbSplitterGripStyle->findData(static_cast<int>(m_currentConfig.splitterGripStyle)));
    ui->chkHighlightSplitterOnDrag->setChecked(m_currentConfig.highlightSplitterOnDrag);
    ui->chkShowGripBackgroundWell->setChecked(m_currentConfig.showGripBackgroundWell);
    ui->chkAccentGrips->setChecked(m_currentConfig.accentGrips);
    ui->chkPersistentDockSplitter->setChecked(m_currentConfig.persistentDockSplitter);
    ui->chkShowGenericDockIcons->setChecked(m_currentConfig.showGenericDockIcons);
    ui->chkShowSpecialDockIcons->setChecked(m_currentConfig.showSpecialDockIcons);
    ui->chkFocusedInputGlow->setChecked(m_currentConfig.useFocusedInputGlow);
    ui->chkUseStatusPillChips->setChecked(m_currentConfig.useStatusPillChips);
    ui->chkUseSpinBoxProgressBar->setChecked(m_currentConfig.useSpinBoxProgressBar);

    ui->chkToolButtonUnderline->setChecked(m_currentConfig.useToolButtonUnderline);
    ui->cbToolButtonIndicatorStyle->setCurrentIndex(
        ui->cbToolButtonIndicatorStyle->findData(static_cast<int>(m_currentConfig.toolButtonIndicatorStyle))
    );

    ui->chkCustomToolTipCard->setChecked(m_currentConfig.customToolTipCard);
    ui->chkCustomVectorIcons->setChecked(m_currentConfig.customVectorIconsInButtons);

    ui->chkTabStripeAtBottom->setChecked(m_currentConfig.tabStripeAtBottom);

    ui->chkUseSegmentedButtons->setChecked(m_currentConfig.useSegmentedToolButtons);
    ui->cbSegmentedSeparationStyle->setCurrentIndex(
        ui->cbSegmentedSeparationStyle->findData(static_cast<int>(m_currentConfig.segmentedSeparationStyle))
    );

    ui->cbSegmentedColorPolicy->setCurrentIndex(
     ui->cbSegmentedColorPolicy->findData(static_cast<int>(m_currentConfig.segmentedColorPolicy))
 );

    m_blockSignals = false;
    updateSplitterUiState();
    updateGroupBoxUiState();
    updateToolButtonUiState();
    m_blockSignals = true;

    ui->chkHighContrastScrollbars->setChecked(m_currentConfig.accentedScrollbars);
    ui->chkTransparentScrollbars->setChecked(m_currentConfig.transparentScrollbars);

    // Linked Icon presets (synchronous directory query)
    ui->cbLinkedIconStyle->blockSignals(true);
    ui->cbLinkedIconStyle->clear();
    ui->cbLinkedIconStyle->addItem(tr("Default (Classic)"), DEFAULT_THEME_KEY);
    ui->cbLinkedIconStyle->addItems(m_styleManager->getIconsStyleRepository()->getAvailableNames());
    ui->cbLinkedIconStyle->blockSignals(false);

    ui->chkUseThemeIcons->setChecked(m_currentConfig.useThemeDefaultIcons);
    ui->cbLinkedIconStyle->setEnabled(m_currentConfig.useThemeDefaultIcons);
    const int linkedIconStyleIdx = ui->cbLinkedIconStyle->findText(m_currentConfig.linkedIconStyleName);
    if (linkedIconStyleIdx >= 0) {
        ui->cbLinkedIconStyle->setCurrentIndex(linkedIconStyleIdx);
    } else {
        ui->cbLinkedIconStyle->setCurrentIndex(0);
    }

    ui->chkUseFloatingHUD->setChecked(m_currentConfig.useFloatingHUD);
    ui->cbCloseColorPolicy->setCurrentIndex(ui->cbCloseColorPolicy->findData(static_cast<int>(m_currentConfig.closeButtonColorPolicy)));
    ui->cbCloseColorPolicy->setEnabled(m_currentConfig.useFloatingHUD);

    ui->chkCustomDialogTitleBar->setChecked(m_currentConfig.customDialogTitleBar);

    // Synchronize UI Color Variants Tab
    m_blockSignals = false;
    loadConfigToUi(m_currentVariantDark);

    return true;
}

void LC_StyleEditorFusionSkin::loadConfigToUi(bool isDarkMode) {
    m_blockSignals = true; // Block signals at start of the load pipeline
    const ColorSchemeData& scheme = isDarkMode ? m_currentConfig.dark : m_currentConfig.light;

    auto loadTableColors = [&](const QList<PaletteRoleMapping>& rolesList, QTableWidget* table, bool shouldLock) {
        for (int row = 0; row < rolesList.size(); ++row) {
            QString roleName = rolesList[row].name;
            for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
                auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, colIdx));
                if (btn) {
                    QString stateName = PALETTE_STATES[colIdx - 1].name;
                    QColor col = scheme.palette.value(roleName).value(stateName, Qt::white);
                    btn->setLocked(shouldLock);
                    btn->setColor(col);
                }
            }
        }
    };

    loadTableColors(BASE_INTERFACE_ROLES, ui->tableInterface, false);
    loadTableColors(BEVEL_HELPER_ROLES, ui->tableBevel, scheme.autoCalculate3DHelpers);

    ui->cbContrastWeight->setEnabled(scheme.autoCalculate3DHelpers);
    ui->cbBevelSeed->setEnabled(scheme.autoCalculate3DHelpers);
    ui->qssEdit->setPlainText(scheme.qss);
    ui->chkAutoCalc3D->setChecked(scheme.autoCalculate3DHelpers);

    int seedIdx = ui->cbBevelSeed->findData(static_cast<int>(scheme.bevelSeedRole));
    if (seedIdx >= 0) ui->cbBevelSeed->setCurrentIndex(seedIdx);

    int weightIdx = ui->cbContrastWeight->findData(static_cast<int>(scheme.contrastWeight));
    if (weightIdx >= 0) ui->cbContrastWeight->setCurrentIndex(weightIdx);

    int policyIdx = ui->cbContrastPolicy->findData(static_cast<int>(scheme.contrastPolicy));
    if (policyIdx >= 0) ui->cbContrastPolicy->setCurrentIndex(policyIdx);

    m_blockSignals = false;

    onControlChanged();
}

bool LC_StyleEditorFusionSkin::saveCurrentPreset() {
    saveUiToConfig(m_currentVariantDark);
    QString outKey;
    return m_styleManager->getSkinsRepository()->save(m_currentConfig.name, m_currentConfig, outKey);
}

bool LC_StyleEditorFusionSkin::doSavePresetAs(const QString& name, QString& key)  {
    saveUiToConfig(m_currentVariantDark);
    m_currentConfig.name = name;
    return m_styleManager->getSkinsRepository()->save(name, m_currentConfig, key);
}


void LC_StyleEditorFusionSkin::saveUiToConfig(bool isDarkMode) {
    ColorSchemeData& scheme = isDarkMode ? m_currentConfig.dark : m_currentConfig.light;

    m_currentConfig.styleArchetype = static_cast<StyleArchetype>(ui->cbStyleArchetype->currentData().toInt());
    m_currentConfig.boxDecoration  = static_cast<BoxDecoration>(ui->cbBoxDecoration->currentData().toInt());
    m_currentConfig.customDockTitleBar = ui->chkCustomDockTitle->isChecked();
    m_currentConfig.dockTitleBarStyle  = static_cast<DockTitleBarStyle>(ui->cbDockTitleStyle->currentData().toInt());

    m_currentConfig.customGroupBoxBar = ui->chkCustomGroupBox->isChecked();
    m_currentConfig.groupBoxHeaderStyle = static_cast<GroupBoxHeaderStyle>(ui->cbGroupBoxHeaderStyle->currentData().toInt());
    m_currentConfig.groupBoxBoundaryStyle = static_cast<GroupBoxBoundaryStyle>(ui->cbGroupBoxBoundaryStyle->currentData().toInt());
    m_currentConfig.groupBoxUseAccent = ui->chkGroupBoxUseAccent->isChecked();

    m_currentConfig.showActiveRowSpotlight = ui->chkActiveRowSpotlight->isChecked();
    m_currentConfig.showItemViewHover = ui->chkShowItemViewHover->isChecked();

    m_currentConfig.branchIndicatorStyle = static_cast<BranchIndicatorStyle>(
       ui->cbBranchIndicatorStyle->currentData().toInt()
   );
    m_currentConfig.showTreeConnectingLines = ui->chkShowTreeConnectingLines->isChecked();

    m_currentConfig.customToolbarOverflowGrip = ui->chkCustomToolbarOverflow->isChecked();
    m_currentConfig.autoPopupToolbarOverflow  = ui->chkAutoPopupToolbarOverflow->isChecked();
    m_currentConfig.autoPopupInstantButtons   = ui->chkAutoPopupInstantButtons->isChecked();
    m_currentConfig.autoPopupMenuBar          = ui->chkAutoPopupMenuBar->isChecked();

    m_currentConfig.customSplitterGrip       = ui->chkCustomSplitterGrip->isChecked();

    m_currentConfig.splitterGripStyle        = static_cast<SplitterGripStyle>(ui->cbSplitterGripStyle->currentData().toInt());
    m_currentConfig.highlightSplitterOnDrag = ui->chkHighlightSplitterOnDrag->isChecked();
    m_currentConfig.showGripBackgroundWell  = ui->chkShowGripBackgroundWell->isChecked();
    m_currentConfig.accentGrips       = ui->chkAccentGrips->isChecked();
    m_currentConfig.persistentDockSplitter   = ui->chkPersistentDockSplitter->isChecked();
    m_currentConfig.showGenericDockIcons = ui->chkShowGenericDockIcons->isChecked();
    m_currentConfig.showSpecialDockIcons = ui->chkShowSpecialDockIcons->isChecked();

    m_currentConfig.useFocusedInputGlow     = ui->chkFocusedInputGlow->isChecked();
    m_currentConfig.useStatusPillChips      = ui->chkUseStatusPillChips->isChecked();
    m_currentConfig.useSpinBoxProgressBar   = ui->chkUseSpinBoxProgressBar->isChecked();

    m_currentConfig.useToolButtonUnderline   = ui->chkToolButtonUnderline->isChecked();
    m_currentConfig.toolButtonIndicatorStyle = static_cast<ToolButtonIndicatorStyle>(
        ui->cbToolButtonIndicatorStyle->currentData().toInt()
    );

    m_currentConfig.customToolTipCard       = ui->chkCustomToolTipCard->isChecked();
    m_currentConfig.customVectorIconsInButtons     = ui->chkCustomVectorIcons->isChecked();

    m_currentConfig.tabStripeAtBottom       = ui->chkTabStripeAtBottom->isChecked();

    m_currentConfig.accentedScrollbars = ui->chkHighContrastScrollbars->isChecked();
    m_currentConfig.transparentScrollbars = ui->chkTransparentScrollbars->isChecked();

    m_currentConfig.useThemeDefaultIcons = ui->chkUseThemeIcons->isChecked();
    m_currentConfig.linkedIconStyleName  = ui->cbLinkedIconStyle->currentText();
    m_currentConfig.useMenuBarHoverCard = ui->chkMenuBarHoverCard->isChecked();
    m_currentConfig.showMenuCommandAliases = ui->chkShowMenuCommandAliases->isChecked();

    m_currentConfig.useSegmentedToolButtons = ui->chkUseSegmentedButtons->isChecked();
    m_currentConfig.segmentedSeparationStyle = static_cast<SegmentedSeparationStyle>(
        ui->cbSegmentedSeparationStyle->currentData().toInt()
    );
    m_currentConfig.segmentedColorPolicy = static_cast<SegmentedColorPolicy>(ui->cbSegmentedColorPolicy->currentData().toInt());

    m_currentConfig.useFloatingHUD          = ui->chkUseFloatingHUD->isChecked();
    m_currentConfig.closeButtonColorPolicy  = static_cast<CloseButtonColorPolicy>(ui->cbCloseColorPolicy->currentData().toInt());

    m_currentConfig.customMenuTearOff = ui->chkCustomMenuForTearOff->isChecked();
    m_currentConfig.syncCheckedMenuState = ui->chkSyncCheckedMenuState->isChecked();

    m_currentConfig.customDialogTitleBar = ui->chkCustomDialogTitleBar->isChecked();

    scheme.qss = ui->qssEdit->toPlainText();
    scheme.autoCalculate3DHelpers = ui->chkAutoCalc3D->isChecked();
    scheme.bevelSeedRole = static_cast<QPalette::ColorRole>(ui->cbBevelSeed->currentData().toInt());
    scheme.contrastWeight = static_cast<ContrastWeight>(ui->cbContrastWeight->currentData().toInt());
    scheme.contrastPolicy = static_cast<ContrastPolicy>(ui->cbContrastPolicy->currentData().toInt());

    auto saveTableColors = [&](const QList<PaletteRoleMapping>& rolesList, QTableWidget* table) {
        for (int row = 0; row < rolesList.size(); ++row) {
            QString roleName = rolesList[row].name;
            for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
                const auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, colIdx));
                if (btn) {
                    QString stateName = PALETTE_STATES[colIdx - 1].name;
                    scheme.palette[roleName][stateName] = btn->color();
                }
            }
        }
    };

    saveTableColors(BASE_INTERFACE_ROLES, ui->tableInterface);
    saveTableColors(BEVEL_HELPER_ROLES, ui->tableBevel);
}

QString LC_StyleEditorFusionSkin::getActivePresetKey() const {
    return m_styleManager->getActiveSkin();
}

QList<QPair<QString, QString>> LC_StyleEditorFusionSkin::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Fusion Skin"), DEFAULT_THEME_KEY));
    choices.append(m_styleManager->getSkinsRepository()->getPresetChoices());
    return choices;
}

void LC_StyleEditorFusionSkin::applyTransientState(QWidget* previewWindow) const {
    SkinConfig tempSkin = m_currentConfig;
    const_cast<LC_StyleEditorFusionSkin*>(this)->saveUiToConfig(m_currentVariantDark);
    tempSkin = m_currentConfig;

    const bool isDarkMode = m_currentVariantDark;
    const ColorSchemeData& scheme = isDarkMode ? tempSkin.dark : tempSkin.light;

    auto cvdType = LC_PaletteColorUtils::CVDType::Normal;
    const auto* container = qobject_cast<LC_PresetContainerWidget*>(parentWidget());
    if (!container && parentWidget()) {
        container = qobject_cast<LC_PresetContainerWidget*>(parentWidget()->parentWidget());
    }
    if (container) {
        cvdType = container->activeCvdType();
    }


    const QPalette previewPalette = LC_PaletteColorUtils::createPaletteFromScheme(scheme, tempSkin.styleArchetype, cvdType);
    previewWindow->setPalette(previewPalette);
    previewWindow->setStyleSheet(scheme.qss);

    const QList<QWidget*> children = previewWindow->findChildren<QWidget*>();
    for (QWidget *child : children) {
        if ((child->objectName() == "lblBrightText_2") || (child->objectName() == "lblBrightText_3")) {
            QPalette brightTextPalette = previewPalette;
            brightTextPalette.setColor(QPalette::WindowText, previewPalette.color(QPalette::BrightText));
            child->setPalette(brightTextPalette);
        } else {
            child->setPalette(previewPalette);
        }
    }

    QStyle* baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle);
    proxyStyle->setSkin(tempSkin);
    previewWindow->setStyle(proxyStyle);


    for (QWidget* child : children) {
        child->setStyle(proxyStyle);
        child->setPalette(previewPalette);
        child->update();
    }
    previewWindow->update();
}

void LC_StyleEditorFusionSkin::applyCurrentPreset() {
    m_styleManager->setActiveSkin(m_currentPresetKey);
    m_styleManager->applyActiveStyleAndTheme();
}

void LC_StyleEditorFusionSkin::rollbackState() {
    // Revert logic on Cancel
}

void LC_StyleEditorFusionSkin::onAutoCalc3DToggled(bool checked) {
    if (m_blockSignals) return;
    ui->cbBevelSeed->setEnabled(checked);
    ui->cbContrastWeight->setEnabled(checked);

    if (checked) {
        calculateProceduralBevels(m_currentVariantDark);
    } else {
        // Unlock color swatches in the bevel table
        for (int row = 0; row < ui->tableBevel->rowCount(); ++row) {
            for (int col = 1; col <= 3; ++col) {
                auto* btn = qobject_cast<LC_ColorButton*>(ui->tableBevel->cellWidget(row, col));
                if (btn) btn->setLocked(false);
            }
        }
    }
    onControlChanged();
}

void LC_StyleEditorFusionSkin::onBevelSeedChanged(int index) {
    Q_UNUSED(index);
    if (ui->chkAutoCalc3D->isChecked()) {
        calculateProceduralBevels(m_currentVariantDark);
    }
    onControlChanged();
}

void LC_StyleEditorFusionSkin::calculateProceduralBevels(bool isDarkMode) {
    m_blockSignals = true;
    saveUiToConfig(isDarkMode); // Sync active UI states first

    ColorSchemeData& scheme = isDarkMode ? m_currentConfig.dark : m_currentConfig.light;

    QPalette tempPalette;
    for (const auto& roleMapping : BASE_INTERFACE_ROLES) {
        for (const auto& stateMapping : PALETTE_STATES) {
            QColor col = scheme.palette[roleMapping.name][stateMapping.name];
            tempPalette.setColor(stateMapping.group, roleMapping.role, col);
        }
    }

    auto calculatedBevel = BevelStyle::Soft;
    const auto archetype = static_cast<StyleArchetype>(ui->cbStyleArchetype->currentIndex());
    if (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline) {
        calculatedBevel = BevelStyle::Flat;
    } else if (archetype == StyleArchetype::SoftSatin) {
        calculatedBevel = BevelStyle::Soft;
    } else if (archetype == StyleArchetype::GlassyGloss) {
        calculatedBevel = BevelStyle::Sharp;
    }

    LC_PaletteColorUtils::calculate3DHelperRoles(
        tempPalette, calculatedBevel,
        static_cast<ContrastWeight>(ui->cbContrastWeight->currentData().toInt()),
        static_cast<QPalette::ColorRole>(ui->cbBevelSeed->currentData().toInt())
    );

    for (int row = 0; row < BEVEL_HELPER_ROLES.size(); ++row) {
        QString roleName = BEVEL_HELPER_ROLES[row].name;
        const QPalette::ColorRole role = BEVEL_HELPER_ROLES[row].role;
        for (int colIdx = 1; colIdx <= PALETTE_STATES.size(); ++colIdx) {
            auto* btn = qobject_cast<LC_ColorButton*>(ui->tableBevel->cellWidget(row, colIdx));
            if (btn) {
                const QPalette::ColorGroup group = PALETTE_STATES[colIdx - 1].group;
                QColor col = tempPalette.color(group, role);
                scheme.palette[roleName][PALETTE_STATES[colIdx - 1].name] = col;
                btn->setColor(col);
                btn->setLocked(true);
            }
        }
    }

    m_blockSignals = false;
}

void LC_StyleEditorFusionSkin::onUseThemeIconsToggled(bool checked) {
    ui->cbLinkedIconStyle->setEnabled(checked);
    onControlChanged();
}

void LC_StyleEditorFusionSkin::onArchetypeIndexChanged(int index) {
    if (index < 0) return;
    ui->lblStyleArchetypeDescription->setText(getArchetypeDescription(static_cast<StyleArchetype>(ui->cbStyleArchetype->currentData().toInt())));
    onControlChanged();
}

void LC_StyleEditorFusionSkin::onDecorationIndexChanged(int index) {
    if (index < 0) return;
    ui->lblCompositionDesc->setText(getDecorationDescription(static_cast<BoxDecoration>(ui->cbBoxDecoration->currentData().toInt())));
    onControlChanged();
}

void LC_StyleEditorFusionSkin::onGenerateHarmonizedTheme() {
    QColor baseColor = QColorDialog::getColor(Qt::blue, this, tr("Select Base Harmony Color"));
    if (!baseColor.isValid()) return;
    saveUiToConfig(m_currentVariantDark);
    LC_PaletteColorUtils::generateHarmonizedTheme(baseColor, m_currentConfig);
    loadConfigToUi(m_currentVariantDark);
}

void LC_StyleEditorFusionSkin::onGenerateTwoColorTheme() {
    QColor surfaceColor = QColorDialog::getColor(QColor(45, 45, 45), this, tr("Select Primary Surface/Canvas Color"));
    if (!surfaceColor.isValid()) return;

    QColor accentColor = QColorDialog::getColor(QColor(42, 130, 218), this, tr("Select Secondary Accent/Highlight Color"));
    if (!accentColor.isValid()) return;

    saveUiToConfig(m_currentVariantDark);
    LC_PaletteColorUtils::generateHarmonizedTheme(surfaceColor, accentColor, m_currentConfig);
    loadConfigToUi(m_currentVariantDark);
}

void LC_StyleEditorFusionSkin::onGenerateHighContrastTheme() {
    QColor baseColor = QColorDialog::getColor(Qt::blue, this, tr("Select Base Contrast Color"));
    if (!baseColor.isValid()) return;
    saveUiToConfig(m_currentVariantDark);
    LC_PaletteColorUtils::generateHighContrastTheme(baseColor, m_currentConfig);
    loadConfigToUi(m_currentVariantDark);
}

void LC_StyleEditorFusionSkin::updateGroupBoxUiState() {
    if (m_blockSignals) return;

    const bool customEnabled = ui->chkCustomGroupBox->isChecked();

    ui->cbGroupBoxHeaderStyle->setEnabled(customEnabled);
    ui->cbGroupBoxBoundaryStyle->setEnabled(customEnabled);
    ui->chkGroupBoxUseAccent->setEnabled(customEnabled);

    if (!customEnabled) return;

    m_blockSignals = true;

    const auto headerStyle = static_cast<GroupBoxHeaderStyle>(ui->cbGroupBoxHeaderStyle->currentData().toInt());
    const auto prevBoundary = static_cast<GroupBoxBoundaryStyle>(ui->cbGroupBoxBoundaryStyle->currentData().toInt());

    ui->cbGroupBoxBoundaryStyle->clear();

   if (headerStyle == GroupBoxHeaderStyle::Overlapping || headerStyle == GroupBoxHeaderStyle::BreakBorder) {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Top Line Only"), static_cast<int>(GroupBoxBoundaryStyle::TopLine));

        if (prevBoundary == GroupBoxBoundaryStyle::Full || prevBoundary == GroupBoxBoundaryStyle::TopLine) {
            ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary)));
        } else {
            ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(GroupBoxBoundaryStyle::Full)));
        }
    }
    else if (headerStyle == GroupBoxHeaderStyle::HeaderUnderline || headerStyle == GroupBoxHeaderStyle::HeaderBanner) {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("None (Frameless)"), static_cast<int>(GroupBoxBoundaryStyle::None));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(GroupBoxBoundaryStyle::LeftStripe));

        if (prevBoundary == GroupBoxBoundaryStyle::None ||
            prevBoundary == GroupBoxBoundaryStyle::Full ||
            prevBoundary == GroupBoxBoundaryStyle::LeftStripe) {
            ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary)));
        } else {
            ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(GroupBoxBoundaryStyle::None)));
        }
    }
    else {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("None (Frameless)"), static_cast<int>(GroupBoxBoundaryStyle::None));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(GroupBoxBoundaryStyle::LeftStripe));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Top Line Only"), static_cast<int>(GroupBoxBoundaryStyle::TopLine));

        ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary)));
    }
    m_blockSignals = false;
}

QString LC_StyleEditorFusionSkin::getArchetypeDescription(StyleArchetype archetype) const {
    switch (archetype) {
        case StyleArchetype::FlatModern:
            return tr("<b>Flat Modern</b>: Flat aesthetic with zero gradients, soft 1px outlines, and subtle responsive lighting. Highly minimal and modern.");
        case StyleArchetype::SoftSatin:
            return tr("<b>Soft Satin</b>: Gentle linear gradients and soft lighting transitions simulating an elegant satin-like depth.");
        case StyleArchetype::GlassyGloss:
            return tr("<b>Glassy Gloss</b>: Implements a soft, curved glossy reflection across control surfaces to mimic a polished, glassy effect.");
        case StyleArchetype::AccentOutline:
            return tr("<b>Accent Outline</b>: Transparent body framed by low-contrast outlines, with vibrant highlights providing visual structure.");
        case StyleArchetype::ClassicFusion:
            default:
            return tr("<b>Classic Fusion</b>: Restores standard native Qt Fusion chiseled bevel geometries, 3D borders, and layout styles.");
    }
}

QString LC_StyleEditorFusionSkin::getDecorationDescription(BoxDecoration dec) const {
    switch (dec) {
        case BoxDecoration::Frameless:
            return tr("<b>Frameless</b>: Flat container panels with zero border lines or highlight separators. Highly minimal.");
        case BoxDecoration::DividingHairline:
            return tr("<b>Dividing Hairline</b>: Draws a flat 1px bottom line in flat/accent styles, and dynamically upgrades to a chiseled 3D shadow in traditional styles.");
        case BoxDecoration::BoxOutline:
            return tr("<b>Box Outline Frame</b>: Complete flat 1px bounding outline frame wrapping around the widget bounds. Excellent for defining structural panel boundaries.");
        case BoxDecoration::RecessedWell:
            return tr("<b>Recessed Well</b>: Simulates an inset groove. Draws a soft inner-offset frame in flat styles, and a classic chiseled 3D shadow inset in traditional styles.");
        case BoxDecoration::LeftAccentBar:
            return tr("<b>Active Left Sidebar</b>: Left vertical 3px active accent indicator sidebar. Excellent for structured CAD sidebars and tool docks.");
        case BoxDecoration::ActiveAccentFrame:
            return tr("<b>Active Accent Frame</b>: Highlights the entire container panel with a 1px border colored with the active highlight accent color.");
    }
    return QString();
}

QString LC_StyleEditorFusionSkin::getRoleTooltip(QPalette::ColorRole role) const {
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

void LC_StyleEditorFusionSkin::updateSegmentedButtonsUiState() {
    if (m_blockSignals) return;
    const bool enabled = ui->chkUseSegmentedButtons->isChecked();
    ui->lbSegmentedSeparatorStyle->setEnabled(enabled);
    ui->cbSegmentedSeparationStyle->setEnabled(enabled);
    ui->lblSegmentedColorPolicy->setEnabled(enabled);
    ui->cbSegmentedColorPolicy->setEnabled(enabled);
}

void LC_StyleEditorFusionSkin::updateSplitterUiState() {
    if (m_blockSignals) return;

    const bool customEnabled = ui->chkCustomSplitterGrip->isChecked();

    ui->cbSplitterGripStyle->setEnabled(customEnabled);
    ui->chkHighlightSplitterOnDrag->setEnabled(customEnabled);
    ui->chkShowGripBackgroundWell->setEnabled(customEnabled);
    ui->chkAccentGrips->setEnabled(customEnabled);
    ui->lblSplitterGripStyle->setEnabled(customEnabled);

    ui->chkPersistentDockSplitter->setEnabled(customEnabled);
}

void LC_StyleEditorFusionSkin::updateToolButtonUiState() {
    if (m_blockSignals) return;

    const bool customEnabled = ui->chkToolButtonUnderline->isChecked();
    ui->cbToolButtonIndicatorStyle->setEnabled(customEnabled);
}
