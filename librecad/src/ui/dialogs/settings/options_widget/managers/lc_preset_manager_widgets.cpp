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

#include "lc_preset_manager_widgets.h"

#include <QFont>
#include <QStatusBar>

#include "lc_settings_widget.h"
#include "lc_widget_factory.h"
#include "qc_applicationwindow.h"

LC_PresetManagerWidgets::LC_PresetManagerWidgets(LC_UIStyleManager* styleManager)
    : LC_PresetManagerStylingBase<LC_WidgetsConfig, LC_RepositoryWidgets>(
          styleManager,
          styleManager != nullptr ? styleManager->getWidgetsRepository() : nullptr,
          styleManager != nullptr ? styleManager->getActiveWidgetsScheme() : QString(),
          nullptr) {
    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerWidgets::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Layout");
    s.labelText = tr("Widgets & Toolbars preset:");
    s.selectToolTip = tr("Select a toolbars and docking layout configuration preset.");
    s.saveToolTip = tr("Save changes directly to active preset.");
    s.saveAsToolTip = tr("Save current toolbars and docking layout as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected preset from disk.");
    s.applyToolTip = tr("Apply the active toolbars and docking layout globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Toolbars & Docking Preset As");
    s.saveAsDialogLabel = tr("Enter unique preset name:");
    s.defaultNewPresetName = tr("Custom Layout");
    s.deleteConfirmTitle = tr("Delete Toolbars & Docking Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the preset '%1'?");
    s.presetFileFilter = tr("LibreCAD Widgets Config Files (*%1);All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".lcwc"));

    s.defaultReadOnlyMessage = tr(
        "The Default layout preset is a read-only template. To customize widget sizes and styles, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Layout...");
    return s;
}

QString LC_PresetManagerWidgets::getAppliedPresetKey() const {
    if (m_styleManager != nullptr) {
        return m_styleManager->getActiveWidgetsScheme();
    }
    return m_originalActiveKey;
}

void LC_PresetManagerWidgets::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewToolbarsAndDocks();
    }
}

void LC_PresetManagerWidgets::resetToDefaults(LC_WidgetsConfig& config) {
    LC_WidgetsConfigUtils::initializeDefaultConfig(config);
}

void LC_PresetManagerWidgets::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveWidgetsScheme(activeKey);
    }

    // Deferred commit: Write in-memory working config fields directly to CFG_Widgets
    using namespace CFG_Widgets;
    o_ToolbarAllowIconSize = m_workingConfig.toolbarAllowIconSize;
    o_ToolbarIconSize = m_workingConfig.toolbarIconSize;
    o_PickValueButtonsFlatIcons = m_workingConfig.pickValueButtonsFlatIcons;

    o_DockWidgetsFlatButtons = m_workingConfig.dockWidgetsFlatButtons;
    o_DockWidgetsIconSize = m_workingConfig.dockWidgetsIconSize;
    o_DockTabDisplayMode = m_workingConfig.dockTabDisplayMode;
    o_DockTabOverrideIconSize = m_workingConfig.dockTabOverrideIconSize;
    o_DockTabIconSize = m_workingConfig.dockTabIconSize;
    o_DockWidgetTitleBarVertical = m_workingConfig.dockTitleBarVertical;
    o_DockTabVertical = m_workingConfig.dockTabVertical;

    o_CADDockWidgetFlatButtons = m_workingConfig.cadDockWidgetFlatButtons;
    o_CADDockWidgetIconSize = m_workingConfig.cadDockWidgetIconSize;
    o_CADDockWidgetColumnsCount = m_workingConfig.cadDockWidgetColumnsCount;
    o_CADDockTabDisplayMode = m_workingConfig.cadDockTabDisplayMode;
    o_CADDockTabOverrideIconSize = m_workingConfig.cadDockTabOverrideIconSize;
    o_CADDockTabIconSize = m_workingConfig.cadDockTabIconSize;
    o_CADDockWidgetTitleBarVertical = m_workingConfig.cadDockTitleBarVertical;
    o_CADDockTabVertical = m_workingConfig.cadDockVerticalTabs;

    o_CADToolsMatrixFlatButtons = m_workingConfig.cadToolsMatrixFlatButtons;
    o_CADToolsMatrixIconSize = m_workingConfig.cadToolsMatrixIconSize;
    o_CADToolsMatrixColumnsCount = m_workingConfig.cadToolsMatrixColumnsCount;

    o_DockAllowNested = m_workingConfig.dockAllowNested;

    o_StatusBarAllowHeight = m_workingConfig.allowStatusbarHeight;
    o_StatusbarHeight = m_workingConfig.statusbarHeight;
    o_StatusBarAllowFontSize = m_workingConfig.allowStatusbarFontSize;
    o_StatusbarFontSize = m_workingConfig.statusbarFontSize;

    // Apply live updates to main window widgets
    auto* appWindow = QC_ApplicationWindow::getAppWindow();
    if (appWindow != nullptr) {
        appWindow->updateToolbarsIconSize(m_workingConfig.toolbarAllowIconSize,
                                          m_workingConfig.toolbarIconSize);

        if (m_workingConfig.allowStatusbarFontSize) {
            QFont font;
            font.setPointSize(m_workingConfig.statusbarFontSize);
            appWindow->statusBar()->setFont(font);
        }
        if (m_workingConfig.allowStatusbarHeight) {
            appWindow->statusBar()->setMinimumHeight(m_workingConfig.statusbarHeight);
        }

        LC_WidgetFactory::updateDockOptions(appWindow,
                                            m_workingConfig.dockAllowNested,
                                            m_workingConfig.cadDockVerticalTabs,
                                            m_workingConfig.dockTabVertical);

        LC_WidgetFactory::updateDockWidgetsTitleBarType(appWindow,
                                                       m_workingConfig.cadDockTitleBarVertical,
                                                       m_workingConfig.dockTitleBarVertical);

        appWindow->fireWidgetSettingsChanged();
    }
}

void LC_PresetManagerWidgets::emitConfigLoaded() {
    emit configLoaded(m_workingConfig);
}
