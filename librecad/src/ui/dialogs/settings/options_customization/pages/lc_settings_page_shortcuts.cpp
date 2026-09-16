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

#include "lc_action.h"
#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_cheatsheet_generator.h"
#include "lc_cheatsheet_options.h"
#include "lc_dlg_cheatsheet_options.h"
#include "lc_palette_color_utils.h"
#include "lc_preset_manager_shortcuts.h"
#include "lc_settings_colors_semantics.h"
#include "lc_shortcut_search_popup.h"
#include "lc_shortcut_info.h"
#include "lc_shortcuts_tree_model.h"
#include "lc_shortcut_tree_item.h"
#include "lc_pages_utils.h"
#include "lc_shortcuts_manager.h"

LC_SettingsPageShortcuts::LC_SettingsPageShortcuts(LC_ActionGroupManager* groupManager, QObject* parent)
    : LC_SettingsPageBase(tr("Keymap"), nullptr, parent), ui(std::make_unique<Ui::LC_SettingsPageShortcuts>()),
      m_actionGroupManager(groupManager) {
    setSortWeight(10);
}

LC_SettingsPageShortcuts::~LC_SettingsPageShortcuts() {
    delete m_mappingTreeModel;
}

void LC_SettingsPageShortcuts::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerShortcuts*>(manager);
    if (m_presetManager != nullptr && m_mappingTreeModel != nullptr) {
        m_presetManager->setTreeModel(m_mappingTreeModel);
        m_mappingTreeModel->setFilterForConflicts(false);

        ui->tvMappingsTree->clearSelection();
        ui->tvMappingsTree->setCurrentIndex(QModelIndex());
        m_selectedRow = -1;
        m_selectedParentRow = -1;
        selectItem(nullptr, -1, -1);

        rebuildTree(false);
        validateCollisions();
    }
}

void LC_SettingsPageShortcuts::loadSettings() {
    if (m_mappingTreeModel != nullptr) {
        m_mappingTreeModel->setFilterForConflicts(false);
    }
    if (m_presetManager != nullptr) {
        m_presetManager->loadPreset(m_presetManager->getActivePresetKey());
    }

    ui->tvMappingsTree->clearSelection();
    ui->tvMappingsTree->setCurrentIndex(QModelIndex());
    m_selectedRow = -1;
    m_selectedParentRow = -1;
    selectItem(nullptr, -1, -1);

    rebuildTree(false);
    validateCollisions();
}


void LC_SettingsPageShortcuts::setupUi() {
    ui->setupUi(m_widget);

    const QColor filteredCol = LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors::FilteredItem, m_widget->palette());
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
    trackSplitter(ui->splShortcuts, 480);

    connect(ui->bannerConflict, &LC_SettingsBannerWidget::linkActivated,
           this, &LC_SettingsPageShortcuts::onShowConflictsRequested);

    // Default to empty placeholder state
    ui->swDetails->setCurrentWidget(ui->pageEmpty);

    // Dynamic typography scaling based on active application style
    LC_PagesUtils::updateLabelFont(ui->lblActionName, 1);

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
        }
        else {
            const QPoint pos = ui->tbFindByShortcut->mapToGlobal(QPoint(0, ui->tbFindByShortcut->height() + 2));
            m_searchPopup->move(pos);
            m_searchPopup->show();
            m_searchPopup->setFocus();
        }
    });

    connect(m_searchPopup, &LC_ShortcutSearchPopup::shortcutFilterChanged, this, &LC_SettingsPageShortcuts::onShortcutFilterChanged);

    connect(ui->tvMappingsTree, &QTreeView::customContextMenuRequested, this, &LC_SettingsPageShortcuts::onTreeContextMenuRequested);

    connect(ui->tvMappingsTree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &LC_SettingsPageShortcuts::onTreeSelectionChanged);

    connect(ui->tvMappingsTree, &QTreeView::doubleClicked, this, [this](const QModelIndex&) {
          if (m_isReadOnly) {
              return;
          }
          ui->btnRecord->setChecked(true);
          ui->btnRecord->setFocus();
      });

    connect(ui->btnRecord, &LC_ShortcutButton::keySequenceChanged, this, &LC_SettingsPageShortcuts::onKeySequenceRecorded);
    connect(ui->btnRecord, &QPushButton::toggled, this, &LC_SettingsPageShortcuts::onRecordToggled);

    connect(ui->btnReset, &QPushButton::clicked, this, &LC_SettingsPageShortcuts::onResetItemClicked);
    connect(ui->pbClear, &QPushButton::clicked, this, &LC_SettingsPageShortcuts::onClearItemClicked);

    connect(ui->bannerConflict, &LC_SettingsBannerWidget::linkActivated,
           this, &LC_SettingsPageShortcuts::onShowConflictsRequested);

    connect(ui->tbExportCheatsheet, &QToolButton::clicked,
            this, &LC_SettingsPageShortcuts::onExportCheatsheetClicked);
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
    if (!validateCollisions()) {
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


void LC_SettingsPageShortcuts::onExportCheatsheetClicked() {
    LC_CheatsheetOptions options;
    options.type = CheatsheetType::Keymap;
    options.schemeName = (m_presetManager != nullptr)
        ? m_presetManager->currentPresetDisplayName()
        : tr("Default");

    options.isModelFiltered = (m_mappingTreeModel != nullptr) &&
        (m_mappingTreeModel->isFiltered() || m_mappingTreeModel->hasShortcutFilter());
    options.onlyFilteredItems = options.isModelFiltered;

    auto parent = getEditingWidget();
    LC_DlgCheatsheetOptions dlg(parent, options);
    if (dlg.exec() == QDialog::Accepted) {
        if (options.isModelFiltered && !options.onlyFilteredItems) {
            // User requested full catalog despite active UI filter: generate from unfiltered model
            LC_ShortcutsTreeModel fullModel;
            fullModel.rebuildModel(m_actionGroupManager);
            LC_CheatsheetGenerator::generate(parent, options, &fullModel);
        }
        else {
            LC_CheatsheetGenerator::generate(parent, options, m_mappingTreeModel);
        }
    }
}
void LC_SettingsPageShortcuts::autoIndexLabels() {
    LC_SettingsPageBase::autoIndexLabels();
    autoIndexActionGroupManager(m_actionGroupManager, ui->tvMappingsTree);
}

void LC_SettingsPageShortcuts::highlightSearchPattern(const QString& pattern) {
    ui->leFilter->setText(pattern);
}

void LC_SettingsPageShortcuts::clearSearchHighlight() {
    ui->leFilter->clear();
}

void LC_SettingsPageShortcuts::setReadOnly(bool readOnly) {
    m_isReadOnly = readOnly;

    // The tree view, search filter, and expansion controls remain fully interactive for browsing
    ui->paneTree->setEnabled(true);
    ui->tvMappingsTree->setEnabled(true);
    ui->leFilter->setEnabled(true);
    ui->cbMatchHighlight->setEnabled(true);
    ui->tbExpandAll->setEnabled(true);
    ui->tbCollapseAll->setEnabled(true);
    ui->tbFindByShortcut->setEnabled(true);

    // Detail headers and descriptions remain visible, but shortcut recorder is locked
    ui->gbActionHeader->setEnabled(true);
    ui->gbDescription->setEnabled(true);
    ui->gbShortcutEditor->setEnabled(!readOnly);
    ui->btnRecord->setEnabled(!readOnly);
    ui->btnReset->setEnabled(!readOnly);
    ui->pbClear->setEnabled(!readOnly);
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
        validateCollisions();
        return;
    }

    ui->swDetails->setCurrentWidget(ui->pageDetails);
    ui->lblActionName->setText(item->getName());
    ui->lblActionIcon->setPixmap(item->getIcon().pixmap(24, 24));
    ui->leKeySequence->setText(item->getShortcutViewString());

    // Display Category Name and Icon
    auto* parentItem = item->parent();
    if (parentItem != nullptr) {
        ui->lblGroupName->setText(parentItem->name());
        if (!parentItem->icon().isNull()) {
            ui->lblGroupIcon->setPixmap(parentItem->icon().pixmap(16, 16));
            ui->lblGroupIcon->setVisible(true);
        }
        else {
            ui->lblGroupIcon->setVisible(false);
        }
    }
    else {
        ui->lblGroupName->setText(tr("General"));
        ui->lblGroupIcon->setVisible(false);
    }

    const QString desc = item->getDescription();
    ui->lblDescription->setText(!desc.isEmpty() ? desc : tr("No description available for this action."));

    m_currentItem = item;
    m_selectedRow = row;
    m_selectedParentRow = parentRow;

    validateCollisions();

    if (m_isReadOnly) {
        ui->gbShortcutEditor->setEnabled(false);
        ui->btnRecord->setEnabled(false);
        ui->btnReset->setEnabled(false);
        ui->pbClear->setEnabled(false);
    }
}

void LC_SettingsPageShortcuts::onRecordToggled(bool recording) {
    if (!recording && m_currentItem != nullptr) {
        LC_ShortcutInfo* info = m_currentItem->getShortcutInfo();
        if (info != nullptr) {
            info->setKey(m_editingKeySequence);
            if (info->isModified()) {
                rebuildTree(true);
                validateCollisions();
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
        rebuildTree(true);
        validateCollisions();
        onControlChanged();
    }
}

void LC_SettingsPageShortcuts::onClearItemClicked() {
    if (m_currentItem != nullptr) {
        m_currentItem->clearShortcut();
        ui->leKeySequence->setText("");
        m_editingKeySequence = QKeySequence();
        rebuildTree(true);
        validateCollisions();
        onControlChanged();
    }
}
void LC_SettingsPageShortcuts::onClearConflictsFilterRequested() {
    if (m_mappingTreeModel != nullptr) {
        m_mappingTreeModel->setFilterForConflicts(false);
        rebuildTree(false);
        validateCollisions();
    }
}

bool LC_SettingsPageShortcuts::validateCollisions() {
    if (m_mappingTreeModel == nullptr) {
        return true;
    }

    QString conflictMsg;
    const bool hasCollisions = m_mappingTreeModel->checkForCollisions(nullptr, &conflictMsg);
    const bool isFilteringConflicts = m_mappingTreeModel->isFilterForConflicts();

    const QString message = hasCollisions
        ? tr("<b>Conflict Detected:</b> %1").arg(conflictMsg)
        : tr("All shortcuts are unique and valid.");

    const auto bannerType = hasCollisions
        ? LC_SettingsBannerWidget::BannerType::Error
        : LC_SettingsBannerWidget::BannerType::Info;

    QString actionText;
    std::function<void()> actionCallback;

    if (isFilteringConflicts) {
        actionText = tr("Show All Shortcuts");
        actionCallback = [this]() { onClearConflictsFilterRequested(); };
    }
    else if (hasCollisions) {
        actionText = tr("Filter Conflicts");
        actionCallback = [this]() { onShowConflictsRequested(); };
    }

    ui->bannerConflict->setMessage(message, bannerType, actionText, actionCallback);
    return !hasCollisions;
}

void LC_SettingsPageShortcuts::onShowConflictsRequested() {
    if (m_mappingTreeModel != nullptr) {
        m_mappingTreeModel->setFilterForConflicts(true);
        rebuildTree(false);
        validateCollisions();
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

    // Read-Only / Default Scheme: Only show tree navigation actions
    if (m_isReadOnly) {
        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvMappingsTree, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvMappingsTree, &QTreeView::collapseAll);
        menu.exec(ui->tvMappingsTree->viewport()->mapToGlobal(pos));
        return;
    }

    if (item != nullptr && !item->isGroup()) {
        // Editable mode: leaf action item
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
        // Editable mode: group item
        menu.addAction(tr("Reset All"), this, &LC_SettingsPageShortcuts::onResetAllClicked);
    }

    menu.addSeparator();
    menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvMappingsTree, &QTreeView::expandAll);
    menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvMappingsTree, &QTreeView::collapseAll);

    menu.exec(ui->tvMappingsTree->viewport()->mapToGlobal(pos));
}

void LC_SettingsPageShortcuts::onResetAllClicked() {
    loadSettings(); // Reloads active preset from disk/defaults and rebuilds tree
    onControlChanged();
}
