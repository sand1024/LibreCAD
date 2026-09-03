
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

#include "lc_skin_preview_window.h"


#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QIcon>
#include <QLabel>
#include <QMenuBar>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyleFactory>
#include <qtablewidget.h>
#include <QToolBar>
#include <qtreewidget.h>
#include <QVBoxLayout>

#include "lc_caddockwidget.h"
#include "lc_dlg_widget_creator.h"
#include "lc_dockwidget.h"
#include "lc_highlight_overlay.h"
#include "lc_proxy_style.h"
#include "lc_settings_colors_semantics.h"
#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "ui_lc_skin_preview_window.h"

void LC_SkinPreviewWindow::hideSliderAndDialControls() const {
    ui->lblSlidersHead_2->setVisible(false);
    ui->lblSlidersHead_3->setVisible(false);
    ui->slider_2->setVisible(false);
    ui->slider_3->setVisible(false);
    ui->dial_2->setVisible(false);
    ui->dial_3->setVisible(false);
}

LC_SkinPreviewWindow::LC_SkinPreviewWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LC_SkinPreviewWindow) {
    ui->setupUi(this);

    // Create a persistent, local-only custom proxy style
    QStyle *baseStyle = QStyleFactory::create("Fusion");
    auto *localStyle = new LC_ProxyStyle(baseStyle);
    this->setStyle(localStyle);

    // Force background repainting with the palette
    setAutoFillBackground(true);
    if (ui->centralWidget) {
        ui->centralWidget->setAutoFillBackground(true);
    }

    setupScrollArea();
    setupMenuBar();
    setupToolbars();
    setupDockWidget();
    setupAdvancedViewsWidgets();
    setupEditors();
    setupTabPreviews();
    setupStatusPillToolbars();


    // Force the custom local style pointer explicitly onto all child widgets ONCE
    const QList<QWidget*> children = findChildren<QWidget*>();
    for (QWidget *child : children) {
        child->setStyle(localStyle);
    }

    setupToolButtonMenu();

    hideSliderAndDialControls(); // reserved for the future if needed
}

LC_SkinPreviewWindow::~LC_SkinPreviewWindow() {
    delete ui;
}

void LC_SkinPreviewWindow::applyToolbarsAndDocksConfig() {
    using namespace CFG_Widgets;

    // 1. Main Toolbar Icon Sizes
    const int tbSize = o_AllowToolbarIconSize.get() ? o_ToolbarIconSize.get() : 24;
    setIconSize(QSize(tbSize, tbSize));
    for (auto* tb : m_mainToolBars) {
        if (tb != nullptr) {
            tb->setIconSize(QSize(tbSize, tbSize));
            for (auto* btn : tb->findChildren<QToolButton*>()) {
                if (btn != nullptr) {
                    btn->setIconSize(QSize(tbSize, tbSize));
                    btn->update();
                }
            }
            tb->updateGeometry();
            tb->update();
        }
    }

    // 2. Dock Mini-Toolbar Buttons (Size and Flat/AutoRaise)
    const int dockIconSize = qBound(12, o_DockWidgetsIconSize.get(), 64);
    const bool dockFlat = o_DockWidgetsFlatIcons.get();
    for (auto* btn : m_dockToolButtons) {
        if (btn != nullptr) {
            btn->setIconSize(QSize(dockIconSize, dockIconSize));
            btn->setAutoRaise(dockFlat);
            btn->setFixedSize(dockIconSize + 8, dockIconSize + 8);
            btn->update();
        }
    }

    // 3. CAD Tools Matrix (Columns, Icon Size, and Flat/AutoRaise)
    const bool cadSidebarUngrouped = CFG_Startup::o_CADSideBarUngrouped;
    const int matrixIconSize = qBound(12, cadSidebarUngrouped ? o_LeftToolbarAllIconSize.get() : o_LeftToolbarIconSize.get(), 64);
    const int columns = qBound(1, cadSidebarUngrouped ? o_LeftToolbarAllColumnsCount.get() : o_LeftToolbarColumnsCount.get(), 12);
    const bool matrixFlat = cadSidebarUngrouped ? o_LeftToolbarAllFlatIcons.get() : o_LeftToolbarFlatIcons.get();

    if (m_matrixGridLayout != nullptr && !m_matrixToolButtons.isEmpty()) {
        // Clear existing grid positioning
        while (m_matrixGridLayout->count() > 0) {
            m_matrixGridLayout->takeAt(0);
        }

        // Place buttons in grid with fixed dimensions
        for (int i = 0; i < m_matrixToolButtons.size(); ++i) {
            auto* btn = m_matrixToolButtons[i];
            if (btn != nullptr) {
                btn->setIconSize(QSize(matrixIconSize, matrixIconSize));
                btn->setAutoRaise(matrixFlat);
                btn->setFixedSize(matrixIconSize + 8, matrixIconSize + 8);
                btn->update();

                const int row = i / columns;
                const int col = i % columns;
                m_matrixGridLayout->addWidget(btn, row, col);
            }
        }

        // Reset column stretches so buttons pack tightly to the left
        for (int c = 0; c < columns; ++c) {
            m_matrixGridLayout->setColumnStretch(c, 0);
        }
        // Right-side expanding horizontal spacer absorbs all extra width
        m_matrixGridLayout->setColumnStretch(columns, 1);

        // Vertical bottom spacer absorbs all extra height
        const int totalRows = (m_matrixToolButtons.size() + columns - 1) / columns;
        for (int r = 0; r < totalRows; ++r) {
            m_matrixGridLayout->setRowStretch(r, 0);
        }
        m_matrixGridLayout->setRowStretch(totalRows, 1);
    }

    // 4. Docking Features & Nested Docking Option
    QMainWindow::DockOptions opts = QMainWindow::AnimatedDocks;
    if (o_DockAllowNested.get()) {
        opts |= QMainWindow::AllowNestedDocks;
        opts |= QMainWindow::GroupedDragging;
    }
    setDockOptions(opts);

    // 5. Dock Title Bar Orientation (Horizontal vs Vertical)
    const bool verticalTitleBar = o_DockTitleBarVertical.get();
    for (auto* dock : m_allDockWidgets) {
        if (dock != nullptr && !dock->property(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET).toBool()) {
            QDockWidget::DockWidgetFeatures features = dock->features();
            if (verticalTitleBar) {
                features |= QDockWidget::DockWidgetVerticalTitleBar;
            } else {
                features &= ~QDockWidget::DockWidgetVerticalTitleBar;
            }
            dock->setFeatures(features);
        }
    }

    // 6. Side Dock Tab Position
    const bool verticalTabs = o_DockVerticalTabs.get();
    setTabPosition(Qt::LeftDockWidgetArea, verticalTabs ? QTabWidget::West : QTabWidget::South);
    setTabPosition(Qt::RightDockWidgetArea, verticalTabs ? QTabWidget::East : QTabWidget::South);

    // 7. Status Bar Height and Font Size
    if (statusBar() != nullptr) {
        if (o_AllowStatusbarHeight.get() && o_StatusbarHeight.get() > 0) {
            statusBar()->setMinimumHeight(o_StatusbarHeight.get());
        }
        if (o_AllowStatusbarFontSize.get() && o_StatusbarFontSize.get() > 0) {
            QFont f = statusBar()->font();
            f.setPointSize(o_StatusbarFontSize.get());
            statusBar()->setFont(f);
        }
    }

    update();
}

void LC_SkinPreviewWindow::setupScrollArea() const {
    // // Force the scroll area viewport and its container to use the Window background role
    // if (ui->scrollArea) {
    //     ui->scrollArea->viewport()->setBackgroundRole(QPalette::Window);
    //     ui->scrollArea->viewport()->setAutoFillBackground(true);
    // }
    // if (ui->scrollAreaWidgetContents) {
    //     ui->scrollAreaWidgetContents->setBackgroundRole(QPalette::Window);
    //     ui->scrollAreaWidgetContents->setAutoFillBackground(true);
    // }
}

void LC_SkinPreviewWindow::setupMenuBar() {
    auto *previewMenuBar = new QMenuBar(this);

    QMenu *fileMenu = previewMenuBar->addMenu(tr("&File"));
    fileMenu->QWidget::addAction(QIcon::fromTheme("document-new"), tr("&New Drawing"));
    fileMenu->QWidget::addAction(QIcon::fromTheme("document-open"), tr("&Open Project..."));

    QAction *actClose = fileMenu->QWidget::addAction(tr("&Close File"));
    actClose->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->setTearOffEnabled(true);

    QMenu *exportSubMenu = fileMenu->addMenu(tr("&Export"));
    exportSubMenu->QWidget::addAction(tr("Export to &PDF"));
    QAction *actDwg = exportSubMenu->QWidget::addAction(tr("Export to &DWG"));
    actDwg->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->QWidget::addAction(QIcon::fromTheme("application-exit"), tr("&Exit"));

    QMenu *editMenu = previewMenuBar->addMenu(tr("&Edit"));
    editMenu->QWidget::addAction(QIcon::fromTheme("edit-undo"), tr("&Undo"));
    editMenu->QWidget::addAction(QIcon::fromTheme("edit-redo"), tr("&Redo"));

    editMenu->addSeparator();
    QAction *actPrefs = editMenu->QWidget::addAction(tr("&Preferences"));
    actPrefs->setEnabled(false);

    editMenu->setTearOffEnabled(true);

    QMenu *cmdMenu = previewMenuBar->addMenu(tr("&Command line support"));
    cmdMenu->setTearOffEnabled(true);

    auto line = cmdMenu->QWidget::addAction(QIcon(":/icons/line_2p.lci"), tr("&Line 2 points"));
    line->setProperty("cmdLine", "line2p");
    line->setShortcut(QKeySequence("CTRL+L"));

    auto circle = cmdMenu->QWidget::addAction(QIcon(":/icons/circle_2_points.lci"), tr("&Circle 2 points"));
    circle->setProperty("cmdLine", "c2p");
    circle->setShortcut(QKeySequence("CTRL+SHIFT+C"));

    auto grid = cmdMenu->QWidget::addAction(QIcon(":/icons/grid.lci"), tr("&Grid"));

    setMenuBar(previewMenuBar);
}

void LC_SkinPreviewWindow::setupToolbars() {
    auto *toolbar1 = new QToolBar(this);
    toolbar1->setMovable(true);

    toolbar1->addAction(QIcon(":/icons/copy.lci"), "Copy\n\nThis is example of rich tooltip");
    toolbar1->addAction(QIcon(":/icons/paste.lci"), "This is one line tooltip");
    auto draft = toolbar1->addAction(QIcon(":/icons/draft.lci"), "Draft\n\nTooltip that contains both caption and text.\nWe test \n\n multiline");
    draft->setCheckable(true);
    draft->setChecked(true);

    toolbar1->addSeparator();
    toolbar1->addAction(QIcon(":/icons/circle_center_radius.lci"), "");
    auto grid = toolbar1->addAction(QIcon(":/icons/grid.lci"), "");
    grid->setCheckable(true);
    grid->setChecked(true);
    auto line = toolbar1->addAction(QIcon(":/icons/line_2p.lci"), "");
    line->setCheckable(true);
    line->setChecked(true);
    toolbar1->addAction(QIcon(":/icons/zoom_auto.lci"), "");
    toolbar1->addAction(QIcon(":/icons/create_toolbar.lci"), "");

    addToolBar(Qt::TopToolBarArea, toolbar1);
    m_mainToolBars.append(toolbar1);

    auto *toolbar2 = new QToolBar(this);
    toolbar1->setMovable(true);

    toolbar2->addAction(QIcon(":/icons/copy.lci"), "Copy\n\nThis is example of rich tooltip");
    toolbar2->addAction(QIcon(":/icons/paste.lci"), "This is one line tooltip");
    auto draft1 = toolbar2->addAction(QIcon(":/icons/draft.lci"), "Draft\n\nTooltip that contains both caption and text.\nWe test \n\n multiline");
    draft1->setCheckable(true);
    draft1->setChecked(true);

    toolbar2->addSeparator();
    toolbar2->addAction(QIcon(":/icons/circle_center_radius.lci"), "");
    auto grid1 = toolbar2->addAction(QIcon(":/icons/grid.lci"), "");
    grid1->setCheckable(true);
    grid1->setChecked(true);
    auto line1 = toolbar1->addAction(QIcon(":/icons/line_2p.lci"), "");
    line1->setCheckable(true);
    line1->setChecked(true);
    toolbar2->addAction(QIcon(":/icons/zoom_auto.lci"), "");
    toolbar2->addAction(QIcon(":/icons/create_toolbar.lci"), "");

    addToolBar(Qt::TopToolBarArea, toolbar2);
    m_mainToolBars.append(toolbar2);

    // 4. Programmatic Toolbar 2 Setup
    auto *toolbar3 = new QToolBar(this);
    toolbar3->setMovable(true);

    toolbar3->QWidget::addAction(tr("Cut"));
    toolbar3->QWidget::addAction(tr("Copy"));
    toolbar3->QWidget::addAction(tr("Paste"));
    toolbar3->addSeparator();

    toolbar3->addWidget(new QLabel(tr(" Zoom: "), toolbar2));
    auto *zoomCombo = new QComboBox(toolbar2);
    zoomCombo->addItems({"100%", "150%", "200%", "Fit to Screen"});
    toolbar3->addWidget(zoomCombo);

    toolbar3->addSeparator();

    auto *gridSnap = new QSpinBox(toolbar2);
    gridSnap->setValue(10);
    toolbar3->addWidget(new QLabel(tr(" Snap: "), toolbar2));
    toolbar3->addWidget(gridSnap);

    toolbar3->addSeparator();

    toolbar3->QWidget::addAction(tr("New"));
    toolbar3->QWidget::addAction(tr("Open"));
    toolbar3->QWidget::addAction(tr("Save"));

    addToolBar(Qt::TopToolBarArea, toolbar3);
    insertToolBarBreak(toolbar3);
    m_mainToolBars.append(toolbar3);
}

QWidget* LC_SkinPreviewWindow::setupDockContent(LC_DockWidget* propertiesDock, bool addTopToolbar) {
    if (propertiesDock == nullptr) {
        return nullptr;
    }

    auto* dockContents = new QWidget(propertiesDock);
    auto* dockLayout = new QVBoxLayout(dockContents);
    dockLayout->setContentsMargins(6, 6, 6, 6);
    dockLayout->setSpacing(4);

    // Mini Top Toolbar with action tool buttons
    if (addTopToolbar) {
        auto* miniToolbar = new QHBoxLayout();
        miniToolbar->setContentsMargins(0, 0, 0, 2);
        miniToolbar->setSpacing(2);

        const QList<QPair<QString, QString>> miniActions = {
            { ":/icons/add.lci", tr("Add Item") },
            { ":/icons/visible.lci", tr("Toggle Visibility") },
            { ":/icons/draft.lci", tr("Lock Layer") },
            { ":/icons/close_all.lci", tr("Remove Item") }
        };

        for (const auto& act : miniActions) {
            auto* btn = new QToolButton(dockContents);
            btn->setIcon(QIcon(act.first));
            btn->setToolTip(act.second);
            miniToolbar->addWidget(btn);
            m_dockToolButtons.append(btn);
        }
        miniToolbar->addStretch();
        dockLayout->addLayout(miniToolbar);
    }

    dockLayout->addWidget(new QLabel(tr("<b>Layer:</b> 0"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>Color:</b> ByLayer"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>LineType:</b> Continuous"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>LineWeight:</b> 0.25 mm"), dockContents));
    dockLayout->addStretch();
    return dockContents;
}

QWidget* LC_SkinPreviewWindow::setupCadToolsMatrixContent(LC_DockWidget* matrixDock) {
    if (matrixDock == nullptr) {
        return nullptr;
    }

    auto* scrollArea = new QScrollArea(matrixDock);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_matrixGridContainer = new QWidget(scrollArea);
    m_matrixGridLayout = new QGridLayout(m_matrixGridContainer);
    m_matrixGridLayout->setContentsMargins(4, 4, 4, 4);
    m_matrixGridLayout->setSpacing(2);

    const QStringList matrixIcons = {
        ":/icons/line_2p.lci", ":/icons/circle_2_points.lci", ":/icons/circle_center_radius.lci",
        ":/icons/move_copy.lci", ":/icons/copy.lci", ":/icons/paste.lci",
        ":/icons/visible.lci", ":/icons/draft.lci", ":/icons/grid.lci",
        ":/icons/interactive_pick_angle.lci", ":/icons/spline_explode.lci", ":/icons/add.lci",
        ":/icons/close_all.lci", ":/icons/save.lci", ":/icons/fileopen.lci",
        ":/icons/zoom_auto.lci", ":/icons/create_toolbar.lci", ":/icons/halign_middle.lci",
        ":/icons/new.lci", ":/icons/undo.lci"
    };

    m_matrixToolButtons.clear();
    for (int i = 0; i < matrixIcons.size(); ++i) {
        auto* btn = new QToolButton(m_matrixGridContainer);
        btn->setIcon(QIcon(matrixIcons[i]));
        btn->setCheckable(true);
        if (i == 0 || i == 3) {
            btn->setChecked(true);
        }
        m_matrixToolButtons.append(btn);
    }

    // Default layout with 5 columns
    const int defaultCols = 5;
    for (int i = 0; i < m_matrixToolButtons.size(); ++i) {
        m_matrixGridLayout->addWidget(m_matrixToolButtons[i], i / defaultCols, i % defaultCols);
        m_matrixGridLayout->setColumnStretch(i % defaultCols, 0);
    }
    m_matrixGridLayout->setColumnStretch(defaultCols, 1);

    scrollArea->setWidget(m_matrixGridContainer);
    return scrollArea;
}

void LC_SkinPreviewWindow::setupDockWidget() {
    m_allDockWidgets.clear();
    m_dockToolButtons.clear();

    // ================= 1. LEFT DOCK AREA (CAD TOOLS MATRIX EMULATION) =================
    auto* dockCadMatrix = new LC_DockWidget(this, tr("CAD Tools Matrix"), tr("CAD Tools Matrix"));
    dockCadMatrix->setWindowIcon(QIcon(":/icons/line_2p.lci"));
    dockCadMatrix->setProperty(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET, true);
    dockCadMatrix->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    dockCadMatrix->setWidget(setupCadToolsMatrixContent(dockCadMatrix));
    addDockWidget(Qt::LeftDockWidgetArea, dockCadMatrix);
    m_allDockWidgets.append(dockCadMatrix);

    // ================= 2. RIGHT DOCK AREA (STANDALONE + 3 TABBED DOCKS) =================

    // Standalone Dock: Layer Properties with top mini-toolbar
    auto* dockLayers = new LC_DockWidget(this, tr("Layer Properties"), tr("Layer Properties"));
    dockLayers->setWindowIcon(QIcon(":/icons/visible.lci"));
    dockLayers->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    dockLayers->setWidget(setupDockContent(dockLayers, true));

    // 3 Tabified Docks on the Right
    auto* dockBlocks = new LC_DockWidget(this, tr("Block List"), tr("Block List"));
    dockBlocks->setWindowIcon(QIcon(":/icons/draft.lci"));
    dockBlocks->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    dockBlocks->setWidget(setupDockContent(dockBlocks, true));

    auto* dockExplorer = new LC_DockWidget(this, tr("Drawing Explorer"), tr("Drawing Explorer"));
    dockExplorer->setWindowIcon(QIcon(":/icons/fileopen.lci"));
    dockExplorer->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    dockExplorer->setWidget(setupDockContent(dockExplorer, false));

    auto* dockSnap = new LC_DockWidget(this, tr("Snapping & Grid"), tr("Snapping & Grid"));
    dockSnap->setWindowIcon(QIcon(":/icons/grid.lci"));
    dockSnap->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
    dockSnap->setWidget(setupDockContent(dockSnap, false));

    addDockWidget(Qt::RightDockWidgetArea, dockLayers);
    addDockWidget(Qt::RightDockWidgetArea, dockBlocks);
    addDockWidget(Qt::RightDockWidgetArea, dockExplorer);
    addDockWidget(Qt::RightDockWidgetArea, dockSnap);

    tabifyDockWidget(dockLayers, dockBlocks);
    tabifyDockWidget(dockBlocks, dockExplorer);
    tabifyDockWidget(dockExplorer, dockSnap);
    dockBlocks->raise();

    splitDockWidget(dockLayers, dockBlocks, Qt::Vertical);

    m_allDockWidgets.append(dockLayers);
    m_allDockWidgets.append(dockBlocks);
    m_allDockWidgets.append(dockExplorer);
    m_allDockWidgets.append(dockSnap);
}

void LC_SkinPreviewWindow::setupAdvancedViewsWidgets() const {
    if (ui->label_4 != nullptr) {
        new LC_HighlightOverlay(ui->label_4);
    }

    // 1. Table View Setup
    ui->table->setRowCount(4);

    auto* self = const_cast<LC_SkinPreviewWindow*>(this);
    self->m_tableSearchItem = new QTableWidgetItem(tr("Line 1"));
    self->m_tableSearchItem->setToolTip(tr("[Search Highlight Text Demonstration]"));
    ui->table->setItem(0, 0, self->m_tableSearchItem);
    ui->table->setItem(0, 1, new QTableWidgetItem(tr("Layer: 0")));

    self->m_tableConflictItem = new QTableWidgetItem(tr("Circle 2"));
    self->m_tableConflictItem->setToolTip(tr("[Conflicting Item Text Demonstration]"));
    ui->table->setItem(1, 0, self->m_tableConflictItem);
    ui->table->setItem(1, 1, new QTableWidgetItem(tr("Layer: Construction")));

    ui->table->setItem(2, 0, new QTableWidgetItem(tr("Dimension 3")));
    ui->table->setItem(2, 1, new QTableWidgetItem(tr("Layer: Annotation")));
    ui->table->setItem(3, 0, new QTableWidgetItem(tr("Text 4")));
    ui->table->setItem(3, 1, new QTableWidgetItem(tr("Layer: TitleBlock")));

    ui->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table->setSortingEnabled(true);
    ui->table->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

    // 2. Tree View Setup
    const auto rootNode = new QTreeWidgetItem(ui->tree, QStringList(tr("Model Workspace")));
    const auto layersFolder = new QTreeWidgetItem(rootNode, QStringList(tr("Layers")));

    self->m_treeSearchItem = new QTreeWidgetItem(layersFolder, QStringList(tr("Layer 0 (Visible)")));
    self->m_treeSearchItem->setToolTip(0, tr("[Search Highlight Text Demonstration]"));

    self->m_treeConflictItem = new QTreeWidgetItem(layersFolder, QStringList(tr("Layer Construction (Conflict)")));
    self->m_treeConflictItem->setToolTip(0, tr("[Conflicting Item Text Demonstration]"));

    const auto blocksFolder = new QTreeWidgetItem(rootNode, QStringList(tr("Blocks")));
    new QTreeWidgetItem(blocksFolder, QStringList(tr("Title Block A1")));
    new QTreeWidgetItem(blocksFolder, QStringList(tr("Detail Marker Section")));

    ui->tree->addTopLevelItem(rootNode);
    ui->tree->expandAll();
    ui->tree->setCurrentItem(self->m_treeSearchItem);

    // 3. List View Setup
    self->m_listSearchItem = new QListWidgetItem(tr("drawing_floor_plan.dwg"), ui->listWidget);
    self->m_listSearchItem->setToolTip(tr("[Search Highlight Text Demonstration]"));
    ui->listWidget->addItem(self->m_listSearchItem);

    self->m_listConflictItem = new QListWidgetItem(tr("drawing_elevation.dwg"), ui->listWidget);
    self->m_listConflictItem->setToolTip(tr("[Conflicting Item Text Demonstration]"));
    ui->listWidget->addItem(self->m_listConflictItem);

    ui->listWidget->addItem(tr("template_metric_standard.dwt"));

    const auto selectedListItem = new QListWidgetItem(tr("layout_presentation_sheet.dwg"), ui->listWidget);
    ui->listWidget->addItem(selectedListItem);
    ui->listWidget->setCurrentItem(selectedListItem);
}

void LC_SkinPreviewWindow::updateSemanticViews(const QColor& searchHighlightColor, const QColor& conflictingItemColor) {
    
    // 1. Update Table Item Foregrounds
    if (m_tableSearchItem != nullptr) {
        m_tableSearchItem->setForeground(QBrush(searchHighlightColor));
    }
    if (m_tableConflictItem != nullptr) {
        m_tableConflictItem->setForeground(QBrush(conflictingItemColor));
    }

    // 2. Update Tree Item Foregrounds
    if (m_treeSearchItem != nullptr) {
        m_treeSearchItem->setForeground(0, QBrush(searchHighlightColor));
    }
    if (m_treeConflictItem != nullptr) {
        m_treeConflictItem->setForeground(0, QBrush(conflictingItemColor));
    }

    // 3. Update List Item Foregrounds
    if (m_listSearchItem != nullptr) {
        m_listSearchItem->setForeground(QBrush(searchHighlightColor));
    }
    if (m_listConflictItem != nullptr) {
        m_listConflictItem->setForeground(QBrush(conflictingItemColor));
    }

    // 4. Force overlay repaint on label_4
    if (ui->label_4 != nullptr) {
        ui->label_4->update();
    }

    ui->table->viewport()->update();
    ui->tree->viewport()->update();
    ui->listWidget->viewport()->update();
}

void LC_SkinPreviewWindow::activateTab(const QString& tag) {
    if (tag == "advanced") {
        ui->tabs->setCurrentWidget(ui->tabAdvanced);
    }
    else if (tag == "standard") {
        ui->tabs->setCurrentWidget(ui->tabBasic);
    }
}

void LC_SkinPreviewWindow::setupEditors() const {
    // 9. Reset QTextEdit plain-text to ignore inline HTML layout styles
    ui->textEdit_2->setPlainText(tr("Multiline text editor content demonstrating paragraph rendering, monospace alignments, and scrolling inside a frame."));
    ui->teSelection_2->setPlainText(tr("Multiline text editor content demonstrating selected text rendering."));
    ui->textEdit_3->setPlainText(tr("Multiline text editor content demonstrating paragraph rendering, monospace alignments, and scrolling inside a frame."));
    ui->teSelection_3->setPlainText(tr("Multiline text editor content demonstrating selected text rendering."));

    // 10. Highlight default characters
    ui->lineEditNormal_2->setSelection(0, 6);

    // 11. Setup hover tooltips
    ui->btnNormal_2->setToolTip(tr("Tooltip demonstration showing ToolTipBase and ToolTipText colors."));
    ui->btnPressed_2->setToolTip(tr("Tooltip demonstration showing ToolTipBase and ToolTipText colors."));

    ui->teSelection_2->selectAll();
    ui->teSelection_3->selectAll();
}

void LC_SkinPreviewWindow::setupToolButtonMenu() const {
    ui->tbOptions_2->setPopupMode(QToolButton::InstantPopup);
    const auto generateMenu_2 = new QMenu(ui->tbOptions_2);

    generateMenu_2->QWidget::addAction(QIcon(":/icons/palette_1_point.lci"), tr("Popup Option 1"));
    generateMenu_2->QWidget::addAction(QIcon(":/icons/palette_2_points.lci"), tr("Popup Option 2"));
    generateMenu_2->QWidget::addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("Another popup option"));

    ui->tbOptions_2->setMenu(generateMenu_2);
    
    ui->tbOptions_3->setPopupMode(QToolButton::MenuButtonPopup);
    // ui->tbOptions_3->setArrowType(Qt::RightArrow);
    const auto generateMenu_3 = new QMenu(ui->tbOptions_3);

    generateMenu_3->QWidget::addAction(QIcon(":/icons/palette_1_point.lci"), tr("Popup Option 1"));
    generateMenu_3->QWidget::addAction(QIcon(":/icons/palette_3_points.lci"), tr("Popup Option 2"));
    generateMenu_3->QWidget::addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("Another popup option"));

    ui->tbOptions_3->setMenu(generateMenu_3);
}

void LC_SkinPreviewWindow::setupTabPreviews() {
    auto *previewTabsContainer = new QWidget(this);
    auto *containerLayout = new QVBoxLayout(previewTabsContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(previewTabsContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollWidget = new QWidget(scrollArea);
    auto *vertLayout = new QVBoxLayout(scrollWidget);
    vertLayout->setContentsMargins(10, 10, 10, 10);
    vertLayout->setSpacing(16);

    const int vertTabMinWidth = 48;
    const int vertTabMaxHeight = 220;

    // ================= 1. ROUNDED HORIZONTAL PREVIEWS =================
    {
        auto *roundedGroup = new QGroupBox(tr("Rounded Horizontal Previews"), scrollWidget);
        auto *groupLayout = new QVBoxLayout(roundedGroup);
        groupLayout->setContentsMargins(8, 8, 8, 8);
        groupLayout->setSpacing(12);

        // North (Top Tabs — Mixed Modes)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("North (Top Tabs — Mixed Modes)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedNorth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);

            bar->addTab(QIcon(":/icons/fileopen.lci"), tr("Project A"));
            bar->addTab(QIcon(":/icons/line_2p.lci"), tr("Model Space (Active)"));
            bar->addTab(QIcon(":/icons/grid.lci"), QString());
            bar->setTabToolTip(2, tr("Grid Settings (Icon Only)"));
            bar->addTab(tr("Plain Text"));
            bar->addTab(QIcon(":/icons/close_all.lci"), tr("Locked Sheet"));
            bar->setTabEnabled(4, false);

            bar->setCurrentIndex(1);
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // North (Top Tabs — Scrollable with Icons)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("North (Top Tabs — Scrollable with Icons)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedNorth);
            bar->setTabsClosable(true);
            bar->setUsesScrollButtons(true);
            bar->setAttribute(Qt::WA_Hover, true);

            const QList<QPair<QString, QString>> iconTabs = {
                { ":/icons/fileopen.lci", tr("Floorplan") },
                { ":/icons/line_2p.lci", tr("Elevation (Active)") },
                { ":/icons/circle_center_radius.lci", tr("Section A-A") },
                { ":/icons/grid.lci", tr("Grid Detail") },
                { ":/icons/draft.lci", tr("Draft Overlay") },
                { ":/icons/save.lci", tr("Sheet 1") },
                { ":/icons/copy.lci", tr("Sheet 2") },
                { ":/icons/paste.lci", tr("Sheet 3") }
            };

            for (const auto &entry : iconTabs) {
                bar->addTab(QIcon(entry.first), entry.second);
            }
            bar->setCurrentIndex(1);
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // South (Bottom Tabs — Icon + Text)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("South (Bottom Tabs — Icon + Text)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedSouth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);

            bar->addTab(QIcon(":/icons/visible.lci"), tr("Model"));
            bar->addTab(QIcon(":/icons/draft.lci"), tr("Layout 1 (Active)"));
            bar->addTab(QIcon(":/icons/save.lci"), tr("Layout 2"));
            bar->setCurrentIndex(1);
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        vertLayout->addWidget(roundedGroup);
    }

    // ================= 2. BEVELED HORIZONTAL PREVIEWS =================
    {
        auto *beveledGroup = new QGroupBox(tr("Beveled Horizontal Previews"), scrollWidget);
        auto *groupLayout = new QVBoxLayout(beveledGroup);
        groupLayout->setContentsMargins(8, 8, 8, 8);
        groupLayout->setSpacing(12);

        // Beveled North
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("Beveled North (Top Tabs — Mixed Modes)"), beveledGroup));
            auto *bar = new QTabBar(beveledGroup);
            bar->setShape(QTabBar::TriangularNorth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);

            bar->addTab(QIcon(":/icons/line_2p.lci"), tr("Viewport 1"));
            bar->addTab(QIcon(":/icons/move_copy.lci"), tr("Viewport 2 (Active)"));
            bar->addTab(QIcon(":/icons/grid.lci"), QString());
            bar->setTabToolTip(2, tr("Grid View (Icon Only)"));
            bar->addTab(tr("Viewport 3"));

            bar->setCurrentIndex(1);
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // Beveled South (Scrollable with Icons)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("Beveled South (Bottom Tabs — Scrollable with Icons)"), beveledGroup));
            auto *bar = new QTabBar(beveledGroup);
            bar->setShape(QTabBar::TriangularSouth);
            bar->setTabsClosable(true);
            bar->setUsesScrollButtons(true);
            bar->setAttribute(Qt::WA_Hover, true);

            for (int i = 1; i <= 10; ++i) {
                const QString iconPath = (i % 2 == 0) ? ":/icons/circle_center_radius.lci" : ":/icons/line_2p.lci";
                bar->addTab(QIcon(iconPath), tr("Page %1").arg(i));
            }
            bar->setCurrentIndex(1);
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        vertLayout->addWidget(beveledGroup);
    }

    // ================= 3. ROUNDED VERTICAL PREVIEWS =================
    {
        auto *roundedVertGroup = new QGroupBox(tr("Rounded Vertical Previews"), scrollWidget);
        auto *pairLayout = new QHBoxLayout(roundedVertGroup);
        pairLayout->setContentsMargins(8, 8, 8, 8);
        pairLayout->setSpacing(12);

        // West
        {
            auto *westVBox = new QVBoxLayout();
            westVBox->addWidget(new QLabel(tr("West (Left)"), roundedVertGroup));
            auto *westBar = new QTabBar(roundedVertGroup);
            westBar->setShape(QTabBar::RoundedWest);
            westBar->setTabsClosable(true);
            westBar->setAttribute(Qt::WA_Hover, true);

            westBar->addTab(QIcon(":/icons/line_2p.lci"), tr("Draw"));
            westBar->addTab(QIcon(":/icons/move_copy.lci"), tr("Modify (Active)"));
            westBar->addTab(QIcon(":/icons/grid.lci"), QString());
            westBar->setTabToolTip(2, tr("Grid (Icon Only)"));
            westBar->addTab(QIcon(":/icons/close_all.lci"), tr("Locked"));
            westBar->setTabEnabled(3, false);

            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth);
            westVBox->addWidget(westBar);
            westVBox->addStretch();
            pairLayout->addLayout(westVBox);
        }

        // East
        {
            auto *eastVBox = new QVBoxLayout();
            eastVBox->addWidget(new QLabel(tr("East (Right)"), roundedVertGroup));
            auto *eastBar = new QTabBar(roundedVertGroup);
            eastBar->setShape(QTabBar::RoundedEast);
            eastBar->setTabsClosable(true);
            eastBar->setAttribute(Qt::WA_Hover, true);

            eastBar->addTab(QIcon(":/icons/visible.lci"), tr("Layers"));
            eastBar->addTab(QIcon(":/icons/draft.lci"), tr("Blocks (Active)"));
            eastBar->addTab(QIcon(":/icons/save.lci"), tr("Sheets"));

            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth);
            eastVBox->addWidget(eastBar);
            eastVBox->addStretch();
            pairLayout->addLayout(eastVBox);
        }

        // West Scrollable
        {
            auto *westScrollVBox = new QVBoxLayout();
            westScrollVBox->addWidget(new QLabel(tr("West (Scrollable)"), roundedVertGroup));
            auto *westBar = new QTabBar(roundedVertGroup);
            westBar->setShape(QTabBar::RoundedWest);
            westBar->setTabsClosable(true);
            westBar->setUsesScrollButtons(true);
            westBar->setAttribute(Qt::WA_Hover, true);

            for (int i = 1; i <= 10; ++i) {
                const QString iconPath = (i % 2 == 0) ? ":/icons/circle_center_radius.lci" : ":/icons/line_2p.lci";
                westBar->addTab(QIcon(iconPath), tr("Tool %1").arg(i));
            }
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth);
            westBar->setMaximumHeight(vertTabMaxHeight);
            westScrollVBox->addWidget(westBar);
            westScrollVBox->addStretch();
            pairLayout->addLayout(westScrollVBox);
        }

        // East Scrollable
        {
            auto *eastScrollVBox = new QVBoxLayout();
            eastScrollVBox->addWidget(new QLabel(tr("East (Scrollable)"), roundedVertGroup));
            auto *eastBar = new QTabBar(roundedVertGroup);
            eastBar->setShape(QTabBar::RoundedEast);
            eastBar->setTabsClosable(true);
            eastBar->setUsesScrollButtons(true);
            eastBar->setAttribute(Qt::WA_Hover, true);

            for (int i = 1; i <= 10; ++i) {
                const QString iconPath = (i % 2 == 0) ? ":/icons/save.lci" : ":/icons/draft.lci";
                eastBar->addTab(QIcon(iconPath), tr("Prop %1").arg(i));
            }
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth);
            eastBar->setMaximumHeight(vertTabMaxHeight);
            eastScrollVBox->addWidget(eastBar);
            eastScrollVBox->addStretch();
            pairLayout->addLayout(eastScrollVBox);
        }

        vertLayout->addWidget(roundedVertGroup);
    }

    // ================= 4. BEVELED VERTICAL PREVIEWS =================
    {
        auto *beveledVertGroup = new QGroupBox(tr("Beveled Vertical Previews (Scrollable)"), scrollWidget);
        auto *pairLayout = new QHBoxLayout(beveledVertGroup);
        pairLayout->setContentsMargins(8, 8, 8, 8);
        pairLayout->setSpacing(12);

        // Triangular West
        {
            auto *westVBox = new QVBoxLayout();
            westVBox->addWidget(new QLabel(tr("West (Left)"), beveledVertGroup));
            auto *westBar = new QTabBar(beveledVertGroup);
            westBar->setShape(QTabBar::TriangularWest);
            westBar->setTabsClosable(true);
            westBar->setAttribute(Qt::WA_Hover, true);

            westBar->addTab(QIcon(":/icons/line_2p.lci"), tr("CAD Tools"));
            westBar->addTab(QIcon(":/icons/visible.lci"), tr("Viewport (Active)"));
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth);
            westVBox->addWidget(westBar);
            westVBox->addStretch();
            pairLayout->addLayout(westVBox);
        }

        // Triangular East
        {
            auto *eastVBox = new QVBoxLayout();
            eastVBox->addWidget(new QLabel(tr("East (Right)"), beveledVertGroup));
            auto *eastBar = new QTabBar(beveledVertGroup);
            eastBar->setShape(QTabBar::TriangularEast);
            eastBar->setTabsClosable(true);
            eastBar->setAttribute(Qt::WA_Hover, true);

            eastBar->addTab(QIcon(":/icons/fileopen.lci"), tr("Layout A"));
            eastBar->addTab(QIcon(":/icons/save.lci"), tr("Layout B (Active)"));
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth);
            eastVBox->addWidget(eastBar);
            eastVBox->addStretch();
            pairLayout->addLayout(eastVBox);
        }

        // Triangular West Scrollable
        {
            auto *westScrollVBox = new QVBoxLayout();
            westScrollVBox->addWidget(new QLabel(tr("West (Scrollable)"), beveledVertGroup));
            auto *westBar = new QTabBar(beveledVertGroup);
            westBar->setShape(QTabBar::TriangularWest);
            westBar->setTabsClosable(true);
            westBar->setUsesScrollButtons(true);
            westBar->setAttribute(Qt::WA_Hover, true);

            for (int i = 1; i <= 10; ++i) {
                const QString iconPath = (i % 2 == 0) ? ":/icons/circle_center_radius.lci" : ":/icons/move_copy.lci";
                westBar->addTab(QIcon(iconPath), tr("Model %1").arg(i));
            }
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth);
            westBar->setMaximumHeight(vertTabMaxHeight);
            westScrollVBox->addWidget(westBar);
            westScrollVBox->addStretch();
            pairLayout->addLayout(westScrollVBox);
        }

        // Triangular East Scrollable
        {
            auto *eastScrollVBox = new QVBoxLayout();
            eastScrollVBox->addWidget(new QLabel(tr("East (Scrollable)"), beveledVertGroup));
            auto *eastBar = new QTabBar(beveledVertGroup);
            eastBar->setShape(QTabBar::TriangularEast);
            eastBar->setTabsClosable(true);
            eastBar->setUsesScrollButtons(true);
            eastBar->setAttribute(Qt::WA_Hover, true);

            for (int i = 1; i <= 10; ++i) {
                const QString iconPath = (i % 2 == 0) ? ":/icons/save.lci" : ":/icons/draft.lci";
                eastBar->addTab(QIcon(iconPath), tr("Sheet %1").arg(i));
            }
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth);
            eastBar->setMaximumHeight(vertTabMaxHeight);
            eastScrollVBox->addWidget(eastBar);
            eastScrollVBox->addStretch();
            pairLayout->addLayout(eastScrollVBox);
        }

        vertLayout->addWidget(beveledVertGroup);
    }

    vertLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    containerLayout->addWidget(scrollArea);

    ui->tabs->addTab(previewTabsContainer, tr("Tabs Alignment"));
}

void LC_SkinPreviewWindow::setupStatusPillToolbars() {
    auto *statusToolbarCoords = new QToolBar(this);
    auto *statusToolbarSnap = new QToolBar(this);

    // 1. Assign visual property toggles via centralized constants
    statusToolbarCoords->setProperty(PROP_USE_STATUS_PILL_CHIPS, true);
    statusToolbarSnap->setProperty(PROP_USE_STATUS_PILL_CHIPS, true);
    statusToolbarSnap->setMovable(true);
    statusToolbarCoords->setMovable(true);

    // 2. Mark the Snap Status toolbar as active to demonstrate the high-contrast toggled state
    statusToolbarSnap->setProperty(PROP_ACTIVE, true);

    // 3. Populate Coordinates (Demonstrates low-contrast recess/idle state)
    auto *lblX = new QLabel(tr(" <b>X:</b> 150.32 "), this);
    auto *lblY = new QLabel(tr(" <b>Y:</b> -85.74 "), this);


    statusToolbarCoords->addWidget(lblX);
    statusToolbarCoords->addSeparator();
    statusToolbarCoords->addWidget(lblY);

    // 4. Populate Snap Status (Demonstrates active highlighted state with standard grips)
    auto *btnSnapGrid = new QToolButton(this);
    btnSnapGrid->setIcon(QIcon(":/icons/grid.lci"));
    btnSnapGrid->setCheckable(true);
    btnSnapGrid->setChecked(true);
    btnSnapGrid->setToolTip(tr("Grid Snapping: Enabled"));

    auto *btnSnapEnd = new QToolButton(this);
    btnSnapEnd->setIcon(QIcon(":/icons/line_2p.lci"));
    btnSnapEnd->setCheckable(true);
    btnSnapEnd->setToolTip(tr("Endpoint Snapping: Disabled"));

    statusToolbarSnap->addWidget(btnSnapGrid);
    statusToolbarSnap->addWidget(btnSnapEnd);

    QSizePolicy tbPolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    statusToolbarCoords->setSizePolicy(tbPolicy);
    statusToolbarSnap->setSizePolicy(tbPolicy);

    addToolBar(Qt::BottomToolBarArea, statusToolbarCoords);
    addToolBar(Qt::BottomToolBarArea, statusToolbarSnap);
}
void LC_SkinPreviewWindow::closeEvent(QCloseEvent *event) {
    emit windowClosed();
    QMainWindow::closeEvent(event);
}
