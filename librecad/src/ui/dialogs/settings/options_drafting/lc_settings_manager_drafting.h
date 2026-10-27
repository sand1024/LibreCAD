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

#ifndef LC_SETTINGS_MANAGER_DRAFTING_H
#define LC_SETTINGS_MANAGER_DRAFTING_H

#include <QString>
#include <QWidget>

#include "lc_settings_manager_base.h"

class QC_ApplicationWindow;

namespace LC_SettingsPagesDrafting {
    const QString DLG_DRAFTING_PREFERENCES = "DlgDraftingPreferences";

    // ─────────────────────────────────────────────────────────────────────────────
    // 1. VISUAL THEME DOMAIN (Routing Key: drafting.theme)
    // ─────────────────────────────────────────────────────────────────────────────
    const QString GROUP_DRAFTING_THEME                      = "drafting.theme";

    // Canvas Styling & Colors
    const QString PAGE_DRAFTING_THEME                       = "drafting.theme";
    const QString PAGE_DRAFTING_THEME_MAIN_COLORS           = "drafting.theme.colors";
    const QString PAGE_DRAFTING_THEME_SELECTION             = "drafting.theme.selection";
    const QString PAGE_DRAFTING_THEME_PREVIEW               = "drafting.theme.preview";
    // Grid Display
    const QString PAGE_DRAFTING_THEME_GRID                  = "drafting.theme.grid";
    const QString PAGE_DRAFTING_THEME_GRID_POINTS           = "drafting.theme.grid.points";
    const QString PAGE_DRAFTING_THEME_GRID_LINES            = "drafting.theme.grid.lines";

    // Coordinate System Markers
    const QString PAGE_DRAFTING_THEME_COORDINATE_SYSTEM           = "drafting.theme.coordinate_system";

    // Cursor & Indicator Visuals
    const QString PAGE_DRAFTING_THEME_SNAP_INDICATOR            = "drafting.theme.snap_indicator";
    const QString PAGE_DRAFTING_THEME_SNAP_VISUAL               = "drafting.theme.snap_visual";
    const QString PAGE_DRAFTING_THEME_INFO_CURSOR               = "drafting.theme.info_cursor";

    // ─────────────────────────────────────────────────────────────────────────────
    // 2. SIZING / METRICS DOMAIN (Routing Key: drafting.metrics)
    // ─────────────────────────────────────────────────────────────────────────────
    const QString GROUP_DRAFTING_METRICS                    = "drafting.metrics";

    const QString PAGE_DRAFTING_METRICS                     = "drafting.metrics";
    const QString PAGE_DRAFTING_METRICS_MARKERS             = "drafting.metrics.markers";
    const QString PAGE_DRAFTING_METRICS_COORDINATE_SYSTEM   = "drafting.metrics.coordinate_system";
    const QString PAGE_DRAFTING_METRICS_SNAPPING            = "drafting.metrics.snapping";
    const QString PAGE_DRAFTING_METRICS_OVERLAYS            = "drafting.metrics.overlays";
    const QString PAGE_DRAFTING_METRICS_RENDERER            = "drafting.metrics.renderer";

    // ─────────────────────────────────────────────────────────────────────────────
    // 3. DRAFTING BEHAVIOR (Direct Settings: No Bar, No Preview)
    // ─────────────────────────────────────────────────────────────────────────────
    const QString PAGE_DRAFTING_BEHAVIOR                    = "drafting.behavior";

    const QString PAGE_DRAFTING_BEHAVIOR_NAV_GRID           = "drafting.behavior.navigation";
    const QString PAGE_DRAFTING_BEHAVIOR_SNAP_GENERAL       = "drafting.behavior.snap";

    // Visual Snap Solver
    const QString PAGE_DRAFTING_BEHAVIOR_SNAP_VISUAL         = "drafting.behavior.visual_snap";
    const QString PAGE_DRAFTING_BEHAVIOR_SNAP_VISUAL_TIMING  = "drafting.behavior.visual_snap.timing";

    // Cursor Workflow
    const QString PAGE_DRAFTING_BEHAVIOR_INFO_CURSOR         = "drafting.behavior.info_cursor";
    const QString PAGE_DRAFTING_BEHAVIOR_INPUT_ASSISTANT     = "drafting.behavior.input_assistant";

    // Renderer Engine
    const QString PAGE_DRAFTING_RENDERER_TEXT_OPS             = "drafting.behavior.renderer";

    const QString PAGE_DRAFTING_PROFILES = "drafting.profiles";

}

class LC_SettingsManagerDrafting : public LC_SettingsManagerBase {
public:
    static void initialize(QC_ApplicationWindow* appWindow);
    static bool showDraftingSettings(QWidget* parent = nullptr, const QString& initialPageId = QString());
};

#endif 
