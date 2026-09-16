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

#include "lc_repository_commands.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

LC_RepositoryCommands::LC_RepositoryCommands(const QString& configDir)
    : LC_PresetRepositoryBase<CommandsConfig>(
          configDir, COMMANDS_EXTENSION, COMMANDS_FILE_IDENTIFIER, COMMANDS_INDEX_FILE) {
    QDir().mkpath(configDir);
    initializeIndex();
}


QJsonObject LC_RepositoryCommands::configToJson(const CommandsConfig& config) const {
    QJsonObject root;
    root["name"] = config.name;

    // Commands array
    QJsonArray cmdArray;
    for (const auto& cmd : config.commands) {
        QJsonObject obj;
        obj["action"] = cmd.actionName;
        if (!cmd.customCommand.isEmpty()) {
            obj["command"] = cmd.customCommand;
        }
        if (!cmd.customKeycode.isEmpty()) {
            obj["keycode"] = cmd.customKeycode;
        }
        if (!cmd.customAlias.isEmpty()) {
            obj["alias"] = cmd.customAlias;
        }
        cmdArray.append(obj);
    }
    root["commands"] = cmdArray;

    // Keywords array
    QJsonArray kwArray;
    for (const auto& kw : config.keywords) {
        QJsonObject obj;
        obj["key"] = kw.key;
        if (!kw.customKeyword.isEmpty()) {
            obj["keyword"] = kw.customKeyword;
        }

        if (!kw.customAlias.isEmpty()) {
            obj["alias"] = kw.customAlias;
        }
        kwArray.append(obj);
    }
    root["keywords"] = kwArray;

    return root;
}

bool LC_RepositoryCommands::configFromJson(const QJsonObject& json, CommandsConfig& config) const {
    if (!json.contains("name")) {
        return false;
    }

    config.name = json.value("name").toString();

    // Parse commands (supporting new keys with backward compatibility for legacy keys)
    config.commands.clear();
    const QJsonArray cmdArray = json.value("commands").toArray();
    for (const auto& val : cmdArray) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        CommandDefinition cmd;
        cmd.actionName = obj.value("action").toString();

        cmd.customCommand = obj.contains("command")
            ? obj.value("command").toString()
            : obj.value("main_cmd").toString();

        cmd.customKeycode = obj.contains("keycode")
            ? obj.value("keycode").toString()
            : obj.value("alias1").toString();

        cmd.customAlias = obj.contains("alias")
            ? obj.value("alias").toString()
            : obj.value("alias2").toString();

        config.commands.append(cmd);
    }

    // Parse keywords
    config.keywords.clear();
    const QJsonArray kwArray = json.value("keywords").toArray();
    for (const auto& val : kwArray) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        KeywordDefinition kw;
        kw.key = obj.value("key").toString();

        kw.customKeyword = obj.contains("keyword")
            ? obj.value("keyword").toString()
            : obj.value("custom_kw").toString();

        kw.customAlias = obj.contains("alias")
            ? obj.value("alias").toString()
            : obj.value("alias2").toString();

        config.keywords.append(kw);
    }

    return true;
}
