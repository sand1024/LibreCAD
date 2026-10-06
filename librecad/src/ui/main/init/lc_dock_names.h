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

#ifndef LC_DOCK_NAMES_H
#define LC_DOCK_NAMES_H

#include <QString>
#include <cstring>

#include "lc_group_names.h"

namespace LC_DockNames {
    // Prefixes
    inline constexpr auto PREFIX_DOCK       = "dock_";
    inline constexpr auto PREFIX_DOCK_CAD   = "dock_cad_";

    // Standard Tool Window Docks
    inline constexpr auto PROPERTIES        = "properties";
    inline constexpr auto LAYERS            = "layers";
    inline constexpr auto LAYER_TREE        = "layer_tree";
    inline constexpr auto COMMAND           = "command";
    inline constexpr auto BLOCKS            = "blocks";
    inline constexpr auto LIBRARY           = "library";
    inline constexpr auto ENTITY_INFO       = "entity_info";
    inline constexpr auto PEN_PALETTE       = "pen_palette";
    inline constexpr auto PEN_WIZARD        = "pen_wizard";
    inline constexpr auto UCS               = "ucs";
    inline constexpr auto NAMED_VIEWS       = "named_views";

    // CAD Dock Widgets
    inline constexpr auto CAD_MEGA                    = "mega";
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

    inline QString standardDockName(const QString& name) {
        if (name.startsWith(QLatin1String(PREFIX_DOCK))) {
            return name.toLower();
        }
        return QString(PREFIX_DOCK) + name.toLower();
    }

    inline QString cadDockName(const QString& category) {
        if (category.startsWith(QLatin1String(PREFIX_DOCK_CAD))) {
            return category.toLower();
        }
        return QString(PREFIX_DOCK_CAD) + category.toLower();
    }

    inline QString cleanName(const QString& name) {
        QString token = name;
        if (token.startsWith(QLatin1String(PREFIX_DOCK_CAD))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_DOCK_CAD)));
        }
        else if (token.startsWith(QLatin1String(PREFIX_DOCK))) {
            token.remove(0, static_cast<qsizetype>(std::strlen(PREFIX_DOCK)));
        }
        return token;
    }
}

#endif // LC_DOCK_NAMES_H
