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

#include "lc_commands_tree_model.h"

#include <QAction>
#include <QMultiMap>

#include "lc_action.h"
#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_action_type_mapper.h"
#include "lc_commandItems.h"
#include "lc_commands_tree_item.h"
#include "lc_shortcuts_manager.h"

namespace {
    struct TriggerOccurrence {
        QString trigger;
        QString displayName;
        QString slotTitle;
        QString ownerId;
    };

    struct DomainCheckResult {
        bool hasExactCollision{false};
        bool hasAmbiguity{false};
        QString conflictMessage;
    };

  DomainCheckResult checkDomainCollisions(const QList<TriggerOccurrence>& occurrences) {
        DomainCheckResult result;

        // 1. Check for Exact Hard Collisions (Identical case)
        QMultiMap<QString, TriggerOccurrence> exactMap;
        for (const auto& occ : occurrences) {
            exactMap.insert(occ.trigger, occ);
        }

        for (auto it = exactMap.begin(); it != exactMap.end();) {
            const QString trigger = it.key();
            const int count = exactMap.count(trigger);

            if (count > 1) {
                result.hasExactCollision = true;
                if (result.conflictMessage.isEmpty()) {
                    auto firstIt = it;
                    const TriggerOccurrence firstOcc = firstIt.value();
                    ++firstIt;
                    const TriggerOccurrence secondOcc = firstIt.value();

                    if (firstOcc.ownerId == secondOcc.ownerId) {
                        result.conflictMessage = QObject::tr("Trigger '%1' is duplicated between %2 and %3 in '%4'")
                            .arg(trigger, firstOcc.slotTitle, secondOcc.slotTitle, firstOcc.displayName);
                    }
                    else {
                        result.conflictMessage = QObject::tr("Trigger '%1' is assigned to both '%2' (%3) and '%4' (%5)")
                            .arg(trigger, firstOcc.displayName, firstOcc.slotTitle, secondOcc.displayName, secondOcc.slotTitle);
                    }
                }
                it += count;
            }
            else {
                ++it;
            }
        }

        if (result.hasExactCollision) {
            return result;
        }

        // 2. Check for Case-Only Ambiguity Warning (differs only by letter case)
        QMultiMap<QString, TriggerOccurrence> lowerMap;
        for (const auto& occ : occurrences) {
            lowerMap.insert(occ.trigger.toLower(), occ);
        }

        for (auto it = lowerMap.begin(); it != lowerMap.end();) {
            const QString lowerTrigger = it.key();
            const int count = lowerMap.count(lowerTrigger);

            if (count > 1) {
                auto firstIt = it;
                const TriggerOccurrence firstOcc = firstIt.value();
                ++firstIt;
                const TriggerOccurrence secondOcc = firstIt.value();

                if (firstOcc.ownerId != secondOcc.ownerId) {
                    result.hasAmbiguity = true;
                    if (result.conflictMessage.isEmpty()) {
                        result.conflictMessage = QObject::tr("Triggers '%1' and '%2' differ only by letter case ('%3' and '%4'). Exact case is required in the command line.")
                            .arg(firstOcc.trigger, secondOcc.trigger, firstOcc.displayName, secondOcc.displayName);
                    }
                }
                it += count;
            }
            else {
                ++it;
            }
        }

        return result;
    }

    void collectActionDefaults(RS2::ActionType actionType,
                               QStringList& outCommands,
                               QStringList& outKeycodes,
                               QStringList& outAliases) {
        for (const auto& item : g_commandList) {
            if (item.actionType == actionType) {
                for (const auto& [fullCmd, cmdTrans] : item.fullCmdList) {
                    if (!cmdTrans.isEmpty() && !outCommands.contains(cmdTrans, Qt::CaseInsensitive)) {
                        outCommands.append(cmdTrans);
                    }
                    if (!fullCmd.isEmpty() && !outCommands.contains(fullCmd, Qt::CaseInsensitive)) {
                        outCommands.append(fullCmd);
                    }
                }
                if (!item.shortCmdList.empty()) {
                    const auto& [firstKey, firstTrans] = item.shortCmdList[0];
                    if (!firstTrans.isEmpty() && !outKeycodes.contains(firstTrans, Qt::CaseInsensitive)) {
                        outKeycodes.append(firstTrans);
                    }
                    if (!firstKey.isEmpty() && !outKeycodes.contains(firstKey, Qt::CaseInsensitive)) {
                        outKeycodes.append(firstKey);
                    }
                }
                for (size_t i = 1; i < item.shortCmdList.size(); ++i) {
                    const auto& [aliasKey, aliasTrans] = item.shortCmdList[i];
                    if (!aliasTrans.isEmpty() && !outAliases.contains(aliasTrans, Qt::CaseInsensitive)) {
                        outAliases.append(aliasTrans);
                    }
                    if (!aliasKey.isEmpty() && !outAliases.contains(aliasKey, Qt::CaseInsensitive)) {
                        outAliases.append(aliasKey);
                    }
                }
                break;
            }
        }
    }

    void collectKeywordDefaults(const QString& key, QString& outKw, QStringList& outAliases) {
        outKw = key;
        outAliases.clear();
        for (const auto& [cmd, trans] : g_transList) {
            if (cmd == key) {
                outKw = trans;
                break;
            }
        }

        for (const auto& [cmd, trans] : g_transList) {
            if (trans == key) {
                const QString lower = cmd.trimmed().toLower();
                if (!lower.isEmpty() && lower != key.toLower() && lower != outKw.toLower()) {
                    if (!outAliases.contains(lower, Qt::CaseInsensitive)) {
                        outAliases.append(lower);
                    }
                }
            }
        }
    }

    QString formatSlotDisplay(const QString& overrideVal, const QStringList& defaults) {
        if (overrideVal == "-") {
            return QObject::tr("— (suppressed)");
        }

        QStringList list;
        if (!overrideVal.trimmed().isEmpty()) {
            list.append(overrideVal.trimmed());
        }
        for (const QString& def : defaults) {
            const QString lower = def.trimmed().toLower();
            if (!lower.isEmpty() && !list.contains(lower, Qt::CaseInsensitive)) {
                list.append(lower);
            }
        }
        return list.join(", ");
    }

    QString resolveSlot(const QString& customOverride, const QString& fallbackDefault) {
        if (!customOverride.isEmpty()) {
            return (customOverride == "-") ? QString() : customOverride.trimmed();
        }
        return fallbackDefault;
    }

    bool checkDomainCollisions(const QMultiMap<QString, TriggerOccurrence>& triggerMap,
                              QString* outFirstConflict) {
        bool hasCollision = false;
        for (auto it = triggerMap.begin(); it != triggerMap.end();) {
            const QString trigger = it.key();
            const int count = triggerMap.count(trigger);

            if (count > 1) {
                hasCollision = true;
                if (outFirstConflict != nullptr && outFirstConflict->isEmpty()) {
                    auto firstIt = it;
                    const TriggerOccurrence firstOcc = firstIt.value();
                    ++firstIt;
                    const TriggerOccurrence secondOcc = firstIt.value();

                    if (firstOcc.ownerId == secondOcc.ownerId) {
                        *outFirstConflict = QObject::tr("Trigger '%1' is duplicated between %2 and %3 in '%4'")
                            .arg(trigger, firstOcc.slotTitle, secondOcc.slotTitle, firstOcc.displayName);
                    }
                    else {
                        *outFirstConflict = QObject::tr("Trigger '%1' is assigned to both '%2' (%3) and '%4' (%5)")
                            .arg(trigger, firstOcc.displayName, firstOcc.slotTitle, secondOcc.displayName, secondOcc.slotTitle);
                    }
                }
                it += count;
            }
            else {
                ++it;
            }
        }
        return hasCollision;
    }
}

LC_CommandsTreeModel::LC_CommandsTreeModel(QObject* parent, const QColor& filterColor, const QColor& conflictColor)
    : LC_ActionMappingTreeModelBase(parent, filterColor, conflictColor) {
}

int LC_CommandsTreeModel::columnCount(const QModelIndex&) const {
    return LAST;
}

QVariant LC_CommandsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == NAME) {
            return tr("Action / Category");
        }
        if (section == KEYCODE) {
            return tr("Keycode");
        }
        if (section == COMMAND) {
            return tr("Command");
        }
        if (section == ALIAS) {
            return tr("Alias");
        }
    }
    return QVariant();
}
LC_CommandsTreeItem* LC_CommandsTreeModel::getItemForIndex(const QModelIndex& index) const {
    return static_cast<LC_CommandsTreeItem*>(LC_ActionMappingTreeModelBase::getItemForIndex(index));
}

QVariant LC_CommandsTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto* item = getItemForIndex(index);
    if (item == nullptr) {
        return QVariant();
    }

    const int col = index.column();

    if (role == Qt::DisplayRole) {
        if (col == NAME) {
            return item->name();
        }
        if (!item->isGroup()) {
            if (col == KEYCODE) {
                return item->getKeyCode();
            }
            if (col == COMMAND) {
                return item->getMainCommand();
            }
            if (col == ALIAS) {
                return item->getAlias();
            }
        }
        return QVariant();
    }

    if (role == Qt::DecorationRole && col == NAME) {
        return item->icon();
    }

    return LC_ActionMappingTreeModelBase::data(index, role);
}




bool LC_CommandsTreeModel::checkForCollisions(const CommandsConfig& config,
                                              LC_ActionGroupManager* agm,
                                              const LC_ActionTypeMapper* mapper,
                                              QString* outFirstConflict) {
    if (agm == nullptr || mapper == nullptr) {
        return false;
    }

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    QList<TriggerOccurrence> actionOccurrences;

    for (const auto* group : agm->allGroupsList()) {
        if (group == nullptr || !group->isActionMappingsMayBeConfigured()) {
            continue;
        }

        for (const auto* action : group->actions()) {
            if (action == nullptr) {
                continue;
            }

            const QString actionName = action->objectName();
            if (!mapper->hasAction(actionName)) {
                continue;
            }

            const auto actionType = mapper->actionTypeFromName(actionName);
            const QStringList triggers = computeActionEffectiveTriggers(actionType, cmdMap.value(actionName));
            const QString display = action->text().remove('&').trimmed();

            for (const QString& trg : triggers) {
                actionOccurrences.append({trg, display, tr("Command Trigger"), actionName});
            }
        }
    }

    const auto actionCheck = checkDomainCollisions(actionOccurrences);

    // In-Prompt Keywords: intra-item duplicate checks only
    bool hasKeywordCollision = false;
    for (const auto& kwDef : config.keywords) {
        const QStringList triggers = computeKeywordEffectiveTriggers(kwDef.key, kwDef);
        QSet<QString> seen;
        for (const QString& trg : triggers) {
            if (seen.contains(trg)) {
                hasKeywordCollision = true;
                if (outFirstConflict != nullptr && outFirstConflict->isEmpty()) {
                    *outFirstConflict = tr("Keyword '%1' has duplicate trigger '%2' assigned to itself.")
                        .arg(kwDef.key, trg);
                }
                break;
            }
            seen.insert(trg);
        }
        if (hasKeywordCollision) {
            break;
        }
    }

    if (actionCheck.hasExactCollision || hasKeywordCollision) {
        if (outFirstConflict != nullptr && outFirstConflict->isEmpty()) {
            *outFirstConflict = actionCheck.conflictMessage;
        }
        return true; // Hard error: blocks saving
    }

    if (actionCheck.hasAmbiguity) {
        if (outFirstConflict != nullptr && outFirstConflict->isEmpty()) {
            *outFirstConflict = actionCheck.conflictMessage;
        }
        // Returns false to allow saving, but outFirstConflict communicates warning banner
        return false;
    }

    return false;
}

bool LC_CommandsTreeModel::evaluateActionCollision(LC_CommandsTreeItem* item,
                                                   const CommandDefinition& cmdDef,
                                                   const LC_ActionTypeMapper* mapper,
                                                   const QMultiMap<QString, QString>& actionTriggers) const {
    if (item == nullptr || mapper == nullptr) {
        return false;
    }

    const auto actionType = mapper->actionTypeFromName(item->identifier());
    const QStringList effTrg = computeActionEffectiveTriggers(actionType, cmdDef);
    item->setEffectiveTriggers(effTrg);

    // 1. Exact-case duplicate check in global actions domain
    for (const QString& trg : effTrg) {
        if (actionTriggers.count(trg) > 1) {
            return true;
        }
    }

    // 2. Intra-action slot collision check
    const QString c = cmdDef.customCommand.trimmed();
    const QString k = cmdDef.customKeycode.trimmed();
    const QString a = cmdDef.customAlias.trimmed();
    if (!c.isEmpty() && c != "-" && (c == k || c == a)) {
        return true;
    }
    if (!k.isEmpty() && k != "-" && k == a) {
        return true;
    }

    return false;
}

bool LC_CommandsTreeModel::evaluateKeywordCollision(LC_CommandsTreeItem* item,
                                                    const KeywordDefinition& kwDef) const {
    if (item == nullptr) {
        return false;
    }

    const QStringList effTrg = computeKeywordEffectiveTriggers(item->identifier(), kwDef);
    item->setEffectiveTriggers(effTrg);

    // Keywords only collide on intra-item duplicates
    QSet<QString> seen;
    for (const QString& trg : effTrg) {
        if (seen.contains(trg)) {
            return true;
        }
        seen.insert(trg);
    }
    return false;
}

void LC_CommandsTreeModel::recalculateGroupCollisions(LC_CommandsTreeItem* groupItem,
                                                      int groupRow,
                                                      const QMap<QString, CommandDefinition>& cmdMap,
                                                      const QMap<QString, KeywordDefinition>& kwMap,
                                                      const LC_ActionTypeMapper* mapper,
                                                      const QMultiMap<QString, QString>& actionTriggers) {
    if (groupItem == nullptr) {
        return;
    }

    const QModelIndex pIdx = index(groupRow, 0, QModelIndex());

    for (int c = 0; c < groupItem->childCount(); ++c) {
        auto* item = static_cast<LC_CommandsTreeItem*>(groupItem->child(c));
        if (item == nullptr) {
            continue;
        }

        const bool itemCollides = !item->isKeyword()
            ? evaluateActionCollision(item, cmdMap.value(item->identifier()), mapper, actionTriggers)
            : evaluateKeywordCollision(item, kwMap.value(item->identifier()));

        if (item->hasCollision() != itemCollides) {
            item->setCollision(itemCollides);
            const QModelIndex sIdx = index(c, NAME, pIdx);
            const QModelIndex eIdx = index(c, ALIAS, pIdx);
            emit dataChanged(sIdx, eIdx, {Qt::DisplayRole, Qt::FontRole, Qt::ForegroundRole});
        }
    }
}

void LC_CommandsTreeModel::recalculateCollisions(const CommandsConfig& config,
                                                LC_ActionGroupManager* agm,
                                                const LC_ActionTypeMapper* mapper) {
    if (agm == nullptr || mapper == nullptr || m_rootItem == nullptr) {
        return;
    }

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    QMap<QString, KeywordDefinition> kwMap;
    for (const auto& kw : config.keywords) {
        kwMap.insert(kw.key, kw);
    }

    // 1. Build global frequency maps via shared helper
    QMultiMap<QString, QString> actionTriggers;
    QMultiMap<QString, QString> keywordTriggers;
    buildGlobalTriggerMaps(config, agm, mapper, cmdMap, actionTriggers, keywordTriggers);

    // 2. Delegate group item updates
    for (int g = 0; g < m_rootItem->childCount(); ++g) {
        auto* groupItem = static_cast<LC_CommandsTreeItem*>(m_rootItem->child(g));
        if (groupItem != nullptr) {
            recalculateGroupCollisions(groupItem, g, cmdMap, kwMap, mapper, actionTriggers);
        }
    }
}

void LC_CommandsTreeModel::updateItemTriggers(LC_CommandsTreeItem* item, const QString& mainCmd,
                                              const QString& a1, const QString& a2,
                                              const QStringList& effectiveTriggers, bool isModified) {
    if (item == nullptr) {
        return;
    }

    item->setMainCommand(mainCmd);
    item->setKeyCode(a1);
    item->setAlias(a2);
    item->setEffectiveTriggers(effectiveTriggers);
    item->setModified(isModified);

    if (m_hasFilter && m_highlightOnly) {
        const bool matches = item->name().contains(m_filterRegex) ||
                             mainCmd.contains(m_filterRegex) ||
                             a1.contains(m_filterRegex) ||
                             a2.contains(m_filterRegex);
        item->setMatched(matches);
    }
    else {
        item->setMatched(false);
    }

    auto* parentItem = item->parent();
    QModelIndex parentIdx;
    if (parentItem != nullptr && parentItem != m_rootItem.get()) {
        parentIdx = createIndex(parentItem->row(), 0, parentItem);
    }

    const QModelIndex startIdx = index(item->row(), KEYCODE, parentIdx);
    const QModelIndex endIdx   = index(item->row(), ALIAS, parentIdx);
    emit dataChanged(startIdx, endIdx);
}



QStringList LC_CommandsTreeModel::computeActionEffectiveTriggers(RS2::ActionType actionType, const CommandDefinition& def) {
    QStringList triggers;
    auto appendUnique = [&triggers](const QString& str) {
        const QString trimmed = str.trimmed();
        if (!trimmed.isEmpty() && trimmed != "-" && !triggers.contains(trimmed)) {
            triggers.append(trimmed);
        }
    };

    QStringList sysCmds;
    QStringList sysKeys;
    QStringList sysAliases;
    collectActionDefaults(actionType, sysCmds, sysKeys, sysAliases);

    // 1. Command slot
    if (def.customCommand != "-") {
        if (!def.customCommand.trimmed().isEmpty()) {
            appendUnique(def.customCommand);
        }
        for (const QString& cmd : sysCmds) {
            appendUnique(cmd);
        }
    }

    // 2. Keycode slot (override + unsuppressed keycode defaults)
    if (def.customKeycode != "-") {
        if (!def.customKeycode.trimmed().isEmpty()) {
            appendUnique(def.customKeycode);
        }
        for (const QString& key : sysKeys) {
            appendUnique(key);
        }
    }

    // 3. Alias slot (override + unsuppressed alias defaults)
    if (def.customAlias != "-") {
        if (!def.customAlias.trimmed().isEmpty()) {
            appendUnique(def.customAlias);
        }
        for (const QString& als : sysAliases) {
            appendUnique(als);
        }
    }

    // Sort shortest string first for exact alignment with LC_CommandManager::getCommandsForAction()
    std::sort(triggers.begin(), triggers.end(), [](const QString& a, const QString& b) {
        if (a.length() != b.length()) {
            return a.length() < b.length();
        }
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });


    return triggers;
}

QStringList LC_CommandsTreeModel::computeKeywordEffectiveTriggers(const QString& key, const KeywordDefinition& def) {
    QStringList triggers;
    auto appendUnique = [&triggers](const QString& str) {
        const QString trimmed = str.trimmed();
        if (!trimmed.isEmpty() && trimmed != "-" && !triggers.contains(trimmed)) {
            triggers.append(trimmed);
        }
    };

    QString defKw;
    QStringList defAliases;
    collectKeywordDefaults(key, defKw, defAliases);

    // 1. Keyword slot
    if (def.customKeyword != "-") {
        if (!def.customKeyword.trimmed().isEmpty()) {
            appendUnique(def.customKeyword);
        }
        appendUnique(defKw);
        appendUnique(key);
    }

    // 2. Alias slot
    if (def.customAlias != "-") {
        if (!def.customAlias.trimmed().isEmpty()) {
            appendUnique(def.customAlias);
        }
        for (const auto& a : defAliases) {
            appendUnique(a);
        }
    }

    // Sort by length: shortest trigger appears first at the left
    std::stable_sort(triggers.begin(), triggers.end(), [](const QString& a, const QString& b) {
        return a.length() < b.length();
    });

    return triggers;
}

void LC_CommandsTreeModel::rebuildModel(const CommandsConfig& config,
                                        LC_ActionGroupManager* agm,
                                        LC_ActionTypeMapper* mapper) {
    beginResetModel();
    m_rootItem = std::make_unique<LC_CommandsTreeItem>(nullptr, QString(), QIcon());

    if (agm == nullptr || mapper == nullptr) {
        endResetModel();
        return;
    }

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    QMultiMap<QString, QString> actionTriggerMap;
    QMultiMap<QString, QString> keywordTriggerMap;
    buildGlobalTriggerMaps(config, agm, mapper, cmdMap, actionTriggerMap, keywordTriggerMap);

    auto* root = static_cast<LC_CommandsTreeItem*>(m_rootItem.get());
    buildActionGroups(cmdMap, agm, mapper, actionTriggerMap, root);
    buildKeywordsGroup(config, keywordTriggerMap, root);

    endResetModel();
}

void LC_CommandsTreeModel::buildGlobalTriggerMaps(const CommandsConfig& config,
                                                  LC_ActionGroupManager* agm,
                                                  const LC_ActionTypeMapper* mapper,
                                                  const QMap<QString, CommandDefinition>& cmdMap,
                                                  QMultiMap<QString, QString>& outActionTriggerMap,
                                                  QMultiMap<QString, QString>& outKeywordTriggerMap) const {
    for (const auto* group : agm->allGroupsList()) {
        if (group == nullptr || !group->isActionMappingsMayBeConfigured()) {
            continue;
        }

        for (const auto* action : group->actions()) {
            if (action == nullptr) {
                continue;
            }

            const QString actionName = action->objectName();
            if (mapper->hasAction(actionName)) {
                const auto triggers = computeActionEffectiveTriggers(mapper->actionTypeFromName(actionName), cmdMap.value(actionName));
                for (const QString& trg : triggers) {
                    outActionTriggerMap.insert(trg, actionName);
                }
            }
        }
    }

    for (const auto& kwDef : config.keywords) {
        const auto triggers = computeKeywordEffectiveTriggers(kwDef.key, kwDef);
        for (const QString& trg : triggers) {
            outKeywordTriggerMap.insert(trg, kwDef.key);
        }
    }
}

bool LC_CommandsTreeModel::matchesSearchFilter(const QStringList& searchableStrings) const {
    if (!m_hasFilter) {
        return false;
    }

    for (const QString& str : searchableStrings) {
        if (str.contains(m_filterRegex)) {
            return true;
        }
    }
    return false;
}

LC_CommandsTreeItem* LC_CommandsTreeModel::createActionTreeItem(LC_CommandsTreeItem* parentGroup,
                                                                const QAction* action,
                                                                const CommandDefinition& cmdDef,
                                                                const LC_ActionTypeMapper* mapper,
                                                                const QMultiMap<QString, QString>& actionTriggerMap) {
    if (action == nullptr || mapper == nullptr) {
        return nullptr;
    }

    const QString actionName = action->objectName();
    if (!mapper->hasAction(actionName)) {
        return nullptr;
    }

    const auto actionType = mapper->actionTypeFromName(actionName);

    QStringList sysCmds;
    QStringList sysKeys;
    QStringList sysAliases;
    collectActionDefaults(actionType, sysCmds, sysKeys, sysAliases);

    const QString colCmd = formatSlotDisplay(cmdDef.customCommand, sysCmds);
    const QString colKey = formatSlotDisplay(cmdDef.customKeycode, sysKeys);
    const QString colAls = formatSlotDisplay(cmdDef.customAlias, sysAliases);

    const QStringList effectiveTriggers = computeActionEffectiveTriggers(actionType, cmdDef);

    // 1. Evaluate exact-case collision against global actions domain
    bool itemHasCollision = false;
    for (const QString& trg : effectiveTriggers) {
        if (actionTriggerMap.count(trg) > 1) {
            itemHasCollision = true;
            break;
        }
    }

    // 2. Evaluate intra-action slot duplicates
    const QString c = cmdDef.customCommand.trimmed();
    const QString k = cmdDef.customKeycode.trimmed();
    const QString a = cmdDef.customAlias.trimmed();
    if (!c.isEmpty() && c != "-" && (c == k || c == a)) {
        itemHasCollision = true;
    }
    if (!k.isEmpty() && k != "-" && k == a) {
        itemHasCollision = true;
    }

    if (m_filterForConflicts && !itemHasCollision) {
        return nullptr;
    }

    // 3. Search filter evaluation
    QString cleanActionText = action->text().remove('&');
    const bool matches = matchesSearchFilter({cleanActionText, colCmd, colKey, colAls, action->toolTip()});

    if (m_hasFilter && !m_highlightOnly && !matches) {
        return nullptr;
    }

    // 4. Construct item
    auto* childItem = new LC_CommandsTreeItem(parentGroup, cleanActionText, action->icon(), actionName, false);
    childItem->setMainCommand(colCmd);
    childItem->setKeyCode(colKey);
    childItem->setAlias(colAls);
    childItem->setEffectiveTriggers(effectiveTriggers);
    childItem->setModified(!cmdDef.customCommand.isEmpty() || !cmdDef.customKeycode.isEmpty() || !cmdDef.customAlias.isEmpty());
    childItem->setCollision(itemHasCollision);
    childItem->setMatched(m_hasFilter && m_highlightOnly && matches);

    const auto* lcAct = dynamic_cast<const LC_Action*>(action);
    const QString actDesc = (lcAct != nullptr && !lcAct->description().isEmpty())
                                ? lcAct->description()
                                : LC_ShortcutsManager::getPlainActionToolTip(action);
    childItem->setDescription(actDesc);


    return childItem;
}

void LC_CommandsTreeModel::buildActionGroups(const QMap<QString, CommandDefinition>& cmdMap,
                                             LC_ActionGroupManager* agm,
                                             const LC_ActionTypeMapper* mapper,
                                             const QMultiMap<QString, QString>& actionTriggerMap,
                                             LC_CommandsTreeItem* rootItem) {
    for (const auto* group : agm->allGroupsList()) {
        if (group == nullptr || !group->isActionMappingsMayBeConfigured()) {
            continue;
        }

        auto* groupItem = new LC_CommandsTreeItem(rootItem, group->getTitle(), group->getIcon());

        for (const auto* action : group->actions()) {
            if (action == nullptr) {
                continue;
            }

            const QString actionName = action->objectName();
            const auto cmdDef = cmdMap.value(actionName);
            auto* childItem = createActionTreeItem(groupItem, action, cmdDef, mapper, actionTriggerMap);

            if (childItem != nullptr) {
                groupItem->appendChild(childItem);
            }
        }

        if (groupItem->childCount() > 0) {
            rootItem->appendChild(groupItem);
        }
        else {
            delete groupItem;
        }
    }
}

LC_CommandsTreeItem* LC_CommandsTreeModel::createKeywordTreeItem(LC_CommandsTreeItem* parentGroup,
                                                                 const KeywordDefinition& kwDef,
                                                                 const QMultiMap<QString, QString>& keywordTriggerMap) {
    Q_UNUSED(keywordTriggerMap);

    QString defKw;
    QStringList defAliases;
    collectKeywordDefaults(kwDef.key, defKw, defAliases);

    const QString colKw  = formatSlotDisplay(kwDef.customKeyword, QStringList{defKw, kwDef.key});
    const QString colKey = QString(); // In-prompt keywords do not participate in keycode mode
    const QString colAls = formatSlotDisplay(kwDef.customAlias, defAliases);

    const QStringList effectiveTriggers = computeKeywordEffectiveTriggers(kwDef.key, kwDef);

    // Keywords only collide on intra-item duplicates
    bool kwCollision = false;
    QSet<QString> seen;
    for (const QString& trg : effectiveTriggers) {
        if (seen.contains(trg)) {
            kwCollision = true;
            break;
        }
        seen.insert(trg);
    }

    if (m_filterForConflicts && !kwCollision) {
        return nullptr;
    }

    const bool matches = matchesSearchFilter({kwDef.key, colKw, colAls});

    if (m_hasFilter && !m_highlightOnly && !matches) {
        return nullptr;
    }

    auto* kwItem = new LC_CommandsTreeItem(parentGroup, kwDef.key, QIcon(":/icons/text.lci"), kwDef.key, true);
    kwItem->setMainCommand(colKw);
    kwItem->setKeyCode(colKey);
    kwItem->setAlias(colAls);
    kwItem->setEffectiveTriggers(effectiveTriggers);
    kwItem->setModified(!kwDef.customKeyword.isEmpty() || !kwDef.customAlias.isEmpty());
    kwItem->setCollision(kwCollision);
    kwItem->setMatched(m_hasFilter && m_highlightOnly && matches);

    return kwItem;
}

void LC_CommandsTreeModel::buildKeywordsGroup(const CommandsConfig& config,
                                              const QMultiMap<QString, QString>& keywordTriggerMap,
                                              LC_CommandsTreeItem* rootItem) {
    if (config.keywords.isEmpty() || rootItem == nullptr) {
        return;
    }

    auto* kwGroupItem = new LC_CommandsTreeItem(rootItem,
                                               tr("In-Prompt Keywords (Sub-Commands)"),
                                               QIcon(":/icons/text.lci"));

    for (const auto& kwDef : config.keywords) {
        auto* kwItem = createKeywordTreeItem(kwGroupItem, kwDef, keywordTriggerMap);
        if (kwItem != nullptr) {
            kwGroupItem->appendChild(kwItem);
        }
    }

    if (kwGroupItem->childCount() > 0) {
        rootItem->appendChild(kwGroupItem);
    }
    else {
        delete kwGroupItem;
    }
}
