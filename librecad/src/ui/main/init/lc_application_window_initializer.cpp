/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/
#include "lc_application_window_initializer.h"

#include <QMdiArea>
#include <QMdiSubWindow>

#include "lc_action_command_updater.h"
#include "lc_action_factory.h"
#include "lc_action_group_manager.h"
#include "lc_action_options_manager.h"
#include "lc_action_tooltip_builder.h"
#include "lc_action_type_mapper.h"
#include "lc_appwindowdialogsinvoker.h"
#include "lc_centralwidget.h"
#include "lc_command_manager.h"
#include "lc_customization_manager.h"
#include "lc_defaultactioncontext.h"
#include "lc_graphic_view_context_menu_provider.h"
#include "lc_gridviewinvoker.h"
#include "lc_infocursorsettingsmanager.h"
#include "lc_lastopenfilesopener.h"
#include "lc_navigation_creator.h"
#include "lc_optionswidgetsholder.h"
#include "lc_plugininvoker.h"
#include "lc_propertysheetwidget.h"
#include "lc_releasechecker.h"
#include "lc_settings_app_state.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_commands_promotion.h"
#include "lc_settings_defaults.h"
#include "lc_settings_manager_application.h"
#include "lc_settings_manager_customization.h"
#include "lc_settings_manager_drawing.h"
#include "lc_settings_manager_styling.h"
#include "lc_settings_paths.h"
#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "lc_settings_window_options.h"
#include "lc_shortcuts_manager.h"
#include "lc_snapmanager.h"
#include "lc_special_menu_service.h"
#include "lc_widget_factory.h"
#include "lc_workspacesinvoker.h"
#include "main.h"
#include "qc_applicationwindow.h"
#include "qc_dialogfactory.h"
#include "qg_actionhandler.h"
#include "qg_commandwidget.h"
#include "qg_pentoolbar.h"
#include "qg_recentfiles.h"
#include "qg_snaptoolbar.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_settings.h"

LC_ApplicationWindowInitializer::LC_ApplicationWindowInitializer(QC_ApplicationWindow *appWindow)
    : LC_AppWindowAware{appWindow} {
}

void LC_ApplicationWindowInitializer::initGeneralMembers() {
    m_appWin->m_actionHandler = std::make_unique<QG_ActionHandler>(m_appWin);
    m_appWin->m_dlgHelpr = std::make_unique<LC_AppWindowDialogsInvoker>(m_appWin);
    m_appWin->m_workspacesInvoker = std::make_unique<LC_WorkspacesInvoker>(m_appWin);
    m_appWin->m_gridViewInvoker = std::make_unique<LC_GridViewInvoker>(m_appWin);
    m_appWin->m_infoCursorSettingsManager = std::make_unique<LC_InfoCursorSettingsManager>(m_appWin);
    m_appWin->m_lastFilesOpener = std::make_unique<LC_LastOpenFilesOpener>(m_appWin);
}

void LC_ApplicationWindowInitializer::initSpecialMenus() {
    m_appWin->m_recentFilesMenu = std::make_unique<QMenu>(tr("Recent Files"), m_appWin);
    m_appWin->m_pluginsMenu = std::make_unique<QMenu>(tr("Pl&ugins"), m_appWin);
    m_appWin->m_pluginsMenu->setToolTipsVisible(true);
}

void LC_ApplicationWindowInitializer::initApplication(){
    initGeneralMembers();

    auto customizationManager = new LC_CustomizationManager();
    m_appWin->m_customizationManager.reset(customizationManager);

    initActionGroupManager();

    m_appWin->m_commandManager = std::make_unique<LC_CommandManager>(customizationManager->getCommandsRepository());
    m_appWin->m_specialMenuService = std::make_unique<LC_SpecialMenuService>(m_appWin);

    initSpecialMenus();

    //accept drop events to open files
    m_appWin->setAcceptDrops(true);
    initDockCorners();
    initIconSize();
    initActionContext();
    initActionFactory();
    initWidgets();

    auto agm = m_appWin->m_actionGroupManager.get();
    auto specialMenuService = m_appWin->getSpecialMenuService();

    m_appWin->m_navigationControlsCreator = std::make_unique<LC_NavigationControlsCreator>(
        customizationManager->getMenuAndToolbarsRepository(), m_appWin, agm, specialMenuService);

    m_appWin->m_contextMenuProvider = std::make_unique<LC_GraphicViewContextMenuProvider>(
        customizationManager->getGraphicViewContextMenusRepository(), m_appWin->m_actionFactory.get(), agm, specialMenuService);

    m_appWin->m_shortcutsManager = std::make_unique<LC_ShortcutsManager>(customizationManager->getKeymapsRepository());
    auto shortcutsManager = m_appWin->m_shortcutsManager.get();
    connect(RS_SETTINGS, &RS_Settings::optionsChanged, [shortcutsManager, agm]() {
        shortcutsManager->updateActionTooltips(agm->getActionsMap());
    });

    m_appWin->m_uiStyleManager = std::make_unique<LC_UIStyleManager>(m_appWin);

    initSnapManager();
    initCentralWidget();
    initDockAndToolbarAreasActions();
    completeActionFactoryInit();

    initActionOptionsManager();
    setupActionContextWidgets();
    initPropertySheetWidget();
    initDialogFactory();
    initRecentFilesList();
    initNavigationLayout();
    m_appWin->initSettings(true);
    loadCmdWidgetVariablesFile();
    initAutoSaveTimer();
    initCommandsScheme();
    initPlugins();
    m_appWin->showStatusMessage(qApp->applicationName() + " Ready", 2000);
    initReleaseChecker();

    m_appWin->m_uiStyleManager->initialize(m_appWin);

    initSettingsDialogs();
}

void LC_ApplicationWindowInitializer::initSnapManager() const {
    m_appWin->m_snapManager = std::make_unique<LC_SnapManager>(m_appWin->m_snapToolBar);
    m_appWin->m_actionHandler->setSnapManager(m_appWin->m_snapManager.get());
}

void LC_ApplicationWindowInitializer::initReleaseChecker(){
    auto ownBuildVersion = XSTR(LC_VERSION);
    m_appWin->m_releaseChecker = std::make_unique<LC_ReleaseChecker>( ownBuildVersion,XSTR(LC_PRERELEASE));
    connect(m_appWin->m_releaseChecker.get(), &LC_ReleaseChecker::updatesAvailable, m_appWin, &QC_ApplicationWindow::onNewVersionAvailable);
}

void LC_ApplicationWindowInitializer::initActionGroupManager(){
    LC_ActionGroupManager* result = new LC_ActionGroupManager(m_appWin);
    m_appWin->m_actionGroupManager.reset(result);
}

void LC_ApplicationWindowInitializer::initActionOptionsManager(){
    LC_SnapOptionsWidgetsHolder *snapOptionsHolder = m_appWin->m_snapToolBar->getSnapOptionsHolder();
    m_appWin->m_actionOptionsManager = new LC_ActionOptionsManager(m_appWin, m_appWin->m_toolOptionsToolbar, snapOptionsHolder);
    const LC_OptionsWidgetsHolder* optionsWidgetsHolder = m_appWin->m_actionOptionsManager->getActionOptionWidgetHolder();
    connect(m_appWin, &QC_ApplicationWindow::currentActionIconChanged, optionsWidgetsHolder, &LC_OptionsWidgetsHolder::setCurrentQAction);
}

void LC_ApplicationWindowInitializer::initPropertySheetWidget() {
    const LC_PropertySheetWidget* propertySheetWidget = m_appWin->getPropertySheetWidget();
    connect(m_appWin, &QC_ApplicationWindow::currentActionIconChanged, propertySheetWidget, &LC_PropertySheetWidget::setCurrentQAction);
}

void LC_ApplicationWindowInitializer::initActionFactory() const {
    m_appWin->m_actionFactory = std::make_unique<LC_ActionFactory>(m_appWin, m_appWin->m_actionHandler.get());
    const bool using_theme = CFG_AppStyling::o_AllowTheme;
    m_appWin->m_actionFactory->initActions(m_appWin->m_actionGroupManager.get(), using_theme);
}

void LC_ApplicationWindowInitializer::completeActionFactoryInit() const {
    m_appWin->m_actionFactory->setupDefaultShortcutsAndCompleteInit(m_appWin->m_actionGroupManager.get());
    auto actionsMap = m_appWin->m_actionGroupManager->getActionsMap();
    m_appWin->m_shortcutsManager->loadActiveScheme(actionsMap);
}

void LC_ApplicationWindowInitializer::initDockCorners() const {
    {
        using namespace CFG_Widgets;
        const bool allowDockNesting = o_DockAllowNested;
        const bool verticalTabs = o_DockVerticalTabs;
        LC_WidgetFactory::updateDockOptions(m_appWin, allowDockNesting, verticalTabs);
    }

    // make the left and right dock areas dominant
    m_appWin->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    m_appWin->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    m_appWin->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    m_appWin->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

void LC_ApplicationWindowInitializer::initCentralWidget(){
    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: creating LC_CentralWidget");

    const auto central = new LC_CentralWidget(m_appWin);
    m_appWin->setCentralWidget(central);
    m_appWin->m_mdiAreaCAD = central->getMdiArea();
    m_appWin->m_mdiAreaCAD->setDocumentMode(true);

    using namespace CFG_WindowOptions;
    m_appWin->setTabLayout(o_TabShape, o_TabPosition);

    const bool tabMode = CFG_Startup::o_TabMode;
    if (tabMode) {
        m_appWin->setupCADAreaTabbar();
    }

    connect(m_appWin->m_mdiAreaCAD, &QMdiArea::subWindowActivated, m_appWin, &QC_ApplicationWindow::slotWindowActivated);

    // This event filter allows sending key events to the command widget, therefore, no
    // need to activate the command widget before typing commands.
    // Since this nice feature causes a bug of lost key events when the command widget is on
    // a screen different from the main window, disabled for the time being
    // send key events for mdiAreaCAD to command widget by default
    m_appWin->m_mdiAreaCAD->installEventFilter(m_appWin->m_commandWidget);

    connect(m_appWin->getAction("FileClose"), &QAction::triggered, m_appWin->m_mdiAreaCAD, &QMdiArea::closeActiveSubWindow);
}

void LC_ApplicationWindowInitializer::initIconSize() const {
    using namespace CFG_Widgets;
    const bool hasCustomIconSize = o_AllowToolbarIconSize;
    if (hasCustomIconSize) {
        const int iconSize = o_ToolbarIconSize;
        m_appWin->setIconSize(QSize(iconSize, iconSize));
    }
}

void LC_ApplicationWindowInitializer::loadCmdWidgetVariablesFile() const {
    const QString commandFile = CFG_Paths::o_VariableFile;
    if (!commandFile.isEmpty()) {
        m_appWin->m_commandWidget->leCommand->readCommandFile(commandFile);
    }
}

void LC_ApplicationWindowInitializer::initDockAndToolbarAreasActions() const {
    m_appWin->m_dockAreasToggleActions.left = m_appWin->getAction("LeftDockAreaToggle");
    m_appWin->m_dockAreasToggleActions.right = m_appWin->getAction("RightDockAreaToggle");
    m_appWin->m_dockAreasToggleActions.top = m_appWin->getAction("TopDockAreaToggle");
    m_appWin->m_dockAreasToggleActions.bottom = m_appWin->getAction("BottomDockAreaToggle");
    m_appWin->m_dockAreasToggleActions.floating = m_appWin->getAction("FloatingDockwidgetsToggle");

    m_appWin->m_toolbarAreasToggleActions.left = m_appWin->getAction("LeftTBAreaToggle");
    m_appWin->m_toolbarAreasToggleActions.right = m_appWin->getAction("RightTBAreaToggle");
    m_appWin->m_toolbarAreasToggleActions.top = m_appWin->getAction("TopTBAreaToggle");
    m_appWin->m_toolbarAreasToggleActions.bottom = m_appWin->getAction("BottomTBAreaToggle");
    // m_appWin->m_toolbarAreasToggleActions.floating = m_appWin->getAction("FloatingDockwidgetsToggle");
}


void LC_ApplicationWindowInitializer::initCommandsScheme() const {
    auto actionGroupManager = m_appWin->m_actionGroupManager.get();
    if (m_appWin->m_commandManager != nullptr && actionGroupManager != nullptr) {
            const LC_ActionTypeMapper mapper(actionGroupManager);
            m_appWin->m_commandManager->loadActiveScheme(&mapper);
            bool clearTooltips = !(CFG_Appearance::o_ShowKeyboardShortcutsInTooltips || !CFG_CommandsPromotion::o_ShowCommandInMenu);
            if (clearTooltips) {
                LC_ActionCommandUpdater::clearActions(actionGroupManager);
            }
            else {
                const bool keycodeMode = CFG_AppState::o_KeycodeMode;
                LC_ActionCommandUpdater::updateActions(actionGroupManager, m_appWin->m_commandManager.get(), keycodeMode);
            }
    }

    LC_ActionTooltipBuilder::updateAllTooltips(m_appWin->m_actionGroupManager->getActionsMap());
}

void LC_ApplicationWindowInitializer::initRecentFilesList() const {
    m_appWin->m_recentFilesList = new QG_RecentFiles(m_appWin, 9);
    m_appWin->m_recentFilesList->addFiles(m_appWin->getRecentFilesMenu());
}

void LC_ApplicationWindowInitializer::initDialogFactory() const {
    LC_SnapOptionsWidgetsHolder *snapOptionsHolder = m_appWin->m_snapToolBar->getSnapOptionsHolder();
    const auto factory = new QC_DialogFactory(m_appWin, m_appWin->m_toolOptionsToolbar, snapOptionsHolder);
    RS_DialogFactory::instance()->setFactoryObject(factory);
    m_appWin->m_dialogFactory = factory;

    // fixme - sand - temporary setters, remove later
    factory->set_command_widget(m_appWin->m_commandWidget);
    factory->set_rel_zero_coordinates_widget(m_appWin->m_relativeZeroCoordinatesWidget);
    factory->set_selection_widget(m_appWin->m_selectionWidget);
}

void LC_ApplicationWindowInitializer::initWidgets() const {
    LC_WidgetFactory widgetFactory(m_appWin);
    widgetFactory.initWidgets();
}

void LC_ApplicationWindowInitializer::initNavigationLayout() const {
    auto* creator = m_appWin->getCreatorInvoker();
    if (creator != nullptr) {
        creator->applyActiveLayoutScheme();
    }
}

void LC_ApplicationWindowInitializer::initPlugins(){
    m_appWin->m_pluginInvoker = std::make_unique<LC_PluginInvoker>(m_appWin, m_appWin->m_actionContext);
    m_appWin->m_pluginInvoker->loadPlugins();
}

void LC_ApplicationWindowInitializer::initAutoSaveTimer() const {
    const bool allowAutoSave = CFG_Defaults::o_AutoBackupDocument;
    m_appWin->startAutoSaveTimer(allowAutoSave);
}

/**
 * NOTE: potentially, the main application window may represent implementation of ActionContext instead of
 * LC_DefaultActionContext. Thinks whether this is practical..
 */
void LC_ApplicationWindowInitializer::initActionContext() const {
    const auto actionHandler = m_appWin->m_actionHandler.get();
    const auto action_context = new LC_DefaultActionContext(actionHandler);
    m_appWin->m_actionContext = action_context;
    actionHandler->setActionContext(action_context);
}

void LC_ApplicationWindowInitializer::setupActionContextWidgets() const {
    m_appWin->m_actionContext->setActionOptionsManager(m_appWin->m_actionOptionsManager);
    m_appWin->m_actionContext->setCommandWidget(m_appWin->m_commandWidget);
    m_appWin->m_actionContext->setCoordinateWidget(m_appWin->m_coordinateWidget);
    m_appWin->m_actionContext->setMouseWidget(m_appWin->m_mouseWidget);
    m_appWin->m_actionContext->setStatusBarManager(m_appWin->m_statusbarManager);
    m_appWin->m_actionContext->setPropertySheetWidget(m_appWin->m_propertySheetWidget);
    m_appWin->m_actionContext->setCommandManager(m_appWin->m_commandManager.get());
}

void LC_ApplicationWindowInitializer::initSettingsDialogs() {
    LC_SettingsManagerApplication::initialize(m_appWin);
    LC_SettingsManagerDrawing::initialize();
    LC_SettingsManagerStyling::initialize(m_appWin);
    // fixme - review initialization
    LC_SettingsManagerCustomization::initialize(m_appWin);
}
