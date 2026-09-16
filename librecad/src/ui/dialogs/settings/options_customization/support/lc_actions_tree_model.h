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

#ifndef LC_ACTIONS_TREE_MODEL_H
#define LC_ACTIONS_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QString>

class QAction;
class LC_ActionGroupManager;
class LC_ActionTreeItem;


enum class ActionsFilterMode {
    ToolbarsAndMenus,
    ContextMenusOnly,
    CadToolsOnly
};


class LC_ActionsTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit LC_ActionsTreeModel(QObject* parent = nullptr);
    ~LC_ActionsTreeModel() override;

    void setup(LC_ActionGroupManager* manager, ActionsFilterMode mode = ActionsFilterMode::ToolbarsAndMenus);
    void setFilterText(const QString& filter);

    bool isAction(const QModelIndex& index) const;
    QAction* getAction(const QModelIndex& index) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool isGroup(const QModelIndex& index) const;
    QString getGroupTitle(const QModelIndex& index) const;
    QIcon getGroupIcon(const QModelIndex& index) const;
    QList<QAction*> getGroupActions(const QModelIndex& index) const;
    QString getCanonicalGroupToken(const QModelIndex& index) const;
private:
    void rebuildTree();

    LC_ActionTreeItem* m_rootItem = nullptr;
    LC_ActionGroupManager* m_groupManager = nullptr;
    QString m_filterText;
    ActionsFilterMode m_filterMode = ActionsFilterMode::ToolbarsAndMenus;
};

#endif
