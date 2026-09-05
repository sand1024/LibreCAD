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

#include "lc_settings_page_shortcuts.h"

#include <QMenu>

#include "ui_lc_settings_page_shortcuts.h"

#include <QScrollBar>

#include "lc_actiongroup.h"
#include "lc_actiongroupmanager.h"
#include "lc_palette_color_utils.h"
#include "lc_preset_manager_shortcuts.h"
#include "lc_settings_colors_semantics.h"
#include "lc_shortcut_search_popup.h"
#include "lc_shortcutinfo.h"
#include "lc_shortcutstreemodel.h"
#include "../support/lc_shortcuttreeitem.h"

LC_SettingsPageShortcuts::LC_SettingsPageShortcuts(LC_ActionGroupManager* groupManager, QObject* parent)
    : LC_SettingsPageBase(tr("Keymap"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageShortcuts>())
    , m_actionGroupManager(groupManager) {
    setSortWeight(10);
}

LC_SettingsPageShortcuts::~LC_SettingsPageShortcuts() {
    delete m_mappingTreeModel;
}

void LC_SettingsPageShortcuts::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerShortcuts*>(manager);
    if (m_presetManager != nullptr && m_mappingTreeModel != nullptr) {
        m_presetManager->setTreeModel(m_mappingTreeModel);
    }
}

void LC_SettingsPageShortcuts::setupUi() {
    ui->setupUi(m_widget);

    const QColor filteredCol = LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors::SearchResultItem, m_widget->palette());
    const QColor conflictCol = LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors::ConflictingItem, m_widget->palette());

    m_mappingTreeModel = new LC_ShortcutsTreeModel(this, filteredCol, conflictCol);
    if (m_actionGroupManager != nullptr) {
        m_mappingTreeModel->rebuildModel(m_actionGroupManager);
    }

    ui->tvMappingsTree->setup(m_mappingTreeModel);

    // --- Header and Tree Configuration ---
    QHeaderView* pTreeHeader = ui->tvMappingsTree->header();
    pTreeHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    pTreeHeader->setStretchLastSection(true);
    pTreeHeader->hide();

    ui->tvMappingsTree->setUniformRowHeights(true);
    ui->tvMappingsTree->setAlternatingRowColors(false);
    ui->tvMappingsTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvMappingsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvMappingsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvMappingsTree->expandAll();

    // --- Master-Detail Splitter Configuration ---
    ui->splShortcuts->setChildrenCollapsible(false);
    ui->splShortcuts->setStretchFactor(0, 3);
    ui->splShortcuts->setStretchFactor(1, 2);
    ui->splShortcuts->setSizes(QList<int>{480, 270});

    // Default to empty placeholder state
    ui->swDetails->setCurrentWidget(ui->pageEmpty);
    ui->frameConflict->setVisible(false);
    ui->lblConflict->setTextFormat(Qt::RichText);

    // Dynamic typography scaling based on active application style
    QFont actionTitleFont = QApplication::font("QLabel");
    actionTitleFont.setBold(true);
    if (actionTitleFont.pointSize() > 0) {
        actionTitleFont.setPointSize(actionTitleFont.pointSize() + 1);
    } else if (actionTitleFont.pixelSize() > 0) {
        actionTitleFont.setPixelSize(actionTitleFont.pixelSize() + 1);
    }
    ui->lblActionName->setFont(actionTitleFont);

    m_searchPopup = new LC_ShortcutSearchPopup(m_widget);

    if (m_presetManager != nullptr) {
        m_presetManager->setTreeModel(m_mappingTreeModel);
    }
}

void LC_SettingsPageShortcuts::setupBehavior() {
    connect(ui->tbExpandAll, &QToolButton::clicked, ui->tvMappingsTree, &QTreeView::expandAll);
    connect(ui->tbCollapseAll, &QToolButton::clicked, ui->tvMappingsTree, &QTreeView::collapseAll);

    connect(ui->leFilter, &QLineEdit::textChanged, this, &LC_SettingsPageShortcuts::onFilteringMaskChanged);
    connect(ui->cbMatchHighlight, &QCheckBox::toggled, this, &LC_SettingsPageShortcuts::onFilteringMaskChanged);

    connect(ui->tbFindByShortcut, &QToolButton::clicked, this, [this]() {
        if (m_searchPopup->isVisible()) {
            m_searchPopup->hide();
        } else {
            const QPoint pos = ui->tbFindByShortcut->mapToGlobal(QPoint(0, ui->tbFindByShortcut->height() + 2));
            m_searchPopup->move(pos);
            m_searchPopup->show();
            m_searchPopup->setFocus();
        }
    });

    connect(m_searchPopup, &LC_ShortcutSearchPopup::shortcutFilterChanged,
            this, &LC_SettingsPageShortcuts::onShortcutFilterChanged);

    connect(ui->tvMappingsTree, &QTreeView::customContextMenuRequested,
           this, &LC_SettingsPageShortcuts::onTreeContextMenuRequested);

    connect(ui->tvMappingsTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &LC_SettingsPageShortcuts::onTreeSelectionChanged);

    connect(ui->tvMappingsTree, &QTreeView::doubleClicked, this, [this](const QModelIndex&) {
        ui->btnRecord->setChecked(true);
        ui->btnRecord->setFocus();
    });

    connect(ui->btnRecord, &LC_ShortcutButton::keySequenceChanged, this, &LC_SettingsPageShortcuts::onKeySequenceRecorded);
    connect(ui->btnRecord, &QPushButton::toggled, this, &LC_SettingsPageShortcuts::onRecordToggled);

    connect(ui->btnReset, &QPushButton::clicked, this, &LC_SettingsPageShortcuts::onResetItemClicked);
    connect(ui->pbClear, &QPushButton::clicked, this, &LC_SettingsPageShortcuts::onClearItemClicked);

    connect(ui->lblConflict, &QLabel::linkActivated, this, &LC_SettingsPageShortcuts::onShowConflictsRequested);
}

void LC_SettingsPageShortcuts::loadSettings() {
    if (m_presetManager != nullptr) {
        m_presetManager->loadPreset(m_presetManager->getActivePresetKey());
    }
    rebuildTree(false);
}

bool LC_SettingsPageShortcuts::saveSettings() {
    if (m_presetManager != nullptr) {
        return m_presetManager->saveCurrentPreset();
    }
    return true;
}

bool LC_SettingsPageShortcuts::isModified() const {
    return m_mappingTreeModel ? m_mappingTreeModel->isModified() : false;
}

bool LC_SettingsPageShortcuts::validate(QString& outErrorMessage) {
    if (checkHasCollisions(nullptr)) {
        outErrorMessage = tr("There are unresolved shortcut conflicts. Please resolve all collisions before saving.");
        onShowConflictsRequested();
        return false;
    }
    return true;
}

void LC_SettingsPageShortcuts::onControlChanged() {
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
    emit livePreviewRequested();
}

void LC_SettingsPageShortcuts::autoIndexLabels() {
    LC_SettingsPageBase::autoIndexLabels();

    if (m_actionGroupManager != nullptr) {
        for (const auto* group : m_actionGroupManager->allGroupsList()) {
            if (group == nullptr || !group->isActionMappingsMayBeConfigured()) {
                continue;
            }
            for (const auto* action : group->actions()) {
                if (action == nullptr) {
                    continue;
                }
                const QString name = action->text().remove('&').trimmed();
                registerSearchTarget(ui->tvMappingsTree, name);
                if (!action->toolTip().isEmpty()) {
                    registerSearchTarget(ui->tvMappingsTree, action->toolTip().trimmed());
                }
            }
        }
    }
}

void LC_SettingsPageShortcuts::highlightSearchPattern(const QString& pattern) {
    ui->leFilter->setText(pattern);
}

void LC_SettingsPageShortcuts::clearSearchHighlight() {
    ui->leFilter->clear();
}

void LC_SettingsPageShortcuts::onFilteringMaskChanged() {
    if (m_mappingTreeModel != nullptr) {
        const QString text = ui->leFilter->text();
        const bool highlightOnly = ui->cbMatchHighlight->isChecked();
        m_mappingTreeModel->setFilteringRegexp(text, highlightOnly);
        rebuildTree(false);
    }
}

void LC_SettingsPageShortcuts::onShortcutFilterChanged(const QKeySequence& sequence) {
    if (m_mappingTreeModel != nullptr) {
        m_mappingTreeModel->setFilteringShortcut(sequence);
        rebuildTree(false);
    }
}

void LC_SettingsPageShortcuts::onTreeSelectionChanged() {
    const QModelIndexList selected = ui->tvMappingsTree->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        selectItem(nullptr, -1, -1);
        return;
    }
    const QModelIndex idx = selected.first();
    LC_ShortcutTreeItem* item = m_mappingTreeModel->getItemForIndex(idx);
    const QModelIndex parentIdx = m_mappingTreeModel->parent(idx);
    selectItem(item, idx.row(), parentIdx.row());
}

void LC_SettingsPageShortcuts::selectItem(LC_ShortcutTreeItem* item, int row, int parentRow) {
    if (item == nullptr || item->isGroup()) {
        ui->swDetails->setCurrentWidget(ui->pageEmpty);
        m_currentItem = nullptr;
        m_selectedRow = -1;
        m_selectedParentRow = -1;
    } else {
        ui->swDetails->setCurrentWidget(ui->pageDetails);
        ui->lblActionName->setText(item->getName());
        ui->lblGroupName->setText(tr("%1").arg(item->parent() != nullptr ? item->parent()->getName() : tr("General")));
        ui->lblActionIcon->setPixmap(item->getIcon().pixmap(24, 24));
        ui->leKeySequence->setText(item->getShortcutViewString());

        const QString desc = /*item->getDescription().trimmed();*/"";
        ui->lblDescription->setText(desc.isEmpty() ? tr("No description available for this action.") : desc);

        checkHasCollisions(item->getShortcutInfo());

        m_currentItem = item;
        m_selectedRow = row;
        m_selectedParentRow = parentRow;
    }
}

void LC_SettingsPageShortcuts::onRecordToggled(bool recording) {
    if (!recording && m_currentItem != nullptr) {
        LC_ShortcutInfo* info = m_currentItem->getShortcutInfo();
        if (info != nullptr) {
            info->setKey(m_editingKeySequence);
            if (info->isModified()) {
                checkHasCollisions(info);
                rebuildTree(true);
                onControlChanged();
            }
        }
        m_editingKeySequence = QKeySequence();
    }
}

void LC_SettingsPageShortcuts::onKeySequenceRecorded(const QKeySequence& key) {
    m_editingKeySequence = key;
    ui->leKeySequence->setText(key.toString(QKeySequence::PortableText));
}

void LC_SettingsPageShortcuts::onResetItemClicked() {
    if (m_currentItem != nullptr) {
        m_currentItem->resetShortcutToDefault();
        ui->leKeySequence->setText(m_currentItem->getShortcutViewString());
        checkHasCollisions(m_currentItem->getShortcutInfo());
        rebuildTree(true);
        onControlChanged();
    }
}

void LC_SettingsPageShortcuts::onClearItemClicked() {
    if (m_currentItem != nullptr) {
        m_currentItem->clearShortcut();
        ui->leKeySequence->setText("");
        m_editingKeySequence = QKeySequence();
        checkHasCollisions(m_currentItem->getShortcutInfo());
        rebuildTree(true);
        onControlChanged();
    }
}

bool LC_SettingsPageShortcuts::checkHasCollisions(LC_ShortcutInfo* shortcutInfo) const {
    if (m_mappingTreeModel == nullptr) {
        return false;
    }

    const bool hasCollisions = m_mappingTreeModel->checkForCollisions(shortcutInfo);
    if (hasCollisions) {
        ui->frameConflict->setVisible(true);
        ui->lblConflict->setText(tr("<font color='#d9534f'><b>Potential Conflict:</b></font><br/>"
                                    "This shortcut sequence is assigned to multiple actions. "
                                    "<a href=\"show\">Filter conflicting actions</a>"));
    } else {
        ui->frameConflict->setVisible(false);
    }
    return hasCollisions;
}

void LC_SettingsPageShortcuts::onShowConflictsRequested() {
    if (m_mappingTreeModel != nullptr) {
        m_mappingTreeModel->setFilterForConflicts(true);
        rebuildTree(false);
    }
}

void LC_SettingsPageShortcuts::rebuildTree(bool restoreSelection) {
    if (m_mappingTreeModel == nullptr) {
        return;
    }

    QScrollBar* scroll = ui->tvMappingsTree->verticalScrollBar();
    const int scrollPos = (scroll != nullptr) ? scroll->value() : 0;

    m_mappingTreeModel->rebuildModel(m_actionGroupManager);

    if (restoreSelection && m_selectedRow >= 0) {
        const QModelIndex parentIdx = m_mappingTreeModel->index(m_selectedParentRow, 0, QModelIndex());
        if (parentIdx.isValid()) {
            const QModelIndex itemIdx = m_mappingTreeModel->index(m_selectedRow, 0, parentIdx);
            ui->tvMappingsTree->selectionModel()->select(itemIdx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
    ui->tvMappingsTree->expandAll();
    if (scroll != nullptr) {
        scroll->setValue(scrollPos);
    }
}

void LC_SettingsPageShortcuts::onTreeContextMenuRequested(const QPoint& pos) {
    const QModelIndex index = ui->tvMappingsTree->indexAt(pos);
    LC_ShortcutTreeItem* item = nullptr;

    if (index.isValid()) {
        ui->tvMappingsTree->setCurrentIndex(index);
        item = m_mappingTreeModel->getItemForIndex(index);
    }

    QMenu menu(ui->tvMappingsTree);

    if (item != nullptr && !item->isGroup()) {
        // Case B: Clicked on a leaf action item
        QAction* actAssign = menu.addAction(tr("Assign Shortcut..."), this, [this]() {
            ui->btnRecord->setChecked(true);
            ui->btnRecord->setFocus();
        });

        QAction* actReset = menu.addAction(tr("Reset"), this, &LC_SettingsPageShortcuts::onResetItemClicked);
        QAction* actClear = menu.addAction(tr("Clear"), this, &LC_SettingsPageShortcuts::onClearItemClicked);

        const LC_ShortcutInfo* info = item->getShortcutInfo();
        if (info != nullptr) {
            actReset->setEnabled(info->isModified());
            actClear->setEnabled(!info->getKey().isEmpty());
        }

        menu.addSeparator();
        menu.addAction(tr("Reset All"), this, &LC_SettingsPageShortcuts::onResetAllClicked);
    }
    else {
        // Case A: Clicked on a group or outside of any item
        menu.addAction(tr("Reset All"), this, &LC_SettingsPageShortcuts::onResetAllClicked);
    }

    menu.exec(ui->tvMappingsTree->viewport()->mapToGlobal(pos));
}

void LC_SettingsPageShortcuts::onResetAllClicked() {
    loadSettings(); // Reloads active preset from disk/defaults and rebuilds tree
    onControlChanged();
}
