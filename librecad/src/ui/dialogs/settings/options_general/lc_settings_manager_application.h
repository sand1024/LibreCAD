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

#ifndef LC_SETTINGS_MANAGER_GENERIC_OPTIONS_H
#define LC_SETTINGS_MANAGER_GENERIC_OPTIONS_H

#include "lc_settings_manager_base.h"
#include "lc_settings_registry.h"

namespace LC_SettingsPagesApplication {
    inline const QString DLG_GENERAL_PREFERENCES                    = "general_preferences";

    inline const QString PAGE_DRAW                                  = "draw";
    inline const QString PAGE_DRAW_GRAPHIC_VIEW                     = "draw.graphic_view";
    inline const QString PAGE_DRAW_VIEW_BEHAVIOR                    = "draw.view.behavior";
    inline const QString PAGE_DRAW_VIEW_COLORS                      = "draw.view.colors";
    inline const QString PAGE_DRAW_VIEW_SELECTION                   = "draw.view.selection";
    inline const QString PAGE_DRAW_VIEW_HANDLES                     = "draw.view.handles";
    inline const QString PAGE_DRAW_VIEW_DRAFT_MARKER                = "draw.view.draft_marker";
    inline const QString PAGE_DRAW_GRID                             = "draw.grid";
    inline const QString PAGE_DRAW_GRID_POINTS                      = "draw.grid.points";
    inline const QString PAGE_DRAW_GRID_LINES                       = "draw.grid.lines";
    inline const QString PAGE_DRAW_PREVIEW                          = "draw.preview";
    inline const QString PAGE_DRAW_HIGHLIGHT                        = "draw.highlight";
    inline const QString PAGE_DRAW_SNAP                             = "draw.snap";
    inline const QString PAGE_DRAW_SNAP_GENERAL                     = "draw.snap.general";
    inline const QString PAGE_DRAW_SNAP_ANGLE                       = "draw.snap.angle";
    inline const QString PAGE_DRAW_SNAP_VISUAL                      = "draw.snap.visual";
    inline const QString PAGE_DRAW_SNAP_VISUAL_APPEARANCE           = "draw.snap.visual.appearance";
    inline const QString PAGE_DRAW_SNAP_VISUAL_BEHAVIOR             = "draw.snap.visual.behavior";
    inline const QString PAGE_DRAW_SNAP_VISUAL_TIMING               = "draw.snap.visual.timing";
    inline const QString PAGE_DRAW_INFO_CURSOR                      = "draw.info_cursor";
    inline const QString PAGE_DRAW_INPUT_ASSISTANT                  = "draw.input_assistant";
    inline const QString PAGE_DRAW_COORDINATE_SYSTEM                = "draw.coordinate_system";
    inline const QString PAGE_DRAW_COORDINATE_SYSTEM_ORIGIN         = "draw.coordinate_system.origin";
    inline const QString PAGE_DRAW_COORDINATE_SYSTEM_RELATIVE_ZERO  = "draw.coordinate_system.relative_zero";
    inline const QString PAGE_DRAW_COORDINATE_SYSTEM_AXIS_LINES     = "draw.coordinate_system.axis_lines";
    inline const QString PAGE_DRAW_COORDINATE_SYSTEM_ANGLES_BASIS   = "draw.coordinate_system.angles_basis";
    inline const QString PAGE_DRAW_RENDERER                         = "draw.renderer";
    inline const QString PAGE_DRAW_RENDERER_TEXT_OPS                = "draw.renderer.text_ops";
    inline const QString PAGE_DRAW_RENDERER_ADVANCED                = "draw.renderer.advanced";
    inline const QString PAGE_DRAW_RENDERER_MINIMUMS                = "draw.renderer.minimums";
    inline const QString PAGE_DRAW_RENDERER_ARCS                    = "draw.renderer.arcs";

    inline const QString PAGE_APP                                   = "app";
    inline const QString PAGE_APP_PROGRAM_DEFAULTS                  = "app.program_defaults";
    inline const QString PAGE_APP_DEFAULTS                          = "app.defaults";
    inline const QString PAGE_APP_AUTOSAVE                          = "app.autosave";
    inline const QString PAGE_APP_CAD_PREFERENCES                   = "app.cad_preferences";
    inline const QString PAGE_APP_KEYBOARD                          = "app.keyboard";
    inline const QString PAGE_APP_COMMAND_LINE                     = "app.command_line";
    inline const QString PAGE_APP_PATHS                             = "app.paths";
    inline const QString PAGE_APP_UPDATES                           = "app.updates";
    inline const QString PAGE_APP_MAINTENANCE                       = "app.maintenance";
    inline const QString PAGE_APP_STARTUP                           = "app.startup";
    inline const QString PAGE_APP_LANGUAGE                          = "app.language";
}


class LC_SettingsManagerApplication: public LC_SettingsManagerBase {
public:
    static QWidget* createGraphicViewPreview(QWidget* parent);
    static void initialize();

    static bool showOptionsApplication(QWidget* parent) {
        const auto settingsRegistry = LC_SettingsRegistry::instance();
        const bool accepted = settingsRegistry->showDialog(LC_SettingsPagesApplication::DLG_GENERAL_PREFERENCES, "",
                                                           parent);

        if (accepted) {
            parent->update();
        }
        return accepted;
    }
};

#endif
