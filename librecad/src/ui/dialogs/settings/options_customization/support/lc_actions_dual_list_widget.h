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

#ifndef LC_ACTIONS_DUAL_LIST_WIDGET_H
#define LC_ACTIONS_DUAL_LIST_WIDGET_H

#include <QWidget>
#include <memory>

#include "lc_actions_tree_model.h"
#include "lc_action_node.h"

class LC_ActionNamingServiceInterface;

namespace Ui {
    class LC_ActionsDualListWidget;
}

class LC_ActionGroupManager;
class LC_ActionsTreeModel;
class QTreeWidgetItem;

class LC_ActionsDualListWidget : public QWidget {
    Q_OBJECT
public:
    enum class GroupsPolicy {
        NoGroups,          // Depth 1: Flat actions only (Individual CAD Dock Widgets)
        SingleLevelAtRoot, // Depth 2: Groups at top-level only (Toolbars)
        NestedGroups       // Unlimited Depth: Full hierarchy (Menus & CAD Tools Matrix)
    };
    explicit LC_ActionsDualListWidget(QWidget* parent = nullptr);
    bool eventFilter(QObject* obj, QEvent* event) override;
    ~LC_ActionsDualListWidget() override;

    void setActionGroupManager(LC_ActionGroupManager* manager,
                              ActionsFilterMode mode = ActionsFilterMode::ToolbarsAndMenus);
    void setGroupsPolicy(GroupsPolicy policy);
    void setAllowNestedGroups(bool allow);

    void setNodes(const QList<ActionNode>& nodes);
    QList<ActionNode> getNodes() const;
    void clear();
    void setReadOnly(bool readOnly);

 signals:
        void actionsChanged();

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onAddGroupClicked();
    void onAddSeparatorClicked();
    void onRemoveAllClicked();
    void onEditClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onChosenDoubleClicked(QTreeWidgetItem* item, int column);
    void onAvailableDoubleClicked(const QModelIndex& index);
    void updateChosenToolbarButtons();
    void onPromoteClicked();
    void onAvailableContextMenu(const QPoint& pos);
    void onChosenContextMenu(const QPoint& pos);

private:
    QSet<QString> saveTreeExpansionState() const;
    void restoreTreeExpansionState(const QSet<QString>& expandedState);
    void collectExpansionStateRecursive(QTreeWidgetItem* item, const QString& parentPath, QSet<QString>& expandedState) const;
    void applyExpansionStateRecursive(QTreeWidgetItem* item, const QString& parentPath, const QSet<QString>& expandedState);

    void moveTreeItem(QTreeWidgetItem* item, QTreeWidgetItem* newParent, int newIndex);
    void addActionToChosen(QAction* action, bool forceTopLevel);
    void addGroupToChosen(const QString& title, const QString& iconPath, const QList<QAction*>& actions, bool forceTopLevel = false);
    void populateTreeRecursive(QTreeWidgetItem* parentItem, const QList<ActionNode>& nodes);
    QList<ActionNode> serializeTreeRecursive(QTreeWidgetItem* parentItem) const;
    void editGroupItem(QTreeWidgetItem* item);

    std::unique_ptr<Ui::LC_ActionsDualListWidget> ui;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
    std::unique_ptr<LC_ActionsTreeModel> m_availableModel;
    bool m_allowNestedGroups = false;
    bool m_blockSignals = false;
    GroupsPolicy m_groupsPolicy{GroupsPolicy::SingleLevelAtRoot};
    bool m_isReadOnly{false};
};

#endif
