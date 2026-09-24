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

#include "lc_command_manager.h"

#include <QRegularExpression>

#include "lc_action_type_mapper.h"
#include "lc_commandItems.h"
#include "lc_default_commands_builder.h"
#include "lc_repository_commands.h"
#include "lc_settings_app_state.h"
#include "lc_wait_cursor_guard.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"

namespace {
    constexpr auto PREFIX_FN = "Fn";
    constexpr auto PREFIX_ALT = "Alt-";
    constexpr auto PREFIX_META = "Meta-";

    template<typename T1, typename T2>
    bool isCollisionFree(const std::map<T1, T2>& lookUp, const T1& key, const T2& value, const QString& cmd = QString()) {
        if (key == cmd) {
            return false;
        }
        if (lookUp.count(key) == 0 || lookUp.at(key) == value) {
            return true;
        }
        return false;
    }

    // Resolves an action ignoring case only if the match is completely unambiguous
    RS2::ActionType findActionIgnoringCase(const QString& command,
                                           const std::map<QString, RS2::ActionType>& mainCommands,
                                           const std::map<QString, RS2::ActionType>& shortCommands,
                                           QString* outAmbiguityDetails = nullptr) {
        QMap<RS2::ActionType, QString> matchingActions;

        for (const auto* table : {&mainCommands, &shortCommands}) {
            for (const auto& [key, action] : *table) {
                if (key.compare(command, Qt::CaseInsensitive) == 0) {
                    matchingActions.insert(action, key);
                }
            }
        }

        if (matchingActions.isEmpty()) {
            return RS2::ActionNone;
        }

        if (matchingActions.size() == 1) {
            return matchingActions.begin().key();
        }

        // Multiple distinct actions match case-insensitively -> Ambiguity!
        if (outAmbiguityDetails != nullptr) {
            QStringList triggers;
            for (auto it = matchingActions.constBegin(); it != matchingActions.constEnd(); ++it) {
                triggers.append(it.value());
            }
            *outAmbiguityDetails = triggers.join(", ");
        }

        return RS2::ActionNone;
    }
}

LC_CommandManager::LC_CommandManager(LC_RepositoryCommands* repo): m_repository(repo) {
    populateFactoryDefaults();
}

LC_CommandManager::~LC_CommandManager() {
}

void LC_CommandManager::populateFactoryDefaults() {
    m_mainCommands.clear();
    m_shortCommands.clear();
    m_actionToCommand.clear();
    m_cmdTranslation.clear();
    m_revTranslation.clear();

    for (const auto& [fullCmdList, aliasList, action] : g_commandList) {
        for (const auto& [fullCmd, cmdTranslation] : fullCmdList) {
            if (fullCmd == cmdTranslation) {
                continue;
            }
            if (isCollisionFree(m_cmdTranslation, fullCmd, cmdTranslation)) {
                m_cmdTranslation.emplace(fullCmd, cmdTranslation);
            }
            if (isCollisionFree(m_mainCommands, cmdTranslation, action,
                                m_actionToCommand.count(action) ? m_actionToCommand.at(action) : QString())) {
                m_mainCommands.emplace(cmdTranslation, action);
                m_actionToCommand.emplace(action, cmdTranslation);
            }
        }
        for (const auto& [fullCmd, cmdTranslation] : fullCmdList) {
            if (isCollisionFree(m_mainCommands, fullCmd, action,
                                m_actionToCommand.count(action) ? m_actionToCommand.at(action) : QString())) {
                m_mainCommands.emplace(fullCmd, action);
                m_actionToCommand.emplace(action, fullCmd);
            }
        }
        for (const auto& [alias, aliasTranslation] : aliasList) {
            if (alias == aliasTranslation) {
                continue;
            }
            if (isCollisionFree(m_cmdTranslation, alias, aliasTranslation)) {
                m_cmdTranslation.emplace(alias, aliasTranslation);
            }
            if (isCollisionFree(m_shortCommands, aliasTranslation, action,
                                m_actionToCommand.count(action) ? m_actionToCommand.at(action) : QString())) {
                m_shortCommands.emplace(aliasTranslation, action);
                if (m_actionToCommand.count(action) == 0) {
                    m_actionToCommand.emplace(action, aliasTranslation);
                }
            }
        }
        for (const auto& [alias, aliasTranslation] : aliasList) {
            if (isCollisionFree(m_shortCommands, alias, action,
                                m_actionToCommand.count(action) ? m_actionToCommand.at(action) : QString())) {
                m_shortCommands.emplace(alias, action);
                if (m_actionToCommand.count(action) == 0) {
                    m_actionToCommand.emplace(action, aliasTranslation);
                }
            }
        }
    }

    for (const auto& [cmd, trans] : g_transList) {
        m_cmdTranslation[cmd] = trans;
    }

    for (const auto& [cmd, trans] : m_cmdTranslation) {
        m_revTranslation[trans] = cmd;
        if (m_shortCommands.count(trans) == 1) {
            m_shortCommands[cmd] = m_shortCommands[trans];
        }
    }

    for (const auto& [cmd, action] : m_mainCommands) {
        m_actionToCommand[action] = cmd;
    }
}

RS2::ActionType LC_CommandManager::commandToAction(const QString& cmd) const {
    if (m_mainCommands.count(cmd) == 1) {
        return m_mainCommands.at(cmd);
    }
    if (m_shortCommands.count(cmd) == 1) {
        return m_shortCommands.at(cmd);
    }
    if (m_cmdTranslation.count(cmd) == 1) {
        const QString trans = m_cmdTranslation.at(cmd);
        if (m_mainCommands.count(trans) == 1) {
            return m_mainCommands.at(trans);
        }
        if (m_shortCommands.count(trans) == 1) {
            return m_shortCommands.at(trans);
        }
    }
    return RS2::ActionNone;
}

RS2::ActionType LC_CommandManager::cmdToAction(const QString& cmd, const bool verbose, QString* outAmbiguityDetails) const {
    const QString trimmed = cmd.trimmed();
    if (trimmed.isEmpty()) {
        return RS2::ActionNone;
    }

    RS2::ActionType ret = RS2::ActionNone;

    // 1. Tier 1: Exact case lookup (O(1))
    for (const auto& table : {m_mainCommands, m_shortCommands}) {
        if (table.count(trimmed)) {
            ret = table.at(trimmed);
            break;
        }
    }

    // 2. Tier 2: Unambiguous case-insensitive fallback
    if (ret == RS2::ActionNone) {
        ret = findActionIgnoringCase(trimmed, m_mainCommands, m_shortCommands, outAmbiguityDetails);
    }

    if (ret == RS2::ActionNone) {
        return RS2::ActionNone;
    }

    if (!verbose) {
        return ret;
    }

    for (const auto& p : m_mainCommands) {
        if (p.second == ret) {
            return ret;
        }
    }
    return ret;
}


RS2::ActionType LC_CommandManager::keycodeToAction(const QString& code) const {
    const QString trimmed = code.trimmed();
    if (trimmed.isEmpty()) {
        return RS2::ActionNone;
    }

    if (!(trimmed.startsWith(PREFIX_FN) || trimmed.startsWith(PREFIX_ALT) || trimmed.startsWith(PREFIX_META))) {
        if (!trimmed.contains(QRegularExpression("^[a-zA-Z].*"))) {
            return RS2::ActionNone;
        }
    }

    // 1. Tier 1: Exact case lookup
    auto action = commandToAction(trimmed);

    // 2. Tier 2: Unambiguous case-insensitive fallback
    if (action == RS2::ActionNone) {
        action = findActionIgnoringCase(trimmed, m_mainCommands, m_shortCommands);
    }

    if (action != RS2::ActionNone) {
        const QString& cmd = (m_actionToCommand.count(action) == 1) ? m_actionToCommand.at(action) : QString();
        RS_DIALOGFACTORY->commandMessage(QObject::tr("keycode: %1 (%2)").arg(trimmed, cmd));
    }
    else {
        RS_DIALOGFACTORY->commandMessage(QObject::tr("invalid keycode: %1").arg(trimmed));
    }

    return action;
}

QString LC_CommandManager::command(const QString& cmd) const {
    const auto it = m_cmdTranslation.find(cmd);
    if (it != m_cmdTranslation.end()) {
        return it->second;
    }
    return QString();
}

bool LC_CommandManager::checkCommand(const QString& cmd, const QString& str, RS2::ActionType) const {
    const QString strl = str.toLower().trimmed();
    const QString cmdLower = cmd.toLower().trimmed();

    const auto it = m_cmdTranslation.find(cmdLower);
    if (it != m_cmdTranslation.end()) {
        const RS2::ActionType type0 = cmdToAction(it->second, false);
        if (type0 != RS2::ActionNone) {
            return type0 == cmdToAction(strl, false);
        }
    }

    const auto itStr = m_cmdTranslation.find(strl);
    if (itStr != m_cmdTranslation.end()) {
        return itStr->second == cmdLower;
    }
    return false;
}

QStringList LC_CommandManager::complete(const QString& prefix) const {
    QStringList ret;
    for (const auto& [fst, snd] : m_mainCommands) {
        if (fst.startsWith(prefix, Qt::CaseInsensitive)) {
            ret << fst;
        }
    }
    ret.sort();
    return ret;
}

QString LC_CommandManager::getCommandForAction(RS2::ActionType action) const {
    if (m_actionToCommand.count(action)) {
        return m_actionToCommand.at(action);
    }
    return QString();
}

QStringList LC_CommandManager::getCommandsForAction(RS2::ActionType action) const {
    QStringList triggers;

    // 1. Primary command first
    const auto it = m_actionToCommand.find(action);
    if (it != m_actionToCommand.end() && !it->second.isEmpty()) {
        triggers.append(it->second);
    }

    // 2. Full command triggers
    for (const auto& [cmd, act] : m_mainCommands) {
        if (act == action && !triggers.contains(cmd, Qt::CaseInsensitive)) {
            triggers.append(cmd);
        }
    }

    // 3. Short aliases and keycodes
    for (const auto& [alias, act] : m_shortCommands) {
        if (act == action && !triggers.contains(alias, Qt::CaseInsensitive)) {
            triggers.append(alias);
        }
    }

    // Sort shortest string first, tie-breaking alphabetically ignoring case
    std::sort(triggers.begin(), triggers.end(), [](const QString& a, const QString& b) {
        if (a.length() != b.length()) {
            return a.length() < b.length();
        }
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });

    return triggers;
}

QString LC_CommandManager::msgAvailableCommands() const {
    return QObject::tr("Available commands:");
}

LC_RepositoryCommands* LC_CommandManager::getRepository() const {
    return m_repository;
}

void LC_CommandManager::applyCommandsScheme(const CommandsConfig& config, const LC_ActionTypeMapper* mapper) {
    m_activeConfig = config;
    populateFactoryDefaults();

    if (mapper == nullptr) {
        return;
    }

    // Apply custom command overrides
    for (const auto& cmd : config.commands) {
        const RS2::ActionType action = mapper->actionTypeFromName(cmd.actionName);
        if (action == RS2::ActionNone) {
            continue;
        }

        // 1. Full command slot
        if (!cmd.customCommand.isEmpty()) {
            const QString cTrimmed = cmd.customCommand.trimmed();
            if (cTrimmed == "-") {
                for (auto it = m_mainCommands.begin(); it != m_mainCommands.end();) {
                    if (it->second == action) {
                        it = m_mainCommands.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }
            else {
                m_mainCommands[cTrimmed] = action;
                m_actionToCommand[action] = cTrimmed;
            }
        }

        // 2. Keycode slot
        if (!cmd.customKeycode.isEmpty()) {
            const QString kTrimmed = cmd.customKeycode.trimmed();
            if (kTrimmed == "-") {
                // Erase default keycode from short commands
                for (auto it = m_shortCommands.begin(); it != m_shortCommands.end();) {
                    if (it->second == action && it->first.length() == 2) {
                        it = m_shortCommands.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }
            else {
                m_shortCommands[kTrimmed] = action;
            }
        }

        // 3. Alias slot
        if (!cmd.customAlias.isEmpty()) {
            const QString aTrimmed = cmd.customAlias.trimmed();
            if (aTrimmed == "-") {
                // Erase default aliases (non-keycodes) from short commands
                for (auto it = m_shortCommands.begin(); it != m_shortCommands.end();) {
                    if (it->second == action && it->first.length() != 2) {
                        it = m_shortCommands.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }
            else {
                m_shortCommands[aTrimmed] = action;
            }
        }
    }

    // Apply custom keyword overrides
    for (const auto& kw : config.keywords) {
        if (!kw.customKeyword.isEmpty() && kw.customKeyword != "-") {
            m_cmdTranslation[kw.key] = kw.customKeyword.trimmed();
        }
        if (!kw.customAlias.isEmpty() && kw.customAlias != "-") {
            m_cmdTranslation[kw.customAlias.trimmed()] = kw.key;
        }
    }

    for (const auto& [cmd, trans] : m_cmdTranslation) {
        m_revTranslation[trans] = cmd;
        if (m_shortCommands.count(trans) == 1) {
            m_shortCommands[cmd] = m_shortCommands[trans];
        }
    }
}

void LC_CommandManager::loadActiveScheme(const LC_ActionTypeMapper* mapper) {
    LC_WaitCursorGuard guard;
    if (m_repository == nullptr) {
        populateFactoryDefaults();
        return;
    }

    const QString activeKey = CFG_AppState::o_ActiveCommandsScheme;
    CommandsConfig config;

    if (activeKey == CFG_AppState::DEFAULT_THEME_KEY || activeKey.isEmpty() || !m_repository->loadByKey(activeKey, config)) {
        config = LC_DefaultCommandsBuilder::createDefaultConfig(mapper);
    }

    applyCommandsScheme(config, mapper);
}
