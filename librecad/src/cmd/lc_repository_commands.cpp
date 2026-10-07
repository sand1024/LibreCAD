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

#include "lc_action_type_mapper.h"
#include "lc_default_command_aliases.h"
#include "lc_command_manager.h"
#include "lc_default_commands_builder.h"
#include "lc_settings_paths.h"

namespace {
    inline constexpr const char* COMMANDS_EXTENSION = ".lcca";
    inline constexpr const char* COMMANDS_FILE_IDENTIFIER = "LibreCAD Config: Command Aliases";
    inline constexpr const char* COMMANDS_INDEX_FILE = "commands_index.lcix";
}

LC_RepositoryCommands::LC_RepositoryCommands(const QString& configDir)
    : LC_PresetRepositoryBase<CommandsConfig>(
          configDir, COMMANDS_EXTENSION, COMMANDS_FILE_IDENTIFIER, COMMANDS_INDEX_FILE) {
}

QJsonObject LC_RepositoryCommands::configToJson(const CommandsConfig& config) const {
    QJsonObject root;

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
        if (!cmd.customAliases.isEmpty()) {
            obj["aliases"] = QJsonArray::fromStringList(cmd.customAliases);
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
    config.commands.clear();
    const QJsonArray cmdArray = json.value("commands").toArray();
    for (const auto& val : cmdArray) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        CommandDefinition cmd;
        cmd.actionName = obj.value("action").toString();
        cmd.customCommand = obj.contains("command") ? obj.value("command").toString() : "";
        cmd.customKeycode = obj.contains("keycode") ? obj.value("keycode").toString() : "";
        if (obj.contains("aliases") && obj.value("aliases").isArray()) {
            const QJsonArray arr = obj.value("aliases").toArray();
            for (const auto& a : arr) {
                const QString str = a.toString().trimmed();
                if (!str.isEmpty()) {
                    cmd.customAliases.append(str);
                }
            }
        }
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
        kw.customKeyword = obj.contains("keyword") ? obj.value("keyword").toString() :"";
        kw.customAlias = obj.contains("alias") ? obj.value("alias").toString() : "";
        config.keywords.append(kw);
    }

    return true;
}

bool LC_RepositoryCommands::importLegacyAliasFile(const QString& filePath, CommandsConfig& outConfig,
                                                  const LC_ActionTypeMapper* mapper) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    outConfig = LC_DefaultCommandsBuilder::createDefaultConfig(mapper);

    struct ActionDefaults {
        QStringList cmds;
        QStringList keys;
        QStringList aliases;
    };

    QMap<QString, QString> cmdToName;
    QMap<QString, ActionDefaults> actionDefaultsMap;

    auto insertTrigger = [&cmdToName](const LC_CommandTrigger& trigger, const QString& actionName) {
        if (trigger.isEmpty()) {
            return;
        }
        cmdToName.insert(QString::fromUtf8(trigger.text).toLower(), actionName);
        const QString trans = LC_CommandManager::resolveCommandText(trigger);
        if (!trans.isEmpty()) {
            cmdToName.insert(trans.toLower(), actionName);
        }
    };

    if (mapper != nullptr) {
        for (const auto& item : g_commandList) {
            const QString actionName = mapper->actionNameFromType(item.actionType);
            if (!actionName.isEmpty()) {
                insertTrigger(item.primary, actionName);
                insertTrigger(item.keycode, actionName);
                for (const auto& aliasTrigger : item.aliases) {
                    insertTrigger(aliasTrigger, actionName);
                }

                ActionDefaults defs;
                LC_CommandManager::collectActionDefaults(item.actionType, defs.cmds, defs.keys, defs.aliases);
                actionDefaultsMap.insert(actionName, defs);
            }
        }
    }

    QTextStream ts(&file);
    static const QRegularExpression wsRe(R"(\s+)");

    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QStringList tokens = line.split(wsRe, Qt::SkipEmptyParts);
        if (tokens.size() < 2) {
            continue;
        }

        const QString alias = tokens.at(0).toLower();
        const QString targetCmd = tokens.at(1).toLower();

        const auto it = cmdToName.find(targetCmd);
        if (it == cmdToName.end()) {
            continue;
        }

        const QString actionName = it.value();
        const ActionDefaults& defs = actionDefaultsMap.value(actionName);

        // If this alias is already part of the action's built-in defaults, do not treat it as a user override
        const bool isBuiltInDefault = defs.keys.contains(alias, Qt::CaseInsensitive)
                                   || defs.aliases.contains(alias, Qt::CaseInsensitive)
                                   || defs.cmds.contains(alias, Qt::CaseInsensitive);
        if (isBuiltInDefault) {
            continue;
        }

        // Genuine custom override: assign to customKeycode if 2 letters and empty, otherwise append to customAliases
        for (auto& def : outConfig.commands) {
            if (def.actionName == actionName) {
                if (alias.length() == 2 && def.customKeycode.isEmpty()) {
                    def.customKeycode = alias;
                }
                else if (!def.customAliases.contains(alias, Qt::CaseInsensitive)) {
                    def.customAliases.append(alias);
                }
                break;
            }
        }
    }

    outConfig.name = QFileInfo(filePath).baseName();
    return true;
}

bool LC_RepositoryCommands::migrateLegacyAliasIfNeeded(const LC_ActionTypeMapper* mapper) {
    const QString legacyAliasPath = CFG_Paths::o_OtherSettingsDir.get() + QStringLiteral("/librecad.alias");
    if (!QFile::exists(legacyAliasPath)) {
        return false;
    }

    const QString presetDisplayName = QObject::tr("Imported Legacy Aliases");
    const QString targetKey = sanitizeFileName(presetDisplayName).toLower();

    // Check if the legacy file has already been migrated into a preset
    if (exists(targetKey)) {
        return false;
    }

    CommandsConfig migratedConfig;
    if (!importLegacyAliasFile(legacyAliasPath, migratedConfig, mapper)) {
        return false;
    }

    migratedConfig.name = presetDisplayName;
    QString outKey;
    if (!save(presetDisplayName, migratedConfig, outKey)) {
        return false;
    }

    // If the active scheme is still the default/unspecified, automatically activate the migrated scheme
    if (CFG_AppState::o_ActiveCommandsScheme.get().isEmpty() ||
        CFG_AppState::o_ActiveCommandsScheme == CFG_AppState::DEFAULT_THEME_KEY) {
        CFG_AppState::o_ActiveCommandsScheme = outKey;
        }

    return true;
}
