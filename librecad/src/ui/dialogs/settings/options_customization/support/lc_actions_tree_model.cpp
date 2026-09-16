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

#include "lc_actions_tree_model.h"

#include <QAction>
#include <QIcon>


#include "lc_action_factory.h"
#include "lc_action_group.h"
#include "lc_action_group_manager.h"

class LC_ActionTreeItem {
public:
    LC_ActionTreeItem(LC_ActionTreeItem* parent, const QString& text, const QIcon& icon,
                         QAction* action = nullptr, const QString& canonicalToken = QString())
           : m_parent(parent), m_text(text), m_icon(icon), m_action(action), m_canonicalToken(canonicalToken) {}

    ~LC_ActionTreeItem() {
        qDeleteAll(m_children);
    }

    void appendChild(LC_ActionTreeItem* child) {
        m_children.append(child);
    }

    LC_ActionTreeItem* child(int row) const {
        return (row >= 0 && row < m_children.size()) ? m_children.at(row) : nullptr;
    }

    int childCount() const { return m_children.size(); }
    LC_ActionTreeItem* parent() const { return m_parent; }
    int row() const {
        return (m_parent != nullptr) ? m_parent->m_children.indexOf(const_cast<LC_ActionTreeItem*>(this)) : 0;
    }

    QString text() const { return m_text; }
    QIcon icon() const { return m_icon; }
    QAction* action() const { return m_action; }
    bool isGroup() const { return m_action == nullptr; }
    QString canonicalToken() const { return m_canonicalToken; }
private:
    LC_ActionTreeItem* m_parent = nullptr;
    QList<LC_ActionTreeItem*> m_children;
    QString m_text;
    QIcon m_icon;
    QAction* m_action = nullptr;
    QString m_canonicalToken;
};

LC_ActionsTreeModel::LC_ActionsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new LC_ActionTreeItem(nullptr, "", QIcon())) {}

LC_ActionsTreeModel::~LC_ActionsTreeModel() {
    delete m_rootItem;
}

void LC_ActionsTreeModel::setup(LC_ActionGroupManager* manager, ActionsFilterMode mode) {
    m_groupManager = manager;
    m_filterMode = mode;
    rebuildTree();
}

void LC_ActionsTreeModel::setFilterText(const QString& filter) {
    m_filterText = filter.trimmed().toLower();
    rebuildTree();
}

void LC_ActionsTreeModel::rebuildTree() {
    beginResetModel();
    delete m_rootItem;
    m_rootItem = new LC_ActionTreeItem(nullptr, "", QIcon());

    if (m_groupManager != nullptr) {
        const LC_ActionNamingServiceInterface* namingService = m_groupManager->getNamingService();
        const QList<LC_ActionGroup*> toolsList = m_groupManager->toolGroups();
        const QSet<LC_ActionGroup*> toolGroupsSet(toolsList.begin(), toolsList.end());

        for (const auto* group : m_groupManager->allGroupsList()) {
            if (group == nullptr || !group->isToolbarMenuConfigurable()) {
                continue;
            }

            // In CAD tools mode: only show groups that are CAD drafting/editing tool groups
            if (m_filterMode == ActionsFilterMode::CadToolsOnly && !toolGroupsSet.contains(const_cast<LC_ActionGroup*>(group))) {
                continue;
            }

            const QString canonicalToken = namingService->canonicalToken(group->getName());
            QString groupTitle = !group->getTitle().isEmpty() ? group->getTitle() : group->getDescription();
            groupTitle.remove('&');
            auto* groupItem = new LC_ActionTreeItem(m_rootItem, groupTitle, group->getIcon(), nullptr, canonicalToken);


            for (const auto* action : group->actions()) {
                if (action == nullptr) {
                    continue;
                }

                const QString name = action->objectName();

                if (m_filterMode == ActionsFilterMode::CadToolsOnly) {
                    if (name.startsWith(LC_ActionNames::PrefixWidget) ||
                        name.startsWith(LC_ActionNames::PrefixSpecialMenu) ||
                        name.startsWith(LC_ActionNames::PrefixSpecialAction)) {
                        continue;
                    }
                }
                else if (m_filterMode == ActionsFilterMode::ContextMenusOnly) {
                    if (name.startsWith(LC_ActionNames::PrefixWidget) || name == LC_ActionNames::MenuRecentFiles) {
                        continue;
                    }
                }
                else if (m_filterMode == ActionsFilterMode::ToolbarsAndMenus) {
                    if (name.startsWith(LC_ActionNames::PrefixSpecialAction) || name == LC_ActionNames::MenuRecentActions) {
                        continue;
                    }
                }

                QString cleanText = action->text();
                cleanText.remove('&');
                const bool matches = m_filterText.isEmpty() ||
                                     cleanText.contains(m_filterText, Qt::CaseInsensitive) ||
                                     action->toolTip().contains(m_filterText, Qt::CaseInsensitive);

                if (matches) {
                    groupItem->appendChild(new LC_ActionTreeItem(groupItem, cleanText, action->icon(), const_cast<QAction*>(action)));
                }
            }

            if (groupItem->childCount() > 0) {
                m_rootItem->appendChild(groupItem);
            }
            else {
                delete groupItem;
            }
        }
    }

    endResetModel();
}

bool LC_ActionsTreeModel::isAction(const QModelIndex& index) const {
    if (!index.isValid()) return false;
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    return item != nullptr && !item->isGroup();
}

QAction* LC_ActionsTreeModel::getAction(const QModelIndex& index) const {
    if (!index.isValid()) return nullptr;
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    return item != nullptr ? item->action() : nullptr;
}

QModelIndex LC_ActionsTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();

    const LC_ActionTreeItem* parentItem = parent.isValid()
        ? static_cast<const LC_ActionTreeItem*>(parent.internalPointer())
        : m_rootItem;

    const LC_ActionTreeItem* childItem = parentItem->child(row);
    return childItem != nullptr ? createIndex(row, column, const_cast<LC_ActionTreeItem*>(childItem)) : QModelIndex();
}

QModelIndex LC_ActionsTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return QModelIndex();

    const auto* childItem = static_cast<const LC_ActionTreeItem*>(child.internalPointer());
    const LC_ActionTreeItem* parentItem = childItem != nullptr ? childItem->parent() : nullptr;

    if (parentItem == nullptr || parentItem == m_rootItem) {
        return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, const_cast<LC_ActionTreeItem*>(parentItem));
}

int LC_ActionsTreeModel::rowCount(const QModelIndex& parent) const {
    const LC_ActionTreeItem* parentItem = parent.isValid()
        ? static_cast<const LC_ActionTreeItem*>(parent.internalPointer())
        : m_rootItem;
    return parentItem != nullptr ? parentItem->childCount() : 0;
}

int LC_ActionsTreeModel::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant LC_ActionsTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    if (item == nullptr) return QVariant();

    if (role == Qt::DisplayRole) {
        return item->text();
    } else if (role == Qt::DecorationRole) {
        return item->icon();
    } else if (role == Qt::ToolTipRole && item->action() != nullptr) {
        return item->action()->toolTip();
    }
    return QVariant();
}

bool LC_ActionsTreeModel::isGroup(const QModelIndex& index) const {
    if (!index.isValid()) {
        return false;
    }
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    return (item != nullptr && item->isGroup());
}

QString LC_ActionsTreeModel::getGroupTitle(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QString();
    }
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    return (item != nullptr) ? item->text() : QString();
}

QIcon LC_ActionsTreeModel::getGroupIcon(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QIcon();
    }
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    return (item != nullptr) ? item->icon() : QIcon();
}

QList<QAction*> LC_ActionsTreeModel::getGroupActions(const QModelIndex& index) const {
    QList<QAction*> list;
    if (!index.isValid()) {
        return list;
    }
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    if (item != nullptr && item->isGroup()) {
        for (int i = 0; i < item->childCount(); ++i) {
            auto* child = item->child(i);
            if (child != nullptr && child->action() != nullptr) {
                list.append(child->action());
            }
        }
    }
    return list;
}

QString LC_ActionsTreeModel::getCanonicalGroupToken(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QString();
    }
    const auto* item = static_cast<const LC_ActionTreeItem*>(index.internalPointer());
    if (item == nullptr || !item->isGroup()) {
        return QString();
    }
    return item->canonicalToken();
}
