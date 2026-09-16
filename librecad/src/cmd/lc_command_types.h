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

#ifndef LC_COMMAND_TYPES_H
#define LC_COMMAND_TYPES_H

#include <QList>
#include <QString>

struct CommandDefinition {
    QString actionName;               // Canonical action name (e.g. "DrawLine")
    QString customCommand;        // User override for full command (empty = inherit default, "-" = suppress)
    QString customKeycode;        // User override for 2-letter keycode
    QString customAlias;          // User override for alternative alias
};

struct KeywordDefinition {
    QString key;                      // Canonical English keyword (e.g. "close", "radius")
    QString customKeyword;        // User override for in-prompt keyword
    QString customAlias;          // User override for secondary keyword trigger
};

struct CommandsConfig {
    QString name;
    QList<CommandDefinition> commands;
    QList<KeywordDefinition> keywords;
};

#endif
