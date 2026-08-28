
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
#include <QStyleFactory>
#include <qtablewidget.h>
#include <QToolBar>
#include <qtreewidget.h>
#include <QVBoxLayout>
#include "lc_dlg_widget_creator.h"
#include "lc_dockwidget.h"
#include "lc_proxy_style.h"
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
}

QWidget* LC_SkinPreviewWindow::setupDockContent(LC_DockWidget* propertiesDock) {
    const auto dockContents = new QWidget(propertiesDock);
    const auto dockLayout = new QVBoxLayout(dockContents);
    dockLayout->setContentsMargins(6, 6, 6, 6);
    dockLayout->setSpacing(4);

    dockLayout->addWidget(new QLabel(tr("<b>Layer:</b> 0"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>Color:</b> ByLayer"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>LineType:</b> Continuous"), dockContents));
    dockLayout->addWidget(new QLabel(tr("<b>LineWeight:</b> 0.25 mm"), dockContents));
    dockLayout->addStretch();
    return dockContents;
}

void LC_SkinPreviewWindow::setupDockWidget() {
    const auto propertiesDock = new LC_DockWidget(this, tr("Dockable Panel 1"), tr("Dockable Panel 1"));
    propertiesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    QWidget* dockContents = setupDockContent(propertiesDock);
    propertiesDock->setWidget(dockContents);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    const auto propertiesDock1 = new LC_DockWidget(this, tr("Dockable Panel 2"), tr("Dockable Panel 2"));
    propertiesDock1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetVerticalTitleBar);
    QWidget* dockContents1 = setupDockContent(propertiesDock1);
    propertiesDock1->setWidget(dockContents1);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock1);
}

void LC_SkinPreviewWindow::setupAdvancedViewsWidgets() const {
    ui->table->setRowCount(4);
    ui->table->setItem(0, 0, new QTableWidgetItem(tr("Line 1")));
    ui->table->setItem(0, 1, new QTableWidgetItem(tr("Layer: 0")));
    ui->table->setItem(1, 0, new QTableWidgetItem(tr("Circle 2")));
    ui->table->setItem(1, 1, new QTableWidgetItem(tr("Layer: Construction")));
    ui->table->setItem(2, 0, new QTableWidgetItem(tr("Dimension 3")));
    ui->table->setItem(2, 1, new QTableWidgetItem(tr("Layer: Annotation")));
    ui->table->setItem(3, 0, new QTableWidgetItem(tr("Text 4")));
    ui->table->setItem(3, 1, new QTableWidgetItem(tr("Layer: TitleBlock")));

    ui->table->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->table->setSortingEnabled(true);
    ui->table->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);

    // 7. Programmatic Tree View Content Population (Multi-node nested dataset)
    const auto rootNode = new QTreeWidgetItem(ui->tree, QStringList(tr("Model Workspace")));

    const auto layersFolder = new QTreeWidgetItem(rootNode, QStringList(tr("Layers")));
    const auto layer0 = new QTreeWidgetItem(layersFolder, QStringList(tr("Layer 0 (Visible)")));
    const auto layerConst = new QTreeWidgetItem(layersFolder, QStringList(tr("Layer Construction (Hidden)")));
    layerConst->setDisabled(true); // Demonstrates item-view disabled text color

    const auto blocksFolder = new QTreeWidgetItem(rootNode, QStringList(tr("Blocks")));
    new QTreeWidgetItem(blocksFolder, QStringList(tr("Title Block A1")));
    new QTreeWidgetItem(blocksFolder, QStringList(tr("Detail Marker Section")));

    ui->tree->addTopLevelItem(rootNode);
    ui->tree->expandAll();
    ui->tree->setCurrentItem(layer0); // Highlight first layer row

    // 8. Programmatic List View Content Population (Note 3)
    ui->listWidget->addItem(tr("drawing_floor_plan.dwg"));
    ui->listWidget->addItem(tr("drawing_elevation.dwg"));
    ui->listWidget->addItem(tr("template_metric_standard.dwt"));

    const auto selectedListItem = new QListWidgetItem(tr("layout_presentation_sheet.dwg"), ui->listWidget);
    ui->listWidget->addItem(selectedListItem);
    ui->listWidget->setCurrentItem(selectedListItem); // Highlight selected item row
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
    auto *mainGridLayout = new QGridLayout(previewTabsContainer);
    mainGridLayout->setContentsMargins(10, 10, 10, 10);
    mainGridLayout->setSpacing(16);

    // Constant parameters for strict visual uniformity
    const int horizTabWidth = 320;
    const int vertTabMinWidth = 40;
    const int vertTabMaxHeight = 200;

    // ================= LEFT COLUMN: HORIZONTAL LAYOUTS =================
    auto *leftColWidget = new QWidget(previewTabsContainer);
    auto *leftColLayout = new QVBoxLayout(leftColWidget);
    leftColLayout->setContentsMargins(0, 0, 0, 0);
    leftColLayout->setSpacing(16);

    // Group Box 1: Rounded Horizontal Previews
    {
        auto *roundedGroup = new QGroupBox(tr("Rounded Horizontal Previews"), leftColWidget);
        auto *groupLayout = new QVBoxLayout(roundedGroup);
        groupLayout->setContentsMargins(8, 8, 8, 8);
        groupLayout->setSpacing(12);

        // 1. Rounded North (Top Tabs - Scrollable)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("North (Top Tabs - Scrollable)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedNorth);
            bar->setTabsClosable(true);
            bar->setUsesScrollButtons(true);
            bar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                bar->addTab(tr("Tab %1").arg(i));
            }
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("South (Bottom Tabs - Scrollable)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedSouth);
            bar->setTabsClosable(true);
            bar->setUsesScrollButtons(true);
            bar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                bar->addTab(tr("Tab %1").arg(i));
            }
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // 2. Rounded South (Bottom Tabs)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("North (Top Tabs)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedNorth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);
            bar->addTab(tr("Files"));
            bar->addTab(tr("Layouts (Active)"));
            bar->addTab(tr("Settings"));
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // 2. Rounded South (Bottom Tabs)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("South (Bottom Tabs)"), roundedGroup));
            auto *bar = new QTabBar(roundedGroup);
            bar->setShape(QTabBar::RoundedSouth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);
            bar->addTab(tr("Files"));
            bar->addTab(tr("Layouts (Active)"));
            bar->addTab(tr("Settings"));
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        leftColLayout->addWidget(roundedGroup);
    }

    // Group Box 2: Beveled Horizontal Previews
    {
        auto *beveledGroup = new QGroupBox(tr("Beveled Horizontal Previews"), leftColWidget);
        auto *groupLayout = new QVBoxLayout(beveledGroup);
        groupLayout->setContentsMargins(8, 8, 8, 8);
        groupLayout->setSpacing(12);

        // 3. Triangular North (Beveled Top Tabs)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("Beveled North (Top Tabs)"), beveledGroup));
            auto *bar = new QTabBar(beveledGroup);
            bar->setShape(QTabBar::TriangularNorth);
            bar->setTabsClosable(true);
            bar->setAttribute(Qt::WA_Hover, true);
            bar->addTab(tr("Viewport 1"));
            bar->addTab(tr("Viewport 2 (Active)"));
            bar->addTab(tr("Viewport 3"));
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        // 4. Triangular South (Beveled Bottom Tabs - Scrollable)
        {
            auto *vBox = new QVBoxLayout();
            vBox->addWidget(new QLabel(tr("Beveled South (Bottom Tabs - Scrollable)"), beveledGroup));
            auto *bar = new QTabBar(beveledGroup);
            bar->setShape(QTabBar::TriangularSouth);
            bar->setTabsClosable(true);
            bar->setUsesScrollButtons(true);
            bar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                bar->addTab(tr("Page %1").arg(i));
            }
            bar->setCurrentIndex(1);
            // bar->setFixedWidth(horizTabWidth); // Enforce same width
            vBox->addWidget(bar);
            groupLayout->addLayout(vBox);
        }

        leftColLayout->addWidget(beveledGroup);
    }

    leftColLayout->addStretch();
    mainGridLayout->addWidget(leftColWidget, 0, 0);

    // ================= RIGHT COLUMN: VERTICAL LAYOUTS (4 Columns Side-by-Side) =================
    auto *rightColWidget = new QWidget(previewTabsContainer);
    auto *rightColLayout = new QVBoxLayout(rightColWidget);
    rightColLayout->setContentsMargins(0, 0, 0, 0);
    rightColLayout->setSpacing(16);

    // Group Box 3: Rounded Vertical combination (4 columns side-by-side)
    {
        auto *pairGroupBox = new QGroupBox(tr("Rounded Vertical Previews"), rightColWidget);
        auto *pairLayout = new QHBoxLayout(pairGroupBox);
        pairLayout->setContentsMargins(8, 8, 8, 8);
        pairLayout->setSpacing(12);

        // 1. Left (West)
        {
            auto *westVBox = new QVBoxLayout();
            westVBox->addWidget(new QLabel(tr("West (Left)"), pairGroupBox));
            auto *westBar = new QTabBar(pairGroupBox);
            westBar->setShape(QTabBar::RoundedWest);
            westBar->setTabsClosable(true);
            westBar->setAttribute(Qt::WA_Hover, true);
            westBar->addTab(tr("Toolbox"));
            westBar->addTab(tr("Skins (Active)"));
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth); // Strict fixed width binds scroll buttons [74]
            westVBox->addWidget(westBar);
            westVBox->addStretch();
            pairLayout->addLayout(westVBox);
        }

        // 2. Right (East)
        {
            auto *eastVBox = new QVBoxLayout();
            eastVBox->addWidget(new QLabel(tr("East (Right)"), pairGroupBox));
            auto *eastBar = new QTabBar(pairGroupBox);
            eastBar->setShape(QTabBar::RoundedEast);
            eastBar->setTabsClosable(true);
            eastBar->setAttribute(Qt::WA_Hover, true);
            eastBar->addTab(tr("Layers"));
            eastBar->addTab(tr("Blocks (Active)"));
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth); // Unified minimum width [74]
            eastVBox->addWidget(eastBar);
            eastVBox->addStretch();
            pairLayout->addLayout(eastVBox);
        }

        // 3. Left Scrollable (West Scrollable)
        {
            auto *westScrollVBox = new QVBoxLayout();
            westScrollVBox->addWidget(new QLabel(tr("West (Scrollable)"), pairGroupBox));
            auto *westBar = new QTabBar(pairGroupBox);
            westBar->setShape(QTabBar::RoundedWest);
            westBar->setTabsClosable(true);
            westBar->setUsesScrollButtons(true);
            westBar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                westBar->addTab(tr("Tool %1").arg(i));
            }
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth);
            westBar->setMaximumHeight(vertTabMaxHeight);
            westScrollVBox->addWidget(westBar);
            westScrollVBox->addStretch();
            pairLayout->addLayout(westScrollVBox);
        }

        // 4. Right Scrollable (East Scrollable)
        {
            auto *eastScrollVBox = new QVBoxLayout();
            eastScrollVBox->addWidget(new QLabel(tr("East (Scrollable)"), pairGroupBox));
            auto *eastBar = new QTabBar(pairGroupBox);
            eastBar->setShape(QTabBar::RoundedEast);
            eastBar->setTabsClosable(true);
            eastBar->setUsesScrollButtons(true);
            eastBar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                eastBar->addTab(tr("Prop %1").arg(i));
            }
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth);
            eastBar->setMaximumHeight(vertTabMaxHeight);
            eastScrollVBox->addWidget(eastBar);
            eastScrollVBox->addStretch();
            pairLayout->addLayout(eastScrollVBox);
        }

        rightColLayout->addWidget(pairGroupBox);
    }

    // Group Box 4: Beveled Vertical combination (4 columns side-by-side)
    {
        auto *pairGroupBox = new QGroupBox(tr("Beveled Vertical Previews (Scrollable)"), rightColWidget);
        auto *pairLayout = new QHBoxLayout(pairGroupBox);
        pairLayout->setContentsMargins(8, 8, 8, 8);
        pairLayout->setSpacing(12);

        // 1. Left (West)
        {
            auto *westVBox = new QVBoxLayout();
            westVBox->addWidget(new QLabel(tr("West (Left)"), pairGroupBox));
            auto *westBar = new QTabBar(pairGroupBox);
            westBar->setShape(QTabBar::TriangularWest);
            westBar->setTabsClosable(true);
            westBar->setAttribute(Qt::WA_Hover, true);
            westBar->addTab(tr("CAD Tools"));
            westBar->addTab(tr("Viewport (Active)"));
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth); // Unified minimum width [74]
            westVBox->addWidget(westBar);
            westVBox->addStretch();
            pairLayout->addLayout(westVBox);
        }

        // 2. Right (East)
        {
            auto *eastVBox = new QVBoxLayout();
            eastVBox->addWidget(new QLabel(tr("East (Right)"), pairGroupBox));
            auto *eastBar = new QTabBar(pairGroupBox);
            eastBar->setShape(QTabBar::TriangularEast);
            eastBar->setTabsClosable(true);
            eastBar->setAttribute(Qt::WA_Hover, true);
            eastBar->addTab(tr("Layout A"));
            eastBar->addTab(tr("Layout B (Active)"));
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth); // Unified minimum width [74]
            eastVBox->addWidget(eastBar);
            eastVBox->addStretch();
            pairLayout->addLayout(eastVBox);
        }

        // 3. Left Scrollable (West Scrollable)
        {
            auto *westScrollVBox = new QVBoxLayout();
            westScrollVBox->addWidget(new QLabel(tr("West (Scrollable)"), pairGroupBox));
            auto *westBar = new QTabBar(pairGroupBox);
            westBar->setShape(QTabBar::TriangularWest);
            westBar->setTabsClosable(true);
            westBar->setUsesScrollButtons(true);
            westBar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                westBar->addTab(tr("Model %1").arg(i));
            }
            westBar->setCurrentIndex(1);
            westBar->setFixedWidth(vertTabMinWidth); // Unified minimum width [74]
            westBar->setMaximumHeight(vertTabMaxHeight);
            westScrollVBox->addWidget(westBar);
            westScrollVBox->addStretch();
            pairLayout->addLayout(westScrollVBox);
        }

        // 4. Right Scrollable (East Scrollable)
        {
            auto *eastScrollVBox = new QVBoxLayout();
            eastScrollVBox->addWidget(new QLabel(tr("East (Scrollable)"), pairGroupBox));
            auto *eastBar = new QTabBar(pairGroupBox);
            eastBar->setShape(QTabBar::TriangularEast);
            eastBar->setTabsClosable(true);
            eastBar->setUsesScrollButtons(true);
            eastBar->setAttribute(Qt::WA_Hover, true);
            for (int i = 1; i <= 16; ++i) {
                eastBar->addTab(tr("Sheet %1").arg(i));
            }
            eastBar->setCurrentIndex(1);
            eastBar->setFixedWidth(vertTabMinWidth); // Unified minimum width [74]
            eastBar->setMaximumHeight(vertTabMaxHeight);
            eastScrollVBox->addWidget(eastBar);
            eastScrollVBox->addStretch();
            pairLayout->addLayout(eastScrollVBox);
        }

        rightColLayout->addWidget(pairGroupBox);
    }

    rightColLayout->addStretch();
    mainGridLayout->addWidget(rightColWidget, 0, 1);

    // Apply strict proportional stretches: Left column 45%, Right column 55%
    mainGridLayout->setColumnStretch(0, 45);
    mainGridLayout->setColumnStretch(1, 55);

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
