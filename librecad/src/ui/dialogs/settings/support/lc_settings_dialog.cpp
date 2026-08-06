/*******************************************************************************
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

#include "lc_settings_dialog.h"
#include <algorithm>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QSplitter>
#include <QToolButton>
#include <QVBoxLayout>
#include "lc_index_settings_page.h"
#include "lc_settings_page_base.h"
#include "ui_lc_settings_dialog.h"

#include "lc_palette_editor_shared.h"
#include "lc_preset_management_bar.h"
#include "rs_debug.h"
#include "rs_settings.h"

LC_SettingsDialog::LC_SettingsDialog(QWidget* parent, const QString& dialogId)
    : LC_Dialog(parent, "Settings." + dialogId), ui(std::make_unique<Ui::LC_SettingsDialog>()) {
    ui->setupUi(this);

    setWindowTitle(tr("Settings")); // fixme - pass as parameter

    ui->saSettingsScrollArea->setWidgetResizable(true);
    ui->splMain->setChildrenCollapsible(false);
    ui->splMain->setStretchFactor(0, 0);
    ui->splMain->setStretchFactor(1, 3);

    ui->tvCategoriesTree->setHeaderHidden(true);
    ui->tvCategoriesTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvCategoriesTree->setMinimumWidth(150);
    ui->tvCategoriesTree->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

    m_treeModel = new QStandardItemModel(this);
    m_filterModel = new LC_SettingsFilterModel(this, m_searchIndex);
    m_filterModel->setSourceModel(m_treeModel);
    ui->tvCategoriesTree->setModel(m_filterModel);

    int idealLeftWidth = ui->tvCategoriesTree->sizeHint().width();
    if (idealLeftWidth < 150) {
        idealLeftWidth = 150;
    }

    ui->splMain->setSizes(QList<int>() << idealLeftWidth << 10000);

    // Connections
    connect(ui->leSearch, &QLineEdit::textChanged, this, &LC_SettingsDialog::onSearchTextChanged);
    connect(ui->tvCategoriesTree, &QTreeView::activated, this, &LC_SettingsDialog::onCategorySelected);
    // connect(ui->tvCategoriesTree, &QTreeView::clicked, this, &LC_SettingsDialog::onCategorySelected);
    connect(ui->tvCategoriesTree->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &LC_SettingsDialog::onCategorySelected);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LC_SettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LC_Dialog::reject);

    connect(ui->btnBack, &QToolButton::clicked, this, &LC_SettingsDialog::onNavigateBack);
    connect(ui->btnForward, &QToolButton::clicked, this, &LC_SettingsDialog::onNavigateForward);
    connect(ui->presetBar, &LC_PresetManagementBar::presetSelected, this, &LC_SettingsDialog::onPresetSelected);
}

LC_SettingsDialog::~LC_SettingsDialog() = default;

void LC_SettingsDialog::registerPage(std::unique_ptr<LC_SettingsPageInterface> pageRef) {
    if (!pageRef) {
        return;
    }
    LC_SettingsPageInterface* page = pageRef.get();
    m_pages.push_back(std::move(pageRef));

    m_pageMap[page->id()] = page;

    if (const auto* basePage = dynamic_cast<LC_SettingsPageBase*>(page)) {
        connect(basePage, &LC_SettingsPageBase::navigateToPage, this, &LC_SettingsDialog::selectPage);

        connect(basePage, &LC_SettingsPageBase::livePreviewRequested, this, [this, page]() {
            LC_PresetManagerInterface* manager = getPresetManagerForPage(page->id());
            const bool isDirty = isPresetManagerScopeDirty(manager);
            ui->presetBar->setDirty(isDirty);

            // Notify the main application viewport to repaint in-memory [2.2]
            // RS_Settings::instance()->emitOptionsChanged();

            // Find and repaint the active preview widget (local or shared) in real-time [4.1, 4.2]
            if (m_activePage == page) {
                QWidget* preview = page->previewWidget();
                if (!preview) {
                    if (auto* manager = getPresetManagerForPage(page->id())) {
                        preview = manager->getSharedPreviewWidget();
                    }
                }
                if (preview) {
                    // fixme - use custom interface to force review
                    auto* livePreview = dynamic_cast<LC_LivePreview*>(preview);
                    if (livePreview != nullptr) {
                        livePreview->updateLivePreview();
                    }
                    else {
                        preview->update(); // Forces instant redraw on screen
                    }
                }
            }
        });
    }
    else if (const auto* indexPage = dynamic_cast<LC_IndexSettingsPage*>(page)) {
        connect(indexPage, &LC_IndexSettingsPage::navigateToPage, this, &LC_SettingsDialog::selectPage);
    }

    if (page->settingEditingWidget()) {
        ui->swContent->addWidget(page->settingEditingWidget());
    }
}

void LC_SettingsDialog::registerPresetManager(const QString& groupPathId, std::unique_ptr<LC_PresetManagerInterface> manager) {
    if (manager) {
        LC_PresetManagerInterface* rawPtr = manager.get();
        m_presetManagers[groupPathId] = std::move(manager);

        // Dynamic Callback Link: routes preset-defaults reset requests strictly to the pages in active scope
        rawPtr->setResetCallback([this, rawPtr]() {
            for (const auto& page : m_pages) {
                if (getPresetManagerForPage(page->id()) == rawPtr) {
                    page->resetDefaults();
                    page->saveSettings();
                }
            }
            RS_Settings::instance()->emitOptionsChanged();
        });

        // Symmetrical Save-Commit Link: commits all active page baselines on save
        rawPtr->setSaveCommitCallback([this, rawPtr]() {
            for (const auto& page : m_pages) {
                if (getPresetManagerForPage(page->id()) == rawPtr) {
                    page->saveSettings(); // Writes widget data to RS_Settings & resets baseline
                }
            }
            RS_Settings::instance()->emitOptionsChanged(); // Force redraw
        });
    }
}

QList<QPair<QString, QString>> LC_SettingsDialog::gatherChildLinks(const QString& parentPageId) const {
    QList<QPair<QString, QString>> links;

    // Symmetrical Child Collection: directly scans the vector using parentId matching
    for (const auto& page : m_pages) {
        if (page->parentId() == parentPageId) {
            links.append({page->id(), page->displayName()});
        }
    }
    return links;
}

LC_PresetManagerInterface* LC_SettingsDialog::getPresetManagerForPage(const QString& pageId) const {
    const auto itPage = m_pageMap.find(pageId);
    if (itPage == m_pageMap.end())
        return nullptr;

    const LC_SettingsPageInterface* page = itPage->second;
    const QString parentId = page->parentId();

    // Symmetrical Preset Matching: find longest matching parent ID path
    QString bestMatchGroup;
    for (const auto& [group, manager] : m_presetManagers) {
        if (pageId == group || pageId.startsWith(group + ".") || parentId == group || parentId.startsWith(group + ".")) {
            if (group.length() > bestMatchGroup.length()) {
                bestMatchGroup = group;
            }
        }
    }

    if (!bestMatchGroup.isEmpty()) {
        return m_presetManagers.at(bestMatchGroup).get();
    }
    return nullptr;
}

void LC_SettingsDialog::finalizeInitialization() {
    buildCategoryTree();

    for (const auto& [id, page] : m_pageMap) {
        auto pageIt = m_treeItemMap.find(id);
        if (pageIt != m_treeItemMap.end()) {
            QList<LC_SettingsPageInterface*> childPages;

            const auto pageItem = pageIt->second;
            const int count = pageItem->rowCount();
            for (int i=0; i< count;i++) {
                const auto childItem = pageItem->child(i, 0);
                auto childPageIdVariant = childItem->data(Qt::UserRole);
                if (childPageIdVariant.isValid()) {
                    auto childPageId = childPageIdVariant.toString();
                    auto childPageIt = m_pageMap.find(childPageId);
                    if (childPageIt != m_pageMap.end()) {
                        childPages.append(childPageIt->second);
                    }
                }
            }

            if (!childPages.isEmpty()) {
                page->setChildPages(childPages);
            }
        }
    }

    // Fully automate search caching indexing for all widgets on leaf pages
    for (const auto& [id, page] : m_pageMap) {
        page->autoIndexLabels();

        QStringList keywords;
        for (const auto& target : page->searchTargets()) {
            if (!target.originalText.isEmpty()) {
                keywords.append(target.originalText);
            }
        }
        m_searchIndex[page->id()] = keywords;
    }

    restoreTreeExpandedState();

    if (m_treeModel->rowCount() > 0) {
        const QModelIndex firstIdx = m_filterModel->index(0, 0);
        ui->tvCategoriesTree->setCurrentIndex(firstIdx);
        onCategorySelected(firstIdx);
    }
}

void LC_SettingsDialog::buildCategoryTree() {
    m_treeModel->clear();
    m_treeItemMap.clear();

    // 1. Copy raw page pointers and perform stable sort by priority weights
    std::vector<LC_SettingsPageInterface*> sortedPages;
    for (const auto& page : m_pages) {
        sortedPages.push_back(page.get());
    }
    std::stable_sort(sortedPages.begin(), sortedPages.end(), [](const LC_SettingsPageInterface* a, const LC_SettingsPageInterface* b) {
        return a->sortWeight() < b->sortWeight();
    });

    // 2. Symmetrical Adjacency List Tree Building:
    // Resolves parent-child hierarchies dynamically without hardcoded path parsing
    std::vector<LC_SettingsPageInterface*> pending = sortedPages;
    bool insertedAny = true;

    while (!pending.empty() && insertedAny) {
        LC_ERR << "Tree Building Pass +++";
        insertedAny = false;
        for (auto it = pending.begin(); it != pending.end();) {
            const LC_SettingsPageInterface* page = *it;
            QString parentId = page->parentId();

            auto pageId = page->id();
            LC_ERR << "Page: " << pageId;
            if (parentId.isEmpty()) {
                // Root Node Insertion
                auto* item = new QStandardItem(page->displayName());
                item->setData(pageId, Qt::UserRole);
                m_treeModel->appendRow(item);

                m_treeItemMap[pageId] = item;
                it = pending.erase(it);
                insertedAny = true;
            }
            else {
                // Child Node Insertion: verify parent has been created first
                auto parentIt = m_treeItemMap.find(parentId);
                if (parentIt != m_treeItemMap.end()) {
                    auto* parentItem = parentIt->second;
                    auto* item = new QStandardItem(page->displayName());
                    item->setData(pageId, Qt::UserRole);
                    parentItem->appendRow(item);

                    m_treeItemMap[pageId] = item;
                    it = pending.erase(it);
                    insertedAny = true;
                }
                else {
                    ++it; // Parent not in tree yet, defer to next pass
                }
            }
        }
    }
    Q_ASSERT_X(pending.empty(), "LC_SettingsDialog::buildCategoryTree()", "Orphaned not processed settings pages are found");
}

bool LC_SettingsDialog::selectPage(const QString& pageId) {
    const auto itPage = m_pageMap.find(pageId);
    if (itPage == m_pageMap.end()) {
        return false;
    }

    const LC_SettingsPageInterface* page = itPage->second;

    // Direct lookup of tree item by pageId
    const auto itItem = m_treeItemMap.find(page->id());
    if (itItem == m_treeItemMap.end()) {
        return false;
    }

    const QStandardItem* item = itItem->second;

    const QModelIndex sourceIndex = item->index();
    const QModelIndex proxyIndex = m_filterModel->mapFromSource(sourceIndex);

    if (proxyIndex.isValid()) {
        QModelIndex parent = proxyIndex.parent();
        while (parent.isValid()) {
            ui->tvCategoriesTree->expand(parent);
            parent = parent.parent();
        }
        ui->tvCategoriesTree->setCurrentIndex(proxyIndex);
        ui->tvCategoriesTree->scrollTo(proxyIndex);
        onCategorySelected(proxyIndex);
        return true;
    }
    return false;
}

void LC_SettingsDialog::onSearchTextChanged(const QString& text) {
    m_filterModel->setFilterText(text);
    if (!text.isEmpty()) {
        ui->tvCategoriesTree->expandAll(); // Show all deep matches dynamically
    }
    else {
        restoreTreeExpandedState(); // Revert back to clean root-only state
    }
    if (const auto* currWidget = ui->swContent->currentWidget()) {
        for (const auto& [id, page] : m_pageMap) {
            if (page->settingEditingWidget() == currWidget) {
                highlightPageContent(page, text);
                break;
            }
        }
    }
}

// context around LC_SettingsDialog::onCategorySelected & updateBreadcrumbs in lc_settings_dialog.cpp:
void LC_SettingsDialog::onCategorySelected(const QModelIndex& index) {
    if (!index.isValid())
        return;

    const QString pageId = index.data(Qt::UserRole).toString();
    const auto it = m_pageMap.find(pageId);
    if (it == m_pageMap.end())
        return;

    LC_SettingsPageInterface* page = it->second;

    LC_PresetManagerInterface* manager = getPresetManagerForPage(pageId);
    if (manager) {
        ui->presetBar->bindToManager(manager);
        const bool isDirty = isPresetManagerScopeDirty(manager);
        ui->presetBar->setDirty(isDirty);
    }
    else {
        ui->presetBar->bindToManager(nullptr);
    }


    auto* bottomLayout = ui->wContainerBottom->layout();
    const QWidget* currentBottom = (bottomLayout->count() > 0) ? bottomLayout->itemAt(0)->widget() : nullptr;
    QWidget* targetBottom = page->bottomWidget();
    // Three-tiered fallback: query page first, then active preset manager
    if (!targetBottom && manager) {
        targetBottom = manager->getSharedBottomWidget();
    }

    if (targetBottom != currentBottom) {
        if (bottomLayout->count() > 0) {
            const QLayoutItem* item = bottomLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->hide();
                bottomLayout->removeWidget(item->widget());
            }
            delete item;
        }

        if (targetBottom) {
            bottomLayout->addWidget(targetBottom);
            targetBottom->show();
            ui->wContainerBottom->setVisible(true);
        }
        else {
            ui->wContainerBottom->setVisible(false);
        }
    }

    // Three-tiered fallback: query page first, then active preset manager
    auto* previewLayout = ui->wContainerPreview->layout();
    QWidget* currentPreview = (previewLayout->count() > 0) ? previewLayout->itemAt(0)->widget() : nullptr;
    QWidget* targetPreview = page->previewWidget();
    if (!targetPreview && manager) {
        if (page->acceptsSharedPreview()) {
            targetPreview = manager->getSharedPreviewWidget();
        }
    }

    if (currentPreview != nullptr) {
        auto* livePreview = dynamic_cast<LC_LivePreview*>(currentPreview);
        if (livePreview != nullptr) {
            if (m_activePage != nullptr) {
                livePreview->cleanupPreviewForContentCategory(m_activePage->id());
            }
        }
    }

    if (targetPreview != currentPreview) {
        if (previewLayout->count() > 0) {
            const QLayoutItem* item = previewLayout->takeAt(0);
            if (item->widget()) {
                item->widget()->hide();
                previewLayout->removeWidget(item->widget());
            }
            delete item;
        }

        if (targetPreview) {
            previewLayout->addWidget(targetPreview);
            targetPreview->show();
            ui->wContainerPreview->setVisible(true);
            ui->splPreview->setSizes(QList<int>{500, 250}); // Symmetrical default ratio
        }
        else {
            ui->wContainerPreview->setVisible(false);
            ui->splPreview->setSizes(QList<int>{750, 0});
        }

    }

    if (targetPreview != nullptr) {
        auto* livePreview = dynamic_cast<LC_LivePreview*>(targetPreview);
        if (livePreview != nullptr) {
            livePreview->updatePreviewForContentCategory(pageId);
        }
    }


    if (m_activePage && m_activePage != page) {
        m_activePage->onAboutToHide();
    }
    m_activePage = page;

    if (!m_isNavigatingHistory) {
        bool isDuplicate = false;
        if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_history.size())) {
            isDuplicate = (m_history[m_historyIndex] == pageId);
        }

        if (!isDuplicate) {
            // Truncate any forward history if the user moved back and selected a new tab [2]
            if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
                m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
            }
            m_history.push_back(pageId);
            m_historyIndex = static_cast<int>(m_history.size()) - 1;
        }
        updateHistoryButtons();
    }

    if (page->settingEditingWidget()) {
        ui->swContent->setCurrentWidget(page->settingEditingWidget());

        // LC_ERR << "LC_SettingsDialog::onCategorySelected: Checking if page is initialized. ID:" << pageId
        // << " | Already initialized:" << m_initializedPages.contains(pageId);

        // Symmetrical Scroll Reset: Ensure the viewport starts unscrolled [4.2]
        if (ui->saSettingsScrollArea->verticalScrollBar()) {
            ui->saSettingsScrollArea->verticalScrollBar()->setValue(0);
        }
        if (ui->saSettingsScrollArea->horizontalScrollBar()) {
            ui->saSettingsScrollArea->horizontalScrollBar()->setValue(0);
        }

        if (!m_initializedPages.contains(pageId)) {
            m_initializedPages.insert(pageId);
            // LC_ERR << "LC_SettingsDialog::onCategorySelected: Executing lazy loadSettings() for ID:" << pageId;
            page->loadSettings();
        }

        page->onAboutToShow();

        updateBreadcrumbs(index);
        highlightPageContent(page, ui->leSearch->text());
    }
}

void LC_SettingsDialog::updateBreadcrumbs(const QModelIndex& index) const {
    QStringList segments;
    QModelIndex current = index;

    // Symmetrical Breadcrumb Generation: recursively walks up the tree model
    while (current.isValid()) {
        segments.prepend(current.data(Qt::DisplayRole).toString());
        current = current.parent();
    }
    ui->lblBreadcrumbs->setText(segments.join(" > "));
}

// context around LC_SettingsDialog::accept inside lc_settings_dialog.cpp:
void LC_SettingsDialog::accept() {


    for (const auto& [id, page] : m_pageMap) {
        if (m_initializedPages.contains(page->id()) && page->isModified()) {
            QString errMsg;
            if (!page->validate(errMsg)) {
                selectPage(page->id());
                QMessageBox::critical(this, tr("Invalid Input"), tr("Error in '%1':\n%2").arg(page->displayName(), errMsg));
                return;
            }
        }
    }

    bool restartNeeded = false;
    for (const auto& [id, page] : m_pageMap) {
        if (m_initializedPages.contains(page->id()) && page->isModified()) {
            if (page->saveSettings()) {
                if (page->requiresRestart()) {
                    restartNeeded = true;
                }
            }
        }
    }

    if (restartNeeded) {
        QMessageBox::warning(this, tr("Preferences"), tr("Please restart the application to apply all changes."));
        emit restartRequired();
    }

    LC_Dialog::accept();
}

void LC_SettingsDialog::highlightPageContent(LC_SettingsPageInterface* page, const QString& filterText) const {
    page->highlightSearchPattern(filterText);
    if (filterText.isEmpty()) {
        return;
    }

    for (const auto& target : page->searchTargets()) {
        if (target.targetWidget && target.originalText.contains(filterText, Qt::CaseInsensitive)) {
            ui->saSettingsScrollArea->ensureWidgetVisible(target.targetWidget);
            break;
        }
    }
}

void LC_SettingsDialog::onNavigateBack() {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        navigateToHistoryIndex(m_historyIndex);
    }
}

void LC_SettingsDialog::onNavigateForward() {
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_history.size()) - 1) {
        m_historyIndex++;
        navigateToHistoryIndex(m_historyIndex);
    }
}

void LC_SettingsDialog::navigateToHistoryIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_history.size())) {
        return;
    }

    const QString pageId = m_history[index];

    m_isNavigatingHistory = true;
    selectPage(pageId); // Programmatically focuses the tree node and active stacked widget
    m_isNavigatingHistory = false;

    updateHistoryButtons();
}

void LC_SettingsDialog::updateHistoryButtons() const {
    ui->btnBack->setEnabled(m_historyIndex > 0);
    ui->btnForward->setEnabled(m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_history.size()) - 1);
}

void LC_SettingsDialog::restoreTreeExpandedState() const {
    ui->tvCategoriesTree->collapseAll();

    // Iterate through Level 0 root items in the source model and expand them
    const int rootCount = m_treeModel->rowCount();
    for (int i = 0; i < rootCount; ++i) {
        QModelIndex sourceRootIdx = m_treeModel->index(i, 0);
        QModelIndex proxyRootIdx = m_filterModel->mapFromSource(sourceRootIdx);
        if (proxyRootIdx.isValid()) {
            ui->tvCategoriesTree->expand(proxyRootIdx);
        }
    }
}

namespace {
    class LC_SettingsDlgInnerPositions {
    public:
        LC_SettingsDlgInnerPositions(const LC_DialogPositionSettingsGroup* original) :
            o_CategoriesTreeWidth(original, "CategoriesTreeWidth", 150),
            o_PreviewWidth(original, "PreviewWidth", 150) {
        }

        LC_Setting<int> o_CategoriesTreeWidth;
        LC_Setting<int> o_PreviewWidth;
    };
}

void LC_SettingsDialog::saveInnerDialogPositions(LC_DialogPositionSettingsGroup& group) const {
    LC_SettingsDlgInnerPositions CFG_Positions(&group);
    QList<int> currentMainSplitterSizes = ui->splMain->sizes();
    if (!currentMainSplitterSizes.isEmpty()) {
        const int width = currentMainSplitterSizes[0];
        CFG_Positions.o_CategoriesTreeWidth = width;
    }

    const QList<int> currentPreviewSplitterSizes = ui->splPreview->sizes();
    if (!currentPreviewSplitterSizes.isEmpty()) {
        const int width = currentPreviewSplitterSizes[0];
        CFG_Positions.o_PreviewWidth = width;
    }
}

void LC_SettingsDialog::loadInnerDialogPositions(LC_DialogPositionSettingsGroup& group) {
    const LC_SettingsDlgInnerPositions CFG_Positions(&group);
    int treeViewWidth = CFG_Positions.o_CategoriesTreeWidth;
    if (treeViewWidth < 150) {
        treeViewWidth = 150;
    }
    ui->splMain->setSizes(QList<int>() << treeViewWidth << 10000);

    const int previewWidth = CFG_Positions.o_PreviewWidth;
    const int otherWidth = ui->splPreview->width() - previewWidth;
    ui->splPreview->setSizes(QList<int>() << previewWidth << otherWidth);
}

void LC_SettingsDialog::onPresetSelected(const QString& key) const {
    if (!m_activePage)
        return;

    LC_PresetManagerInterface* manager = getPresetManagerForPage(m_activePage->id());
    if (!manager) {
        return;
    }

    if (manager->loadPreset(key)) {
        m_activePage->loadSettings();
        m_activePage->updateLivePreview();
        ui->presetBar->setDirty(false);
    }
}

bool LC_SettingsDialog::isPresetManagerScopeDirty(LC_PresetManagerInterface* manager) const {
    if (!manager) {
        return false;
    }

    for (const auto& page : m_pages) {
        const auto& pageId = page->id();
        // Safe Guard: Ignore pages that have not been initialized by the user yet
        if (m_initializedPages.contains(pageId)) {
            if (getPresetManagerForPage(pageId) == manager) {
                if (page->isModified()) {
                    return true;
                }
            }
        }
    }
    return false;
}
