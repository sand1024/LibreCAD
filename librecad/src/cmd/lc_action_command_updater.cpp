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

#include "lc_action_command_updater.h"

#include <QAction>
#include <QMap>
#include <QStringList>

#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_action_tooltip_builder.h"
#include "lc_action_type_mapper.h"
#include "lc_command_manager.h"
#include "lc_commands_tree_model.h"
#include "lc_proxy_style_shared.h"
#include "lc_repository_commands.h"
#include "lc_settings_commands_promotion.h"
#include "qc_applicationwindow.h"

QString LC_ActionCommandUpdater::selectDisplayTrigger(const QStringList& effectiveTriggers, const QString& keycode,
                                                      const bool keycodeMode) {
    if (effectiveTriggers.isEmpty()) {
        return QString();
    }

    const QString cleanKey = keycode.trimmed();
    if (keycodeMode && !cleanKey.isEmpty() && cleanKey != "-" && cleanKey.length() == 2) {
        // In keycode mode, prioritize the exact 2-letter keycode preserving its case
        for (const auto& trg : effectiveTriggers) {
            if (trg == cleanKey) {
                return trg;
            }
        }
    }

    if (effectiveTriggers.size() == 1) {
        return effectiveTriggers.first();
    }

    // Otherwise, select the shortest trigger, preserving its exact case
    QString shortest;
    for (const auto& trg : effectiveTriggers) {
        const QString trimmed = trg.trimmed();
        if (trimmed.isEmpty() || trimmed == "-") {
            continue;
        }
        if (trimmed != cleanKey) {
            if (shortest.isEmpty() || trimmed.length() < shortest.length()) {
                shortest = trimmed;
            }
        }
    }
    return shortest;
}

void LC_ActionCommandUpdater::updateActionsForScheme(LC_ActionGroupManager* agm, const CommandsConfig& config,
                                                     const LC_ActionTypeMapper* mapper, const bool keycodeMode) {
    if (agm == nullptr || mapper == nullptr) {
        return;
    }

    QMap<QString, CommandDefinition> cmdMap;
    for (const auto& def : config.commands) {
        cmdMap.insert(def.actionName, def);
    }

    for (const auto* group : agm->allGroupsList()) {
        if (group == nullptr) {
            continue;
        }

        for (auto* action : group->actions()) {
            if (action == nullptr) {
                continue;
            }

            const QString actionName = action->objectName();
            if (!mapper->hasAction(actionName)) {
                action->setProperty(PROP_CMD_LINE, QVariant());
                continue;
            }

            const auto actionType = mapper->actionTypeFromName(actionName);
            const auto cmdDef = cmdMap.value(actionName);
            const QStringList effectiveTriggers = LC_CommandsTreeModel::computeActionEffectiveTriggers(actionType, cmdDef);

            const QString displayTrigger = selectDisplayTrigger(effectiveTriggers, cmdDef.customKeycode, keycodeMode);
            if (!displayTrigger.isEmpty()) {
                action->setProperty(PROP_CMD_LINE, displayTrigger);
            }
            else {
                action->setProperty(PROP_CMD_LINE, QVariant());
            }
        }

        LC_ActionTooltipBuilder::updateAllTooltips(agm->getActionsMap());
    }
}

void LC_ActionCommandUpdater::updateActions(LC_ActionGroupManager* agm, const LC_CommandManager* commandManager, const bool keycodeMode) {
    if (agm == nullptr || commandManager == nullptr) {
        return;
    }

    const LC_ActionTypeMapper mapper(agm);
    updateActionsForScheme(agm, commandManager->activeConfig(), &mapper, keycodeMode);
}

void LC_ActionCommandUpdater::clearActions(LC_ActionGroupManager* agm) {
    if (agm == nullptr) {
        return;
    }

    for (const auto* group : agm->allGroupsList()) {
        if (group == nullptr) {
            continue;
        }

        for (auto* action : group->actions()) {
            if (action != nullptr) {
                action->setProperty(PROP_CMD_LINE, QVariant());
            }
        }
    }
}
