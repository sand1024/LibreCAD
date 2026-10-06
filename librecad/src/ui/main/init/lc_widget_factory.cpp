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
#include "lc_dock_names.h"
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
#include "lc_toolbar_names.h"
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

struct StatusBarInfo{
    const char* name;
    QString title;
    QString iconName;
    QWidget* widget;
};

namespace {

    template <class T>
    void setWidgetToggleActionIcon(T* result, const QString& iconName) {
        if (!iconName.isEmpty()) {
            auto toggleAction = result->toggleViewAction();
            toggleAction->setIcon(QIcon(iconName));
        }
    }

    template <class T>
    void setWidgetToggleActionIcon(T* result, const QIcon& icon) {
        auto toggleAction = result->toggleViewAction();
        toggleAction->setIcon(icon);
    }

}

LC_WidgetFactory::LC_WidgetFactory(QC_ApplicationWindow* mainWin)
    : QObject(nullptr), LC_AppWindowAware(mainWin), m_agm(mainWin->m_actionGroupManager.get()),
      m_actionFactory{mainWin->m_actionFactory.get()} {
}

void LC_WidgetFactory::createStatusToolbars(std::initializer_list<StatusBarInfo> toolbars) {
    constexpr QSizePolicy tbPolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    const bool showToolbarTooltips = CFG_Startup::o_ShowToolbarsTooltip;
    for (auto i: toolbars) {
        createStatusBarToolbar(tbPolicy, i.widget, i.title,  i.iconName,  i.name, showToolbarTooltips, true);
    }
}


QToolBar* LC_WidgetFactory::createStatusBarToolbar(const QSizePolicy& tbPolicy, QWidget* widget, const QString& title, const QString& iconName,
                                     const char* name, const bool showToolTip, bool usePillChips) {
    const auto tb = new QToolBar(title, m_appWin);
    tb->setSizePolicy(tbPolicy);
    tb->addWidget(widget);
    tb->setObjectName(LC_ToolbarNames::standardToolBarName(name));
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
    return tb;
}

void LC_WidgetFactory::updateDockOptions(QC_ApplicationWindow* mainWin, const bool allowDockNesting,
                                         const bool cadVerticalTabs, const bool normalVerticalTabs) {
    if (mainWin == nullptr) {
        return;
    }

    auto dockOptions = QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks;
    if (allowDockNesting) {
        dockOptions |= QMainWindow::AllowNestedDocks;
    }
    mainWin->setDockOptions(dockOptions);

    mainWin->setTabPosition(Qt::LeftDockWidgetArea, cadVerticalTabs ? QTabWidget::West : QTabWidget::South);
    mainWin->setTabPosition(Qt::RightDockWidgetArea, normalVerticalTabs ? QTabWidget::East : QTabWidget::South);
}



void LC_WidgetFactory::updateDockWidgetsTitleBarType(const QC_ApplicationWindow* mainWin,
                                                     const bool cadVerticalTitle,
                                                     const bool normalVerticalTitle) {
    if (mainWin == nullptr) {
        return;
    }

    const QList<QDockWidget*> dockwidgetsList = mainWin->findChildren<QDockWidget*>();
    for (QDockWidget* dw : std::as_const(dockwidgetsList)) {
        if (dw == nullptr) {
            continue;
        }

        const bool isCad = LC_CADDockWidget::isCADDockWidget(dw);
        if (isCad) {
            setDockWidgetTitleType(dw, cadVerticalTitle);
            continue;
        }

        const bool isDoc = dw->property("_lc_doc_widget").isValid();

        if (isDoc) {
            setDockWidgetTitleType(dw, isCad ? cadVerticalTitle : normalVerticalTitle);
        }
    }
}


void LC_WidgetFactory::initWidgets() {
    initStatusBar();
    createCADDockWidgetsSidebar();
    createRightSidebar(m_appWin->m_actionHandler.get());
    initSpecialToolbars();
}

void LC_WidgetFactory::createCADDockWidgetsSidebar() {
    const bool enabledCADDockWidgets = CFG_Startup::o_EnableCADDockWidgets;
    if (enabledCADDockWidgets) {
        createCADToolsMatrixSidebar();
        createCADSidebar();
    }

    updateDockWidgetsTitleBarType(m_appWin, CFG_Widgets::o_CADDockWidgetTitleBarVertical, CFG_Widgets::o_DockWidgetTitleBarVertical);
}

void LC_WidgetFactory::createCADToolsMatrixSidebar() {
    auto* result = new LC_CADToolMatrixDockWidget(m_appWin);
    result->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    result->setObjectName(LC_DockNames::cadDockName(LC_DockNames::CAD_MEGA));
    result->setWindowTitle(tr("Tools"));
    result->setProperty(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET, true);

    auto iconPath = ":/icons/line_polygon_star.lci";
    auto title = tr("CAD Tools Matrix");
    auto* titleBar = new LC_CustomTitleBarWidget(tr("Matrix"), title, iconPath, result, []()->bool {
        return CFG_Widgets::o_CADDockWidgetTitleBarVertical;
    });
    result->setTitleBarWidget(titleBar);
    result->hide();
    result->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    auto toggleViewAction = result->toggleViewAction();
    QIcon icon(iconPath);
    toggleViewAction->setIcon(icon);
    result->setWindowIcon(icon);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, result, &LC_CADDockWidget::updateWidgetSettings);
    const QString actionName = QString("ToggleDock_CAD_All");
      registerDockWidgetAction(LC_GroupNames::CAD_DOCK_WIDGETS, result, actionName, tr("%1 (CAD Dock)").arg(title), iconPath,
                             tr("Toggles visibility of %1 CAD tool window.").arg(title));

    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, result);
}

void LC_WidgetFactory::createCADSidebar() {
    auto* line = cadDockWidget(LC_DockNames::CAD_LINE);
    auto* point = cadDockWidget(LC_DockNames::CAD_POINT);
    auto* shape = cadDockWidget(LC_DockNames::CAD_SHAPE);
    auto* circle = cadDockWidget(LC_DockNames::CAD_CIRCLE);
    auto* curve = cadDockWidget(LC_DockNames::CAD_CURVE);
    auto* spline = cadDockWidget(LC_DockNames::CAD_SPLINE);
    auto* ellipse = cadDockWidget(LC_DockNames::CAD_ELLIPSE);
    auto* polyline = cadDockWidget(LC_DockNames::CAD_POLYLINE);
    auto* select = cadDockWidget(LC_DockNames::CAD_SELECT);
    auto* text = cadDockWidget(LC_DockNames::CAD_TEXT);
    auto* dimension = cadDockWidget(LC_DockNames::CAD_DIMENSION);
    auto* other = cadDockWidget(LC_DockNames::CAD_OTHER);
    auto* modify = cadDockWidget(LC_DockNames::CAD_MODIFY);
    auto* info = cadDockWidget(LC_DockNames::CAD_INFO);
    auto* order = cadDockWidget(LC_DockNames::CAD_ORDER);

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
    m_appWin->tabifyDockWidget(other, text);
    m_appWin->tabifyDockWidget(text, info);
    m_appWin->tabifyDockWidget(info, select);
    dimension->raise();
    m_appWin->addDockWidget(Qt::LeftDockWidgetArea, modify);
    m_appWin->tabifyDockWidget(modify, order);
}

QDockWidget* LC_WidgetFactory::createDockWidget(const QString& horizontalTitle, const char* name, const QString& iconName,
                                                const QString& verticalTitle, const char* toggleActionName,
                                                const QString& toggleActionDescription) const {
    const auto result = new LC_DockWidget(m_appWin, horizontalTitle, verticalTitle);
    result->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    result->setWindowTitle(horizontalTitle);
    result->setObjectName(LC_DockNames::standardDockName(name));
    result->setProperty("_lc_doc_widget", true);
    auto toggleViewAction = result->toggleViewAction();
    if (!iconName.isEmpty()) {
        const QIcon icon(iconName);
        toggleViewAction->setIcon(icon);
        result->setWindowIcon(icon);
    }

    auto* titleBar = new LC_CustomTitleBarWidget(horizontalTitle, verticalTitle, iconName, result, []()->bool {
        return CFG_Widgets::o_DockWidgetTitleBarVertical;
    });
    result->setTitleBarWidget(titleBar);

    registerDockWidgetAction(LC_GroupNames::DOCK_WIDGETS, result, toggleActionName, horizontalTitle, iconName, toggleActionDescription);
    return result;
}

void LC_WidgetFactory::setupDockWidget(QDockWidget* dock, LC_GraphicViewAwareWidget* const widget) {
    widget->setFocusPolicy(Qt::NoFocus);
    dock->setWidget(widget);

    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, widget, &LC_GraphicViewAwareWidget::updateWidgetSettings);
    connect(dock, &QDockWidget::dockLocationChanged, widget, &LC_GraphicViewAwareWidget::onDockLocationChanged);
}

QDockWidget* LC_WidgetFactory::createPenPalletteWidget() {
    const auto dock = createDockWidget(tr("Pens Palette"), LC_DockNames::PEN_PALETTE, ":/icons/widget_pens_palette.lci", tr("Pens"),
                                       LC_ActionNames::ToggleDockPenPalette, tr("Toggles visibility of Pens Palette tool window."));
    const auto widget = new LC_PenPaletteWidget("PenPalette", dock);

    setupDockWidget(dock, widget);

    connect(widget, &LC_PenPaletteWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    m_appWin->m_penPaletteWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLayerWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Layers"), LC_DockNames::LAYERS, ":/icons/widget_layer_list.lci", tr("Layers"),
                                       LC_ActionNames::ToggleDockLayers, tr("Toggles visibility of Layers tool window."));
    const auto widget = new QG_LayerWidget(m_agm, actionHandler, dock, "Layer");

    setupDockWidget(dock, widget);

    connect(widget, &QG_LayerWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    m_appWin->m_layerWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createNamedViewsWidget() {
    const auto dock = createDockWidget(tr("Named Views"), LC_DockNames::NAMED_VIEWS, ":/icons/widget_views.lci", tr("Views"),
                                       LC_ActionNames::ToggleDockNamedViews, tr("Toggles visibility of Named Views tool window."));
    const auto widget = new LC_NamedViewsListWidget("View", dock);

    setupDockWidget(dock, widget);

    connect(m_appWin->m_ucsListWidget, &LC_UCSListWidget::ucsListChanged, widget, &LC_NamedViewsListWidget::onUcsListChanged);
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
    const auto dock = createDockWidget(tr("User Coordinate Systems"), LC_DockNames::UCS, ":/icons/widget_ucs.lci", tr("UCSs"),
                                       LC_ActionNames::ToggleDockUCS, tr("Toggles visibility of UCS tool window."));
    const auto widget = new LC_UCSListWidget("UCS", dock);
    setupDockWidget(dock, widget);

    m_appWin->m_ucsListWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLayerTreeWidget(const QG_ActionHandler* actionHandler) {
    QDockWidget* dock = createDockWidget(tr("Layers Tree"), LC_DockNames::LAYER_TREE, ":/icons/widget_layer_tree.lci", tr("Layers Tree"),
                                         LC_ActionNames::ToggleDockLayerTree, tr("Toggles visibility of Layers Tree tool window."));
    const auto widget = new LC_LayerTreeWidget(actionHandler, dock, "Layer Tree");

    setupDockWidget(dock, widget);

    connect(widget, &LC_LayerTreeWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    m_appWin->m_layerTreeWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createEntityInfoWidget() {
    QDockWidget* dock = createDockWidget(tr("Entity Info"), LC_DockNames::ENTITY_INFO, ":/icons/widget_info.lci", tr("Info"),
                                         LC_ActionNames::ToggleDockQuickInfo, tr("Toggles visibility of Entity Info tool window."));
    const auto widget = new LC_QuickInfoWidget(dock, m_agm->getActionsMap());

    setupDockWidget(dock, widget);

    m_appWin->m_quickInfoWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createPropertySheetWidget() {
    QDockWidget* dock = createDockWidget(tr("Properties"), LC_DockNames::PROPERTIES, ":/icons/widget_properties.lci", tr("Properties"),
                                         LC_ActionNames::ToggleDockProperties, tr("Toggles visibility of Properties tool window."));

    const auto widget = new LC_PropertySheetWidget(dock, m_appWin->getActionContext(), m_agm);

    setupDockWidget(dock, widget);

    connect(dock, &QDockWidget::visibilityChanged, widget, &LC_PropertySheetWidget::onDockVisibilityChanged);

    m_appWin->m_propertySheetWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createBlockListWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Blocks"), LC_DockNames::BLOCKS, ":/icons/widget_blocks.lci", tr("Blocks"),
                                       LC_ActionNames::ToggleDockBlocks, tr("Toggles visibility of Blocks tool window."));

    const auto widget = new QG_BlockWidget(m_agm, actionHandler, dock, "Block");

    setupDockWidget(dock, widget);

    connect(widget, &QG_BlockWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);

    m_appWin->m_blockWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createLibraryWidget(const QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Library Browser"), LC_DockNames::LIBRARY, ":/icons/widget_library.lci", tr("Library"),
                                       LC_ActionNames::ToggleDockLibrary, tr("Toggles visibility of Library tool window."));

    const auto widget = new QG_LibraryWidget(actionHandler, dock, "Library");

    setupDockWidget(dock, widget);

    connect(widget, &QG_LibraryWidget::escape, m_appWin, &QC_ApplicationWindow::slotFocus);
    m_appWin->m_libraryWidget = widget;
    return dock;
}

QDockWidget* LC_WidgetFactory::createCmdWidget(QG_ActionHandler* actionHandler) {
    const auto dock = createDockWidget(tr("Command Line"), LC_DockNames::COMMAND, ":/icons/widget_cmd.lci", tr("Cmd"),
                                       LC_ActionNames::ToggleDockCommandLine, tr("Toggles visibility of Command Line tool window."));

    const auto widget = new QG_CommandWidget(actionHandler, dock, "Command");
    widget->setActionHandler(actionHandler);

    dock->setWidget(widget);
    widget->getDockingAction()->setText(dock->isFloating() ? tr("Dock") : tr("Float"));

    connect(widget->leCommand, &QG_CommandEdit::escape, m_appWin, &QC_ApplicationWindow::slotFocus);

    if (auto* dockBase = qobject_cast<LC_DockWidgetBase*>(dock)) {
        dockBase->setFocusTargetWidget(widget->leCommand);
    }

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
    auto* cmdDockWidget = findChild<QDockWidget*>(LC_DockNames::standardDockName(LC_DockNames::COMMAND));
    if (cmdDockWidget == nullptr) {
        return;
    }

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


void LC_WidgetFactory::createRightSidebar(QG_ActionHandler* actionHandler) {
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

    updateDockWidgetsTitleBarType(m_appWin,
                                    CFG_Widgets::o_CADDockWidgetTitleBarVertical,
                                    CFG_Widgets::o_DockWidgetTitleBarVertical);


    // Only resize when the app is opened for the first time
    initializeRightDockWidgets();
}

void LC_WidgetFactory::initializeRightDockWidgets() const {
    LC_GROUP_GUARD("Geometry");
    if (!LC_GET_STR("WindowGeometry").isEmpty()) {
        // No-op, if previous Window geometry is found
        return;
    }

    int preferredWidth = 390;
    if (const auto* screen = QGuiApplication::primaryScreen()) {
        int leftWidth = 0;
        for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
            if (!dock->isFloating() && m_appWin->dockWidgetArea(dock) == Qt::LeftDockWidgetArea && !dock->isHidden()) {
                leftWidth = qMax(leftWidth, dock->sizeHint().width());
            }
        }
        preferredWidth = qMin(preferredWidth,
                              qMax(120, screen->availableGeometry().width() - leftWidth - 480 - 48));
    }

    for (QDockWidget* dock : m_appWin->findChildren<QDockWidget*>()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        if (dock != nullptr && dock->dockLocation() == Qt::RightDockWidgetArea) {
#else
            if (dock != nullptr && m_appWin->dockWidgetArea(dock) == Qt::RightDockWidgetArea) {

#endif
            dock->resize(preferredWidth, dock->height());
        }
    }
}


QDockWidget* LC_WidgetFactory::createPenWizardWidget() {
    const auto dock = createDockWidget(tr("Pen Wizard"), LC_DockNames::PEN_WIZARD, ":/icons/widget_pen_wiz.lci", tr("PenWiz"),
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

void setupSpecialToolbar(QToolBar* toolbar, const char* objectName, int group, QString icon) {
    constexpr QSizePolicy tbPolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    toolbar->setSizePolicy(tbPolicy);
    toolbar->setObjectName(LC_ToolbarNames::standardToolBarName(objectName));
    toolbar->setProperty("_group", group);
    toolbar->setProperty("_lc_toolbar_name", objectName);
    toolbar->toggleViewAction()->setIcon(QIcon(icon));
}

void LC_WidgetFactory::initSpecialToolbars() {
    // 1. Pen Toolbar
    const auto penTitle = tr("Pen");
    auto* penTb = new QG_PenToolBar(penTitle, m_appWin);
    setupSpecialToolbar(penTb, LC_ToolbarNames::PEN, 1, QStringLiteral(":/icons/pen_apply.lci"));
    m_appWin->m_penToolBar = penTb;

    connect(penTb, &QG_PenToolBar::penChanged, m_appWin, &QC_ApplicationWindow::slotPenChanged);
    connect(penTb, &QG_PenToolBar::penChanged, m_appWin->getPropertySheetWidget(), &LC_PropertySheetWidget::onActivePenChanged);

    // 2. Snap Selection Toolbar
    const auto snapTitle = tr("Snap Selection");
    auto* snapTb = new QG_SnapToolBar(m_appWin, m_appWin->m_actionHandler.get(), m_agm, m_agm->getActionsMap());
    snapTb->setWindowTitle(snapTitle);
    setupSpecialToolbar(snapTb, LC_ToolbarNames::SNAP, 3, QStringLiteral(":/icons/snap_visual.lci"));
    m_appWin->m_snapToolBar = snapTb;

    // 3. Tool Options Toolbar (Container for interactive CAD options)
    auto* optTb = new QToolBar(tr("Tool Options"), m_appWin);
    setupSpecialToolbar(optTb, LC_ToolbarNames::TOOL_OPTIONS, 1, QStringLiteral(":/icons/tool_options.lci"));
    m_appWin->m_toolOptionsToolbar = optTb;
}

void LC_WidgetFactory::setDockWidgetTitleType(QDockWidget* widget, const bool verticalTitleBar) {
    QDockWidget::DockWidgetFeatures features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable;

    if (verticalTitleBar) {
        features |= QDockWidget::DockWidgetVerticalTitleBar;
    }
    widget->setFeatures(QDockWidget::DockWidgetFeatures());
    widget->setFeatures(features);
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
    result->setObjectName(LC_DockNames::cadDockName(groupName));
    result->setWindowTitle(title);
    result->hide();

    result->setProperty(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET, true);
    if (!iconName.isEmpty()) {
        QIcon icon = QIcon(iconName);
        setWidgetToggleActionIcon(result, icon);
        result->setWindowIcon(icon);
    }
    auto* titleBar = new LC_CustomTitleBarWidget(title, title, iconName, result, []()->bool {
        return CFG_Widgets::o_CADDockWidgetTitleBarVertical;
    });
    result->setTitleBarWidget(titleBar);
    connect(m_appWin, &QC_ApplicationWindow::widgetSettingsChanged, result, &LC_CADDockWidget::updateWidgetSettings);

    // Dynamic registration in cad_dock_widgets action group
    const QString actionName = QString("ToggleDock_CAD_") + groupName;
    registerDockWidgetAction(LC_GroupNames::CAD_DOCK_WIDGETS, result, actionName, tr("%1 (CAD Dock)").arg(title), iconName,
                             tr("Toggles visibility of %1 CAD tool window.").arg(title));
    return result;
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
            const bool allow_statusbar_fontsize = o_StatusBarAllowFontSize;
            const bool allow_statusbar_height = o_StatusBarAllowHeight;

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
        using namespace  LC_ToolbarNames;
        createStatusToolbars({
            {STAT_COORDINATES,  tr("Coordinates"),   ":/icons/info_point.lci",         m_appWin->m_coordinateWidget},
            {STAT_REL_ZERO,     tr("Relative Zero"), ":/icons/set_rel_zero.lci",       m_appWin->m_relativeZeroCoordinatesWidget},
            {STAT_MOUSE,        tr("Mouse"),         ":/icons/mouse.lci",              m_appWin->m_mouseWidget},
            {STAT_SELECTION,    tr("Selection Info"),":/icons/select_conditional.lci", m_appWin->m_selectionWidget},
            {STAT_ACTIVE_LAYER, tr("Active Layer"),  ":/icons/item_by_layer.lci",      m_appWin->m_activeLayerNameWidget},
            {STAT_GRID_STATUS,  tr("Grid Status"),   ":/icons/grid.lci",               m_appWin->m_gridStatusWidget},
            {STAT_UCS_STATUS,   tr("UCS Status"),    ":/icons/ucs_ucs.lci",            m_appWin->m_ucsStateWidget},
            {STAT_ANGLES_BASIS, tr("Angles Basis"),  ":/icons/dirpos.lci",             m_appWin->m_anglesBasisWidget}
        });


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

void LC_WidgetFactory::dockAndTabifyGroup(QC_ApplicationWindow* mainWin, Qt::DockWidgetArea area, const QList<QDockWidget*>& docks, QDockWidget* toRaise) {
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

void LC_WidgetFactory::dockAndTabifyCadDocks(QC_ApplicationWindow* mainWin, Qt::DockWidgetArea area,
                                             const std::vector<const char*>& categories, const char* raiseCategory) {
    if (mainWin == nullptr) {
        return;
    }

    QList<QDockWidget*> docks;
    QDockWidget* toRaise = nullptr;

    for (const char* cat : categories) {
        const QString objName = LC_DockNames::cadDockName(QLatin1String(cat));
        auto* dw = mainWin->findChild<QDockWidget*>(objName);
        if (dw != nullptr) {
            docks.append(dw);
            if (raiseCategory != nullptr && strcmp(cat, raiseCategory) == 0) {
                toRaise = dw;
            }
        }
    }

    dockAndTabifyGroup(mainWin, area, docks, toRaise);
}

void LC_WidgetFactory::dockAndTabifyStandardDocks(QC_ApplicationWindow* mainWin, Qt::DockWidgetArea area,
                                                  const std::vector<const char*>& names, const char* raiseName) {
    if (mainWin == nullptr) {
        return;
    }

    QList<QDockWidget*> docks;
    QDockWidget* toRaise = nullptr;

    for (const char* name : names) {
        const QString objName = LC_DockNames::standardDockName(QLatin1String(name));
        auto* dw = mainWin->findChild<QDockWidget*>(objName);
        if (dw != nullptr) {
            docks.append(dw);
            if (raiseName != nullptr && strcmp(name, raiseName) == 0) {
                toRaise = dw;
            }
        }
    }

    dockAndTabifyGroup(mainWin, area, docks, toRaise);
}


void LC_WidgetFactory::redockAllDockWidgets(QC_ApplicationWindow* mainWin) {
    if (mainWin == nullptr) {
        return;
    }

    using namespace LC_DockNames;

    // 1. Left CAD Sidebar (Helpers use m_appWin and compose "dock_cad_" internally)
    dockAndTabifyCadDocks(mainWin, Qt::LeftDockWidgetArea, {
        CAD_MEGA
    });
    dockAndTabifyCadDocks(mainWin, Qt::LeftDockWidgetArea,
                          {
                              CAD_LINE,
                              CAD_POLYLINE,
                              CAD_POINT,
                              CAD_SHAPE},
                              CAD_LINE);

    dockAndTabifyCadDocks(mainWin, Qt::LeftDockWidgetArea,
                          {
                              CAD_CIRCLE,
                              CAD_CURVE,
                              CAD_SPLINE,
                              CAD_ELLIPSE
                          },CAD_CIRCLE);

    dockAndTabifyCadDocks(mainWin, Qt::LeftDockWidgetArea,
                          {
                              CAD_DIMENSION,
                              CAD_OTHER,
                              CAD_TEXT,
                              CAD_INFO,
                              CAD_SELECT
                          }, CAD_DIMENSION);

    dockAndTabifyCadDocks(mainWin, Qt::LeftDockWidgetArea,
                          {
                              CAD_MODIFY,
                              CAD_ORDER});

    // 2. Right Sidebar (Helper uses m_appWin and composes "dock_" internally)
    dockAndTabifyStandardDocks(mainWin, Qt::RightDockWidgetArea, {
        LIBRARY,
        BLOCKS,
        PEN_WIZARD,
        PEN_PALETTE,
        LAYER_TREE,
        LAYERS,
        ENTITY_INFO,
        PROPERTIES,
        UCS,
        NAMED_VIEWS,
        COMMAND
    });
}
