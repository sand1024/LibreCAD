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
#include "lc_settings_page_index.h"
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
#include "lc_preset_manager_viewport.h"
#include "lc_viewport_theme_repository.h"
#include "lc_visual_snap_data.h"

class LC_OverlayDrawablesContainer;

QWidget* LC_SettingsManagerApplication::createGraphicViewPreview(QWidget* parent) {
    QWidget* result = new LC_GraphicViewPreviewWidget(parent);
    return result;
}

void LC_SettingsManagerApplication::initialize() {
    using namespace LC_SettingsPagesApplication;

    auto* reg = LC_SettingsRegistry::instance();
    const QString targetDialog = DLG_GENERAL_PREFERENCES;

    reg->configureDialog(targetDialog, {QObject::tr("Application Preferences"), true, false});

    // Viewport Theme Preset Manager Registration
    reg->registerPresetManager(targetDialog, PAGE_DRAW, []() {
        QWidget* previewLabel = createGraphicViewPreview(nullptr);
        return std::make_unique<LC_PresetManagerViewport>(nullptr, previewLabel);
    });

    // Pages Registration
    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {
        { PAGE_DRAW, "", index(QObject::tr("Drawing Area"),
            QObject::tr("Configure the drawing view environment, snapping parameters, "
                        "coordinate systems, visual overlays, and renderers.")) },
        { PAGE_DRAW_GRAPHIC_VIEW, PAGE_DRAW, index(QObject::tr("Graphic View"),
            QObject::tr("Configure view behaviors, scrollbars, selection overlays, colors,"
                        " handles, and draft mode markers.")) },
        { PAGE_DRAW_VIEW_BEHAVIOR, PAGE_DRAW_GRAPHIC_VIEW, page<LC_SettingsPageGraphicViewBehavior>() },
        { PAGE_DRAW_VIEW_COLORS, PAGE_DRAW_GRAPHIC_VIEW, page<LC_SettingsPageGraphicViewColors>() },
        { PAGE_DRAW_VIEW_SELECTION, PAGE_DRAW_GRAPHIC_VIEW, page<LC_SettingsPageGraphicViewSelection>() },
        { PAGE_DRAW_VIEW_HANDLES, PAGE_DRAW_GRAPHIC_VIEW, page<LC_SettingsPageGraphicViewHandles>() },
        { PAGE_DRAW_VIEW_DRAFT_MARKER, PAGE_DRAW_GRAPHIC_VIEW, page<LC_SettingsPageGraphicViewDraftMarker>() },
        { PAGE_DRAW_GRID, PAGE_DRAW, page<LC_SettingsPageGridGeneral>() },
        { PAGE_DRAW_GRID_POINTS, PAGE_DRAW_GRID, page<LC_SettingsPageGridPoints>() },
        { PAGE_DRAW_GRID_LINES, PAGE_DRAW_GRID, page<LC_SettingsPageGridLines>() },
        { PAGE_DRAW_PREVIEW, PAGE_DRAW, page<LC_SettingsPagePreviewOptions>() },
        { PAGE_DRAW_HIGHLIGHT, PAGE_DRAW, page<LC_SettingsPagePreviewHighlight>() },
        { PAGE_DRAW_SNAP, PAGE_DRAW, index(QObject::tr("Snapping"),
            QObject::tr("Snap functionality is a precision drawing tool that forces cursor to lock exactly onto "
                        "specific geometric points (defined automatically by current geometry and enabled snap modes).")) },
        { PAGE_DRAW_SNAP_GENERAL, PAGE_DRAW_SNAP, page<LC_SettingsPageSnapGeneral>() },
        { PAGE_DRAW_SNAP_ANGLE, PAGE_DRAW_SNAP, page<LC_SettingsPageSnapAngle>() },
        { PAGE_DRAW_SNAP_VISUAL, PAGE_DRAW_SNAP, index(QObject::tr("Visual Snap"),
            QObject::tr("Visual Snap is a way to snap to specific points in a drawing by interpreting the natural geometry "
                        "of existing elements (lines, points, intersections, and circles), "
                        "reducing the need for manual calculations and the construction of auxiliary geometry.")), 100 },
        { PAGE_DRAW_SNAP_VISUAL_APPEARANCE, PAGE_DRAW_SNAP_VISUAL, page<LC_SettingsPageSnapVisualAppearance>() },
        { PAGE_DRAW_SNAP_VISUAL_BEHAVIOR, PAGE_DRAW_SNAP_VISUAL, page<LC_SettingsPageSnapVisualBehavior>() },
        { PAGE_DRAW_SNAP_VISUAL_TIMING, PAGE_DRAW_SNAP_VISUAL, page<LC_SettingsPageSnapVisualTiming>() },
        { PAGE_DRAW_INFO_CURSOR, PAGE_DRAW, page<LC_SettingsPageInfoCursor>() },
        { PAGE_DRAW_INPUT_ASSISTANT, PAGE_DRAW, page<LC_SettingsPageInputAssistant>() },
        { PAGE_DRAW_COORDINATE_SYSTEM, PAGE_DRAW, index(QObject::tr("Coordinate System"),
            QObject::tr("Configure coordinate zero markers, relative zero markers, axis lines, and "
                        "angles basis representations.")) },
        { PAGE_DRAW_COORDINATE_SYSTEM_ORIGIN, PAGE_DRAW_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemOrigin>() },
        { PAGE_DRAW_COORDINATE_SYSTEM_RELATIVE_ZERO, PAGE_DRAW_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemRelativeZero>() },
        { PAGE_DRAW_COORDINATE_SYSTEM_AXIS_LINES, PAGE_DRAW_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemAxisLines>() },
        { PAGE_DRAW_COORDINATE_SYSTEM_ANGLES_BASIS, PAGE_DRAW_COORDINATE_SYSTEM, page<LC_SettingsPageCoordinateSystemAnglesBasis>() },
        { PAGE_DRAW_RENDERER, PAGE_DRAW, index(QObject::tr("Renderer"),
            QObject::tr("Configure line segments interpolations, minimum rendering "
                        "pixel limits, and font files configurations.")), 100 },
        { PAGE_DRAW_RENDERER_TEXT_OPS, PAGE_DRAW_RENDERER, page<LC_SettingsPageRendererTextOps>(), 110 },
        { PAGE_DRAW_RENDERER_ADVANCED, PAGE_DRAW_RENDERER, index(QObject::tr("Advanced"),
            QObject::tr("Configure advanced rendering settings, such as line "
                        "segments interpolations, minimum rendering pixel limits.")), 115 },
        { PAGE_DRAW_RENDERER_MINIMUMS, PAGE_DRAW_RENDERER_ADVANCED, page<LC_SettingsPageRendererMinimums>() },
        { PAGE_DRAW_RENDERER_ARCS, PAGE_DRAW_RENDERER_ADVANCED, page<LC_SettingsPageRendererArcs>() },
        { PAGE_APP, "", index(QObject::tr("Application"),
            QObject::tr("Configure general application settings, defaults, workspace "
                        "profiles, paths, and update preferences.")) },
        { PAGE_APP_PROGRAM_DEFAULTS, PAGE_APP, page<LC_SettingsPageProgramDefaults>() },
        { PAGE_APP_DEFAULTS, PAGE_APP, page<LC_SettingsPageDrawingDefaults>() },
        { PAGE_APP_AUTOSAVE, PAGE_APP, page<LC_SettingsPageAutosave>() },
        { PAGE_APP_CAD_PREFERENCES, PAGE_APP, page<LC_SettingsPageCadPreferences>() },
        { PAGE_APP_KEYBOARD, PAGE_APP, page<LC_SettingsPageKeyboard>() },
        { PAGE_APP_COMMAND_LINE, PAGE_APP, page<LC_SettingsPageCommandLine>() },
        { PAGE_APP_PATHS, PAGE_APP, page<LC_SettingsPagePaths>() },
        { PAGE_APP_UPDATES, PAGE_APP, page<LC_SettingsPageUpdates>() },
        { PAGE_APP_MAINTENANCE, PAGE_APP, page<LC_SettingsPageMaintenance>() },
        { PAGE_APP_STARTUP, PAGE_APP, page<LC_SettingsPageGeneralStartup>() },
        { PAGE_APP_LANGUAGE, PAGE_APP, page<LC_SettingsPageLanguage>() }
    };

    reg->registerPages(targetDialog, pages);
}
