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
#include "lc_default_command_aliases.h"
#include "lc_default_commands_builder.h"
#include "lc_repository_commands.h"
#include "lc_settings_app_state.h"
#include "lc_wait_cursor_guard.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_system.h"

namespace {
    constexpr auto PREFIX_FN = "Fn";
    constexpr auto PREFIX_ALT = "Alt-";
    constexpr auto PREFIX_META = "Meta-";

}

LC_CommandManager::LC_CommandManager(LC_RepositoryCommands* repo) : m_repository(repo) {
    populateFactoryDefaults();
}

LC_CommandManager::~LC_CommandManager() {
}


void LC_CommandManager::registerCommandTrigger(const QString& trigger, RS2::ActionType action) {
    if (trigger.isEmpty() || action == RS2::ActionNone) {
        return;
    }

    // Tier 1: Exact case lookup
    m_exactCommands.insert(trigger, action);

    // Tier 2: Precomputed unambiguous lowercase lookup
    const QString lower = trigger.toLower();
    auto it = m_caseInsensitiveCommands.find(lower);
    if (it == m_caseInsensitiveCommands.end()) {
        CaseInsensitiveEntry entry;
        entry.action = action;
        entry.isAmbiguous = false;
        entry.candidateTriggers.append(trigger);
        m_caseInsensitiveCommands.insert(lower, entry);
    }
    else {
        if (!it->candidateTriggers.contains(trigger)) {
            it->candidateTriggers.append(trigger);
        }
        if (it->action != action) {
            it->isAmbiguous = true;
            it->action = RS2::ActionNone;
        }
    }
}

void LC_CommandManager::populateFactoryDefaults() {
    applyCommandsScheme(CommandsConfig{}, nullptr);
}

RS2::ActionType LC_CommandManager::cmdToAction(const QString& cmd, const bool verbose, QString* outAmbiguityDetails) const {
    Q_UNUSED(verbose);
    const QString trimmed = cmd.trimmed();
    if (trimmed.isEmpty()) {
        return RS2::ActionNone;
    }

    // Tier 1: Exact case lookup (O(1))
    const auto itExact = m_exactCommands.constFind(trimmed);
    if (itExact != m_exactCommands.constEnd()) {
        return itExact.value();
    }

    // Tier 2: Unambiguous case-insensitive fallback (O(1))
    const auto itLower = m_caseInsensitiveCommands.constFind(trimmed.toLower());
    if (itLower != m_caseInsensitiveCommands.constEnd()) {
        const auto& entry = itLower.value();
        if (!entry.isAmbiguous) {
            return entry.action;
        }
        if (outAmbiguityDetails != nullptr) {
            *outAmbiguityDetails = entry.candidateTriggers.join(QStringLiteral(", "));
        }
    }

    return RS2::ActionNone;
}

RS2::ActionType LC_CommandManager::keycodeToAction(const QString& code) const {
    const QString trimmed = code.trimmed();
    if (trimmed.isEmpty()) {
        return RS2::ActionNone;
    }

    if (!trimmed.startsWith(QLatin1String(PREFIX_FN)) &&
        !trimmed.startsWith(QLatin1String(PREFIX_ALT)) &&
        !trimmed.startsWith(QLatin1String(PREFIX_META))) {
        if (!trimmed.at(0).isLetter()) {
            return RS2::ActionNone;
        }
    }

    const RS2::ActionType action = cmdToAction(trimmed, false);
    if (action != RS2::ActionNone) {
        const QString cmd = m_actionToCommand.value(action);
        RS_DIALOGFACTORY->commandMessage(QObject::tr("keycode: %1 (%2)").arg(trimmed, cmd));
    }
    else {
        RS_DIALOGFACTORY->commandMessage(QObject::tr("invalid keycode: %1").arg(trimmed));
    }

    return action;
}

bool LC_CommandManager::checkCommand(const QString& keyword, const QString& input, RS2::ActionType) const {
    const QString cleanInput = input.trimmed().toLower();
    const QString canonicalTarget = keyword.trimmed().toLower();

    if (cleanInput.isEmpty() || canonicalTarget.isEmpty()) {
        return false;
    }

    if (cleanInput == canonicalTarget) {
        return true;
    }

    const auto it = m_keywordToCanonical.constFind(cleanInput);
    if (it != m_keywordToCanonical.constEnd()) {
        return it.value() == canonicalTarget;
    }

    return false;
}

QString LC_CommandManager::command(const QString& cmd) const {
    const QString cleanCmd = cmd.trimmed();
    return m_canonicalToLocalizedKeyword.value(cleanCmd.toLower(), cleanCmd);
}

QStringList LC_CommandManager::complete(const QString& prefix) const {
    const QString cleanPrefix = prefix.trimmed();
    if (cleanPrefix.isEmpty()) {
        return QStringList();
    }

    QStringList results;
    for (const auto& candidate : m_completionCandidates) {
        if (candidate.startsWith(cleanPrefix, Qt::CaseInsensitive)) {
            results.append(candidate);
        }
    }
    return results;
}

QString LC_CommandManager::getCommandForAction(RS2::ActionType action) const {
    return m_actionToCommand.value(action);
}

QStringList LC_CommandManager::getCommandsForAction(RS2::ActionType action) const {
    return m_actionCommandsCache.value(action);
}

void LC_CommandManager::rebuildActionCommandsCache() {
    m_actionCommandsCache.clear();

    // 1. Group triggers directly by ActionType in the cache
    for (auto it = m_exactCommands.constBegin(); it != m_exactCommands.constEnd(); ++it) {
        const RS2::ActionType act = it.value();
        const QString& trigger = it.key();

        if (act != RS2::ActionNone && !trigger.isEmpty()) {
            m_actionCommandsCache[act].append(trigger);
        }
    }

    // 2. Sort each action's triggers: shortest first, tie-breaking alphabetically
    for (auto it = m_actionCommandsCache.begin(); it != m_actionCommandsCache.end(); ++it) {
        QStringList& triggers = it.value();
        std::sort(triggers.begin(), triggers.end(), [](const QString& a, const QString& b) {
            if (a.length() != b.length()) {
                return a.length() < b.length();
            }
            return a.compare(b, Qt::CaseInsensitive) < 0;
        });
    }
}

void LC_CommandManager::rebuildCompletionCandidates(const CommandsConfig& config, const LC_ActionTypeMapper* mapper) {
    m_completionCandidates.clear();

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    QStringList primaryList;
    QStringList secondaryList;

    for (const auto& item : g_commandList) {
        const QString actionName = (mapper != nullptr) ? mapper->actionNameFromType(item.actionType) : QString();
        const CommandDefinition cmdDef = (!actionName.isEmpty()) ? cmdMap.value(actionName) : CommandDefinition{};

        // 1. Primary commands & user custom overrides
        if (cmdDef.customCommand == "-") {
            // Suppressed: omit from completion
        }
        else if (!cmdDef.customCommand.trimmed().isEmpty()) {
            primaryList.append(cmdDef.customCommand.trimmed());
        }
        else if (!item.primary.isEmpty()) {
            const QString trans = resolveCommandText(item.primary);
            const QString raw = QString::fromUtf8(item.primary.text);
            if (!trans.isEmpty()) {
                primaryList.append(trans);
            }
            if (!raw.isEmpty() && raw != trans) {
                primaryList.append(raw);
            }
        }

        // Custom aliases (excluding '-')
        for (const auto& ca : cmdDef.customAliases) {
            const QString trimmed = ca.trimmed();
            if (!trimmed.isEmpty() && trimmed != "-") {
                primaryList.append(trimmed);
            }
        }

        // 2. Secondary built-in aliases (only if not suppressed, length > 2)
        const bool suppressDefaultAliases = cmdDef.customAliases.contains("-");
        if (!suppressDefaultAliases) {
            for (const auto& al : item.aliases) {
                if (al.isEmpty()) {
                    continue;
                }
                const QString raw = QString::fromUtf8(al.text);
                if (raw.length() > 2) {
                    secondaryList.append(raw);
                    const QString trans = resolveCommandText(al);
                    if (!trans.isEmpty() && trans != raw) {
                        secondaryList.append(trans);
                    }
                }
            }
        }
    }

    primaryList.sort(Qt::CaseInsensitive);
    secondaryList.sort(Qt::CaseInsensitive);

    for (const auto& str : primaryList) {
        if (!str.isEmpty() && !m_completionCandidates.contains(str, Qt::CaseInsensitive)) {
            m_completionCandidates.append(str);
        }
    }

    for (const auto& str : secondaryList) {
        if (!str.isEmpty() && !m_completionCandidates.contains(str, Qt::CaseInsensitive)) {
            m_completionCandidates.append(str);
        }
    }
}

void LC_CommandManager::applyCommandsScheme(const CommandsConfig& config, const LC_ActionTypeMapper* mapper) {
    m_activeConfig = config;

    m_exactCommands.clear();
    m_caseInsensitiveCommands.clear();
    m_actionToCommand.clear();
    m_keywordToCanonical.clear();
    m_canonicalToLocalizedKeyword.clear();
    m_actionCommandsCache.clear();
    m_completionCandidates.clear();

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    QMap<QString, KeywordDefinition> kwMap;
    for (const auto& kw : config.keywords) {
        kwMap.insert(kw.key, kw);
    }

    // 1. Process Actions from g_commandList
    for (const auto& item : g_commandList) {
        const RS2::ActionType action = item.actionType;
        const QString actionName = (mapper != nullptr) ? mapper->actionNameFromType(action) : QString();
        const CommandDefinition cmdDef = (!actionName.isEmpty()) ? cmdMap.value(actionName) : CommandDefinition{};

        // Primary command slot
        if (cmdDef.customCommand == "-") {
            // Suppressed entirely: do not register any primary command trigger
        }
        else if (!cmdDef.customCommand.trimmed().isEmpty()) {
            const QString custom = cmdDef.customCommand.trimmed();
            registerCommandTrigger(custom, action);
            m_actionToCommand.insert(action, custom);
        }
        else if (!item.primary.isEmpty()) {
            const QString transCmd = resolveCommandText(item.primary);
            const QString rawCmd = QString::fromUtf8(item.primary.text);

            registerCommandTrigger(transCmd, action);
            if (!rawCmd.isEmpty() && rawCmd != transCmd) {
                registerCommandTrigger(rawCmd, action);
            }
            if (!m_actionToCommand.contains(action)) {
                m_actionToCommand.insert(action, transCmd);
            }
        }

        // Keycode slot
        if (cmdDef.customKeycode == "-") {
            // Suppressed entirely: do not register any keycode
        }
        else if (!cmdDef.customKeycode.trimmed().isEmpty()) {
            registerCommandTrigger(cmdDef.customKeycode.trimmed(), action);
        }
        else if (!item.keycode.isEmpty()) {
            const QString transKey = resolveCommandText(item.keycode);
            const QString rawKey = QString::fromUtf8(item.keycode.text);

            registerCommandTrigger(transKey, action);
            if (!rawKey.isEmpty() && rawKey != transKey) {
                registerCommandTrigger(rawKey, action);
            }
        }

        // Aliases slot (Custom + unsuppressed defaults)
        const bool suppressDefaultAliases = cmdDef.customAliases.contains("-");

        // Register custom aliases (excluding '-')
        for (const auto& ca : cmdDef.customAliases) {
            const QString trimmed = ca.trimmed();
            if (!trimmed.isEmpty() && trimmed != "-") {
                registerCommandTrigger(trimmed, action);
            }
        }

        // If default aliases are not suppressed, register built-in aliases
        if (!suppressDefaultAliases) {
            for (const auto& aliasTrigger : item.aliases) {
                if (aliasTrigger.isEmpty()) {
                    continue;
                }
                const QString transAls = resolveCommandText(aliasTrigger);
                const QString rawAls = QString::fromUtf8(aliasTrigger.text);

                registerCommandTrigger(transAls, action);
                if (!rawAls.isEmpty() && rawAls != transAls) {
                    registerCommandTrigger(rawAls, action);
                }
            }
        }
    }

    // 2. Process Keywords from g_keywordList
    for (const auto& kwItem : g_keywordList) {
        if (kwItem.primary.isEmpty()) {
            continue;
        }
        const QString rawKw = QString::fromUtf8(kwItem.primary.text);
        const QString transKw = resolveCommandText(kwItem.primary);
        const QString canonicalLower = rawKw.toLower();
        const KeywordDefinition kwDef = kwMap.value(rawKw);

        // Primary keyword
        if (kwDef.customKeyword == "-") {
            // Suppressed
        }
        else if (!kwDef.customKeyword.trimmed().isEmpty()) {
            const QString customLower = kwDef.customKeyword.trimmed().toLower();
            m_keywordToCanonical.insert(customLower, canonicalLower);
            m_canonicalToLocalizedKeyword.insert(canonicalLower, kwDef.customKeyword.trimmed());
        }
        else {
            if (!rawKw.isEmpty()) {
                m_keywordToCanonical.insert(canonicalLower, canonicalLower);
                m_keywordToCanonical.insert(transKw.toLower(), canonicalLower);
                m_canonicalToLocalizedKeyword.insert(canonicalLower, transKw);
            }
        }

        // Keyword aliases
        const bool suppressKwAliases = (kwDef.customAlias == "-");
        if (!kwDef.customAlias.trimmed().isEmpty() && kwDef.customAlias != "-") {
            m_keywordToCanonical.insert(kwDef.customAlias.trimmed().toLower(), canonicalLower);
        }

        if (!suppressKwAliases) {
            for (const auto& aliasTrigger : kwItem.aliases) {
                if (aliasTrigger.isEmpty()) {
                    continue;
                }
                const QString rawAls = QString::fromUtf8(aliasTrigger.text);
                const QString transAls = resolveCommandText(aliasTrigger);

                if (!rawAls.isEmpty()) {
                    m_keywordToCanonical.insert(rawAls.toLower(), canonicalLower);
                }
                if (!transAls.isEmpty()) {
                    m_keywordToCanonical.insert(transAls.toLower(), canonicalLower);
                }
            }
        }
    }

    rebuildActionCommandsCache();
    rebuildCompletionCandidates(config, mapper);
}

QString LC_CommandManager::msgAvailableCommands() const {
    return QObject::tr("Available commands:");
}

LC_RepositoryCommands* LC_CommandManager::getRepository() const {
    return m_repository;
}

void LC_CommandManager::retranslate(const LC_ActionTypeMapper* mapper) {
    applyCommandsScheme(m_activeConfig, mapper);
}

QStringList LC_CommandManager::tokenizeAliases(const QString& rawInput) {
    QStringList result;
    const QStringList tokens = rawInput.split(',', Qt::SkipEmptyParts);
    for (const QString& t : tokens) {
        const QString clean = t.trimmed();
        if (!clean.isEmpty() && !result.contains(clean, Qt::CaseInsensitive)) {
            result.append(clean);
        }
    }
    return result;
}

QString LC_CommandManager::formatAliases(const QStringList& aliases) {
    return aliases.join(QStringLiteral(", "));
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

QString LC_CommandManager::resolveCommandText(const LC_CommandTrigger& trigger) {
    if (trigger.isEmpty()) {
        return QString();
    }
    return RS_SYSTEM->translateCommand(trigger.text, trigger.disambiguation, "cmd");
}

void LC_CommandManager::collectActionDefaults(const RS2::ActionType actionType,
                                              QStringList& outCommands,
                                              QStringList& outKeycodes,
                                              QStringList& outAliases) {
    outCommands.clear();
    outKeycodes.clear();
    outAliases.clear();

    auto appendTrigger = [](const LC_CommandTrigger& trigger, QStringList& targetList) {
        if (trigger.isEmpty()) {
            return;
        }
        const QString trans = resolveCommandText(trigger);
        if (!trans.isEmpty() && !targetList.contains(trans, Qt::CaseInsensitive)) {
            targetList.append(trans);
        }
        const QString raw = QString::fromUtf8(trigger.text);
        if (!raw.isEmpty() && !targetList.contains(raw, Qt::CaseInsensitive)) {
            targetList.append(raw);
        }
    };

    for (const auto& item : g_commandList) {
        if (item.actionType == actionType) {
            appendTrigger(item.primary, outCommands);
            appendTrigger(item.keycode, outKeycodes);
            for (const auto& alias : item.aliases) {
                appendTrigger(alias, outAliases);
            }
            break;
        }
    }
}

void LC_CommandManager::collectKeywordDefaults(const QString& key, QString& outKw, QStringList& outAliases) {
    outKw = key;
    outAliases.clear();

    for (const auto& item : g_keywordList) {
        if (item.primary.isEmpty()) {
            continue;
        }
        if (key.compare(QLatin1String(item.primary.text), Qt::CaseInsensitive) == 0) {
            outKw = resolveCommandText(item.primary);
            for (const auto& alias : item.aliases) {
                if (alias.isEmpty()) {
                    continue;
                }
                const QString trans = resolveCommandText(alias);
                if (!trans.isEmpty() && !outAliases.contains(trans, Qt::CaseInsensitive)) {
                    outAliases.append(trans);
                }
                const QString raw = QString::fromUtf8(alias.text);
                if (!raw.isEmpty() && !outAliases.contains(raw, Qt::CaseInsensitive)) {
                    outAliases.append(raw);
                }
            }
            break;
        }
    }
}

void LC_CommandManager::migrateLegacyAliasIfNeeded(const LC_ActionTypeMapper* mapper) {
    if (m_repository != nullptr) {
        m_repository->migrateLegacyAliasIfNeeded(mapper);
    }
}
