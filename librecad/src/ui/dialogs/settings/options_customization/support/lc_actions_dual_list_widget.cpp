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

#include "lc_actions_dual_list_widget.h"
#include "ui_lc_actions_dual_list_widget.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>

#include "lc_actions_tree_model.h"
#include "lc_action_group_manager.h"
#include "lc_dlg_action_group_config.h"

namespace {
    const int ROLE_NODE_TYPE = Qt::UserRole + 1;
    const int ROLE_ACTION_NAME = Qt::UserRole + 2;
    const int ROLE_POPUP_MODE = Qt::UserRole + 3;
    const int ROLE_GROUP_ICON = Qt::UserRole + 4;
    const int ROLE_GROUP_RAW_TITLE = Qt::UserRole + 5;
}

LC_ActionsDualListWidget::LC_ActionsDualListWidget(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::LC_ActionsDualListWidget>()), m_availableModel(std::make_unique<LC_ActionsTreeModel>(this)) {
    ui->setupUi(this);

    ui->tvAvailable->setModel(m_availableModel.get());

    connect(ui->tbExpandAll, &QToolButton::clicked, ui->tvAvailable, &QTreeView::expandAll);
    connect(ui->tbCollapseAll, &QToolButton::clicked, ui->tvAvailable, &QTreeView::collapseAll);

    connect(ui->tbExpandAllChosen, &QToolButton::clicked, ui->twChosen, &QTreeWidget::expandAll);
    connect(ui->tbCollapseAllChosen, &QToolButton::clicked, ui->twChosen, &QTreeWidget::collapseAll);

    connect(ui->leFilter, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_availableModel->setFilterText(text);
        if (!text.isEmpty()) {
            ui->tvAvailable->expandAll();
        }
    });

    connect(ui->btnAdd, &QPushButton::clicked, this, &LC_ActionsDualListWidget::onAddClicked);
    connect(ui->btnRemove, &QPushButton::clicked, this, &LC_ActionsDualListWidget::onRemoveClicked);
    connect(ui->btnAddGroup, &QPushButton::clicked, this, &LC_ActionsDualListWidget::onAddGroupClicked);
    connect(ui->btnAddSeparator, &QPushButton::clicked, this, &LC_ActionsDualListWidget::onAddSeparatorClicked);
    connect(ui->btnRemoveAll, &QPushButton::clicked, this, &LC_ActionsDualListWidget::onRemoveAllClicked);

    connect(ui->tbEdit, &QToolButton::clicked, this, &LC_ActionsDualListWidget::onEditClicked);
    connect(ui->tbMoveUp, &QToolButton::clicked, this, &LC_ActionsDualListWidget::onMoveUpClicked);
    connect(ui->tbMoveDown, &QToolButton::clicked, this, &LC_ActionsDualListWidget::onMoveDownClicked);
    connect(ui->tbDelete, &QToolButton::clicked, this, &LC_ActionsDualListWidget::onRemoveClicked);
    connect(ui->tbPromote, &QToolButton::clicked, this, &LC_ActionsDualListWidget::onPromoteClicked);

    ui->tbPromote->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));

    connect(ui->tvAvailable, &QTreeView::customContextMenuRequested, this, &LC_ActionsDualListWidget::onAvailableContextMenu);
    connect(ui->twChosen, &QTreeWidget::customContextMenuRequested, this, &LC_ActionsDualListWidget::onChosenContextMenu);

    connect(ui->twChosen, &QTreeWidget::itemSelectionChanged, this, &LC_ActionsDualListWidget::updateChosenToolbarButtons);
    connect(ui->twChosen, &QTreeWidget::itemDoubleClicked, this, &LC_ActionsDualListWidget::onChosenDoubleClicked);
    connect(ui->tvAvailable, &QTreeView::doubleClicked, this, &LC_ActionsDualListWidget::onAvailableDoubleClicked);

    ui->twChosen->viewport()->installEventFilter(this);
    connect(ui->twChosen, &QTreeWidget::itemSelectionChanged, this, &LC_ActionsDualListWidget::updateChosenToolbarButtons);

    connect(ui->tvAvailable->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &LC_ActionsDualListWidget::updateChosenToolbarButtons);

    connect(ui->twChosen->model(), &QAbstractItemModel::rowsMoved, this, [this]() {
        if (!m_blockSignals) {
            emit actionsChanged();
        }
    });

    updateChosenToolbarButtons();
}

bool LC_ActionsDualListWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->twChosen->viewport() && event->type() == QEvent::MouseButtonPress) {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent != nullptr && mouseEvent->button() == Qt::LeftButton) {
            if (ui->twChosen->itemAt(mouseEvent->pos()) == nullptr) {
                ui->twChosen->clearSelection();
                ui->twChosen->setCurrentItem(nullptr);
                updateChosenToolbarButtons();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

LC_ActionsDualListWidget::~LC_ActionsDualListWidget() = default;

void LC_ActionsDualListWidget::setActionGroupManager(LC_ActionGroupManager* manager, ActionsFilterMode mode) {
    m_actionGroupManager = manager;
    if (m_availableModel != nullptr) {
        m_availableModel->setup(manager, mode);
        ui->tvAvailable->expandAll();
    }
}

void LC_ActionsDualListWidget::setGroupsPolicy(GroupsPolicy policy) {
    m_groupsPolicy = policy;
    m_allowNestedGroups = (policy == GroupsPolicy::NestedGroups);
    updateChosenToolbarButtons();
}

void LC_ActionsDualListWidget::setAllowNestedGroups(bool allow) {
    setGroupsPolicy(allow ? GroupsPolicy::NestedGroups : GroupsPolicy::SingleLevelAtRoot);
}

void LC_ActionsDualListWidget::updateChosenToolbarButtons() {
    const auto* item = ui->twChosen->currentItem();
    const bool hasItem = (item != nullptr);
    const bool isGroup = hasItem && (item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group));
    const bool hasChosenItems = (ui->twChosen->topLevelItemCount() > 0);

    // 1. Read-only lockout: disable all modifying actions
    if (m_isReadOnly) {
        ui->btnAdd->setEnabled(false);
        ui->btnAddSeparator->setEnabled(false);
        ui->btnAddGroup->setEnabled(false);
        ui->btnRemove->setEnabled(false);
        ui->btnRemoveAll->setEnabled(false);

        ui->tbEdit->setEnabled(false);
        ui->tbDelete->setEnabled(false);
        ui->tbMoveUp->setEnabled(false);
        ui->tbMoveDown->setEnabled(false);
        ui->tbPromote->setEnabled(false);
        return;
    }

    // 2. Middle Transfer Buttons
    const QModelIndex availIdx = ui->tvAvailable->currentIndex();
    const bool hasAvailAction = (m_availableModel != nullptr && m_availableModel->isAction(availIdx));
    const bool hasAvailGroup = (m_availableModel != nullptr && m_availableModel->isGroup(availIdx));

    bool canAdd = false;
    if (hasAvailAction) {
        canAdd = true;
    }
    else if (hasAvailGroup) {
        if (m_groupsPolicy == GroupsPolicy::NoGroups) {
            canAdd = true; // Allowed: unpacks all child actions flat
        }
        else if (m_groupsPolicy == GroupsPolicy::SingleLevelAtRoot) {
            canAdd = !isGroup;
        }
        else {
            canAdd = true;
        }
    }
    ui->btnAdd->setEnabled(canAdd);
    ui->btnAddSeparator->setEnabled(true);

    // Group creation button
    if (m_groupsPolicy == GroupsPolicy::NoGroups) {
        ui->btnAddGroup->setEnabled(false);
    }
    else if (m_groupsPolicy == GroupsPolicy::SingleLevelAtRoot && isGroup) {
        ui->btnAddGroup->setEnabled(false);
    }
    else {
        ui->btnAddGroup->setEnabled(true);
    }

    ui->btnRemove->setEnabled(hasItem);
    ui->btnRemoveAll->setEnabled(hasChosenItems);

    // 3. Right Header Toolbar Buttons
    const bool supportsNesting = (m_groupsPolicy != GroupsPolicy::NoGroups);
    ui->tbExpandAllChosen->setVisible(supportsNesting);
    ui->tbCollapseAllChosen->setVisible(supportsNesting);
    ui->lineSepChosen->setVisible(supportsNesting);
    ui->tbEdit->setEnabled(isGroup);
    ui->tbDelete->setEnabled(hasItem);

    if (hasItem) {
        auto* parent = item->parent();
        const int row = (parent != nullptr)
                            ? parent->indexOfChild(const_cast<QTreeWidgetItem*>(item))
                            : ui->twChosen->indexOfTopLevelItem(const_cast<QTreeWidgetItem*>(item));
        const int maxRow = (parent != nullptr) ? parent->childCount() - 1 : ui->twChosen->topLevelItemCount() - 1;

        ui->tbMoveUp->setEnabled(row > 0);
        ui->tbMoveDown->setEnabled(row < maxRow);
        ui->tbPromote->setEnabled(parent != nullptr);
    }
    else {
        ui->tbMoveUp->setEnabled(false);
        ui->tbMoveDown->setEnabled(false);
        ui->tbPromote->setEnabled(false);
    }
}

void LC_ActionsDualListWidget::setNodes(const QList<ActionNode>& nodes) {
    m_blockSignals = true;
    ui->twChosen->clear();
    populateTreeRecursive(nullptr, nodes);
    ui->twChosen->expandAll();
    m_blockSignals = false;
    updateChosenToolbarButtons();
}

QList<ActionNode> LC_ActionsDualListWidget::getNodes() const {
    return serializeTreeRecursive(nullptr);
}

void LC_ActionsDualListWidget::clear() {
    m_blockSignals = true;
    ui->twChosen->clear();
    m_blockSignals = false;
    updateChosenToolbarButtons();
}

void LC_ActionsDualListWidget::setReadOnly(bool readOnly) {
    m_isReadOnly = readOnly;
    ui->twChosen->setDragDropMode(readOnly ? QAbstractItemView::NoDragDrop : QAbstractItemView::InternalMove);
    ui->tvAvailable->setEnabled(!readOnly);
    updateChosenToolbarButtons();
}

void LC_ActionsDualListWidget::populateTreeRecursive(QTreeWidgetItem* parentItem, const QList<ActionNode>& nodes) {
    for (const auto& node : nodes) {
        auto* item = (parentItem != nullptr) ? new QTreeWidgetItem(parentItem) : new QTreeWidgetItem(ui->twChosen);
        item->setData(0, ROLE_NODE_TYPE, static_cast<int>(node.type));

        if (node.type == ActionNodeType::Separator) {
            item->setText(0, "------------------------");
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        }
        else if (node.type == ActionNodeType::Action) {
            item->setData(0, ROLE_ACTION_NAME, node.actionName);
            const auto* act = (m_actionGroupManager != nullptr) ? m_actionGroupManager->getActionByName(node.actionName) : nullptr;
            if (act != nullptr) {
                QString cleanText = act->text();
                cleanText.remove('&');
                item->setText(0, cleanText);
                item->setIcon(0, act->icon());
                item->setToolTip(0, act->toolTip());
            }
            else {
                QString cleanText = node.actionName;
                cleanText.remove('&');
                item->setText(0, cleanText);
            }

            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        }
        else if (node.type == ActionNodeType::Group) {
            const QString cleanTitle = (m_actionGroupManager != nullptr)
                ? m_actionGroupManager->displayName(node.groupTitle, /*stripAmpersand=*/true)
                : node.groupTitle;

            item->setText(0, cleanTitle);
            item->setData(0, ROLE_GROUP_RAW_TITLE, node.groupTitle);
            item->setData(0, ROLE_GROUP_ICON, node.groupIcon);
            item->setData(0, ROLE_POPUP_MODE, static_cast<int>(node.popupMode));
            if (!node.groupIcon.isEmpty()) {
                item->setIcon(0, QIcon(node.groupIcon));
            }
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            populateTreeRecursive(item, node.children);
        }
    }
}

QList<ActionNode> LC_ActionsDualListWidget::serializeTreeRecursive(QTreeWidgetItem* parentItem) const {
    QList<ActionNode> nodes;
    const int count = (parentItem != nullptr) ? parentItem->childCount() : ui->twChosen->topLevelItemCount();

    for (int i = 0; i < count; ++i) {
        const auto* item = (parentItem != nullptr) ? parentItem->child(i) : ui->twChosen->topLevelItem(i);
        if (item == nullptr)
            continue;

        ActionNode node;
        node.type = static_cast<ActionNodeType>(item->data(0, ROLE_NODE_TYPE).toInt());

        if (node.type == ActionNodeType::Separator) {
            nodes.append(node);
        }
        else if (node.type == ActionNodeType::Action) {
            node.actionName = item->data(0, ROLE_ACTION_NAME).toString();
            nodes.append(node);
        }
        else if (node.type == ActionNodeType::Group) {
            const QVariant rawTitle = item->data(0, ROLE_GROUP_RAW_TITLE);
            node.groupTitle = (rawTitle.isValid() && !rawTitle.toString().isEmpty()) ? rawTitle.toString() : item->text(0);
            node.groupIcon = item->data(0, ROLE_GROUP_ICON).toString();
            node.popupMode = static_cast<ToolButtonPopupMode>(item->data(0, ROLE_POPUP_MODE).toInt());
            node.children = serializeTreeRecursive(const_cast<QTreeWidgetItem*>(item));
            nodes.append(node);
        }
    }
    return nodes;
}

void LC_ActionsDualListWidget::addActionToChosen(QAction* action, bool forceTopLevel) {
    if (action == nullptr) {
        return;
    }

    auto* current = ui->twChosen->currentItem();
    QTreeWidgetItem* targetParent = nullptr;

    if (!forceTopLevel) {
        if (current != nullptr && current->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group)) {
            targetParent = current;
        }
        else if (current != nullptr && current->parent() != nullptr) {
            targetParent = current->parent();
        }
    }

    auto* item = (targetParent != nullptr) ? new QTreeWidgetItem(targetParent) : new QTreeWidgetItem(ui->twChosen);
    item->setData(0, ROLE_NODE_TYPE, static_cast<int>(ActionNodeType::Action));
    item->setData(0, ROLE_ACTION_NAME, action->objectName());

    QString cleanText = action->text();
    cleanText.remove('&');
    item->setText(0, cleanText);

    item->setIcon(0, action->icon());
    item->setToolTip(0, action->toolTip());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    if (targetParent != nullptr) {
        targetParent->setExpanded(true);
    }
    ui->twChosen->setCurrentItem(item);
    emit actionsChanged();
}

void LC_ActionsDualListWidget::addGroupToChosen(const QString& title, const QString& iconPath, const QList<QAction*>& actions,
                                                bool forceTopLevel) {
    auto* current = ui->twChosen->currentItem();
    QTreeWidgetItem* targetParent = nullptr;

    if (!forceTopLevel && m_allowNestedGroups) {
        if (current != nullptr && current->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group)) {
            targetParent = current;
        }
        else if (current != nullptr && current->parent() != nullptr) {
            targetParent = current->parent();
        }
    }

    auto* groupItem = (targetParent != nullptr) ? new QTreeWidgetItem(targetParent) : new QTreeWidgetItem(ui->twChosen);
    groupItem->setData(0, ROLE_NODE_TYPE, static_cast<int>(ActionNodeType::Group));
    groupItem->setData(0, ROLE_GROUP_RAW_TITLE, title);

    QString finalIconPath = iconPath;
    if (finalIconPath.isEmpty() && m_actionGroupManager != nullptr) {
        finalIconPath = m_actionGroupManager->iconPath(title);
    }
    groupItem->setData(0, ROLE_GROUP_ICON, finalIconPath);
    groupItem->setData(0, ROLE_POPUP_MODE, static_cast<int>(ToolButtonPopupMode::InstantPopup));

    const QString cleanTitle = (m_actionGroupManager != nullptr)
        ? m_actionGroupManager->displayName(title, /*stripAmpersand=*/true)
        : title;

    groupItem->setText(0, cleanTitle);
    if (!finalIconPath.isEmpty()) {
        groupItem->setIcon(0, QIcon(finalIconPath));
    }
    groupItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    // Populate child actions
    for (auto* action : actions) {
        if (action == nullptr) {
            continue;
        }
        auto* childItem = new QTreeWidgetItem(groupItem);
        childItem->setData(0, ROLE_NODE_TYPE, static_cast<int>(ActionNodeType::Action));
        childItem->setData(0, ROLE_ACTION_NAME, action->objectName());

        QString cleanText = action->text();
        cleanText.remove('&');
        childItem->setText(0, cleanText);
        childItem->setIcon(0, action->icon());
        childItem->setToolTip(0, action->toolTip());
        childItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    }

    groupItem->setExpanded(true);
    if (targetParent != nullptr) {
        targetParent->setExpanded(true);
    }

    ui->twChosen->setCurrentItem(groupItem);
    ui->twChosen->scrollToItem(groupItem, QAbstractItemView::EnsureVisible);
    updateChosenToolbarButtons();
    emit actionsChanged();
}

void LC_ActionsDualListWidget::onAddClicked() {
    if (m_availableModel == nullptr || m_isReadOnly) {
        return;
    }
    const QModelIndex idx = ui->tvAvailable->currentIndex();
    if (m_availableModel->isAction(idx)) {
        addActionToChosen(m_availableModel->getAction(idx), false);
    }
    else if (m_availableModel->isGroup(idx)) {
        const auto* item = ui->twChosen->currentItem();
        const bool isGroup = (item != nullptr && item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group));

        if (m_groupsPolicy == GroupsPolicy::NoGroups) {
            // Depth 1: Unpack all actions in the group flat
            const auto actions = m_availableModel->getGroupActions(idx);
            for (auto* act : actions) {
                if (act != nullptr) {
                    addActionToChosen(act, false);
                }
            }
        }
        else {
            const bool forceTop = (m_groupsPolicy == GroupsPolicy::SingleLevelAtRoot && isGroup);
            const QString canonicalToken = m_availableModel->getCanonicalGroupToken(idx);
            const QString iconPath = m_actionGroupManager->iconPath(canonicalToken);

            addGroupToChosen(canonicalToken, iconPath, m_availableModel->getGroupActions(idx), forceTop);
        }
    }
}

void LC_ActionsDualListWidget::onAvailableDoubleClicked(const QModelIndex& index) {
    if (m_isReadOnly) {
        return;
    }
    if (m_availableModel == nullptr) {
        return;
    }
    if (m_availableModel->isAction(index)) {
        addActionToChosen(m_availableModel->getAction(index), false);
    }
}

void LC_ActionsDualListWidget::onAvailableContextMenu(const QPoint& pos) {
    if (m_isReadOnly) {
        return;
    }
    const QModelIndex idx = ui->tvAvailable->indexAt(pos);
    QMenu menu(ui->tvAvailable);

    if (m_availableModel != nullptr && m_availableModel->isAction(idx)) {
        QAction* targetAction = m_availableModel->getAction(idx);
        if (targetAction != nullptr) {
            menu.addAction(QIcon(":/icons/chevron_right.lci"), tr("Add"), this, [this, targetAction]() {
                addActionToChosen(targetAction, false);
            });
            menu.addAction(QIcon(":/icons/chevron_right_all.lci"), tr("Add to Top Level"), this, [this, targetAction]() {
                addActionToChosen(targetAction, true);
            });
        }
    }
    else if (m_availableModel != nullptr && m_availableModel->isGroup(idx)) {
        const auto actions = m_availableModel->getGroupActions(idx);

        if (m_groupsPolicy == GroupsPolicy::NoGroups) {
            // Flat mode: unpack and append all actions in the group flat
            menu.addAction(QIcon(":/icons/chevron_right.lci"), tr("Add Group Actions"), this, [this, actions]() {
                for (auto* act : actions) {
                    if (act != nullptr) {
                        addActionToChosen(act, false);
                    }
                }
            });
        }
        else {
            const QString canonicalToken = m_availableModel->getCanonicalGroupToken(idx);
            const QString iconPath = m_actionGroupManager->iconPath(canonicalToken);

            menu.addAction(QIcon(":/icons/chevron_right.lci"), tr("Add Entire Group"), this, [this, canonicalToken, iconPath, actions]() {
                addGroupToChosen(canonicalToken, iconPath, actions, false);
            });
            menu.addAction(QIcon(":/icons/chevron_right_all.lci"), tr("Add Entire Group to Top Level"), this,
                           [this, canonicalToken, iconPath, actions]() {
                               addGroupToChosen(canonicalToken, iconPath, actions, true);
                           });
        }

        menu.addSeparator();
        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvAvailable, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvAvailable, &QTreeView::collapseAll);
    }
    else {
        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvAvailable, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvAvailable, &QTreeView::collapseAll);
    }

    menu.exec(ui->tvAvailable->viewport()->mapToGlobal(pos));
}

void LC_ActionsDualListWidget::onChosenContextMenu(const QPoint& pos) {
    if (m_isReadOnly) {
        return;
    }

    auto* item = ui->twChosen->itemAt(pos);
    QMenu menu(ui->twChosen);
    const bool supportsNesting = (m_groupsPolicy != GroupsPolicy::NoGroups);

    if (item != nullptr) {
        ui->twChosen->setCurrentItem(item);

        if (item->parent() != nullptr) {
            auto* actPromote = menu.addAction(QIcon(":/icons/back.lci"), tr("Move to Parent Level"), this,
                                              &LC_ActionsDualListWidget::onPromoteClicked);
            actPromote->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
        }

        auto* actUp = menu.addAction(QIcon(":/icons/up.lci"), tr("Move Up"), this, &LC_ActionsDualListWidget::onMoveUpClicked);
        actUp->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Up));

        auto* actDown = menu.addAction(QIcon(":/icons/down.lci"), tr("Move Down"), this, &LC_ActionsDualListWidget::onMoveDownClicked);
        actDown->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Down));

        auto* parent = item->parent();
        const int row = (parent != nullptr) ? parent->indexOfChild(item) : ui->twChosen->indexOfTopLevelItem(item);
        const int maxRow = (parent != nullptr) ? parent->childCount() - 1 : ui->twChosen->topLevelItemCount() - 1;
        actUp->setEnabled(row > 0);
        actDown->setEnabled(row < maxRow);

        menu.addSeparator();

        const bool isGroup = (item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group));
        if (isGroup) {
            menu.addAction(QIcon(":/icons/rename_active_block.lci"), tr("Edit..."), this, &LC_ActionsDualListWidget::onEditClicked);
        }

        auto* actRemove = menu.addAction(QIcon(":/icons/remove.lci"), tr("Remove"), this, &LC_ActionsDualListWidget::onRemoveClicked);
        actRemove->setShortcut(QKeySequence::Delete);

        if (supportsNesting) {
            menu.addSeparator();
            menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->twChosen, &QTreeWidget::expandAll);
            menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->twChosen, &QTreeWidget::collapseAll);
        }
    }
    else {
        if (supportsNesting) {
            const QString groupLabel = m_allowNestedGroups ? tr("Add Sub-Menu...") : tr("Add Group...");
            menu.addAction(QIcon(":/icons/create_menu.lci"), groupLabel, this, &LC_ActionsDualListWidget::onAddGroupClicked);
        }
        menu.addAction(QIcon(":/icons/separator_add.lci"), tr("Add Separator"), this, &LC_ActionsDualListWidget::onAddSeparatorClicked);
        menu.addSeparator();

        if (supportsNesting) {
            menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->twChosen, &QTreeWidget::expandAll);
            menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->twChosen, &QTreeWidget::collapseAll);
            menu.addSeparator();
        }

        auto* actClearAll = menu.addAction(tr("Clear All"), this, &LC_ActionsDualListWidget::onRemoveAllClicked);
        actClearAll->setEnabled(ui->twChosen->topLevelItemCount() > 0);
    }

    menu.exec(ui->twChosen->viewport()->mapToGlobal(pos));
}

void LC_ActionsDualListWidget::moveTreeItem(QTreeWidgetItem* item, QTreeWidgetItem* newParent, int newIndex) {
    if (item == nullptr || newIndex < 0) {
        return;
    }

    auto* oldParent = item->parent();
    const int oldIndex = (oldParent != nullptr) ? oldParent->indexOfChild(item) : ui->twChosen->indexOfTopLevelItem(item);

    if (oldParent == newParent && oldIndex == newIndex) {
        return;
    }

    const QSet<QString> expandedState = saveTreeExpansionState();
    m_blockSignals = true;

    // Remove from old location
    if (oldParent != nullptr) {
        oldParent->takeChild(oldIndex);
    }
    else {
        ui->twChosen->takeTopLevelItem(oldIndex);
    }

    // Insert into target location
    if (newParent != nullptr) {
        newParent->insertChild(newIndex, item);
    }
    else {
        ui->twChosen->insertTopLevelItem(newIndex, item);
    }

    restoreTreeExpansionState(expandedState);

    ui->twChosen->setCurrentItem(item);
    item->setSelected(true);
    ui->twChosen->scrollToItem(item, QAbstractItemView::EnsureVisible);
    ui->twChosen->setFocus();
    m_blockSignals = false;

    updateChosenToolbarButtons();
    emit actionsChanged();
}

void LC_ActionsDualListWidget::onAddSeparatorClicked() {
    auto* current = ui->twChosen->currentItem();
    QTreeWidgetItem* targetParent = (current != nullptr && current->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(
                                        ActionNodeType::Group))
                                        ? current
                                        : (current ? current->parent() : nullptr);

    auto* item = (targetParent != nullptr) ? new QTreeWidgetItem(targetParent) : new QTreeWidgetItem(ui->twChosen);
    item->setData(0, ROLE_NODE_TYPE, static_cast<int>(ActionNodeType::Separator));
    item->setText(0, "------------------------");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    if (targetParent != nullptr)
        targetParent->setExpanded(true);
    ui->twChosen->setCurrentItem(item);
    emit actionsChanged();
}

void LC_ActionsDualListWidget::onAddGroupClicked() {
    LC_DlgActionGroupConfig dlg(this, !m_allowNestedGroups, m_actionGroupManager);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    auto* current = ui->twChosen->currentItem();
    QTreeWidgetItem* targetParent = nullptr;

    if (m_allowNestedGroups && current != nullptr && current->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group)) {
        targetParent = current;
    }

    auto* item = (targetParent != nullptr) ? new QTreeWidgetItem(targetParent) : new QTreeWidgetItem(ui->twChosen);
    item->setData(0, ROLE_NODE_TYPE, static_cast<int>(ActionNodeType::Group));
    item->setData(0, ROLE_GROUP_ICON, dlg.iconPath());
    item->setData(0, ROLE_POPUP_MODE, static_cast<int>(dlg.popupMode()));
    item->setData(0, ROLE_GROUP_RAW_TITLE, dlg.title());

    QString cleanTitle = dlg.title();
    cleanTitle.remove('&');
    item->setText(0, cleanTitle);

    if (!dlg.iconPath().isEmpty()) {
        item->setIcon(0, QIcon(dlg.iconPath()));
    }
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    if (targetParent != nullptr) {
        targetParent->setExpanded(true);
    }
    ui->twChosen->setCurrentItem(item);
    emit actionsChanged();
}

void LC_ActionsDualListWidget::onRemoveClicked() {
    auto* item = ui->twChosen->currentItem();
    if (item == nullptr)
        return;

    const bool isGroup = item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group);
    if (isGroup && item->childCount() > 0) {
        if (QMessageBox::question(this, tr("Delete Group"),
                                  tr("Group '%1' contains %2 action(s). Delete this group and all its contents?").arg(item->text(0)).
                                  arg(item->childCount()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }

    delete item;
    updateChosenToolbarButtons();
    emit actionsChanged();
}

void LC_ActionsDualListWidget::onRemoveAllClicked() {
    if (ui->twChosen->topLevelItemCount() == 0) {
        return;
    }

    if (QMessageBox::question(this, tr("Clear All"), tr("Are you sure you want to remove all items?"), QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        clear();
        emit actionsChanged();
    }
}

void LC_ActionsDualListWidget::onChosenDoubleClicked(QTreeWidgetItem* item, int) {
    if (m_isReadOnly || item == nullptr) {
        return;
    }
    if (item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group)) {
        editGroupItem(item);
    }
    else {
        onRemoveClicked();
    }
}

void LC_ActionsDualListWidget::onEditClicked() {
    auto* item = ui->twChosen->currentItem();
    if (item != nullptr && item->data(0, ROLE_NODE_TYPE).toInt() == static_cast<int>(ActionNodeType::Group)) {
        editGroupItem(item);
    }
}

void LC_ActionsDualListWidget::editGroupItem(QTreeWidgetItem* item) {
    if (item == nullptr) {
        return;
    }

    LC_DlgActionGroupConfig dlg(this, !m_allowNestedGroups, m_actionGroupManager);
    const QString rawTitle = item->data(0, ROLE_GROUP_RAW_TITLE).toString();
    const QString iconPath = item->data(0, ROLE_GROUP_ICON).toString();
    const auto popupMode = static_cast<ToolButtonPopupMode>(item->data(0, ROLE_POPUP_MODE).toInt());

    dlg.setInitialValues(rawTitle, iconPath, popupMode);

    if (dlg.exec() == QDialog::Accepted) {
        const QString newRawTitle = dlg.title();
        const QString cleanTitle = m_actionGroupManager->displayName(newRawTitle, /*stripAmpersand=*/true);

        item->setText(0, cleanTitle);
        item->setData(0, ROLE_GROUP_RAW_TITLE, newRawTitle);
        item->setData(0, ROLE_GROUP_ICON, dlg.iconPath());
        item->setData(0, ROLE_POPUP_MODE, static_cast<int>(dlg.popupMode()));

        if (!dlg.iconPath().isEmpty()) {
            item->setIcon(0, QIcon(dlg.iconPath()));
        }
        else {
            item->setIcon(0, QIcon());
        }

        emit actionsChanged();
    }
}

void LC_ActionsDualListWidget::onMoveUpClicked() {
    auto* item = ui->twChosen->currentItem();
    if (item == nullptr) {
        return;
    }

    auto* parent = item->parent();
    const int idx = (parent != nullptr) ? parent->indexOfChild(item) : ui->twChosen->indexOfTopLevelItem(item);
    if (idx > 0) {
        moveTreeItem(item, parent, idx - 1);
    }
}

void LC_ActionsDualListWidget::onMoveDownClicked() {
    auto* item = ui->twChosen->currentItem();
    if (item == nullptr) {
        return;
    }

    auto* parent = item->parent();
    const int idx = (parent != nullptr) ? parent->indexOfChild(item) : ui->twChosen->indexOfTopLevelItem(item);
    const int maxIdx = (parent != nullptr) ? parent->childCount() - 1 : ui->twChosen->topLevelItemCount() - 1;
    if (idx >= 0 && idx < maxIdx) {
        moveTreeItem(item, parent, idx + 1);
    }
}

void LC_ActionsDualListWidget::onPromoteClicked() {
    auto* item = ui->twChosen->currentItem();
    if (item == nullptr || item->parent() == nullptr) {
        return;
    }

    auto* parent = item->parent();
    auto* grandParent = parent->parent();
    const int parentRow = (grandParent != nullptr) ? grandParent->indexOfChild(parent) : ui->twChosen->indexOfTopLevelItem(parent);

    moveTreeItem(item, grandParent, parentRow + 1);
}

QSet<QString> LC_ActionsDualListWidget::saveTreeExpansionState() const {
    QSet<QString> expandedState;
    const int count = ui->twChosen->topLevelItemCount();
    for (int i = 0; i < count; ++i) {
        collectExpansionStateRecursive(ui->twChosen->topLevelItem(i), QString(), expandedState);
    }
    return expandedState;
}

void LC_ActionsDualListWidget::collectExpansionStateRecursive(QTreeWidgetItem* item, const QString& parentPath,
                                                              QSet<QString>& expandedState) const {
    if (item == nullptr) {
        return;
    }
    const QString currentPath = parentPath.isEmpty() ? item->text(0) : parentPath + "/" + item->text(0);
    if (item->isExpanded()) {
        expandedState.insert(currentPath);
    }
    const int childCount = item->childCount();
    for (int i = 0; i < childCount; ++i) {
        collectExpansionStateRecursive(item->child(i), currentPath, expandedState);
    }
}

void LC_ActionsDualListWidget::restoreTreeExpansionState(const QSet<QString>& expandedState) {
    const int count = ui->twChosen->topLevelItemCount();
    for (int i = 0; i < count; ++i) {
        applyExpansionStateRecursive(ui->twChosen->topLevelItem(i), QString(), expandedState);
    }
}

void LC_ActionsDualListWidget::applyExpansionStateRecursive(QTreeWidgetItem* item, const QString& parentPath,
                                                            const QSet<QString>& expandedState) {
    if (item == nullptr) {
        return;
    }
    const QString currentPath = parentPath.isEmpty() ? item->text(0) : parentPath + "/" + item->text(0);
    if (expandedState.contains(currentPath)) {
        item->setExpanded(true);
    }
    const int childCount = item->childCount();
    for (int i = 0; i < childCount; ++i) {
        applyExpansionStateRecursive(item->child(i), currentPath, expandedState);
    }
}
