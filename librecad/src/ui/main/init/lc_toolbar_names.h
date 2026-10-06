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

#ifndef LC_TOOLBAR_NAMES_H
#define LC_TOOLBAR_NAMES_H

#include <QString>
#include <cstring>

#include "lc_group_names.h"

namespace LC_ToolbarNames {
    // Prefixes & Suffixes
    inline constexpr auto PREFIX_STANDARD   = "tb_s_";
    inline constexpr auto PREFIX_CAD        = "tb_cad_";
    inline constexpr auto PREFIX_CUSTOM     = "tb_c_";
    inline constexpr auto PREFIX_STATUS     = "tb_stat_";
    inline constexpr auto SUFFIX_TOOLBAR    = "_toolbar";

    // Standard Application Toolbars
    inline constexpr auto FILE              = "file";
    inline constexpr auto EDIT              = "edit";
    inline constexpr auto VIEW              = "view";
    inline constexpr auto OPTIONS           = "options";
    inline constexpr auto INFO_CURSOR       = "info_cursor";
    inline constexpr auto WORKSPACES        = "workspaces";
    inline constexpr auto NAMED_VIEWS       = "named_views";
    inline constexpr auto UCS               = "ucs";
    inline constexpr auto ENTITY_LAYER      = "entity_layer";
    inline constexpr auto CATEGORIES        = "categories";
    inline constexpr auto DOCK_AREAS        = "dock_areas";
    inline constexpr auto CREATORS          = "creators";

    // Host Toolbars
    inline constexpr auto PEN               = "pen";
    inline constexpr auto SNAP              = "snap";
    inline constexpr auto TOOL_OPTIONS      = "tool_options";

    // CAD Drafting Toolbars
    inline constexpr auto CAD_LINE          = LC_GroupNames::LINE;
    inline constexpr auto CAD_POINT         = LC_GroupNames::POINT;
    inline constexpr auto CAD_SHAPE         = LC_GroupNames::SHAPE;
    inline constexpr auto CAD_CIRCLE        = LC_GroupNames::CIRCLE;
    inline constexpr auto CAD_CURVE         = LC_GroupNames::CURVE;
    inline constexpr auto CAD_SPLINE        = LC_GroupNames::SPLINE;
    inline constexpr auto CAD_ELLIPSE       = LC_GroupNames::ELLIPSE;
    inline constexpr auto CAD_POLYLINE      = LC_GroupNames::POLYLINE;
    inline constexpr auto CAD_TEXT          = LC_GroupNames::TEXT;
    inline constexpr auto CAD_DIMENSION     = LC_GroupNames::DIMENSION;
    inline constexpr auto CAD_OTHER         = LC_GroupNames::OTHER;
    inline constexpr auto CAD_MODIFY        = LC_GroupNames::MODIFY;
    inline constexpr auto CAD_INFO          = LC_GroupNames::INFO;
    inline constexpr auto CAD_SELECT        = LC_GroupNames::SELECT;
    inline constexpr auto CAD_ORDER         = LC_GroupNames::ORDER;

    // Status Bar Toolbars
    inline constexpr auto STAT_COORDINATES  = "coordinates";
    inline constexpr auto STAT_REL_ZERO     = "rel_zero";
    inline constexpr auto STAT_MOUSE        = "mouse";
    inline constexpr auto STAT_SELECTION    = "selection";
    inline constexpr auto STAT_ACTIVE_LAYER = "active_layer";
    inline constexpr auto STAT_GRID_STATUS  = "grid_status";
    inline constexpr auto STAT_UCS_STATUS   = "ucs_status";
    inline constexpr auto STAT_ANGLES_BASIS = "angles_basis";

    inline QString standardToolBarName(const QString& name) {
        return QString(PREFIX_STANDARD) + name.toLower() + SUFFIX_TOOLBAR;
    }

    inline QString cadToolBarName(const QString& category) {
        return QString(PREFIX_CAD) + category.toLower() + SUFFIX_TOOLBAR;
    }

    inline QString customToolBarName(const QString& name) {
        return QString(PREFIX_CUSTOM) + name.toLower() + SUFFIX_TOOLBAR;
    }

    inline QString statusToolBarName(const QString& name) {
        return QString(PREFIX_STATUS) + name.toLower() + SUFFIX_TOOLBAR;
    }

    inline QString normalizeToolBarName(const QString& name) {
        if (name.endsWith(QLatin1String(SUFFIX_TOOLBAR))) {
            return name;
        }
        return name + SUFFIX_TOOLBAR;
    }

    inline QString cleanName(const QString& name) {
        QString token = name;
        if (token.endsWith(QLatin1String(SUFFIX_TOOLBAR))) {
            token.chop(static_cast<qsizetype>(std::strlen(SUFFIX_TOOLBAR)));
        }
        if (token.startsWith(QLatin1String(PREFIX_CAD))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_CAD)));
        }
        else if (token.startsWith(QLatin1String(PREFIX_STANDARD))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_STANDARD)));
        }
        else if (token.startsWith(QLatin1String(PREFIX_CUSTOM))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_CUSTOM)));
        }
        else if (token.startsWith(QLatin1String(PREFIX_STATUS))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_STATUS)));
        }
        return token;
    }
}

#endif
