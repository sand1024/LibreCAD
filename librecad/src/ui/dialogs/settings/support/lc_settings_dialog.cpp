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
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>
#include <QSplitter>
#include <QToolButton>
#include <QVBoxLayout>

#include "lc_abstract_preset_manager.h"
#include "lc_settings_page_index.h"
#include "lc_settings_page_base.h"
#include "ui_lc_settings_dialog.h"

#include "lc_palette_editor_shared.h"
#include "lc_preset_management_bar.h"

LC_SettingsDialog::LC_SettingsDialog(QWidget* parent, const QString& dialogId)
    : LC_Dialog(parent, "Settings." + dialogId), ui(std::make_unique<Ui::LC_SettingsDialog>()), m_dialogId(dialogId) {
    ui->setupUi(this);

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

    auto* findShortcut = new QShortcut(QKeySequence(QKeySequence::Find), this);
    findShortcut->setContext(Qt::WindowShortcut);

    connect(findShortcut, &QShortcut::activated, this, [this]() {
        ui->leSearch->setFocus(Qt::ShortcutFocusReason);
        ui->leSearch->selectAll(); // Select all text so the user can type over a previous query instantly
    });

    //  Generate the native string representation of the shortcut (e.g. "Ctrl+F" or "⌘F")
    const QString nativeShortcut = QKeySequence(QKeySequence::Find).toString(QKeySequence::NativeText);

    // Set the localized tooltip dynamically
    ui->leSearch->setToolTip(tr("Search settings... (%1)").arg(nativeShortcut));

    connect(ui->leSearch, &QLineEdit::textChanged, this, &LC_SettingsDialog::onSearchTextChanged);
    connect(ui->leSearch, &QLineEdit::returnPressed, this, &LC_SettingsDialog::onSearchReturnPressed);
    connect(ui->tvCategoriesTree, &QTreeView::activated, this, &LC_SettingsDialog::onCategorySelected);
    // connect(ui->tvCategoriesTree, &QTreeView::clicked, this, &LC_SettingsDialog::onCategorySelected);
    connect(ui->tvCategoriesTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &LC_SettingsDialog::onCategorySelected);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LC_SettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LC_Dialog::reject);

    connect(ui->btnBack, &QToolButton::clicked, this, &LC_SettingsDialog::onNavigateBack);
    connect(ui->btnForward, &QToolButton::clicked, this, &LC_SettingsDialog::onNavigateForward);
    connect(ui->presetBar, &LC_PresetManagementBar::presetSelected, this, &LC_SettingsDialog::onPresetSelected);
    connect(ui->presetBar, &LC_PresetManagementBar::dirtyStateChanged, this, [this](bool) {
        updateGatingState();
    });
}

LC_SettingsDialog::~LC_SettingsDialog() = default;

void LC_SettingsDialog::doUpdatePageLivePreview(LC_SettingsPageInterface* page) const {
    QWidget* preview = page->getPreviewWidget();
    if (preview == nullptr) {
        if (auto* manager = getPresetManagerForPage(page->id())) {
            preview = manager->getSharedPreviewWidget();
        }
    }
    if (preview != nullptr) {
        auto* livePreview = dynamic_cast<LC_LivePreview*>(preview);
        if (livePreview != nullptr) {
            livePreview->updateLivePreview();
        }
        else {
            preview->update(); // Forces instant redraw on screen
        }
    }
}

void LC_SettingsDialog::showEvent(QShowEvent* event) {
    LC_Dialog::showEvent(event);
    if (m_activePage != nullptr) {
        m_activePage->onDialogShown();
    }
}

void LC_SettingsDialog::hideEvent(QHideEvent* event) {
    if (m_activePage != nullptr) {
        m_activePage->onAboutToHide(); // Clean up if needed
    }
    LC_Dialog::hideEvent(event);
}

void LC_SettingsDialog::resizeEvent(QResizeEvent* event) {
    LC_Dialog::resizeEvent(event);
    if (m_activePage != nullptr) {
        m_activePage->onPageResized();
    }
}

void LC_SettingsDialog::registerPage(std::unique_ptr<LC_SettingsPageInterface> pageRef) {
    if (!pageRef) {
        return;
    }
    LC_SettingsPageInterface* page = pageRef.get();
    m_pages.push_back(std::move(pageRef));

    const auto pageId = page->id();
    m_pageMapByPageId[pageId] = page;

    if (const auto* basePage = dynamic_cast<LC_SettingsPageBase*>(page)) {
        connect(basePage, &LC_SettingsPageBase::navigateToPage, this, &LC_SettingsDialog::selectPage);

        connect(basePage, &LC_SettingsPageBase::livePreviewRequested, this, [this, page]() {
            LC_PresetManagerInterface* manager = getPresetManagerForPage(page->id());
            if (manager != nullptr) {
                const bool isDirty = isPresetManagerScopeDirty(manager);
                ui->presetBar->setDirty(isDirty);

                if (isDirty) {
                    if (auto* abstractMgr = dynamic_cast<LC_AbstractPresetManager*>(manager)) {
                        abstractMgr->notifyWorkingConfigChanged();
                    }
                }
            }

            if (m_activePage == page) {
                doUpdatePageLivePreview(page);
            }

            updateGatingState();
            emit livePreviewRequested(page->id());
        });
    }
    else if (const auto* indexPage = dynamic_cast<LC_SettingsPageIndex*>(page)) {
        connect(indexPage, &LC_SettingsPageIndex::navigateToPage, this, &LC_SettingsDialog::selectPage);
    }

    if (page->getEditingWidget()) {
        ui->swContent->addWidget(page->getEditingWidget());
    }
}

void LC_SettingsDialog::registerPresetManager(const QString& groupPathId, std::unique_ptr<LC_PresetManagerInterface> manager) {
    if (manager != nullptr) {
        LC_PresetManagerInterface* rawPtr = manager.get();
        m_presetManagers[groupPathId] = std::move(manager);

        // Dynamic Callback Link: routes preset-defaults reset requests strictly to the pages in active scope
        rawPtr->setResetCallback([this, rawPtr]() {
            for (const QString& pageId : m_initializedPages) {
                if (getPresetManagerForPage(pageId) == rawPtr) {
                    m_pageMapByPageId.at(pageId)->loadSettings();
                }
            }
            doUpdatePageLivePreview(m_activePage);
            RS_Settings::instance()->emitOptionsChanged();
        });

        // commits all active page baselines on save
        rawPtr->setSaveCommitCallback([this, rawPtr]() {
            for (const auto& page : m_pages) {
                if (getPresetManagerForPage(page->id()) == rawPtr) {
                    if (m_initializedPages.contains(page->id())) {
                        page->saveSettings();
                    }
                }
            }
            RS_Settings::instance()->emitOptionsChanged(); // Force redraw
        });
    }
}

QList<QPair<QString, QString>> LC_SettingsDialog::gatherChildLinks(const QString& parentPageId) const {
    QList<QPair<QString, QString>> links;

    // Child Collection: directly scans the vector using parentId matching
    for (const auto& page : m_pages) {
        if (page->parentId() == parentPageId) {
            links.append({page->id(), page->displayName()});
        }
    }
    return links;
}

LC_PresetManagerInterface* LC_SettingsDialog::getPresetManagerForPage(const QString& pageId) const {
    const auto itPage = m_pageMapByPageId.find(pageId);
    if (itPage == m_pageMapByPageId.end()) {
        return nullptr;
    }

    const LC_SettingsPageInterface* page = itPage->second;
    const QString parentId = page->parentId();

    // Preset Matching: find longest matching parent ID path
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

LC_PresetManagerInterface* LC_SettingsDialog::getPresetManager(const QString& groupPathId) const {
    const auto it = m_presetManagers.find(groupPathId);
    if (it != m_presetManagers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void LC_SettingsDialog::finalizeInitialization() {
    buildCategoryTree();

    for (const auto& [id, page] : m_pageMapByPageId) {
        if (auto* manager = getPresetManagerForPage(id)) {
            page->bindToPresetManager(manager);
        }
    }

    for (const auto& [id, page] : m_pageMapByPageId) {
        auto pageIt = m_treeItemMapByPageId.find(id);
        if (pageIt != m_treeItemMapByPageId.end()) {
            QList<LC_SettingsPageInterface*> childPages;

            const auto pageItem = pageIt->second;
            const int count = pageItem->rowCount();
            for (int i = 0; i < count; i++) {
                const auto childItem = pageItem->child(i, 0);
                auto childPageIdVariant = childItem->data(Qt::UserRole);
                if (childPageIdVariant.isValid()) {
                    auto childPageId = childPageIdVariant.toString();
                    auto childPageIt = m_pageMapByPageId.find(childPageId);
                    if (childPageIt != m_pageMapByPageId.end()) {
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
    for (const auto& [id, page] : m_pageMapByPageId) {
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
    m_treeItemMapByPageId.clear();

    // Copy raw page pointers and perform stable sort by priority weights
    std::vector<LC_SettingsPageInterface*> sortedPages;
    for (const auto& page : m_pages) {
        sortedPages.push_back(page.get());
    }
    std::stable_sort(sortedPages.begin(), sortedPages.end(), [](const LC_SettingsPageInterface* a, const LC_SettingsPageInterface* b) {
        return a->sortWeight() < b->sortWeight();
    });

    //  Adjacency List Tree Building:
    // Resolves parent-child hierarchies dynamically without hardcoded path parsing
    std::vector<LC_SettingsPageInterface*> pending = sortedPages;
    bool insertedAny = true;

    while (!pending.empty() && insertedAny) {
        // LC_ERR << "Tree Building Pass +++";
        insertedAny = false;
        for (auto it = pending.begin(); it != pending.end();) {
            const LC_SettingsPageInterface* page = *it;
            QString parentId = page->parentId();

            auto pageId = page->id();
            // LC_ERR << "Page: " << pageId;
            if (parentId.isEmpty()) {
                // Root Node Insertion
                auto* item = new QStandardItem(page->displayName());
                item->setData(pageId, Qt::UserRole);
                m_treeModel->appendRow(item);

                m_treeItemMapByPageId[pageId] = item;
                it = pending.erase(it);
                insertedAny = true;
            }
            else {
                // Child Node Insertion: verify parent has been created first
                auto parentIt = m_treeItemMapByPageId.find(parentId);
                if (parentIt != m_treeItemMapByPageId.end()) {
                    auto* parentItem = parentIt->second;
                    auto* item = new QStandardItem(page->displayName());
                    item->setData(pageId, Qt::UserRole);
                    parentItem->appendRow(item);

                    m_treeItemMapByPageId[pageId] = item;
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
    const auto itPage = m_pageMapByPageId.find(pageId);
    if (itPage == m_pageMapByPageId.end()) {
        return false;
    }

    const LC_SettingsPageInterface* page = itPage->second;

    // Direct lookup of tree item by pageId
    const auto itItem = m_treeItemMapByPageId.find(page->id());
    if (itItem == m_treeItemMapByPageId.end()) {
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

        // If the filter returned 0 visible rows, tell the SearchLineEdit
        // to highlight its background in warning red.
        const bool hasMatches = m_filterModel->rowCount() > 0;
        ui->leSearch->setErrorState(!hasMatches);
    }
    else {
        restoreTreeExpandedState(); // Revert back to clean root-only state
        ui->leSearch->setErrorState(false); // Reset background
    }
    if (const auto* currWidget = ui->swContent->currentWidget()) {
        for (const auto& [id, page] : m_pageMapByPageId) {
            if (page->getEditingWidget() == currWidget) {
                highlightPageContent(page, text);
                break;
            }
        }
    }
}

void LC_SettingsDialog::onSearchReturnPressed() {
    if (ui->leSearch->text().trimmed().isEmpty() || ui->leSearch->isErrorState()) {
        return;
    }

    const QModelIndex firstIdx = findFirstVisibleIndex();
    if (firstIdx.isValid()) {
        const QString pageId = firstIdx.data(Qt::UserRole).toString();
        selectPage(pageId); // Switches page and focuses the tree node
    }
}

QModelIndex LC_SettingsDialog::findFirstVisibleIndex(const QModelIndex& parent) const {
    if (!m_filterModel) {
        return QModelIndex();
    }
    // If this node has no visible children in the filtered view, it is the deepest leaf match!
    // NOTE:  Using leaf pages will work for most of searches, yet if page mix own settings and children - like Grid in App Settings
    // that logic may lead to error and miss first parent page - but it's ok, the user may switch the page manually to find
    // more precisely
    if (m_filterModel->rowCount(parent) == 0) {
        return parent;
    }

    // Recurse down to the first visible child
    return findFirstVisibleIndex(m_filterModel->index(0, 0, parent));
}

void LC_SettingsDialog::onCategorySelected(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    const QString pageId = index.data(Qt::UserRole).toString();
    const auto it = m_pageMapByPageId.find(pageId);
    if (it == m_pageMapByPageId.end()) {
        return;
    }

    LC_SettingsPageInterface* page = it->second;

    LC_PresetManagerInterface* manager = getPresetManagerForPage(pageId);

    // 1. Capture previous page and trigger onAboutToHide()
    LC_SettingsPageInterface* previousPage = m_activePage;
    if (previousPage != nullptr && previousPage != page) {
        previousPage->onAboutToHide();
    }
    m_activePage = page;

    // 2. Preset Management Bar Binding & Scope Dirty State
    if (manager != nullptr) {
        ui->presetBar->bindToManager(manager);
        const bool isDirty = isPresetManagerScopeDirty(manager);
        ui->presetBar->setDirty(isDirty);
    }
    else {
        ui->presetBar->bindToManager(nullptr);
    }

    // 2. Dynamic Shared Header Mounting (e.g. Workspace Density bar)
    auto* headerLayout = ui->wContainerPresetHeader->layout();
    const QWidget* currentHeader = (headerLayout->count() > 0) ? headerLayout->itemAt(0)->widget() : nullptr;
    QWidget* targetHeader = (manager != nullptr) ? manager->getSharedHeaderWidget() : nullptr;

    if (targetHeader != currentHeader) {
        if (headerLayout->count() > 0) {
            const QLayoutItem* item = headerLayout->takeAt(0);
            if (item->widget() != nullptr) {
                item->widget()->hide();
                headerLayout->removeWidget(item->widget());
            }
            delete item;
        }

        if (targetHeader != nullptr) {
            headerLayout->addWidget(targetHeader);
            targetHeader->show();
            ui->wContainerPresetHeader->setVisible(true);
        }
        else {
            ui->wContainerPresetHeader->setVisible(false);
        }
    }

    // 3. Dynamic Shared Bottom Mounting (e.g. Preview Controls Bar)
    auto* bottomLayout = ui->wContainerBottom->layout();
    const QWidget* currentBottom = (bottomLayout->count() > 0) ? bottomLayout->itemAt(0)->widget() : nullptr;
    QWidget* targetBottom = page->getBottomWidget();
    if (targetBottom == nullptr && manager != nullptr) {
        // Re-evaluates supportsPreviewWindow() and supportsAccessibilityCheck() for the active manager
        targetBottom = manager->getSharedBottomWidget();
    } else if (targetBottom == nullptr && page->acceptsSharedPreview() && currentBottom != nullptr) {
        targetBottom = const_cast<QWidget*>(currentBottom);
    }

    if (targetBottom != currentBottom) {
        if (bottomLayout->count() > 0) {
            const QLayoutItem* item = bottomLayout->takeAt(0);
            if (item->widget() != nullptr) {
                item->widget()->hide();
                bottomLayout->removeWidget(item->widget());
            }
            delete item;
        }

        if (targetBottom != nullptr) {
            bottomLayout->addWidget(targetBottom);
            targetBottom->show();
            ui->wContainerBottom->setVisible(true);
        }
        else {
            ui->wContainerBottom->setVisible(false);
        }
    } else if (targetBottom != nullptr) {
        // Force refresh visibility of sub-controls even if container pointer is reused
        targetBottom->show();
        ui->wContainerBottom->setVisible(true);
    } else {
        ui->wContainerBottom->setVisible(false);
    }

    // 5. Embedded Preview Layout Handling & Symmetrical Category Cleanup
    auto* previewLayout = ui->wContainerPreview->layout();
    QWidget* currentPreview = (previewLayout->count() > 0) ? previewLayout->itemAt(0)->widget() : nullptr;
    QWidget* targetPreview = page->getPreviewWidget();
    if (!targetPreview && manager != nullptr) {
        if (page->acceptsSharedPreview()) {
            targetPreview = manager->getSharedPreviewWidget();
        }
    }

    if (currentPreview != nullptr) {
        auto* livePreview = dynamic_cast<LC_LivePreview*>(currentPreview);
        if (livePreview != nullptr && previousPage != nullptr && previousPage != page) {
            // Clean up using the PREVIOUS category ID being left
            livePreview->cleanupPreviewForContentCategory(previousPage->id());
            }
        }

    if (targetPreview != currentPreview) {
        if (previewLayout->count() > 0) {
            const QLayoutItem* item = previewLayout->takeAt(0);
            if (item->widget() != nullptr) {
                item->widget()->hide();
                previewLayout->removeWidget(item->widget());
            }
            delete item;
        }

        if (targetPreview != nullptr) {
            previewLayout->addWidget(targetPreview);
            targetPreview->show();
            ui->wContainerPreview->setVisible(true);
            ui->splPreview->setSizes(QList<int>{500, 250});
        }
        else {
            ui->wContainerPreview->setVisible(false);
            ui->splPreview->setSizes(QList<int>{750, 0});
        }
    }

    if (targetPreview != nullptr) {
        auto* livePreview = dynamic_cast<LC_LivePreview*>(targetPreview);
        if (livePreview != nullptr) {
            // Update using the NEW category ID being activated
            livePreview->updatePreviewForContentCategory(pageId);
        }
    }

    // 6. Centralized Gating Resolution Pass
    updateGatingState();

    // 7. History Tracking
    if (!m_isNavigatingHistory) {
        bool isDuplicate = false;
        if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_history.size())) {
            isDuplicate = (m_history[m_historyIndex] == pageId);
        }

        if (!isDuplicate) {
            if (m_historyIndex < static_cast<int>(m_history.size()) - 1) {
                m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
            }
            m_history.push_back(pageId);
            m_historyIndex = static_cast<int>(m_history.size()) - 1;
        }
        updateHistoryButtons();
    }

    // 8. Stacked Widget Switch, Scroll Reset & Lazy Loading
    if (page->getEditingWidget() != nullptr) {
        ui->swContent->setCurrentWidget(page->getEditingWidget());

        // LC_ERR << "LC_SettingsDialog::onCategorySelected: Checking if page is initialized. ID:" << pageId
        // << " | Already initialized:" << m_initializedPages.contains(pageId);

        // Scroll Reset: Ensure the viewport starts unscrolled
        if (ui->saSettingsScrollArea->verticalScrollBar() != nullptr) {
            ui->saSettingsScrollArea->verticalScrollBar()->setValue(0);
        }
        if (ui->saSettingsScrollArea->horizontalScrollBar() != nullptr) {
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

    // 9. Search History Commit on Successful Navigation
    if (!ui->leSearch->text().trimmed().isEmpty() && !ui->leSearch->isErrorState()) {
        ui->leSearch->addCurrentTextToHistory();
    }

    emit categoryChanged(pageId);
}

void LC_SettingsDialog::updateGatingState() {
    if (m_activePage == nullptr) {
        ui->bannerWidget->setVisible(false);
        ui->bannerWidget->clearAction();
        return;
    }

    LC_PresetManagerInterface* manager = getPresetManagerForPage(m_activePage->id());

    // Hard Lockout States
    const bool isManagerGated = (manager != nullptr && manager->isGated());
    const bool isPageGated    = m_activePage->isPageGated();
    const bool isReadOnly = (manager != nullptr && manager->isReadOnlyDefault());
    const bool isSystemGated  = (isManagerGated && !isReadOnly) || isPageGated;

    // Resolve Message & Action
    QString gatedMsg;
    QString actionTxt;
    std::function<void()> actionCb;

    if (manager != nullptr && !manager->gatedMessage().isEmpty()) {
        gatedMsg  = manager->gatedMessage();
        actionTxt = manager->gatedActionText();
        actionCb  = manager->gatedActionCallback();
    } else if (!m_activePage->gatedMessage().isEmpty()) {
        gatedMsg  = m_activePage->gatedMessage();
        actionTxt = m_activePage->gatedActionText();
        actionCb  = m_activePage->gatedActionCallback();
    }

    // 1. Banner Display: Visible whenever any message is provided
    if (!gatedMsg.isEmpty()) {
        auto wrappedActionCb = [this, actionCb, manager]() {
            if (actionCb != nullptr) {
                actionCb();
            }
            else if (manager != nullptr && manager->isReadOnlyDefault()) {
                // Reuses the identical 'Save As' logic from the preset bar
                if (ui->presetBar->savePresetAs()) {
                    if (m_activePage != nullptr) {
                        m_activePage->loadSettings();
                    }
                }
            }
            updateGatingState();
        };

        ui->bannerWidget->setBanner(gatedMsg, actionTxt, wrappedActionCb);
        ui->bannerWidget->setVisible(true);
    }
    else {
        ui->bannerWidget->setVisible(false);
        ui->bannerWidget->clearAction();
    }

    // 2. Control Gating: Only lock editing controls on hard system gate or read-only template
    auto* editingWidget = m_activePage->getEditingWidget();
    if (editingWidget != nullptr) {
        if (dynamic_cast<LC_SettingsPageIndex*>(m_activePage) != nullptr) {
            editingWidget->setEnabled(true);
        }
        else if (isSystemGated) {
            editingWidget->setEnabled(false);
        }
        else {
            editingWidget->setEnabled(true);
            m_activePage->setReadOnly(isReadOnly);
        }
    }

    // 3. Preset Bar & Header Containers: Keep interactive in Classic Fusion
    ui->presetBar->setEnabled(!isSystemGated);
    ui->wContainerPresetHeader->setEnabled(!isSystemGated && !isReadOnly);
    ui->wContainerBottom->setEnabled(!isSystemGated);
}

void LC_SettingsDialog::updateBreadcrumbs(const QModelIndex& index) const {
    QStringList segments;
    QModelIndex current = index;

    // Breadcrumb Generation: recursively walks up the tree model
    while (current.isValid()) {
        segments.prepend(current.data(Qt::DisplayRole).toString());
        current = current.parent();
    }
    ui->lblBreadcrumbs->setText(segments.join(" > "));
}

void LC_SettingsDialog::accept() {
    if (auto* focusWidget = QApplication::focusWidget()) {
        focusWidget->clearFocus();
    }

    // Capture the exact set of modified pages upfront.
    // This protects us from any state mutation side-effects during validation or saving.
    std::vector<std::pair<QString, LC_SettingsPageInterface*>> modifiedPages;

    for (const auto& [id, page] : m_pageMapByPageId) {
        if (isPageInitialized(id)) {
            const bool pageModified = page->isModified();
            // LC_ERR << "Accept checking initialized page: " << id  << " | isModified: " << pageModified;
            if (pageModified) {
                modifiedPages.push_back({id, page});
            }
        }
    }

    // VALIDATION PASS: Validate only the pre-cached modified pages.
    for (const auto& [pageId, page] : modifiedPages) {
        QString errMsg;
        if (!page->validate(errMsg)) {
            selectPage(pageId); // Focus on the failing tab
            QMessageBox::critical(this, tr("Invalid Input"), tr("Error in '%1':\n%2").arg(page->displayName(), errMsg));
            return; // Abort transaction entirely on validation failure
        }
    }

    // PRESET MANAGERS ACCEPT PASS: Delegate acceptance checks to active preset managers
    QSet<LC_PresetManagerInterface*> visitedManagers;
    for (const auto& [id, page] : m_pageMapByPageId) {
        if (isPageInitialized(id)) {
            if (auto* manager = getPresetManagerForPage(id)) {
                if (!visitedManagers.contains(manager)) {
                    visitedManagers.insert(manager);

                    // Synchronize multi-page scope state before accept prompt
                    if (isPresetManagerScopeDirty(manager)) {
                        if (auto* abstractMgr = dynamic_cast<LC_AbstractPresetManager*>(manager)) {
                            abstractMgr->notifyWorkingConfigChanged();
                        }
                    }

                    if (!manager->onDialogAccept(this)) {
                        return; // Abort dialog accept if a manager rejected/cancelled
                    }
                }
            }
        }
    }

    // COMMIT PASS: Save only the validated, cached pages.
    // This is called exactly once when the user commits via "OK".
    bool restartNeeded = false;
    for (const auto& [pageId, page] : modifiedPages) {
        if (page->saveSettings()) {
            if (page->requiresRestart()) {
                restartNeeded = true;
            }
        }
    }

    if (restartNeeded) {
        QMessageBox::warning(this, tr("Preferences"), tr("Please restart the application to apply all changes."));
        emit restartRequired();
    }

    LC_Dialog::accept();
}

void LC_SettingsDialog::reject() {
    // 1. Query active preset managers if cancellation should be confirmed
    QSet<LC_PresetManagerInterface*> visitedManagers;
    for (const auto& [id, page] : m_pageMapByPageId) {
        if (isPageInitialized(id)) {
            if (auto* manager = getPresetManagerForPage(id)) {
                if (!visitedManagers.contains(manager)) {
                    visitedManagers.insert(manager);
                    if (!manager->onDialogReject(this)) {
                        return; // User chose NOT to discard; abort rejection and keep dialog open
                    }
                }
            }
        }
    }

    // 2. Rollback transient states on all preset managers
    for (const auto& [key, mgr] : m_presetManagers) {
        if (mgr != nullptr) {
            mgr->rollbackState();
        }
    }

    LC_Dialog::reject();
}


void LC_SettingsDialog::closeEvent(QCloseEvent* event) {
    // Symmetrically run the same rejection check on window top-right "X" closure
    QSet<LC_PresetManagerInterface*> visitedManagers;
    for (const auto& [id, page] : m_pageMapByPageId) {
        if (isPageInitialized(id)) {
            if (auto* manager = getPresetManagerForPage(id)) {
                if (!visitedManagers.contains(manager)) {
                    visitedManagers.insert(manager);
                    if (!manager->onDialogReject(this)) {
                        event->ignore();
                        return;
                    }
                }
            }
        }
    }

    for (const auto& [key, mgr] : m_presetManagers) {
        if (mgr != nullptr) {
            mgr->rollbackState();
        }
    }

    event->accept();
    LC_Dialog::reject();
}

void LC_SettingsDialog::highlightPageContent(LC_SettingsPageInterface* page, const QString& filterText) const {
    page->highlightSearchPattern(filterText);
    if (filterText.isEmpty()) {
        return;
    }

    for (const auto& target : page->searchTargets()) {
        if (target.targetWidget != nullptr && target.originalText.contains(filterText, Qt::CaseInsensitive)) {
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

    if (m_expandAllCategories) {
        ui->tvCategoriesTree->expandAll();
        return;
    }

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
    class LC_SettingsDlgInnerData {
    public:
        LC_SettingsDlgInnerData(const LC_SettingsGroupDialog* original)
            : o_CategoriesTreeWidth(original, "CategoriesTreeWidth", 150),
              o_PreviewWidth(original, "PreviewWidth", 150),
              o_SearchHistory(original, "SearchHistory", "") {
        }

        LC_Setting<int> o_CategoriesTreeWidth;
        LC_Setting<int> o_PreviewWidth;
        LC_Setting<QString> o_SearchHistory;
    };
}

void LC_SettingsDialog::saveInnerDialogData(LC_SettingsGroupDialog& group, bool savePositions) const {
    LC_SettingsDlgInnerData CFG_InnerData(&group);
    if (savePositions) {
        QList<int> currentMainSplitterSizes = ui->splMain->sizes();
        if (!currentMainSplitterSizes.isEmpty()) {
            const int width = currentMainSplitterSizes[0];
            CFG_InnerData.o_CategoriesTreeWidth = width;
        }

        const QList<int> currentPreviewSplitterSizes = ui->splPreview->sizes();
        if (!currentPreviewSplitterSizes.isEmpty()) {
            const int width = currentPreviewSplitterSizes[0];
            CFG_InnerData.o_PreviewWidth = width;
        }
    }
    const QString searchHistory = ui->leSearch->history().join(";");
    CFG_InnerData.o_SearchHistory = searchHistory;
}

void LC_SettingsDialog::loadInnerDialogData(LC_SettingsGroupDialog& group, bool loadPosition) {
    const LC_SettingsDlgInnerData CFG_InnerData(&group);
    if (loadPosition) {
        int treeViewWidth = CFG_InnerData.o_CategoriesTreeWidth;
        if (treeViewWidth < 150) {
            treeViewWidth = 150;
        }
        ui->splMain->setSizes(QList<int>() << treeViewWidth << 10000);

        const int previewWidth = CFG_InnerData.o_PreviewWidth;
        const int otherWidth = ui->splPreview->width() - previewWidth;
        ui->splPreview->setSizes(QList<int>() << previewWidth << otherWidth);
    }
    const QString searchHistory = CFG_InnerData.o_SearchHistory;
    if (!searchHistory.isEmpty()) {
        const QStringList historyItems = searchHistory.split(";");
        ui->leSearch->setHistory(historyItems);
    }
}

void LC_SettingsDialog::onPresetSelected(const QString& key){
    if (m_activePage == nullptr) {
        return;
    }

    LC_PresetManagerInterface* manager = getPresetManagerForPage(m_activePage->id());
    if (manager == nullptr) {
        return;
    }
    if (manager->getActivePresetKey() == key) {
        return;
    }

    const bool hasUnsavedChanges = isPresetManagerScopeDirty(manager) || manager->isPresetModified();

    if (hasUnsavedChanges) {
        const auto result = QMessageBox::question(this, tr("Unsaved Changes"),
                                                  tr("There are unsaved modifications. "
                                                     "Switching will discard these changes.\n\n"
                                                      "Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::No) {
            ui->presetBar->setCurrentPresetKey(manager->getActivePresetKey());
            return;
        }
    }

    if (manager->loadPreset(key)) {
        // Widgets in all initialized pages are now "stale".
        // We must push the updated cache values into the widgets.
        // If we don't, clicking "Save As" will take the old widget values
        // and overwrite the theme we just loaded.
        for (const QString& pageId : m_initializedPages) {
            auto it = m_pageMapByPageId.find(pageId);
            if (it != m_pageMapByPageId.end()) {
                it->second->loadSettings();
            }
        }
        doUpdatePageLivePreview(m_activePage);

        ui->presetBar->setDirty(false);

        // Re-evaluate gating so switching between Default and Custom instantly updates the banner and controls
        updateGatingState();
    }
}

bool LC_SettingsDialog::isPresetManagerScopeDirty(LC_PresetManagerInterface* manager) const {
    if (manager == nullptr) {
        return false;
    }

    for (const auto& page : m_pages) {
        const auto& pageId = page->id();
        // Safe Guard: Ignore pages that have not been initialized by the user yet
        if (isPageInitialized(pageId)) {
            if (getPresetManagerForPage(pageId) == manager) {
                if (page->isModified()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void LC_SettingsDialog::forEachPresetManager(const std::function<void(LC_PresetManagerInterface*)>& callback) const {
    for (const auto& [key, mgr] : m_presetManagers) {
        if (mgr != nullptr) {
            callback(mgr.get());
        }
    }
}

void LC_SettingsDialog::forEachPage(const std::function<void(LC_SettingsPageInterface*)>& callback) const {
    for (const auto& page : m_pages) {
        if (page != nullptr) {
            callback(page.get());
        }
    }
}
