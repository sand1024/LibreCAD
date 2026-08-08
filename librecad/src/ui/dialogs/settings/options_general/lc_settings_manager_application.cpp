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

#include "lc_settings_manager_application.h"

#include "lc_graphic_view_preview_widget.h"
#include "lc_index_settings_page.h"
#include "lc_settings_page_autosave.h"
#include "lc_settings_page_cad_preferences.h"
#include "lc_settings_page_command_line.h"
#include "lc_settings_page_coordinate_system_angles_basis.h"
#include "lc_settings_page_coordinate_system_axis_lines.h"
#include "lc_settings_page_coordinate_system_origin.h"
#include "lc_settings_page_coordinate_system_relative_zero.h"
#include "lc_settings_page_drawing_defaults.h"
#include "lc_settings_page_general_startup.h"
#include "lc_settings_page_graphic_view_behavior.h"
#include "lc_settings_page_graphic_view_colors.h"
#include "lc_settings_page_graphic_view_draft_marker.h"
#include "lc_settings_page_graphic_view_handles.h"
#include "lc_settings_page_graphic_view_selection.h"
#include "lc_settings_page_grid_general.h"
#include "lc_settings_page_grid_lines.h"
#include "lc_settings_page_grid_points.h"
#include "lc_settings_page_info_cursor.h"
#include "lc_settings_page_input_assistant.h"
#include "lc_settings_page_keyboard.h"
#include "lc_settings_page_language.h"
#include "lc_settings_page_maintenance.h"
#include "lc_settings_page_paths.h"
#include "lc_settings_page_preview_highlight.h"
#include "lc_settings_page_preview_options.h"
#include "lc_settings_page_program_defaults.h"
#include "lc_settings_page_renderer_arcs.h"
#include "lc_settings_page_renderer_minimums.h"
#include "lc_settings_page_renderer_text_ops.h"
#include "lc_settings_page_snap_angle.h"
#include "lc_settings_page_snap_general.h"
#include "lc_settings_page_snap_visual_appearance.h"
#include "lc_settings_page_snap_visual_behavior.h"
#include "lc_settings_page_snap_visual_timing.h"
#include "lc_settings_page_updates.h"
#include "lc_viewport_theme_repository.h"
#include "lc_visual_snap_data.h"

class LC_OverlayDrawablesContainer;

QWidget* LC_SettingsManagerApplication::createGraphicViewPreview(QWidget* parent) {
    QWidget* result = new LC_GraphicViewPreviewWidget(parent);
    return result;
}

template<class T>
static LC_SettingsRegistry::PageCreator page() {
     return [] {return std::make_unique<T>();};
}

static LC_SettingsRegistry::PageCreator index(const QString& displayName, const QString& description) {
    return [displayName, description] {
        return std::make_unique<LC_IndexSettingsPage>(displayName, description);
    };
}

void LC_SettingsManagerApplication::initializeApplicationSettings() {
    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = "application_preferences";

    reg->configureDialog(targetDialog, QObject::tr("Application Preferences"), true);

    //  Viewport Theme Preset Manager Registration

    reg->registerPresetManager(targetDialog, "draw", [] {
        QWidget* previewLabel = createGraphicViewPreview(nullptr);
        return std::make_unique<LC_ViewportThemeEditor>(nullptr, previewLabel);
    });

    // pages
    const std::initializer_list<LC_SettingsRegistry::PageRegistration>& pages = {
        {"draw","",index(QObject::tr("Drawing Area"),
    QObject::tr("Configure the drawing view environment, snapping parameters, "
                "coordinate systems, visual overlays, and renderers."))},
        {"draw.graphic_view","draw",index(QObject::tr("Graphic View"),
    QObject::tr("Configure view behaviors, scrollbars, selection overlays, colors,"
                " handles, and draft mode markers."))},
        {"draw.grid","draw",page<LC_SettingsPageGridGeneral>()},
        {"draw.grid.points","draw.grid",page<LC_SettingsPageGridPoints>()},
        {"draw.grid.lines","draw.grid",page<LC_SettingsPageGridLines>()},
        {"draw.preview","draw",page<LC_SettingsPagePreviewOptions>()},
        {"draw.highlight","draw",page<LC_SettingsPagePreviewHighlight>()},
        {"draw.snap", "draw", index(QObject::tr("Snapping"),
    QObject::tr("Snap functionality is a precision drawing tool that forces cursor to lock exactly onto "
                "specific geometric points (defined automatically by current geometry and enabled snap modes)."))},
        {"draw.coordinate_system","draw",index(QObject::tr("Coordinate System"),
    QObject::tr("Configure coordinate zero markers, relative zero markers, axis lines, and "
                "angles basis representations."))},
        {"app", "",index(QObject::tr("Application"),
    QObject::tr("Configure general application settings, defaults, workspace "
                "profiles, paths, and update preferences."))},
        {"draw.view.behavior", "draw.graphic_view",page<LC_SettingsPageGraphicViewBehavior>()},
        {"draw.view.colors","draw.graphic_view",page<LC_SettingsPageGraphicViewColors>()},
        {"draw.view.selection","draw.graphic_view",page<LC_SettingsPageGraphicViewSelection>()},
        {"draw.view.handles","draw.graphic_view",page<LC_SettingsPageGraphicViewHandles>()},
        {"draw.view.draft_marker","draw.graphic_view",page<LC_SettingsPageGraphicViewDraftMarker>()},
        {"draw.snap.general", "draw.snap", page<LC_SettingsPageSnapGeneral>()},
        {"draw.snap.angle", "draw.snap", page<LC_SettingsPageSnapAngle>()},
        {"draw.snap.visual", "draw.snap", index(QObject::tr("Visual Snap"),
    QObject::tr("Visual Snap is a way to snap to specific points in a drawing by interpreting the natural geometry "
                "of existing elements (lines, points, intersections, and circles), "
                "reducing the need for manual calculations and the construction of auxiliary geometry.")), 100},
        // {"draw.snap.visual.old", "draw.snap.visual", page<LC_SettingsPageSnapVisual>()},
        {"draw.snap.visual.appearance", "draw.snap.visual", page<LC_SettingsPageSnapVisualAppearance>()},
        {"draw.snap.visual.behavior", "draw.snap.visual", page<LC_SettingsPageSnapVisualBehavior>()},
        {"draw.snap.visual.timing", "draw.snap.visual", page<LC_SettingsPageSnapVisualTiming>()},
        {"draw.info_cursor","draw", page<LC_SettingsPageInfoCursor>()},
        {"draw.input_assistant","draw",page<LC_SettingsPageInputAssistant>()},
        {"draw.coordinate_system.origin","draw.coordinate_system",page<LC_SettingsPageCoordinateSystemOrigin>()},
        {"draw.coordinate_system.relative_zero","draw.coordinate_system",page<LC_SettingsPageCoordinateSystemRelativeZero>()},
        {"draw.coordinate_system.axis_lines","draw.coordinate_system",page<LC_SettingsPageCoordinateSystemAxisLines>()},
        {"draw.coordinate_system.angles_basis","draw.coordinate_system",page<LC_SettingsPageCoordinateSystemAnglesBasis>()},
        {"draw.renderer","draw",index(QObject::tr("Renderer"),
            QObject::tr("Configure line segments interpolations, minimum rendering "
                "pixel limits, and font files configurations.")), 100},
        {"draw.renderer.text_ops","draw.renderer",page<LC_SettingsPageRendererTextOps>(), 110},
        {"draw.renderer.advanced","draw.renderer",index(QObject::tr("Advanced"),
            QObject::tr("Configure advanced rendering settings, such as line "
                "segments interpolations, minimum rendering pixel limits.")), 115},
        {"draw.renderer.minimums","draw.renderer.advanced",page<LC_SettingsPageRendererMinimums>()},
        {"draw.renderer.arcs","draw.renderer.advanced",page<LC_SettingsPageRendererArcs>()},
        {"app.program_defaults","app",page<LC_SettingsPageProgramDefaults>()},
        {"app.defaults","app",page<LC_SettingsPageDrawingDefaults>()},
        {"app.autosave","app",page<LC_SettingsPageAutosave>()},
        {"app.cad_preferences","app",page<LC_SettingsPageCadPreferences>()},
        {"app.keyboard","app",page<LC_SettingsPageKeyboard>()},
        {"app.command_line","app",page<LC_SettingsPageCommandLine>()},
        {"app.paths","app",page<LC_SettingsPagePaths>()},
        {"app.updates","app",page<LC_SettingsPageUpdates>()},
        {"app.maintenance","app",page<LC_SettingsPageMaintenance>()},
        {"app.startup","app",page<LC_SettingsPageGeneralStartup>()},
        {"app.language","app",page<LC_SettingsPageLanguage>()},
        };
        reg->registerPages(targetDialog, pages);
    }
