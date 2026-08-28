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

#include "lc_settings_page_metrics_menus_toolbars.h"
#include "ui_lc_settings_page_metrics_menus_toolbars.h"
#include "lc_preset_manager_metrics.h"
#include <QSpinBox>

LC_SettingsPageMetricsMenusToolbars::LC_SettingsPageMetricsMenusToolbars(QObject* parent)
    : LC_SettingsPageBase(tr("Menus & Toolbars"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageMetricsMenusToolbars>()) {
    setSortWeight(20);
}

LC_SettingsPageMetricsMenusToolbars::~LC_SettingsPageMetricsMenusToolbars() = default;

void LC_SettingsPageMetricsMenusToolbars::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMetrics*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerMetrics::configLoaded, this, [this](const StyleMetricsConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageMetricsMenusToolbars::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageMetricsMenusToolbars::setupBehavior() {
    const QList<QSpinBox*> spinBoxes = m_widget->findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LC_SettingsPageMetricsMenusToolbars::onControlChanged);
    }
}

void LC_SettingsPageMetricsMenusToolbars::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageMetricsMenusToolbars::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageMetricsMenusToolbars::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageMetricsMenusToolbars::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageMetricsMenusToolbars::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->sbMenuBarItemSpacing->setValue(config.menuBarItemSpacing);
    ui->sbMenuBarVerticalMargin->setValue(config.menuBarVerticalMargin);
    ui->sbMenuVerticalPadding->setValue(config.menuVerticalPadding);
    ui->sbMenuHorizontalPadding->setValue(config.menuHorizontalPadding);
    ui->sbMenuBorderWidth->setValue(config.menuBorderWidth);
    ui->sbSubMenuOverlap->setValue(config.subMenuOverlap);
    ui->sbToolbarItemSpacing->setValue(config.toolbarItemSpacing);
    ui->sbToolbarSeparatorWidth->setValue(config.toolbarSeparatorWidth);

    m_blockSignals = false;
}

void LC_SettingsPageMetricsMenusToolbars::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.menuBarItemSpacing = ui->sbMenuBarItemSpacing->value();
    config.menuBarVerticalMargin = ui->sbMenuBarVerticalMargin->value();
    config.menuVerticalPadding = ui->sbMenuVerticalPadding->value();
    config.menuHorizontalPadding = ui->sbMenuHorizontalPadding->value();
    config.menuBorderWidth = ui->sbMenuBorderWidth->value();
    config.subMenuOverlap = ui->sbSubMenuOverlap->value();
    config.toolbarItemSpacing = ui->sbToolbarItemSpacing->value();
    config.toolbarSeparatorWidth = ui->sbToolbarSeparatorWidth->value();
}
