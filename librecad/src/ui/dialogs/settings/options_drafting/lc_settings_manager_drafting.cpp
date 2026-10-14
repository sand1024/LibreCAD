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

#include "lc_settings_manager_drafting.h"

#include <QObject>

#include "lc_drafting_preview_bottom_bar.h"
#include "lc_graphic_view_preview_widget.h"
#include "lc_preset_manager_drafting_metrics.h"
#include "lc_preset_manager_drafting_theme.h"
#include "lc_repository_drafting_metrics.h"
#include "lc_repository_drafting_theme.h"
#include "lc_settings_dialog.h"
#include "lc_settings_page_coordinate_system_angles_basis.h"
#include "lc_settings_page_coordinate_system_axis_lines.h"
#include "lc_settings_page_coordinate_system_origin.h"
#include "lc_settings_page_drafting_metrics_markers.h"
#include "lc_settings_page_drafting_metrics_overlays.h"
#include "lc_settings_page_drafting_metrics_snapping.h"
#include "lc_settings_page_drafting_theme_cursor.h"
#include "lc_settings_page_graphic_view_behavior.h"
#include "lc_settings_page_graphic_view_colors.h"
#include "lc_settings_page_graphic_view_handles.h"
#include "lc_settings_page_grid_general.h"
#include "lc_settings_page_grid_lines.h"
#include "lc_settings_page_grid_points.h"
#include "lc_settings_page_info_cursor.h"
#include "lc_settings_page_input_assistant.h"
#include "lc_settings_page_preview_options.h"
#include "lc_settings_page_renderer_advanced.h"
#include "lc_settings_page_renderer_text_ops.h"
#include "lc_settings_page_snap_general.h"
#include "lc_settings_page_snap_indicator.h"
#include "lc_settings_page_snap_visual_behavior.h"
#include "lc_settings_page_snap_visual_timing.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

void LC_SettingsManagerDrafting::initialize(QC_ApplicationWindow* appWindow) {
    using namespace LC_SettingsPagesDrafting;

    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = DLG_DRAFTING_PREFERENCES;

    reg->configureDialog(targetDialog, {
        QObject::tr("Drafting Preferences"),
        true,
        false,
        true,
        ":/icons/drawing.lci"
    });



    reg->registerPresetManager(targetDialog, GROUP_DRAFTING_THEME, [appWindow]() {
        auto* uiStyleManager = appWindow != nullptr ? appWindow->getUiStyleManager() : nullptr;
        auto* repository = uiStyleManager != nullptr ? uiStyleManager->getDraftingThemeRepository() : nullptr;
        return std::make_unique<LC_PresetManagerDraftingTheme>(repository);
    });

    reg->registerPresetManager(targetDialog, GROUP_DRAFTING_METRICS, [appWindow]() {
     auto* uiStyleManager = appWindow != nullptr ? appWindow->getUiStyleManager() : nullptr;
     auto* repository = uiStyleManager != nullptr ? uiStyleManager->getDrafingMetricsRepository() : nullptr;
     return std::make_unique<LC_PresetManagerDraftingMetrics>(repository);
 });

    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {

        // ═════════════════════════════════════════════════════════════════════
        // TOP-LEVEL 1: DRAFTING THEME (Persistent Top Bar + Live CAD Preview)
        // ═════════════════════════════════════════════════════════════════════
        { PAGE_DRAFTING_THEME, "", index(QObject::tr("Visual Theme"),
            QObject::tr("Configure canvas colors, selection overlays, entity handles, and previews.")), 10 },
        { PAGE_DRAFTING_VIEW_COLORS, PAGE_DRAFTING_THEME, page<LC_SettingsPageGraphicViewColors>(), 10 },
        { PAGE_DRAFTING_VIEW_HANDLES, PAGE_DRAFTING_THEME, page<LC_SettingsPageGraphicViewHandles>(), 20 },
        { PAGE_DRAFTING_PREVIEW, PAGE_DRAFTING_THEME, page<LC_SettingsPagePreviewOptions>(), 50 },

        // Grid Display
        { PAGE_DRAFTING_GRID, PAGE_DRAFTING_THEME, page<LC_SettingsPageGridGeneral>(), 70 },
        { PAGE_DRAFTING_GRID_POINTS, PAGE_DRAFTING_GRID, page<LC_SettingsPageGridPoints>(), 10 },
        { PAGE_DRAFTING_GRID_LINES, PAGE_DRAFTING_GRID, page<LC_SettingsPageGridLines>(), 20 },

        // Coordinate System Markers
        { PAGE_DRAFTING_COORDINATE_SYSTEM, PAGE_DRAFTING_THEME, index(QObject::tr("Coordinates System"),
            QObject::tr("Configure origin markers, relative zero, axis lines, and angles basis.")), 80 },
        { PAGE_DRAFTING_COORDINATE_SYSTEM_ORIGIN, PAGE_DRAFTING_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemOrigin>(), 10 },
        { PAGE_DRAFTING_COORDINATE_SYSTEM_AXIS_LINES, PAGE_DRAFTING_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemAxisLines>(), 20 },
        { PAGE_DRAFTING_COORDINATE_SYSTEM_ANGLES_BASIS, PAGE_DRAFTING_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemAnglesBasis>(), 30 },

        // Indicators & Cursor Styling
        { PAGE_DRAFTING_SNAP_INDICATOR, PAGE_DRAFTING_THEME, page<LC_SettingsPageSnapIndicator>(), 90 },
        { PAGE_DRAFTING_THEME_CURSOR, PAGE_DRAFTING_THEME, page<LC_SettingsPageDraftingThemeCursor>(), 100 },

        // ═════════════════════════════════════════════════════════════════════
        // TOP-LEVEL 2: VIEWPORT METRICS (Persistent Sizing Profile Top Bar)
        // ═════════════════════════════════════════════════════════════════════
        { PAGE_DRAFTING_METRICS, "", index(QObject::tr("Drafting Metrics (Sizes)"),
            QObject::tr("Configure handle sizes, marker scales, line thicknesses, and text overlay sizes.")), 20 },
        { PAGE_DRAFTING_METRICS_MARKERS, PAGE_DRAFTING_METRICS, page<LC_SettingsPageDraftingMetricsMarkers>(), 10 },
        { PAGE_DRAFTING_METRICS_SNAPPING, PAGE_DRAFTING_METRICS, page<LC_SettingsPageDraftingMetricsSnapping>(), 20 },
        { PAGE_DRAFTING_METRICS_OVERLAYS, PAGE_DRAFTING_METRICS, page<LC_SettingsPageDraftingMetricsOverlays>(), 30 },

        // ═════════════════════════════════════════════════════════════════════
        // TOP-LEVEL 3: DRAFTING BEHAVIOR (Direct Settings: No Bar, Never Gated)
        // ═════════════════════════════════════════════════════════════════════
        { PAGE_DRAFTING_BEHAVIOR, "", index(QObject::tr("Drafting Behavior"),
            QObject::tr("Configure canvas navigation, snapping rules, solver parameters, and renderer engine.")), 30 },

        // Navigation and Grid (Weight 10)
        { PAGE_DRAFTING_NAV_GRID, PAGE_DRAFTING_BEHAVIOR, page<LC_SettingsPageGraphicViewBehavior>(), 10 },

        // General Snap (Weight 20)
        { PAGE_DRAFTING_SNAP_GENERAL, PAGE_DRAFTING_BEHAVIOR, page<LC_SettingsPageSnapGeneral>(), 20 },

        // Visual Snap (Weight 30) with child Advanced (Weight 10)
        { PAGE_DRAFTING_SNAP_VISUAL, PAGE_DRAFTING_BEHAVIOR, page<LC_SettingsPageSnapVisualBehavior>(), 30 },
        { PAGE_DRAFTING_SNAP_VISUAL_TIMING, PAGE_DRAFTING_SNAP_VISUAL, page<LC_SettingsPageSnapVisualTiming>(), 10 },

        // Cursor Workflow (Direct children of PAGE_DRAFTING_BEHAVIOR)
        { PAGE_DRAFTING_INFO_CURSOR, PAGE_DRAFTING_BEHAVIOR, page<LC_SettingsPageInfoCursor>(), 40 },
        { PAGE_DRAFTING_INPUT_ASSISTANT, PAGE_DRAFTING_BEHAVIOR, page<LC_SettingsPageInputAssistant>(), 50 },

        // Renderer (Weight 60)
        { PAGE_DRAFTING_RENDERER, PAGE_DRAFTING_BEHAVIOR, index(QObject::tr("Renderer"),
        QObject::tr("Configure text rendering, curve segment interpolation, and limits.")), 60 },
        { PAGE_DRAFTING_RENDERER_TEXT_OPS, PAGE_DRAFTING_RENDERER, page<LC_SettingsPageRendererTextOps>(), 10 },
        { PAGE_DRAFTING_RENDERER_ADVANCED, PAGE_DRAFTING_RENDERER, page<LC_SettingsPageRendererAdvanced>(), 20 }
    };

    reg->registerPages(targetDialog, pages);
}

bool LC_SettingsManagerDrafting::showDraftingSettings(QWidget* parent, const QString& initialPageId) {
    QWidget* previewWidget =  new LC_GraphicViewPreviewWidget(nullptr);
    auto* bottomBar = new LC_DraftingPreviewBottomBar(nullptr);

    // Keep hidden until mounted into their respective dialog layouts
    if (previewWidget != nullptr) {
        previewWidget->hide();
    }
    if (bottomBar != nullptr) {
        bottomBar->hide();
    }

    QPointer<QWidget> previewGuard(previewWidget);
    QPointer<QWidget> bottomBarGuard(bottomBar);

   auto preExecHook = [previewWidget, bottomBar](LC_SettingsDialog* dialog) {
        if (dialog != nullptr) {
            dialog->forEachPresetManager([previewWidget, bottomBar](LC_PresetManagerInterface* manager) {
                if (auto* themeMgr = dynamic_cast<LC_PresetManagerDraftingTheme*>(manager)) {
                    themeMgr->setSharedPreviewWidget(previewWidget);
                    themeMgr->setSharedBottomWidget(bottomBar);
                }
                else if (auto* metricsMgr = dynamic_cast<LC_PresetManagerDraftingMetrics*>(manager)) {
                    metricsMgr->setSharedPreviewWidget(previewWidget);
                    metricsMgr->setSharedBottomWidget(bottomBar);
                }
            });

            // Wire shared preview and bottom bar to Informational Cursor page
            dialog->forEachPage([previewWidget, bottomBar](LC_SettingsPageInterface* page) {
                if (auto* infoCursorPage = dynamic_cast<LC_SettingsPageInfoCursor*>(page)) {
                    infoCursorPage->setPreviewWidget(previewWidget);
                    infoCursorPage->setBottomWidget(bottomBar);
                }
            });
        }
    };

    auto postExecHook = [](LC_SettingsDialog* dialog, bool /*accepted*/) {
        if (dialog != nullptr) {
            dialog->forEachPresetManager([](LC_PresetManagerInterface* manager) {
                if (auto* themeMgr = dynamic_cast<LC_PresetManagerDraftingTheme*>(manager)) {
                    themeMgr->setSharedPreviewWidget(nullptr);
                    themeMgr->setSharedBottomWidget(nullptr);
                }
                else if (auto* metricsMgr = dynamic_cast<LC_PresetManagerDraftingMetrics*>(manager)) {
                    metricsMgr->setSharedPreviewWidget(nullptr);
                    metricsMgr->setSharedBottomWidget(nullptr);
                }
            });

            dialog->forEachPage([](LC_SettingsPageInterface* page) {
                if (auto* infoCursorPage = dynamic_cast<LC_SettingsPageInfoCursor*>(page)) {
                    infoCursorPage->setPreviewWidget(nullptr);
                    infoCursorPage->setBottomWidget(nullptr);
                }
            });
        }
    };

    const bool accepted = LC_SettingsRegistry::instance()->showDialog(
        LC_SettingsPagesDrafting::DLG_DRAFTING_PREFERENCES,
        initialPageId,
        parent,
        preExecHook,
        postExecHook);

    // If never mounted in any layout (e.g. user never visited preview pages), delete cleanly
    if (!previewGuard.isNull() && previewGuard->parent() == nullptr) {
        delete previewGuard.data();
    }
    if (!bottomBarGuard.isNull() && bottomBarGuard->parent() == nullptr) {
        delete bottomBarGuard.data();
    }

    return accepted;
}
