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

#include "lc_settings_manager_styling.h"

#include <QObject>
#include <QObject>

#include "lc_preset_manager_fusion_skin.h"
#include "lc_preset_manager_icons_style.h"
#include "lc_preset_manager_metrics.h"
#include "lc_preset_manager_palette.h"
#include "lc_preset_manager_typography.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_dialog.h"
#include "lc_settings_page_fusion_theme.h"
#include "lc_settings_page_general_styling.h"
#include "lc_settings_page_icons_style.h"
#include "lc_settings_page_metrics_behavior.h"
#include "lc_settings_page_metrics_controls.h"
#include "lc_settings_page_metrics_layout.h"
#include "lc_settings_page_metrics_menus_toolbars.h"
#include "lc_settings_page_metrics_views_tabs.h"
#include "lc_settings_page_skin_containers.h"
#include "lc_settings_page_skin_controls.h"
#include "lc_settings_page_skin_palette.h"
#include "lc_settings_page_skin_toolbars_menus.h"
#include "lc_settings_page_toolbars_and_docks.h"
#include "lc_settings_page_typography.h"
#include "lc_settings_page_workspace_profiles.h"
#include "lc_settings_registry.h"
#include "lc_styling_preview_controller.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

void LC_SettingsManagerStyling::initializeStylingSettings() {
     using namespace LC_SettingsPagesStyling;

    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = DLG_STYLING_PREFERENCES;

    reg->configureDialog(targetDialog, {QObject::tr("Application Styling"), true, true});

    // 1. Preset Managers Registration
    reg->registerPresetManager(targetDialog, PAGE_STYLING_TYPOGRAPHY, []() {
        return std::make_unique<LC_PresetManagerTypography>(nullptr);
    });

    reg->registerPresetManager(targetDialog, PAGE_STYLING_ICONS, []() {
        return std::make_unique<LC_PresetManagerIconsStyle>(nullptr);
    });

    reg->registerPresetManager(targetDialog, PAGE_STYLING_SKINS, []() {
        return std::make_unique<LC_PresetManagerFusionSkin>(nullptr);
    });

    reg->registerPresetManager(targetDialog, PAGE_STYLING_METRICS, []() {
        return std::make_unique<LC_PresetManagerMetrics>(nullptr);
    });

    reg->registerPresetManager(targetDialog, PAGE_STYLING_PALETTE, []() {
        return std::make_unique<LC_PresetManagerPalette>();
    });

    // 2. Settings Pages Registrations
    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {
     { PAGE_STYLING_GENERAL, "", page<LC_SettingsPageGeneralStyling>(), 10 },
        { PAGE_STYLING_WIDGETS, "", page<LC_SettingsPageToolbarsAndDocks>(), 15 },
        { PAGE_STYLING_TYPOGRAPHY, "", page<LC_SettingsPageTypography>(), 30 },
        { PAGE_STYLING_ICONS, "", page<LC_SettingsPageIconsStyle>(), 40 },
        { PAGE_STYLING_FUSION, "", page<LC_SettingsPageFusionTheme>(), 50 },
        { PAGE_STYLING_PALETTE, PAGE_STYLING_FUSION, page<LC_SettingsPageSkinPalette>(), 10 },
        { PAGE_STYLING_SKINS, PAGE_STYLING_FUSION,
        indexNoPreview(QObject::tr("Controls Style & Decorators"),
         QObject::tr("Customize visual archetypes, container outlines, dock frames, toolbuttons, and widgets.")),
           20 },
        { PAGE_STYLING_SKINS_CONTAINERS, PAGE_STYLING_SKINS, page<LC_SettingsPageSkinContainers>(), 10 },
        { PAGE_STYLING_SKINS_TOOLBARS_MENUS, PAGE_STYLING_SKINS, page<LC_SettingsPageSkinToolbarsMenus>(), 20 },
        { PAGE_STYLING_SKINS_CONTROLS, PAGE_STYLING_SKINS, page<LC_SettingsPageSkinControls>(), 30 },
        { PAGE_STYLING_METRICS, PAGE_STYLING_FUSION, indexNoPreview(QObject::tr("UI Metrics (Sizes)"),
                QObject::tr("Configure layout margins, paddings, densities, scrollbars, tabs, and menu metrics.")),
          20 },
        { PAGE_STYLING_METRICS_LAYOUT, PAGE_STYLING_METRICS, page<LC_SettingsPageMetricsLayout>(), 10 },
        { PAGE_STYLING_METRICS_MENUS_TOOLBARS, PAGE_STYLING_METRICS, page<LC_SettingsPageMetricsMenusToolbars>(), 20 },
        { PAGE_STYLING_METRICS_CONTROLS, PAGE_STYLING_METRICS, page<LC_SettingsPageMetricsControls>(), 30 },
        { PAGE_STYLING_METRICS_VIEWS_TABS, PAGE_STYLING_METRICS, page<LC_SettingsPageMetricsViewsTabs>(), 40 },
        { PAGE_STYLING_METRICS_BEHAVIOR, PAGE_STYLING_METRICS, page<LC_SettingsPageMetricsBehavior>(), 50 },
        { PAGE_STYLING_PROFILES, "", page<LC_SettingsPageWorkspaceProfiles>(), 120 }
    };

    reg->registerPages(targetDialog, pages);
}

bool LC_SettingsManagerStyling::showStylingSettings(QWidget* parent, const QString& initialPageId) {
    auto previewController = std::make_unique<LC_StylingPreviewController>();

      auto* styleMgr = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();

    if (styleMgr != nullptr) {
        previewController->initFromStyleManager(styleMgr);
    }

    auto preExecHook = [&previewController](LC_SettingsDialog* dialog) {
        previewController->setDialogParent(dialog);
        dialog->forEachPresetManager([&previewController](LC_PresetManagerInterface* manager) {
            if (auto* aware = dynamic_cast<LC_StylingPreviewAware*>(manager)) {
                aware->setPreviewController(previewController.get());
            }
        });

        // Wire standalone preview-aware pages
        dialog->forEachPage([&previewController](LC_SettingsPageInterface* page) {
            if (auto* aware = dynamic_cast<LC_StylingPreviewAware*>(page)) {
                aware->setPreviewController(previewController.get());
            }
        });

        QObject::connect(dialog, &LC_SettingsDialog::categoryChanged, previewController.get(),
                         [&previewController, dialog](const QString& /*pageId*/) {
                             auto* page = dialog->activePage();
                             previewController->onCategoryChanged(page);
                         });
    };;
;

    auto postExecHook = [&previewController](LC_SettingsDialog* dialog, bool /*accepted*/) {
        dialog->forEachPresetManager([](LC_PresetManagerInterface* manager) {
            if (auto* aware = dynamic_cast<LC_StylingPreviewAware*>(manager)) {
                aware->setPreviewController(nullptr);
            }
        });
        previewController->closePreview();
    };

    const bool accepted = LC_SettingsRegistry::instance()->showDialog(LC_SettingsPagesStyling::DLG_STYLING_PREFERENCES, initialPageId, parent, preExecHook,
                                                                      postExecHook);

    // Re-apply style and theme once to reflect either committed changes (OK) or rolled-back baseline (Cancel)
    if (styleMgr != nullptr) {
        styleMgr->applyActiveStyleAndTheme();
    }
    if (parent != nullptr) {
        parent->update();
    }

    return accepted;
}

bool LC_SettingsManagerStyling::isFusionGated() {
    return !(CFG_AppStyling::o_AllowStyle.get() && CFG_AppStyling::o_Style.get() == "Fusion");
}

QString LC_SettingsManagerStyling::fusionGatedMessage(const QString& domainName) {
    if (!CFG_AppStyling::o_AllowStyle.get()) {
        return domainName.isEmpty()
                   ? QObject::tr("Custom UI styling is currently disabled. Enable styling and Fusion theme to customize skins and metrics.")
                   : QObject::tr("Custom UI styling is currently disabled. Enable styling and Fusion theme to customize %1.").arg(
                       domainName);
    }
    return domainName.isEmpty()
               ? QObject::tr("Fusion customization requires the 'Fusion' style (currently '%1').").arg(CFG_AppStyling::o_Style.get())
               : QObject::tr("%1 customization requires the 'Fusion' style (currently '%2').").arg(
                   domainName, CFG_AppStyling::o_Style.get());
}

QString LC_SettingsManagerStyling::fusionGatedActionText() {
    return QObject::tr("Enable Fusion Styling");
}

void LC_SettingsManagerStyling::enableFusionStyling(LC_UIStyleManager* styleMgr) {
    CFG_AppStyling::o_AllowStyle.set(true);
    CFG_AppStyling::o_Style.set("Fusion");

    auto* mgr = styleMgr
                    ? styleMgr
                    : (QC_ApplicationWindow::getAppWindow() ? QC_ApplicationWindow::getAppWindow()->getUiStyleManager() : nullptr);
    if (mgr != nullptr) {
        mgr->setStyleAllowed(true);
        mgr->setActiveStyle("Fusion");
        mgr->applyActiveStyleAndTheme();
    }
}
