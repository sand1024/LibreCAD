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

#include "lc_style_editor_metrics.h"
#include "ui_lc_style_editor_metrics.h"
#include "lc_style_metrics_utils.h"
#include "lc_ui_style_manager.h"
#include "lc_metrics_repository.h"
#include "lc_proxy_style.h"
#include "lc_fusion_skins_repository.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QStyleFactory>
#include <QVBoxLayout>

LC_StyleEditorMetrics::LC_StyleEditorMetrics(QWidget* parent, LC_UIStyleManager* styleManager)
    : LC_StyleEditorBase(parent, styleManager, styleManager->getMetricsRepository())
    , ui(new Ui::LC_StyleEditorMetrics) {
    ui->setupUi(this);

    ui->cbWorkspaceDensity->addItem(tr("Compact (CAD Optimal)"), "compact");
    ui->cbWorkspaceDensity->addItem(tr("Standard (Default)"), "standard");
    ui->cbWorkspaceDensity->addItem(tr("Spacious"), "spacious");
    ui->cbWorkspaceDensity->addItem(tr("Custom"), "custom");

    ui->cbMnemonicUnderlineMode->clear();
    ui->cbMnemonicUnderlineMode->addItem(tr("Hold ALT Key"), static_cast<int>(MnemonicUnderlineMode::PressAndHold));
    ui->cbMnemonicUnderlineMode->addItem(tr("Tap ALT to Toggle"), static_cast<int>(MnemonicUnderlineMode::StickyToggle));

    ui->cbDragCursorStyle->addItem(tr("Standard Arrow"), static_cast<int>(DragCursorStyle::StandardArrow));
    ui->cbDragCursorStyle->addItem(tr("Open Hand"), static_cast<int>(DragCursorStyle::OpenHand));
    ui->cbDragCursorStyle->addItem(tr("4-Way Move Arrows"), static_cast<int>(DragCursorStyle::SizeAll));

    ui->sbDockButtonMargin->setMinimum(-1);  // replaces -1 with text
    ui->sbDockButtonMargin->setSpecialValueText(tr("Auto-Center"));

    ui->sbScrollbarWidth->setMinimum(-1);
    ui->sbScrollbarWidth->setSpecialValueText(tr("System Default"));

    ui->sbDockTitleHeight->setMinimum(-1);
    ui->sbDockTitleHeight->setSpecialValueText(tr("Font-Adaptive"));

    ui->sbTreeBranchIndicatorSize->setMinimum(-1);
    ui->sbTreeBranchIndicatorSize->setSpecialValueText(tr("Auto-Scale"));

    ui->sbSplitterHandleLength->setMinimum(-1);
    ui->sbSplitterHandleLength->setSpecialValueText(tr("Full Length"));

    setupConnections();
}

LC_StyleEditorMetrics::~LC_StyleEditorMetrics() {
    delete ui;
}

void LC_StyleEditorMetrics::setupConnections() {
    // Reflective Wiring: search and bind all input controls recursively
    const QList<QSpinBox*> spinBoxes = findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    }

    const QList<QCheckBox*> checkBoxes = findChildren<QCheckBox*>();
    for (QCheckBox* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_StyleEditorMetrics::onControlChanged);
    }

    connect(ui->cbWorkspaceDensity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorMetrics::onDensityPresetChanged);
    connect(ui->sbGroupBoxTitleLeftPadding, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    connect(ui->sbGroupBoxTitleLineGap, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    connect(ui->sbTreeBranchIndicatorSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    connect(ui->cbMnemonicUnderlineMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    connect(ui->sbSplitterHandleLength, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_StyleEditorMetrics::onControlChanged);
    connect(ui->cbDragCursorStyle, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LC_StyleEditorMetrics::onControlChanged);
}

void LC_StyleEditorMetrics::onControlChanged() {
    if (m_blockSignals) return;

    // If the user manually changes any metric setting, force the preset combobox to "Custom"
    m_blockSignals = true;
    int customIdx = ui->cbWorkspaceDensity->findData("custom");
    if (customIdx >= 0) {
        ui->cbWorkspaceDensity->setCurrentIndex(customIdx);
    }
    m_blockSignals = false;

    notifyChanged(true);
}

void LC_StyleEditorMetrics::onDensityPresetChanged(int index) {
    if (m_blockSignals || index < 0) return;

    QString preset = ui->cbWorkspaceDensity->itemData(index).toString();
    if (preset == "custom") return;

    m_blockSignals = true;

    StyleMetricsConfig tempConfig;
    LC_StyleMetricsUtils::applyDensityPreset(tempConfig, preset);
    loadConfigToUi(tempConfig);

    m_blockSignals = false;
    notifyChanged(true);
}

bool LC_StyleEditorMetrics::doLoadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY) {
        LC_StyleMetricsUtils::initializeDefault(m_currentConfig);
    } else {
        m_styleManager->getMetricsRepository()->loadByKey(key, m_currentConfig);
    }

    m_blockSignals = true;
    loadConfigToUi(m_currentConfig);
    // Default load leaves the preset bar on custom unless a perfect preset match is implemented
    ui->cbWorkspaceDensity->setCurrentIndex(ui->cbWorkspaceDensity->findData("custom"));

    ui->sbTreeBranchIndicatorSize->setValue(m_currentConfig.treeBranchIndicatorSize);


    m_blockSignals = false;
    return true;
}

bool LC_StyleEditorMetrics::saveCurrentPreset() {
    m_currentConfig = getConfigFromUi();
    QString outKey;
    return m_styleManager->getMetricsRepository()->save(m_currentConfig.name, m_currentConfig, outKey);
}

bool LC_StyleEditorMetrics::doSavePresetAs(const QString& name, QString& outKey) {
    m_currentConfig = getConfigFromUi();
    m_currentConfig.name = name;
    return m_styleManager->getMetricsRepository()->save(name, m_currentConfig, outKey);
}

QString LC_StyleEditorMetrics::getActivePresetKey() const {
    return m_styleManager->getActiveMetrics();
}

QList<QPair<QString, QString>> LC_StyleEditorMetrics::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default Fusion Metrics"), DEFAULT_THEME_KEY));
    choices.append(m_styleManager->getMetricsRepository()->getPresetChoices());
    return choices;
}

void LC_StyleEditorMetrics::applyTransientState(QWidget* previewWindow) const {
    StyleMetricsConfig tempConfig = getConfigFromUi();

    // Re-instantiate local style proxy on the preview frame to avoid background leaks
    QStyle* baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle, tempConfig);

    // Read the active skin's aesthetic options to prevent rendering visual breaks
    SkinConfig activeSkin;
    if (m_styleManager->getSkinsRepository()->loadByKey(m_styleManager->getActiveSkin(), activeSkin)) {
        proxyStyle->setSkin(activeSkin);
    }
    previewWindow->setStyle(proxyStyle);

    // Propagate the local proxy recursively to child widgets
    const QList<QWidget*> children = previewWindow->findChildren<QWidget*>();
    for (QWidget* child : children) {
        child->setStyle(proxyStyle);
        child->updateGeometry();
        child->update();
    }

    // Force layouts calculation
    if (previewWindow->layout()) {
        previewWindow->layout()->invalidate();
        previewWindow->layout()->activate();
    }
    previewWindow->update();
}

void LC_StyleEditorMetrics::applyCurrentPreset() {
    m_styleManager->setActiveMetrics(m_currentPresetKey);
    m_styleManager->applyActiveStyleAndTheme(); // Apply modifications globally to the application workspace
}

void LC_StyleEditorMetrics::rollbackState() {
    // Symmetrical rollback discards the current modifications
}

void LC_StyleEditorMetrics::loadConfigToUi(const StyleMetricsConfig& config) const {
    ui->sbScrollbarWidth->setValue(config.scrollBarWidth);
    ui->sbTreeIndentation->setValue(config.treeIndentation);
    ui->sbButtonPadding->setValue(config.buttonPadding);
    ui->sbRowPadding->setValue(config.itemViewRowPadding);
    ui->sbScrollbarMinLength->setValue(config.scrollBarMinLength);
    ui->sbSplitterWidth->setValue(config.splitterWidth);
    ui->sbDockTitleHeight->setValue(config.dockTitleBarHeight);

    ui->sbTabHSpace->setValue(config.tabBarTabHSpace);
    ui->sbTabVSpace->setValue(config.tabBarTabVSpace);
    ui->sbTabOverlap->setValue(config.tabBarTabBaseOverlap);
    ui->sbDockButtonMargin->setValue(config.dockWidgetTitleBarButtonMargin);
    ui->sbSliderThickness->setValue(config.sliderControlThickness);
    ui->sbFocusHMargin->setValue(config.focusFrameHMargin);
    ui->sbFocusVMargin->setValue(config.focusFrameVMargin);
    ui->sbLayoutMargin->setValue(config.layoutMargin);
    ui->sbLayoutSpacing->setValue(config.layoutSpacing);

    ui->sbToolbarItemSpacing->setValue(config.toolbarItemSpacing);
    ui->sbToolbarSeparatorWidth->setValue(config.toolbarSeparatorWidth);
    ui->sbMenuBarItemSpacing->setValue(config.menuBarItemSpacing);
    ui->sbMenuBarVerticalMargin->setValue(config.menuBarVerticalMargin);
    ui->sbIndicatorBoxSize->setValue(config.indicatorBoxSize);
    ui->sbIndicatorLabelSpacing->setValue(config.indicatorLabelSpacing);
    ui->sbTitleBarButtonSize->setValue(config.titleBarButtonSize);
    ui->sbSubMenuOverlap->setValue(config.subMenuOverlap);

    ui->chkFlatDisabledText->setChecked(config.flatDisabledText);
    ui->chkHideShortcuts->setChecked(config.hideShortcutUnderlines);
    ui->chkMenuIcons->setChecked(config.menuSupportsIcons);
    ui->chkMenuDisabledActive->setChecked(config.menuAllowActiveAndDisabled);

    ui->sbTabOverlapHorizontal->setValue(config.tabBarTabOverlap);
    ui->sbTabCloseSize->setValue(config.tabCloseIndicatorSize);
    ui->sbMenuVerticalPadding->setValue(config.menuVerticalPadding);
    ui->sbMenuHorizontalPadding->setValue(config.menuHorizontalPadding);
    ui->sbMenuBorderWidth->setValue(config.menuBorderWidth);
    ui->sbHeaderDefaultHeight->setValue(config.headerDefaultHeight);
    ui->sbSliderHandleLength->setValue(config.sliderHandleLength);

    ui->sbGroupBoxTitleLeftPadding->setValue(m_currentConfig.groupBoxTitleLeftPadding);
    ui->sbGroupBoxTitleLineGap->setValue(m_currentConfig.groupBoxTitleLineGap);


    ui->cbMnemonicUnderlineMode->setCurrentIndex(
    ui->cbMnemonicUnderlineMode->findData(static_cast<int>(m_currentConfig.mnemonicUnderlineMode))
       );

    ui->sbSplitterHandleLength->setValue(m_currentConfig.splitterHandleLength);

    int cursorIndex = ui->cbDragCursorStyle->findData(static_cast<int>(m_currentConfig.dragCursorStyle));
    ui->cbDragCursorStyle->setCurrentIndex(cursorIndex != -1 ? cursorIndex : 1);

    ui->sbScrollbarWidth->setValue(m_currentConfig.scrollBarWidth);
    ui->sbSplitterHandleLength->setValue(m_currentConfig.splitterHandleLength);
    ui->sbTreeBranchIndicatorSize->setValue(m_currentConfig.treeBranchIndicatorSize);
    ui->sbDockTitleHeight->setValue(m_currentConfig.dockTitleBarHeight);
}

StyleMetricsConfig LC_StyleEditorMetrics::getConfigFromUi() const {
    StyleMetricsConfig config = m_currentConfig;

    config.scrollBarWidth = ui->sbScrollbarWidth->value();
    config.treeIndentation = ui->sbTreeIndentation->value();
    config.buttonPadding = ui->sbButtonPadding->value();
    config.itemViewRowPadding = ui->sbRowPadding->value();
    config.scrollBarMinLength = ui->sbScrollbarMinLength->value();
    config.splitterWidth = ui->sbSplitterWidth->value();
    config.dockTitleBarHeight = ui->sbDockTitleHeight->value();

    config.tabBarTabHSpace = ui->sbTabHSpace->value();
    config.tabBarTabVSpace = ui->sbTabVSpace->value();
    config.tabBarTabBaseOverlap = ui->sbTabOverlap->value();
    config.dockWidgetTitleBarButtonMargin = ui->sbDockButtonMargin->value();
    config.sliderControlThickness = ui->sbSliderThickness->value();
    config.focusFrameHMargin = ui->sbFocusHMargin->value();
    config.focusFrameVMargin = ui->sbFocusVMargin->value();
    config.layoutMargin = ui->sbLayoutMargin->value();
    config.layoutSpacing = ui->sbLayoutSpacing->value();

    config.toolbarItemSpacing = ui->sbToolbarItemSpacing->value();
    config.toolbarSeparatorWidth = ui->sbToolbarSeparatorWidth->value();
    config.menuBarItemSpacing = ui->sbMenuBarItemSpacing->value();
    config.menuBarVerticalMargin = ui->sbMenuBarVerticalMargin->value();
    config.indicatorBoxSize = ui->sbIndicatorBoxSize->value();
    config.indicatorLabelSpacing = ui->sbIndicatorLabelSpacing->value();
    config.titleBarButtonSize = ui->sbTitleBarButtonSize->value();
    config.subMenuOverlap = ui->sbSubMenuOverlap->value();

    config.flatDisabledText = ui->chkFlatDisabledText->isChecked();
    config.hideShortcutUnderlines = ui->chkHideShortcuts->isChecked();
    config.menuSupportsIcons = ui->chkMenuIcons->isChecked();
    config.menuAllowActiveAndDisabled = ui->chkMenuDisabledActive->isChecked();

    config.tabBarTabOverlap = ui->sbTabOverlapHorizontal->value();
    config.tabCloseIndicatorSize = ui->sbTabCloseSize->value();
    config.menuVerticalPadding = ui->sbMenuVerticalPadding->value();
    config.menuHorizontalPadding = ui->sbMenuHorizontalPadding->value();
    config.menuBorderWidth = ui->sbMenuBorderWidth->value();
    config.headerDefaultHeight = ui->sbHeaderDefaultHeight->value();
    config.sliderHandleLength = ui->sbSliderHandleLength->value();

    config.groupBoxTitleLeftPadding = ui->sbGroupBoxTitleLeftPadding->value();
    config.groupBoxTitleLineGap = ui->sbGroupBoxTitleLineGap->value();

    config.treeBranchIndicatorSize = ui->sbTreeBranchIndicatorSize->value();

    config.mnemonicUnderlineMode = static_cast<MnemonicUnderlineMode>(
      ui->cbMnemonicUnderlineMode->currentData().toInt()
  );

    config.splitterHandleLength      = ui->sbSplitterHandleLength->value();

    config.dragCursorStyle = static_cast<DragCursorStyle>(ui->cbDragCursorStyle->currentData().toInt());

    return config;
}
