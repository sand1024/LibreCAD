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

#include "lc_index_settings_page.h"
#include "lc_preset_manager_fusion_skin.h"
#include "lc_preset_manager_icons_style.h"
#include "lc_preset_manager_metrics.h"
#include "lc_preset_manager_typography.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_dialog.h"
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
#include "lc_settings_page_skin_qss.h"
#include "lc_settings_page_skin_toolbars_menus.h"
#include "lc_settings_page_typography.h"
#include "lc_settings_page_workspace_profiles.h"
#include "lc_settings_registry.h"
#include "lc_styling_preview_controller.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

void LC_SettingsManagerStyling::initializeStylingSettings() {
    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = "ui_styling_preferences";

    reg->configureDialog(targetDialog, QObject::tr("Application Styling & Theming"), true);

    // 1. Typography Preset Manager Registration
    reg->registerPresetManager(targetDialog, "styling.typography", []() {
        return std::make_unique<LC_PresetManagerTypography>();
    });

    reg->registerPresetManager(targetDialog, "styling.icons", []() {
        return std::make_unique<LC_PresetManagerIconsStyle>();
    });

    reg->registerPresetManager(targetDialog, "styling.metrics", []() {
        return std::make_unique<LC_PresetManagerMetrics>();
    });

    reg->registerPresetManager(targetDialog, "styling.skins", []() {
        return std::make_unique<LC_PresetManagerFusionSkin>();
    });

    // 2. Fusion Gating Condition & Callbacks for the Root Fusion Index Page
    auto isFusionGated = []() {
        return !(CFG_AppStyling::o_AllowStyle.get() && CFG_AppStyling::o_Style.get() == "Fusion");
    };

    auto fusionGatedMsg = []() {
        if (!CFG_AppStyling::o_AllowStyle.get()) {
            return QObject::tr("Custom UI styling is currently disabled. Enable styling and Fusion theme to customize skins and metrics.");
        }
        return QObject::tr("Fusion customization requires the 'Fusion' style (currently '%1').").arg(CFG_AppStyling::o_Style.get());
    };

    auto fusionGatedActionText = QObject::tr("Enable Fusion Styling");

    auto fusionGatedActionCb = []() {
        CFG_AppStyling::o_AllowStyle.set(true);
        CFG_AppStyling::o_Style.set("Fusion");
        auto* styleMgr = QC_ApplicationWindow::getAppWindow() ? QC_ApplicationWindow::getAppWindow()->getUiStyleManager() : nullptr;
        if (styleMgr != nullptr) {
            styleMgr->setStyleAllowed(true);
            styleMgr->setActiveStyle("Fusion");
            styleMgr->applyActiveStyleAndTheme();
        }
    };

    // 3. Settings Pages Registrations
    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {
        { "styling.general", "", []() {
            return std::make_unique<LC_SettingsPageGeneralStyling>();
        }, 10 },
        { "styling.profiles", "", []() {
            return std::make_unique<LC_SettingsPageWorkspaceProfiles>();
        }, 120 },
        // Root: Typography & Icons
        {
            "styling.typography",
            "",
            []() {
                return std::make_unique<LC_SettingsPageTypography>();
            },
            30
        },
        {
            "styling.icons",
            "",
            []() {
                return std::make_unique<LC_SettingsPageIconsStyle>();
            },
            30
        },

        // Root Index: Fusion Styling & Theming (Gated)
        {
            "styling.fusion",
            "",
            []() {
                auto page = std::make_unique<LC_IndexSettingsPage>(QObject::tr("Fusion Theme"),
                                                                   QObject::tr(
                                                                       "Configure Fusion visual skins, color palettes, 3D shading, and metrics layout density."));
                page->setGating(&LC_SettingsManagerStyling::isFusionGated, []() {
                                    return LC_SettingsManagerStyling::fusionGatedMessage();
                                }, LC_SettingsManagerStyling::fusionGatedActionText(), []() {
                                    LC_SettingsManagerStyling::enableFusionStyling();
                                });
                return page;
            },
            30
        },

        // Branch 1: Skins (Parent: styling.fusion)
        {
            "styling.skins",
            "styling.fusion",
            []() {
                return std::make_unique<LC_IndexSettingsPage>(QObject::tr("Skins & Theming"),
                                                              QObject::tr(
                                                                  "Customize visual archetypes, color palettes, containers, dock frames, and controls."));
            },
            10
        },
        {
            "styling.skins.palette",
            "styling.skins",
            []() {
                return std::make_unique<LC_SettingsPageSkinPalette>();
            },
            10
        },
        {
            "styling.skins.containers",
            "styling.skins",
            []() {
                return std::make_unique<LC_SettingsPageSkinContainers>();
            },
            20
        },
        {
            "styling.skins.toolbars_menus",
            "styling.skins",
            []() {
                return std::make_unique<LC_SettingsPageSkinToolbarsMenus>();
            },
            30
        },
        {
            "styling.skins.controls",
            "styling.skins",
            []() {
                return std::make_unique<LC_SettingsPageSkinControls>();
            },
            40
        },
        {
            "styling.skins.qss",
            "styling.skins",
            []() {
                return std::make_unique<LC_SettingsPageSkinQss>();
            },
            50
        },

        // Branch 2: Metrics (Parent: styling.fusion)
        {
            "styling.metrics",
            "styling.fusion",
            []() {
                return std::make_unique<LC_IndexSettingsPage>(QObject::tr("Metrics & Density"),
                                                              QObject::tr(
                                                                  "Configure layout margins, paddings, densities, scrollbars, tabs, and menu metrics."));
            },
            20
        },
        {
            "styling.metrics.layout",
            "styling.metrics",
            []() {
                return std::make_unique<LC_SettingsPageMetricsLayout>();
            },
            10
        },
        {
            "styling.metrics.menus_toolbars",
            "styling.metrics",
            []() {
                return std::make_unique<LC_SettingsPageMetricsMenusToolbars>();
            },
            20
        },
        {
            "styling.metrics.controls",
            "styling.metrics",
            []() {
                return std::make_unique<LC_SettingsPageMetricsControls>();
            },
            30
        },
        {
            "styling.metrics.views_tabs",
            "styling.metrics",
            []() {
                return std::make_unique<LC_SettingsPageMetricsViewsTabs>();
            },
            40
        },
        {
            "styling.metrics.behavior",
            "styling.metrics",
            []() {
                return std::make_unique<LC_SettingsPageMetricsBehavior>();
            },
            50
        }
    };

    reg->registerPages(targetDialog, pages);
}

bool LC_SettingsManagerStyling::showStylingSettings(QWidget* parent, const QString& initialPageId) {
    auto previewController = std::make_unique<LC_StylingPreviewController>();

    auto preExecHook = [&previewController](LC_SettingsDialog* dialog) {
        previewController->setDialogParent(dialog);
        dialog->forEachPresetManager([&previewController](LC_PresetManagerInterface* manager) {
            if (auto* aware = dynamic_cast<LC_StylingPreviewAware*>(manager)) {
                aware->setPreviewController(previewController.get());
            }
        });
    };

    auto postExecHook = [&previewController](LC_SettingsDialog* dialog, bool /*accepted*/) {
        dialog->forEachPresetManager([](LC_PresetManagerInterface* manager) {
            if (auto* aware = dynamic_cast<LC_StylingPreviewAware*>(manager)) {
                aware->setPreviewController(nullptr);
            }
        });
        previewController->closePreview();
    };

    const bool accepted = LC_SettingsRegistry::instance()->showDialog("ui_styling_preferences", initialPageId, parent, preExecHook,
                                                                      postExecHook);

    if (accepted) {
        auto* styleMgr = QC_ApplicationWindow::getAppWindow()->getUiStyleManager();
        if (styleMgr != nullptr) {
            styleMgr->applyActiveStyleAndTheme();
        }
        if (parent != nullptr) {
            parent->update();
        }
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
