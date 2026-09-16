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

#include "lc_shortcuts_tree_model.h"

#include <QAction>
#include <QMultiMap>

#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_shortcut_tree_item.h"

LC_ShortcutsTreeModel::LC_ShortcutsTreeModel(QObject* parent, QColor filteredItemColor, QColor conflictItemColor)
    : LC_ActionMappingTreeModelBase(parent, filteredItemColor, conflictItemColor) {
}

LC_ShortcutsTreeModel::~LC_ShortcutsTreeModel() {
    qDeleteAll(m_shortcuts);
}

int LC_ShortcutsTreeModel::columnCount(const QModelIndex&) const {
    return LAST;
}

LC_ShortcutTreeItem* LC_ShortcutsTreeModel::getItemForIndex(const QModelIndex& index) const {
    return static_cast<LC_ShortcutTreeItem*>(LC_ActionMappingTreeModelBase::getItemForIndex(index));
}

QVariant LC_ShortcutsTreeModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto* item = getItemForIndex(index);
    if (item == nullptr) {
        return QVariant();
    }

    const int col = index.column();

    if (role == Qt::DecorationRole && col == ICON) {
        return item->icon();
    }

    if (role == Qt::DisplayRole) {
        if (col == NAME) {
            return item->name();
        }
        if (col == SHORTCUT) {
            return item->getShortcutViewString();
        }
    }

    if (role == Qt::UserRole) {
        return item->name();
    }

    return LC_ActionMappingTreeModelBase::data(index, role);
}

void LC_ShortcutsTreeModel::rebuildModel(LC_ActionGroupManager* pManager) {
    if (pManager == nullptr) {
        return;
    }

    beginResetModel();
    auto* root = new LC_ShortcutTreeItem(nullptr, QIcon(), "", "");

    const QList<LC_ActionGroup*>& groupsList = pManager->allGroupsList();

    for (const LC_ActionGroup* group : groupsList) {
        if (group == nullptr || !group->isActionMappingsMayBeConfigured()) {
            continue;
        }

        auto* groupChild = new LC_ShortcutTreeItem(root, group->getIcon(), group->getDescription(), group->getDescription());

        for (const QAction* action : group->actions()) {
            if (action == nullptr) {
                continue;
            }

            const QVariant& configurable = action->property(LC_ShortcutInfo::PROPERTY_ACTION_SHORTCUT_CONFIGURABLE);
            if (configurable.isValid() && !configurable.toBool()) {
                continue;
            }

            const QString actionName = action->objectName();
            auto* shortcutInfo = m_shortcuts.value(actionName, nullptr);
            if (shortcutInfo == nullptr) {
                shortcutInfo = new LC_ShortcutInfo(actionName, action->shortcut());
                m_shortcuts[actionName] = shortcutInfo;
            }

            if (m_filterForConflicts && !shortcutInfo->hasCollision()) {
                continue;
            }

            bool hasRegexpMatch = false;
            if (!m_filterForConflicts && m_hasFilter) {
                const QString actionText = action->text().remove('&').toLower();
                hasRegexpMatch = m_filterRegex.match(actionText, 0).hasMatch();

                if (!m_highlightOnly) {
                    if (!hasRegexpMatch) {
                    continue;
                }
                    hasRegexpMatch = false; // Filter-only mode: do not highlight matched row
                }
            }

            bool hasShortcutMatch = false;
            if (!m_filterForConflicts && m_hasShortcutFilter) {
                hasShortcutMatch = shortcutInfo->hasTheSameKey(m_filteringShortcut);

                if (!m_highlightOnly && !hasShortcutMatch) {
                    continue;
                }
            }

            LC_ShortcutTreeItem* child = groupChild->addChild(action, shortcutInfo);
            const bool isMatched = (m_highlightOnly && hasRegexpMatch) || (m_hasShortcutFilter && hasShortcutMatch);
            child->setMatched(isMatched);
            child->setCollision(shortcutInfo->hasCollision());
            child->setModified(shortcutInfo->isModified());
        }

        if (groupChild->childCount() > 0) {
            root->appendChild(groupChild);
        }
        else {
            delete groupChild;
        }
    }

    m_rootItem.reset(root);
    endResetModel();
}

bool LC_ShortcutsTreeModel::checkForCollisions(LC_ShortcutInfo* shortcutInfo, QString* outFirstConflict) {
    for (auto* s : std::as_const(m_shortcuts)) {
        if (s != nullptr) {
            s->setCollision(false);
        }
    }

    QMultiMap<QKeySequence, LC_ShortcutInfo*> keyMap;
    for (auto* s : std::as_const(m_shortcuts)) {
        if (s != nullptr && !s->getKey().isEmpty()) {
            keyMap.insert(s->getKey(), s);
        }
    }

    bool hasAnyCollision = false;
    for (auto it = keyMap.begin(); it != keyMap.end();) {
        const QKeySequence key = it.key();
        const int count = keyMap.count(key);

        if (count > 1) {
            hasAnyCollision = true;
            auto rangeIt = it;
            for (int i = 0; i < count; ++i, ++rangeIt) {
                rangeIt.value()->setCollision(true);
            }

            if (outFirstConflict != nullptr && outFirstConflict->isEmpty()) {
                auto firstIt = it;
                const QString firstAction = firstIt.value()->getName();
                ++firstIt;
                const QString secondAction = firstIt.value()->getName();
                *outFirstConflict = tr("Key sequence '%1' is assigned to both '%2' and '%3'")
                    .arg(key.toString(QKeySequence::NativeText), firstAction, secondAction);
            }

            it = rangeIt;
        }
        else {
            ++it;
        }
    }

    if (m_rootItem != nullptr) {
        for (int g = 0; g < m_rootItem->childCount(); ++g) {
            auto* groupItem = m_rootItem->child(g);
            if (groupItem == nullptr) {
                continue;
            }
            for (int c = 0; c < groupItem->childCount(); ++c) {
                auto* item = static_cast<LC_ShortcutTreeItem*>(groupItem->child(c));
                if (item != nullptr && item->getShortcutInfo() != nullptr) {
                    item->setCollision(item->getShortcutInfo()->hasCollision());
                }
            }
        }
    }

    if (shortcutInfo != nullptr) {
        return shortcutInfo->hasCollision();
    }

    return hasAnyCollision;
}

const QMap<QString, LC_ShortcutInfo*>& LC_ShortcutsTreeModel::getShortcuts() const {
    return m_shortcuts;
}

void LC_ShortcutsTreeModel::resetAllToDefault() {
    for (auto* shortcut : std::as_const(m_shortcuts)) {
        if (shortcut != nullptr) {
            shortcut->resetToDefault();
            shortcut->setCollision(false);
        }
    }
    dataChanged(QModelIndex(), QModelIndex());
}

void LC_ShortcutsTreeModel::applyShortcuts(const QMap<QString, QKeySequence>& map, const bool replace) {
    if (replace) {
        for (auto* shortcut : std::as_const(m_shortcuts)) {
            if (shortcut != nullptr) {
                shortcut->clear();
            }
        }
    }

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        auto* existing = m_shortcuts.value(it.key(), nullptr);
        if (existing != nullptr) {
            existing->setKey(it.value());
        }
    }

    if (replace) {
        commitBaseline();
    }
}

bool LC_ShortcutsTreeModel::isModified() const {
    for (const auto* shortcut : std::as_const(m_shortcuts)) {
        if (shortcut != nullptr && shortcut->isModified()) {
            return true;
        }
    }
    return false;
}

void LC_ShortcutsTreeModel::setFilteringShortcut(const QKeySequence& sequence) {
    m_filteringShortcut = sequence;
    m_hasShortcutFilter = !sequence.isEmpty();
}

void LC_ShortcutsTreeModel::clearFilteringShortcut() {
    m_filteringShortcut = QKeySequence();
    m_hasShortcutFilter = false;
}

bool LC_ShortcutsTreeModel::hasShortcutFilter() const {
    return m_hasShortcutFilter;
}

void LC_ShortcutsTreeModel::commitBaseline() {
    for (auto* shortcut : std::as_const(m_shortcuts)) {
        if (shortcut != nullptr) {
            shortcut->commitBaseline();
        }
    }
    dataChanged(QModelIndex(), QModelIndex());
}
