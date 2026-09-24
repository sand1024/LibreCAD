/* ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * ********************************************************************************
 */

#include "lc_widget_factory.h"

#include <QStatusBar>
#include <QToolBar>

#include "lc_action.h"
#include "lc_action_group_manager.h"
#include "lc_anglesbasiswidget.h"
#include "lc_caddockwidget.h"
#include "lc_cad_tool_matrix_dock_widget.h"
#include "lc_custom_title_bar_widget.h"
#include "lc_dockwidget.h"
#include "lc_layertreewidget.h"
#include "lc_namedviewslistwidget.h"
#include "lc_penpalettewidget.h"
#include "lc_penwizard.h"
#include "lc_propertysheetwidget.h"
#include "lc_proxy_style_shared.h"
#include "lc_qtstatusbarmanager.h"
#include "lc_quickinfowidget.h"
#include "lc_relzerocoordinateswidget.h"
#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "lc_ucslistwidget.h"
#include "lc_ucsstatewidget.h"
#include "qc_applicationwindow.h"
#include "qg_activelayername.h"
#include "qg_blockwidget.h"
#include "qg_commandwidget.h"
#include "qg_coordinatewidget.h"
#include "qg_layerwidget.h"
#include "qg_librarywidget.h"
#include "qg_mousewidget.h"
#include "qg_pentoolbar.h"
#include "qg_selectionwidget.h"
#include "qg_snaptoolbar.h"
#include "rs_debug.h"
#include "rs_settings.h"
#include "twostackedlabels.h"

LC_WidgetFactory::LC_WidgetFactory(QC_ApplicationWindow* mainWin)
    : QObject(nullptr), LC_AppWindowAware(mainWin), m_agm(mainWin->m_actionGroupManager.get()),
      m_actionFactory{mainWin->m_actionFactory.get()} {
}

void LC_WidgetFactory::updateDockOptions(QC_ApplicationWindow* mainWin, const bool allowDockNesting, const bool verticalTabs) {
    auto dockOptions = QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks;
    if (allowDockNesting) {
        dockOptions |= QMainWindow::AllowNestedDocks;
    }
    if (verticalTabs) {
        dockOptions |= QMainWindow::VerticalTabs;
    }

    mainWin->setDockOptions(dockOptions);
}

void LC_WidgetFactory::initWidgets() {
    initStatusBar();
    initLeftCADSidebar();
    createRightSidebar(m_appWin->m_actionHandler.get());
    initSpecialToolbars();
}

void LC_WidgetFactory::initLeftCADSidebar() {
    const bool enable_left_sidebar = CFG_Startup::o_EnableLeftSidebar;
    if (enable_left_sidebar) {
        using namespace CFG_Widgets;

        const int leftSidebarAllColumnsCount = o_LeftToolbarAllColumnsCount;
        const int leftSidebarAllIconSize = o_LeftToolbarAllIconSize;
        const bool flatIconsAll = o_LeftToolbarAllFlatIcons;
        createCADMegaSidebar(leftSidebarAllColumnsCount, leftSidebarAllIconSize, flatIconsAll);

        const int leftSidebarColumnsCount = o_LeftToolbarColumnsCount;
        const int leftSidebarIconSize = o_LeftToolbarIconSize;
        const bool flatIcons = o_LeftToolbarFlatIcons;
        createCADSidebar(leftSidebarColumnsCount, leftSidebarIconSize, flatIcons);
    }
}

void LC_WidgetFactory::createCADMegaSidebar(const int columns, const int iconSize, const bool flatButtons) {
    auto* result = new LC_CADToolMatrixDockWidget(m_appWin, true);
    result->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    result->setObjectName("dock_cad_mega");
    result->setWindowTitle(tr("Tools"));
    result->setProperty(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET, true);

    auto iconPath = ":/icons/line_polygon_star.lci";
    auto title = tr("CAD Tools Matrix");
    auto* titleBar = new LC_CustomTitleBarWidget(tr("Matrix"), title, iconPath, result);
    result->setTitleBarWidget(titleBar);
    result->hide();
    result->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    auto toggleViewAction = result->toggleViewAction();
    QIcon icon(iconPath);
    toggleViewAction->setIcon(icon);
    result->setWindowIcon(icon);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, result, &LC_CADDockWidget::updateWidgetSettings);
    const QString actionName = QString("ToggleDock_CAD_All");
    registerDockWidgetAction("cad_dock_widgets", result, actionName, tr("%1 (CAD Dock)").arg(title), iconPath,
                             tr("Toggles visibility of %1 CAD tool window.").arg(title));

    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, result);
}

void LC_WidgetFactory::createCADSidebar(const int columns, const int iconSize, const bool flatButtons) {
    auto* line = cadDockWidget("line");
    auto* point = cadDockWidget("point");
    auto* shape = cadDockWidget("shape");
    auto* circle = cadDockWidget("circle");
    auto* curve = cadDockWidget("curve");
    auto* spline = cadDockWidget("spline");
    auto* ellipse = cadDockWidget("ellipse");
    auto* polyline = cadDockWidget("polyline");
    auto* select = cadDockWidget("select");
    auto* text = cadDockWidget("text");
    auto* dimension = cadDockWidget("dimension");
    auto* other = cadDockWidget("other");
    auto* modify = cadDockWidget("modify");
    auto* info = cadDockWidget("info");
    auto* order = cadDockWidget("order");

    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, line);
    m_appWin->tabifyDockWidget(line, polyline);
    m_appWin->tabifyDockWidget(polyline, point);
    m_appWin->tabifyDockWidget(polyline, shape);
    line->raise();
    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, circle);
    m_appWin->tabifyDockWidget(circle, curve);
    m_appWin->tabifyDockWidget(curve, spline);
    m_appWin->tabifyDockWidget(spline, ellipse);
    circle->raise();
    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, dimension);
    m_appWin->tabifyDockWidget(dimension, other);
    m_appWin->tabifyDockWidget(other, info);
    m_appWin->tabifyDockWidget(info, select);
    dimension->raise();
    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, modify);
    m_appWin->tabifyDockWidget(modify, order);
}

QDockWidget* LC_WidgetFactory::createDockWidget(const QString& horizontalTitle, const char* name, const QString& iconName,
                                                const QString& verticalTitle, const char* toggleActionName,
                                                const QString& toggleActionDescrition) const {
    const auto result = new LC_DockWidget(m_appWin, horizontalTitle, verticalTitle);
    result->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    result->setWindowTitle(horizontalTitle);
    result->setObjectName(name);
    result->setProperty("_lc_doc_widget", true);
    auto toggleViewAction = result->toggleViewAction();
    if (!iconName.isEmpty()) {
        toggleViewAction->setIcon(QIcon(iconName));
    }

    auto* titleBar = new LC_CustomTitleBarWidget(horizontalTitle, verticalTitle, iconName, result);
    result->setTitleBarWidget(titleBar);

    registerDockWidgetAction("dock_widgets", result, toggleActionName, horizontalTitle, iconName, toggleActionDescrition);
    return result;
}

QDockWidget* LC_WidgetFactory::createPenPalletteWidget() {
    const auto dock = createDockWidget(tr("Pens Palette"), "pen_palette_dockwidget", ":/icons/widget_pens_palette.lci", tr("Pens"),
                                       LC_ActionNames::ToggleDockPenPalette, tr("Toggles visibility of Pens Palette tool window."));
    const auto widget = new LC_PenPaletteWidget("PenPalette", dock);
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(widget, &LC_PenPaletteWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_PenPaletteWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);

    m_appWin->m_penPaletteWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLayerWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Layers"), "layer_dockwidget", ":/icons/widget_layer_list.lci", tr("Layers"),
                                       LC_ActionNames::ToggleDockLayers, tr("Toggles visibility of Layers tool window."));
    const auto widget = new QG_LayerWidget(m_agm, actionHandler, dock, "Layer");
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(widget, &QG_LayerWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &QG_LayerWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_layerWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createNamedViewsWidget() {
    const auto dock = createDockWidget(tr("Named Views"), "view_dockwidget", ":/icons/widget_views.lci", tr("Views"),
                                       LC_ActionNames::ToggleDockNamedViews, tr("Toggles visibility of Named Views tool window."));
    const auto widget = new LC_NamedViewsListWidget("View", dock);
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_NamedViewsListWidget::updateWidgetSettings);
    connect(m_appWin->m_ucsListWidget, &LC_UCSListWidget::ucsListChanged, widget, &LC_NamedViewsListWidget::onUcsListChanged);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_namedViewsWidget = widget;

    QC_ApplicationWindow* win = m_appWin;

    connect(widget, &LC_NamedViewsListWidget::viewListChanged, [win](const int itemsCount) {
        win->enableAction("ZoomViewRestore1", itemsCount > 0);
        win->enableAction("ZoomViewRestore2", itemsCount > 1);
        win->enableAction("ZoomViewRestore3", itemsCount > 2);
        win->enableAction("ZoomViewRestore4", itemsCount > 3);
        win->enableAction("ZoomViewRestore5", itemsCount > 4);
    });
    return dock;
}

QDockWidget* LC_WidgetFactory::createUCSListWidget() {
    const auto dock = createDockWidget(tr("User Coordinate Systems"), "ucs_dockwidget", ":/icons/widget_ucs.lci", tr("UCSs"),
                                       LC_ActionNames::ToggleDockUCS, tr("Toggles visibility of UCS tool window."));
    const auto widget = new LC_UCSListWidget("UCS", dock);
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_UCSListWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_ucsListWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLayerTreeWidget(const QG_ActionHandler* actionHandler) {
    QDockWidget* dock = createDockWidget(tr("Layers Tree"), "layer_tree_dockwidget", ":/icons/widget_layer_tree.lci", tr("Layers Tree"),
                                         LC_ActionNames::ToggleDockLayerTree, tr("Toggles visibility of Layers Tree tool window."));
    const auto widget = new LC_LayerTreeWidget(actionHandler, dock, "Layer Tree");
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(widget, &LC_LayerTreeWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_LayerTreeWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_layerTreeWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createEntityInfoWidget() {
    QDockWidget* dock = createDockWidget(tr("Entity Info"), "quick_entity_info", ":/icons/widget_info.lci", tr("Info"),
                                         LC_ActionNames::ToggleDockQuickInfo, tr("Toggles visibility of Entity Info tool window."));
    const auto widget = new LC_QuickInfoWidget(dock, m_agm->getActionsMap());
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_QuickInfoWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_quickInfoWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createPropertySheetWidget() {
    QDockWidget* dock = createDockWidget(tr("Properties"), "property_sheet", ":/icons/widget_properties.lci", tr("Properties"),
                                         LC_ActionNames::ToggleDockProperties, tr("Toggles visibility of Properties tool window."));

    const auto widget = new LC_PropertySheetWidget(dock, m_appWin->getActionContext(), m_agm);
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_PropertySheetWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::visibilityChanged, widget, &LC_PropertySheetWidget::onDockVisibilityChanged);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_propertySheetWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createBlockListWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Blocks"), "block_dockwidget", ":/icons/widget_blocks.lci", tr("Blocks"),
                                       LC_ActionNames::ToggleDockBlocks, tr("Toggles visibility of Blocks tool window."));

    const auto widget = new QG_BlockWidget(m_agm, actionHandler, dock, "Block");
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(widget, &QG_BlockWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &QG_BlockWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);

    m_appWin->m_blockWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLibraryWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Library Browser"), "library_dockwidget", ":/icons/widget_library.lci", tr("Library"),
                                       LC_ActionNames::ToggleDockLibrary, tr("Toggles visibility of Library tool window."));

    const auto widget = new QG_LibraryWidget(actionHandler, dock, "Library");
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    // result->resize(240, 400);

    connect(widget, &QG_LibraryWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &QG_LibraryWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_libraryWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createCmdWidget(QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Command Line"), "command_dockwidget", ":/icons/widget_cmd.lci", tr("Cmd"),
                                       LC_ActionNames::ToggleDockCommandLine, tr("Toggles visibility of Command Line tool window."));

    const auto widget = new QG_CommandWidget(actionHandler, dock, "Command");
    widget->setActionHandler(actionHandler);

    dock->setWidget(widget);
    widget->getDockingAction()->setText(dock->isFloating() ? tr("Dock") : tr("Float"));

    connect(widget->leCommand, &QG_CommandEdit::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    // fixme - sand - disable setting vertical caption so far as this is now controlled in uniform way by widget
    // setttings.
    // fixme - sand - remove this call and the slot later, if there will no request from the users to recover this
    // connect(dock, &QDockWidget::dockLocationChanged,m_appWin, &QC_ApplicationWindow::modifyCommandTitleBar);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_commandWidget = widget;
    return dock;
}

/**
 * This slot modifies the commandline's title bar
 * depending on the dock area it is moved to.
 */
// fixme - sand - files - remove later, just port from ApppWindow - use uniform way
void LC_WidgetFactory::modifyCommandTitleBar(const Qt::DockWidgetArea area) const {
    auto* cmdDockWidget = findChild<QDockWidget*>("command_dockwidget");

    const auto* commandWidget = static_cast<QG_CommandWidget*>(cmdDockWidget->widget());
    QAction* dockingAction = commandWidget->getDockingAction();
    const bool docked = area & Qt::AllDockWidgetAreas;
    cmdDockWidget->setWindowTitle(docked ? tr("Cmd") : tr("Command Line"));
    dockingAction->setText(docked ? tr("Float") : tr("Dock", "Dock the command widget to the main window"));
    QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable;

    if (docked) {
        features |= QDockWidget::DockWidgetVerticalTitleBar;
    }
    cmdDockWidget->setFeatures(features);
}

void LC_WidgetFactory::updateDockWidgetsTitleBarType(const QC_ApplicationWindow* mainWin, const bool verticalTitle) {
    QList<QDockWidget*> dockwidgetsList = mainWin->findChildren<QDockWidget*>();
    for (QDockWidget* dw : std::as_const(dockwidgetsList)) {
        if (dw->property("_lc_doc_widget").isValid()) {
            setDockWidgetTitleType(dw, verticalTitle);
        }
    }
}

void LC_WidgetFactory::dockAndTabifyGroup(QC_ApplicationWindow* mainWin, const Qt::DockWidgetArea area,
                                          const QList<QDockWidget*>& docks, QDockWidget* toRaise) {
    if (mainWin == nullptr || docks.isEmpty()) {
        return;
    }

    QDockWidget* anchor = nullptr;
    for (auto* dw : docks) {
        if (dw == nullptr) {
            continue;
        }

        mainWin->addDockWidget(area, dw);
        if (anchor == nullptr) {
            anchor = dw;
        }
        else {
            mainWin->tabifyDockWidget(anchor, dw);
        }
    }

    if (toRaise != nullptr) {
        toRaise->raise();
    }
}

void LC_WidgetFactory::dockAndTabifyByName(QC_ApplicationWindow* mainWin, const Qt::DockWidgetArea area,
                                           const std::vector<const char*>& names, const char* raiseName) {
    if (mainWin == nullptr) {
        return;
    }

    QList<QDockWidget*> docks;
    QDockWidget* toRaise = nullptr;

    for (const char* name : names) {
        auto* dw = mainWin->findChild<QDockWidget*>(name);
        if (dw != nullptr) {
            docks.append(dw);
            if (raiseName != nullptr && strcmp(name, raiseName) == 0) {
                toRaise = dw;
            }
        }
    }

    dockAndTabifyGroup(mainWin, area, docks, toRaise);
}

void LC_WidgetFactory::createRightSidebar(QG_ActionHandler* actionHandler) {
    const bool verticalTitle = CFG_Widgets::o_DockTitleBarVertical;

    const QList<QDockWidget*> rightDocks = {
        createLibraryWidget(actionHandler),
        createBlockListWidget(actionHandler),
        createPenWizardWidget(),
        createPenPalletteWidget(),
        createLayerTreeWidget(actionHandler),
        createLayerWidget(actionHandler),
        createEntityInfoWidget(),
        createPropertySheetWidget(),
        createUCSListWidget(),
        createNamedViewsWidget(),
        createCmdWidget(actionHandler)
    };

    dockAndTabifyGroup(m_appWin, Qt::RightDockWidgetArea, rightDocks);

    updateDockWidgetsTitleBarType(m_appWin, verticalTitle);

    // Only resize when the app is opened for the first time
    initializeRightDockWidgets();
}

void LC_WidgetFactory::initializeRightDockWidgets() const {
    LC_GROUP_GUARD("Geometry");
    if (!LC_GET_STR("WindowGeometry").isEmpty()) {
        // No-op, if previous Window geometry is found
        return;
    }

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        if (dock != nullptr && dock->dockLocation() == Qt::RightDockWidgetArea) {
#else
            if (dock != nullptr && m_appWin->dockWidgetArea(dock) == Qt::RightDockWidgetArea) {

#endif
            dock->resize(390, dock->height());
        }
    }
}

// fixme - sand - remove this method
void LC_WidgetFactory::makeActionsInvisible(const std::vector<QString>& actionNames) const {
    for (const QString& actionName : actionNames) {
        QAction* action = m_agm->getActionByName(actionName);
        if (action != nullptr) {
            action->setVisible(false);
        }
    }
}

// fixme - sand - remove this method
void LC_WidgetFactory::addAction(QToolBar* toolbar, const char* actionName) const {
    QAction* action = m_agm->getActionByName(actionName);
    if (action != nullptr) {
        toolbar->addAction(action);
    }
}

QDockWidget* LC_WidgetFactory::createPenWizardWidget() {
    const auto dock = createDockWidget(tr("Pen Wizard"), "pen_wiz_dockwidget", ":/icons/widget_pen_wiz.lci", tr("PenWiz"),
                                       LC_ActionNames::ToggleDockPenWizard, tr("Toggles visibility of Pens Wizard tool window."));
    const auto widget = new LC_PenWizard(dock);
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    // connect(widget, &LC_PenPaletteWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    // connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_PenPaletteWidget::updateWidgetSettings);
    connect(m_appWin, &QC_ApplicationWindow::windowsChanged, widget, &LC_PenWizard::setEnabled);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
    m_appWin->m_penWizard = widget;
    return dock;
}

void LC_WidgetFactory::initSpecialToolbars() {
    constexpr QSizePolicy tbPolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // 1. Pen Toolbar
    const auto penTitle = tr("Pen");
    auto* penTb = new QG_PenToolBar(penTitle, m_appWin);
    penTb->setSizePolicy(tbPolicy);
    penTb->setObjectName("pen_toolbar");
    penTb->setProperty("_group", 1);
    penTb->setProperty("_lc_toolbar_name", "pen");
    penTb->toggleViewAction()->setIcon(QIcon(":/icons/pen_apply.lci"));
    m_appWin->m_penToolBar = penTb;

    connect(penTb, &QG_PenToolBar::penChanged, m_appWin, &QC_ApplicationWindow::slotPenChanged);
    connect(penTb, &QG_PenToolBar::penChanged, m_appWin->getPropertySheetWidget(), &LC_PropertySheetWidget::onActivePenChanged);

    // 2. Snap Selection Toolbar
    const auto snapTitle = tr("Snap Selection");
    auto* snapTb = new QG_SnapToolBar(m_appWin, m_appWin->m_actionHandler.get(), m_agm, m_agm->getActionsMap());
    snapTb->setWindowTitle(snapTitle);
    snapTb->setSizePolicy(tbPolicy);
    snapTb->setObjectName("snap_toolbar");
    snapTb->setProperty("_group", 3);
    snapTb->setProperty("_lc_toolbar_name", "snap");
    snapTb->toggleViewAction()->setIcon(QIcon(":/icons/snap_visual.lci"));
    m_appWin->m_snapToolBar = snapTb;

    // 3. Tool Options Toolbar (Container for interactive CAD options)
    auto* optTb = new QToolBar(tr("Tool Options"), m_appWin);
    optTb->setSizePolicy(tbPolicy);
    optTb->setObjectName("tool_options_toolbar");
    optTb->setProperty("_lc_toolbar_name", "tool_options");
    optTb->setProperty("_group", 1);
    m_appWin->m_toolOptionsToolbar = optTb;
}

void LC_WidgetFactory::setDockWidgetTitleType(QDockWidget* widget, const bool verticalTitleBar) {
    QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable;

    if (verticalTitleBar) {
        features |= QDockWidget::DockWidgetVerticalTitleBar;
    }
    widget->setFeatures(features);
    // const auto lcDocWidget = dynamic_cast<LC_DockWidget*>(widget);
    // if (lcDocWidget != nullptr) {
    // lcDocWidget->updateTitle();
    // }
}

void LC_WidgetFactory::registerDockWidgetAction(const char* groupName, QDockWidget* dockWidget, const QString& actionName,
                                                const QString& title, const QString& iconPath, const QString& description) const {
    if (m_agm == nullptr || dockWidget == nullptr || actionName.isEmpty()) {
        return;
    }

    auto* group = m_agm->getActionGroup(groupName);
    if (group == nullptr) {
        return;
    }

    QAction* toggleAct = dockWidget->toggleViewAction();
    toggleAct->setObjectName(actionName);
    toggleAct->setText(title);
    if (!iconPath.isEmpty()) {
        toggleAct->setIcon(QIcon(iconPath));
    }

    toggleAct->setProperty(LC_ActionKeys::PROP_DESCRIPTION, description);

    group->addAction(toggleAct);
    m_agm->getActionsMap().insert(actionName, toggleAct);
}

LC_CADDockWidget* LC_WidgetFactory::cadDockWidget(const QString& groupName) {
    const auto* group = (m_agm != nullptr) ? m_agm->getActionGroup(groupName) : nullptr;
    const QString title = (group != nullptr) ? group->cleanTitle() : groupName;
    const QString iconName = (group != nullptr) ? group->getIconPath() : QString();

    auto* result = new LC_CADDockWidget(m_appWin);
    result->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    result->setObjectName("dock_" + groupName.toLower());
    result->setWindowTitle(title);
    result->hide();

    result->setProperty(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET, true);
    if (!iconName.isEmpty()) {
        QIcon icon = QIcon(iconName);
        setWidgetToggleActionIcon(result, icon);
        result->setWindowIcon(icon);
    }
    auto* titleBar = new LC_CustomTitleBarWidget(title, title, iconName, result);
    result->setTitleBarWidget(titleBar);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, result, &LC_CADDockWidget::updateWidgetSettings);

    // Dynamic registration in cad_dock_widgets action group
    const QString actionName = QString("ToggleDock_CAD_") + groupName;
    registerDockWidgetAction("cad_dock_widgets", result, actionName, tr("%1 (CAD Dock)").arg(title), iconName,
                             tr("Toggles visibility of %1 CAD tool window.").arg(title));
    return result;
}

QToolBar* LC_WidgetFactory::createStatusBarToolbar(const QSizePolicy& tbPolicy, QWidget* widget, const QString& title,
                                                   const QString& iconName, const char* name, const bool showToolTip,
                                                   bool usePillChips) const {
    const auto tb = new QToolBar(title, m_appWin);
    tb->setSizePolicy(tbPolicy);
    tb->addWidget(widget);
    tb->setObjectName(name);
    tb->setProperty("_group", 3);
    tb->setProperty("_lc_toolbar_name", name);
    if (showToolTip) {
        tb->setToolTip(tr("Toolbar: %1").arg(title));
    }
    if (usePillChips) {
        tb->setProperty(PROP_USE_STATUS_PILL_CHIPS, true);
    }
    setWidgetToggleActionIcon(tb, iconName);
    addToBottom(tb);
    // fixme - sand - registration of action for toggle!!
    return tb;
}

void LC_WidgetFactory::initStatusBar() {
    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: init status bar");
    QStatusBar* status_bar = m_appWin->statusBar();

    m_appWin->m_coordinateWidget = new QG_CoordinateWidget(status_bar, "coordinates");
    m_appWin->m_relativeZeroCoordinatesWidget = new LC_RelZeroCoordinatesWidget(status_bar, "relZeroCordinates");
    m_appWin->m_mouseWidget = new QG_MouseWidget(status_bar, "mouse info");
    m_appWin->m_selectionWidget = new QG_SelectionWidget(status_bar, "selections");
    m_appWin->m_activeLayerNameWidget = new QG_ActiveLayerName(status_bar);

    m_appWin->m_gridStatusWidget = new TwoStackedLabels(status_bar);
    m_appWin->m_gridStatusWidget->setTopLabel(tr("Grid Status"));

    auto* ucsStateWidget = new LC_UCSStateWidget(status_bar, "ucs");
    m_appWin->m_ucsStateWidget = ucsStateWidget;

    auto* anglesBasisWidget = new LC_AnglesBasisWidget(status_bar, "anglesBase");
    m_appWin->m_anglesBasisWidget = anglesBasisWidget;

    m_appWin->m_statusbarManager = new LC_QTStatusbarManager(status_bar);
    m_appWin->m_statusbarManager->loadSettings();

    const bool useClassicalStatusBar = CFG_Startup::o_UseClassicStatusBar;
    if (useClassicalStatusBar) {
        status_bar->addWidget(m_appWin->m_coordinateWidget);
        status_bar->addWidget(m_appWin->m_mouseWidget);
        status_bar->addWidget(m_appWin->m_selectionWidget);
        status_bar->addWidget(m_appWin->m_activeLayerNameWidget);
        status_bar->addWidget(m_appWin->m_gridStatusWidget);
        status_bar->addWidget(m_appWin->m_relativeZeroCoordinatesWidget);
        status_bar->addWidget(m_appWin->m_ucsStateWidget);
        status_bar->addWidget(m_appWin->m_anglesBasisWidget);

        {
            using namespace CFG_Widgets;
            const bool allow_statusbar_fontsize = o_AllowStatusbarFontSize;
            const bool allow_statusbar_height = o_AllowStatusbarHeight;

            if (allow_statusbar_fontsize) {
                const int fontsize = o_StatusbarFontSize;
                QFont font;
                font.setPointSize(fontsize);
                status_bar->setFont(font);
            }
            int height{64};
            if (allow_statusbar_height) {
                height = o_StatusbarHeight;
            }
            status_bar->setMinimumHeight(height);
            status_bar->setMaximumHeight(height);
        }
    }
    else {
        constexpr QSizePolicy tbPolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        const bool showToolbarTooltips = CFG_Startup::o_ShowToolbarsTooltip;
        createStatusBarToolbar(tbPolicy, m_appWin->m_coordinateWidget, tr("Coordinates"), ":/icons/info_point.lci", "TBCoordinates",
                               showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_relativeZeroCoordinatesWidget, tr("Relative Zero"), ":/icons/set_rel_zero.lci",
                               "TBRelZero", showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_mouseWidget, tr("Mouse"), ":/icons/mouse.lci", "TBMouse", showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_selectionWidget, tr("Selection Info"), ":/icons/select_conditional.lci",
                               "TBSelectionInfo", showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_activeLayerNameWidget, tr("Active Layer"), ":/icons/item_by_layer.lci",
                               "TBActiveLayer", showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_gridStatusWidget, tr("Grid Status"), ":/icons/grid.lci", "TBGridStatus",
                               showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_ucsStateWidget, tr("UCS Status"), ":/icons/ucs_ucs.lci", "TBUCSStatus",
                               showToolbarTooltips, true);
        createStatusBarToolbar(tbPolicy, m_appWin->m_anglesBasisWidget, tr("Angles Basis"), ":/icons/dirpos.lci", "TBAnglesBasis",
                               showToolbarTooltips, true);

        m_appWin->m_statusbarManager->setup();

        m_appWin->m_gridStatusWidget->setToolTip(tr("Current size of Grid/MetaGrid. Click to change grid size."));
        connect(m_appWin->m_gridStatusWidget, &TwoStackedLabels::clicked, m_appWin, &QC_ApplicationWindow::slotShowDrawingOptions);
    }
    connect(m_appWin->m_anglesBasisWidget, &LC_AnglesBasisWidget::clicked, m_appWin, &QC_ApplicationWindow::slotShowDrawingOptionsUnits);

    connect(m_appWin, &QC_ApplicationWindow::iconsRefreshed, m_appWin->m_ucsStateWidget, &LC_UCSStateWidget::onIconsRefreshed);
    connect(m_appWin, &QC_ApplicationWindow::iconsRefreshed, m_appWin->m_anglesBasisWidget, &LC_AnglesBasisWidget::onIconsRefreshed);
    connect(m_appWin, &QC_ApplicationWindow::iconsRefreshed, m_appWin->m_mouseWidget, &QG_MouseWidget::onIconsRefreshed);

    connect(m_appWin, &QC_ApplicationWindow::currentActionIconChanged, m_appWin->m_mouseWidget, &QG_MouseWidget::setCurrentQAction);
    connect(m_appWin, &QC_ApplicationWindow::currentActionIconChanged, m_appWin->m_statusbarManager,
            &LC_QTStatusbarManager::setCurrentQAction);

    const bool statusBarVisible = CFG_Appearance::o_StatusBarVisible;
    status_bar->setVisible(statusBarVisible);
}

void LC_WidgetFactory::addToBottom(QToolBar* toolbar) const {
    m_appWin->addToolBar(Qt::BottomToolBarArea, toolbar);
}

void LC_WidgetFactory::redockAllDockWidgets(QC_ApplicationWindow* mainWin) {
    if (mainWin == nullptr) {
        return;
    }

    // 1. Left CAD Sidebar Groups
    dockAndTabifyByName(mainWin, Qt::LeftDockWidgetArea,
                        {"dock_line", "dock_polyline", "dock_point", "dock_shape"}, "dock_line");
    dockAndTabifyByName(mainWin, Qt::LeftDockWidgetArea,
                        {"dock_circle", "dock_curve", "dock_spline", "dock_ellipse"}, "dock_circle");
    dockAndTabifyByName(mainWin, Qt::LeftDockWidgetArea,
                        {"dock_dimension", "dock_other", "dock_info", "dock_select"}, "dock_dimension");
    dockAndTabifyByName(mainWin, Qt::LeftDockWidgetArea,
                        {"dock_modify", "dock_order"});

    // 2. Right Sidebar (Full Tabified Stack)
    dockAndTabifyByName(mainWin, Qt::RightDockWidgetArea, {
        "library_dockwidget",
        "block_dockwidget",
        "pen_wiz_dockwidget",
        "pen_palette_dockwidget",
        "layer_tree_dockwidget",
        "layer_dockwidget",
        "quick_entity_info",
        "property_sheet",
        "ucs_dockwidget",
        "view_dockwidget",
        "command_dockwidget"
    });
}
