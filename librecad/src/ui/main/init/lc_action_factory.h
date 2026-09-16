/*
 * ********************************************************************************
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

#ifndef LC_ACTIONFACTORY_H
#define LC_ACTIONFACTORY_H

#include <complex.h>

#include "lc_action_factory_base.h"
#include "lc_action_group.h"
#include "lc_infocursorsettingsmanager.h"

class QActionGroup;
class QAction;
class LC_ActionGroupManager;
class QC_ApplicationWindow;
class QG_ActionHandler;

namespace LC_ActionNames {
    // Standard actions with dynamic context visibility
    inline constexpr const char* EditUndo             = "EditUndo";
    inline constexpr const char* EditRedo             = "EditRedo";
    inline constexpr const char* EditKillAllActions   = "EditKillAllActions";
    inline constexpr const char* DeselectAll          = "DeselectAll";
    inline constexpr const char* ZoomAuto             = "ZoomAuto";

    // Prefixes
    inline constexpr const char* PrefixSpecialMenu    = "SpecialMenu:";
    inline constexpr const char* PrefixSpecialAction  = "SpecialAction:";
    inline constexpr const char* PrefixWidget         = "Widget:";

    // Special Menu Tokens
    inline constexpr const char* MenuRecentFiles      = "SpecialMenu:RecentFiles";
    inline constexpr const char* MenuDockWidgets      = "SpecialMenu:DockWidgets";
    inline constexpr const char* MenuCadDockWidgets   = "SpecialMenu:CadDockWidgets";
    inline constexpr const char* MenuToolbars         = "SpecialMenu:Toolbars";
    inline constexpr const char* MenuCadToolbars      = "SpecialMenu:CadToolbars";
    inline constexpr const char* MenuWorkspacesList   = "SpecialMenu:WorkspacesList";
    inline constexpr const char* MenuDrawings         = "SpecialMenu:Drawings";
    inline constexpr const char* MenuRecentActions    = "SpecialMenu:RecentActions";
    inline constexpr const char* MenuWorkspacesRescue = "SpecialMenu:WorkspacesRescue";
    inline constexpr const char* MenuNamedViewsList   = "SpecialMenu:NamedViewsList";
    inline constexpr const char* MenuUCSList          = "SpecialMenu:UCSList";

    // Special Context Action Tokens
    inline constexpr const char* ActionEditBlock           = "SpecialAction:EditBlock";
    inline constexpr const char* ActionEntityLayerActivate = "SpecialAction:EntityLayerActivate";

    inline constexpr const char* ToggleDockLayers        = "SpecialAction:ToggleDockLayers";
    inline constexpr const char* ToggleDockLayerTree     = "SpecialAction:ToggleDockLayerTree";
    inline constexpr const char* ToggleDockBlocks        = "SpecialAction:ToggleDockBlocks";
    inline constexpr const char* ToggleDockCommandLine   = "SpecialAction:ToggleDockCommandLine";
    inline constexpr const char* ToggleDockProperties    = "SpecialAction:ToggleDockProperties";
    inline constexpr const char* ToggleDockLibrary       = "SpecialAction:ToggleDockLibrary";
    inline constexpr const char* ToggleDockQuickInfo     = "SpecialAction:ToggleDockQuickInfo";
    inline constexpr const char* ToggleDockPenPalette    = "SpecialAction:ToggleDockPenPalette";
    inline constexpr const char* ToggleDockPenWizard     = "SpecialAction:ToggleDockPenWizard";
    inline constexpr const char* ToggleDockNamedViews    = "SpecialAction:ToggleDockNamedViews";
    inline constexpr const char* ToggleDockUCS           = "SpecialAction:ToggleDockUCS";

    // Embedded Toolbar Widget Tokens
    inline constexpr const char* WidgetPenControls         = "Widget:PenControls";
    inline constexpr const char* WidgetSnapControls        = "Widget:SnapControls";
    inline constexpr const char* WidgetUCSSelector         = "Widget:UCSSelector";
    inline constexpr const char* WidgetNamedViewsSelector  = "Widget:NamedViewsSelector";
    inline constexpr const char* WidgetWorkspaceSelector   = "Widget:WorkspaceSelector";

    inline constexpr const char* PropertyExcludeFromRecent = "LC_ExcludeFromRecent";

}

class LC_ActionFactory : public LC_ActionFactoryBase{
    Q_OBJECT
public:
    LC_ActionFactory(QC_ApplicationWindow* parent, QG_ActionHandler* actionHandler);
    void initActions(LC_ActionGroupManager* agm, bool useTheme);

    QList<QAction*> file_Actions;
    QList<QAction*> lineActions;
    QList<QAction*> pointActions;
    QList<QAction*> shapeActions;
    QList<QAction*> circleActions;
    QList<QAction*> curveActions;
    QList<QAction*> splineActions;
    QList<QAction*> ellipseActions;
    QList<QAction*> polylineActions;
    QList<QAction*> selectActions;
    QList<QAction*> dimension_Actions;
    QList<QAction*> otherDrawingActions;
    QList<QAction*> modifyActions;
    QList<QAction*> orderActions;
    QList<QAction*> infoActions;
    QList<QAction*> layerActions;
    QList<QAction*> blockActions;
    QList<QAction*> penActions;
    QList<QAction*> entityLayerActions;

private:
    void initActionGroupManager(LC_ActionGroupManager* agm);
    void createEntityLayerActions(QMap<QString, QAction*>& map, LC_ActionGroup* group) const;
    void fillActionContainer(LC_ActionGroupManager* agm, bool useTheme);
    void createDrawLineActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawPointsActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawShapeActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createInfoActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createViewActions(QMap<QString, QAction *> &map, QActionGroup *group);
    void createRelativeInputActions(QMap<QString, QAction *> &map, QActionGroup *group);
    void createDrawCircleActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createSelectActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawCurveActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawSplineActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawEllipseActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawPolylineActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawOtherActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawDimensionsActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createModifyActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createPenActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createFileActions(QMap<QString, QAction *> &map, QActionGroup *group);
    void createPenActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createOrderActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createLayerActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createBlockActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createHelpActionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group);
    void createOptionsActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);
    void createFileActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);
    void createWidgetActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);
    void createWidgetActions(QMap<QString, QAction *> &map, QActionGroup *group);
    void createViewActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);
    void createSelectActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createEditActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createDrawDimensionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group) const;
    void createInteractivePickActions(QMap<QString, QAction*>& map, QActionGroup* group) const;
    void createWorkspacesActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);
    void createNamedViewActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group);

    void createEditActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void updateSnapActionsBySettings(const QMap<QString, QAction*>& map);
    void setupActionInfoSettingAction(QMap<QString, QAction*>& map,
                                      std::unique_ptr<LC_InfoCursorSettingsManager>::pointer infoCursorSettingsManager,
                                      const char* actionName, int tag);
    void setupInfoCursorSettingAction(QAction* actionInfoCursorAbs, int tag,
                                      std::unique_ptr<LC_InfoCursorSettingsManager>::pointer infoCursorSettingsManager);
    void createSnapActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createInfoCursorActions(QMap<QString, QAction *> &map, QActionGroup *group);
    void createSnapExtraActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void setDefaultShortcuts(QMap<QString, QAction *> &map, const LC_ActionGroupManager* agm);
    void setupCreatedActions(QMap<QString, QAction *> &map);
    void markNotEditableActionsShortcuts(const QMap<QString, QAction *> &map);
    void fillActionLists(const QMap<QString, QAction *> &map);
    void prepareActionsToDisableInPrintPreview(QList<QAction *> &actionsList, const QMap<QString, QAction *> &map) const;
    void createRestrictActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createRelZeroActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
    void createUCSActions(QMap<QString, QAction *> &map, QActionGroup *group) const;
};

#endif
