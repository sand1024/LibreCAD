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

#ifndef LC_GROUP_NAMES_H
#define LC_GROUP_NAMES_H

namespace LC_GroupNames {
    // Drafting Geometry
    inline constexpr auto LINE              = "line";
    inline constexpr auto POINT             = "point";
    inline constexpr auto SHAPE             = "shape";
    inline constexpr auto CIRCLE            = "circle";
    inline constexpr auto CURVE             = "curve";
    inline constexpr auto SPLINE            = "spline";
    inline constexpr auto ELLIPSE           = "ellipse";
    inline constexpr auto POLYLINE          = "polyline";
    inline constexpr auto TEXT              = "text";
    inline constexpr auto OTHER             = "other";

    // Selection, Transforms & Editing
    inline constexpr auto SELECT            = "select";
    inline constexpr auto MODIFY            = "modify";
    inline constexpr auto ALIGN             = "align";
    inline constexpr auto ORDER             = "order";
    inline constexpr auto EDIT              = "edit";

    // Annotations & Info
    inline constexpr auto DIMENSION         = "dimension";
    inline constexpr auto INFO              = "info";

    // Snapping & Assistant
    inline constexpr auto SNAP              = "snap";
    inline constexpr auto SNAP_EXTRAS       = "snap_extras";
    inline constexpr auto RESTRICTION       = "restriction";
    inline constexpr auto RELATIVE_INPUT    = "relative_input";
    inline constexpr auto REL_ZERO          = "relZero";
    inline constexpr auto INFO_CURSOR       = "infoCursor";

    // Document Structure
    inline constexpr auto LAYER             = "layer";
    inline constexpr auto ENTITY_LAYER      = "entity_layer";
    inline constexpr auto BLOCK             = "block";
    inline constexpr auto UCS               = "ucs";
    inline constexpr auto PEN               = "pen";

    // Workspace & Navigation
    inline constexpr auto FILES              = "file";
    inline constexpr auto VIEW              = "view";
    inline constexpr auto PLUGINS           = "plugins";
    inline constexpr auto DRAW              = "draw";
    inline constexpr auto TOOLS             = "tools";
    inline constexpr auto NAMED_VIEWS       = "namedViews";
    inline constexpr auto WORKSPACES        = "workspaces";
    inline constexpr auto CATEGORIES        = "categories";
    inline constexpr auto CREATORS          = "creators";
    inline constexpr auto OPTIONS           = "options";
    inline constexpr auto INTERACTIVE_PICK  = "interactive_pick";
    inline constexpr auto HELP              = "help";

    // Submenus
    inline constexpr auto IMPORT            = "import";
    inline constexpr auto EXPORT            = "export";
    inline constexpr auto VIEWS_RESTORE     = "views_restore";
    inline constexpr auto DOCK_AREAS        = "dock_areas";
    inline constexpr auto TB_AREAS          = "tb_areas";
    inline constexpr auto ONLINE_DOCS       = "online_docs";

    // Widget Toggles & System
    inline constexpr auto DOCK_WIDGETS      = "dock_widgets";
    inline constexpr auto CAD_DOCK_WIDGETS  = "cad_dock_widgets";
    inline constexpr auto TOOL_OPTIONS      = "tool_options";
    inline constexpr auto BUILTIN_SYSTEM    = "builtin_system";
}

#endif
