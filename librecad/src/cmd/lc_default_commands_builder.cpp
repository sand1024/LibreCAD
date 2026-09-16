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

#include "lc_default_commands_builder.h"

#include <QSet>
#include "lc_action_type_mapper.h"
#include "lc_commandItems.h"

QList<CommandDefinition> LC_DefaultCommandsBuilder::buildDefaultCommands(const LC_ActionTypeMapper* mapper) {
    QList<CommandDefinition> commands;

    for (const auto& item : g_commandList) {
        QString actionName;
        if (mapper != nullptr) {
            actionName = mapper->actionNameFromType(item.actionType);
        }

        if (actionName.isEmpty()) {
            continue;
        }

        CommandDefinition def;
        def.actionName = actionName;
        def.customCommand = "";
        def.customKeycode = "";
        def.customAlias = "";
        commands.append(def);
    }

    return commands;
}

QList<KeywordDefinition> LC_DefaultCommandsBuilder::buildDefaultKeywords() {
    QList<KeywordDefinition> keywords;

    static const QStringList primaryKeywords = {
        "angle", "center", "chordlen", "close", "columns", "columnspacing",
        "distance", "equation", "factor", "height", "length", "number",
        "radius", "reversed", "row", "rowspacing", "size", "text",
        "through", "undo", "redo", "width", "back", "snap"
    };

    for (const auto& key : primaryKeywords) {
        KeywordDefinition kw;
        kw.key = key;
        kw.customKeyword = "";
        kw.customAlias = "";
        keywords.append(kw);
    }

    return keywords;
}

CommandsConfig LC_DefaultCommandsBuilder::createDefaultConfig(const LC_ActionTypeMapper* mapper) {
    CommandsConfig config;
    config.name = tr("Default Command Aliases");
    config.commands = buildDefaultCommands(mapper);
    config.keywords = buildDefaultKeywords();
    return config;
}
