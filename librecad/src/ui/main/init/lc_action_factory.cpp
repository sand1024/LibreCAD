/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

// This file was first published at: github.com/r-a-v-a-s/LibreCAD.git

// lc_actionfactory is a rewrite of qg_actionfactory; some copied content remains.
// qg_actionfactory contributors:
// Andrew Mustun, Claude Sylvain, R. van Twisk, Dongxu Li, Rallaz, Armin Stebich, ravas, korhadris

#include "lc_action_factory.h"

#include <QAction>
#include <QDesktopServices>
#include <QKeySequence>

#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_settings_manager_drawing.h"
#include "lc_infocursorsettingsmanager.h"
#include "lc_settings_keyboard.h"
#include "lc_settings_selection.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "lc_shortcuts_manager.h"
#include "lc_shortcut_info.h"
#include "qc_applicationwindow.h"
#include "qg_actionhandler.h"
#include "rs_actioninterface.h"
#include "rs_previewactioninterface.h"


LC_ActionFactory::LC_ActionFactory(QC_ApplicationWindow* parent, QG_ActionHandler* actionHandler)
    : LC_ActionFactoryBase(parent, actionHandler){
}

void LC_ActionFactory::initActions(LC_ActionGroupManager* agm, const bool useTheme) {
   initActionGroupManager(agm);
   fillActionContainer(agm, useTheme);
}

void LC_ActionFactory::initActionGroupManager(LC_ActionGroupManager* agm) {
    // NOTE: the order is imprortant now!
  createActionGroups(
    {
        // ==========================================
        // 1. DRAFTING GEOMETRY (Core CAD Tools)
        // ==========================================
        {"line",          tr("Line"),                    tr("Line drawing commands"),          ":/icons/line.lci",                   true, true, true, "Menu:Line"},
        {"point",         tr("Point"),                   tr("Point drawing commands"),         ":/icons/points.lci",                 true, true, true, "Menu:Point"},
        {"shape",         tr("Polygon"),                 tr("Polygon drawing commands"),       ":/icons/rectangle_2_points.lci",     true, true, true, "Menu:Polygon"},
        {"circle",        tr("Circle"),                  tr("Circle drawing commands"),        ":/icons/circle.lci",                 true, true, true, "Menu:Circle"},
        {"curve",         tr("Arc"),                     tr("Arc drawing commands"),           ":/icons/arc_center_point_angle.lci", true, true, true, "Menu:Arc"},
        {"spline",        tr("Spline"),                  tr("Spline drawing commands"),        ":/icons/spline_points.lci",          true, true, true, "Menu:Spline"},
        {"ellipse",       tr("Ellipse"),                 tr("Ellipse drawing commands"),       ":/icons/ellipses.lci",               true, true, true, "Menu:Ellipse"},
        {"polyline",      tr("Polyline"),                tr("Polyline drawing commands"),      ":/icons/polylines_polyline.lci",     true, true, true, "Menu:Polyline"},
        {"text",          tr("Text"),                    tr("Text and annotation commands"),   ":/icons/text.lci",                   true, true, true, "Menu:Text"},
        {"other",         tr("Other"),                   tr("Other operations"),               ":/icons/camera.lci",                 true, true, true, "Menu:Other"},

        // ==========================================
        // 2. SELECTION, TRANSFORMS & EDITING
        // ==========================================
        {"select",        tr("Select"),                  tr("Entity selection operations"),    ":/icons/select.lci",                 true,  true, true, "Menu:Select"},
        {"modify",        tr("Modify"),                  tr("Modification operations"),        ":/icons/move_rotate.lci",            true,  true, true, "Menu:Modify"},
        {"align",         tr("Align"),                   tr("Alignment operations"),           ":/icons/align_one.lci",              false, true, true, "Menu:Align"},
        {"order",         tr("Order"),                   tr("Draw order operations"),          ":/icons/order.lci",                  true,  true, true, "Menu:Order"},
        {"edit",          tr("Edit"),                    tr("Editing operations"),             ":/icons/rename_active_block.lci",    false, true, false, "Menu:Edit"},


        // ==========================================
        // 3. ANNOTATIONS & MEASUREMENT
        // ==========================================
        {"dimension",     tr("Dimension"),               tr("Dimensions creation commands"),   ":/icons/dim_horizontal.lci",         true, true, true, "Menu:Dimension"},
        {"info",          tr("Info"),                    tr("Informational commands"),         ":/icons/measure.lci",                true, true, true, "Menu:Info"},

        // ==========================================
        // 4. PRECISION & SNAPPING
        // ==========================================
        {"snap",          tr("Snap"),                    tr("Snapping operations"),            ":/icons/snap_intersection.lci",      false, true, false, "Menu:Snap"},
        {"snap_extras",   tr("Snap Extras"),             tr("Additional Snaps"),               ":/icons/snap_free.lci",              false, true, false},
        {"restriction",   tr("Restriction"),             tr("Snap restrictions"),              ":/icons/restr_ortho.lci",            false, true, false},
        {"relative_input",tr("Relative Point Assistant"),tr("Relative Point Assistant"),       ":/icons/snap_visual.lci",            false, true, false},
        {"relZero",       tr("Relative Zero"),           tr("Relative Zero"),                  ":/icons/set_rel_zero.lci",           false, true, false},
        {"infoCursor",    tr("InfoCursor"),              tr("Informational Cursor"),           ":/icons/info_cursor_enable.lci",     false, true, false},

        // ==========================================
        // 5. CAD DOCUMENT STRUCTURE & CONTEXT
        // ==========================================
        {"layer",         tr("Layer"),                   tr("Layers operations"),              ":/icons/deselect_layer.lci",         false, true, false, "Menu:Layer"},
        {"entity_layer",  tr("Entity Layer"),            tr("Entity's Layer"),                 ":/icons/layer_list.lci",             false, true, false},
        {"block",         tr("Block"),                   tr("Block related operations"),       ":/icons/create_block.lci",           false, true, true,  "Menu:Block"},
        {"ucs",           tr("UCS"),                     tr("UCS operations"),                 ":/icons/set_ucs.lci",                true,  true, false, "Menu:UCS"},
        {"pen",           tr("Pen"),                   tr("Pen related operations"),         ":/icons/pen_apply.lci",              false, true, false},

        // ==========================================
        // 6. APPLICATION & WORKSPACE MANAGEMENT
        // ==========================================
        {"file",          tr("File"),                    tr("File Operations"),                ":/icons/save.lci",                   false, false, false, "Menu:File"},
        {"view",          tr("View"),                    tr("View related operations"),        ":/icons/zoom_in.lci",                false, true,  false, "Menu:View"},
        {"plugins",       tr("&Plugins"),                tr("External plugins"),               ":/icons/plugin.lci",                 false, false, false, "Menu:Plugins"},
        {"draw",          tr("&Draw"),                   tr("CAD drafting commands"),          ":/icons/line.lci",                   false, false, true,  "Menu:Draw"},
        {"tools",         tr("&Tools"),                  tr("CAD tools"),                      ":/icons/line_polygon_star.lci",      false, false, true,  "Menu:Tools"},
        {"namedViews",    tr("Named Views"),             tr("Persistent Views operations"),    ":/icons/visible.lci",                false, true,  false},
        {"workspaces",    tr("Workspaces"),              tr("Workspaces operations"),          ":/icons/workspace.lci",              false, true, true, "Menu:Workspace"},
        {"categories",    tr("Categories"),              tr("CAD tool categories"),            ":/icons/line_polygon_star.lci",      false, false, false},
        {"creators",      tr("Creators"),                tr("Custom toolbars and menus creators"), ":/icons/create_toolbar.lci",     false, false, false},
        {"options",       tr("Options"),                 tr("Options management"),             ":/icons/settings.lci",               false, true,  false, "Menu:Options"},
        {"interactive_pick", tr("Interactive"),          tr("Interactive Pick"),               ":/icons/interactive_pick_point.lci", true,  false, false},
        {"help",          tr("Help"),                    tr("Help and Online Documentation"),  ":/icons/help.lci",                   false, false, false, "Menu:Help"},

        // ==========================================
        // 7. STRUCTURAL NAVIGATION SUBMENUS
       // ==========================================
        {"import",        tr("Import"),                  tr("Import operations"),              ":/icons/import.lci",                 false, false, false, "Menu:Import"},
        {"export",        tr("Export"),                  tr("Export operations"),              ":/icons/export.lci",                 false, false, false, "Menu:Export"},
        {"views_restore", tr("&Views Restore"),          tr("Restore saved views"),            ":/icons/nview_visible.lci",          false, false, false, "Menu:ViewsRestore"},
        {"dock_areas",    tr("Dock Areas"),              tr("Toggle dock areas"),              "",                                   false, true, true, "Menu:DockAreas"},
        {"tb_areas",      tr("Toolbar Areas"),           tr("Toggle toolbar areas"),           "",                                   false, true, true, "Menu:ToolbarAreas"},
        {"online_docs",   tr("On&line Docs"),            tr("Online documentation links"),     ":/icons/help.lci",                   false, false, false, "Menu:OnlineDocs"},
        // ==========================================
        // 8. Widgets toggle actions
        // ==========================================
        {"dock_widgets",     tr("Tool Windows"),    tr("Tool Windows visibility toggles"),     ":/icons/tool_windows_palette.lci",  false, true,  true,  "Menu:DockWidgets"},
        {"cad_dock_widgets", tr("CAD Tool Windows"),tr("CAD Tool Windows visibility toggles"), ":/icons/dockwidgets_left.lci",      false, true,  true,  "Menu:CadDockWidgets"},
        {"tool_options",  tr("Tool Options"),            tr("Active Tool Options bar"),           ":/icons/drawing_settings.lci",       false, true,  false},
    }, agm);


    const auto fileGroup = agm->getGroupByName("file");
    const auto optionsGroup = agm->getGroupByName("options");

    for (const auto& actionGroup : agm->findChildren<LC_ActionGroup*>()) {
        actionGroup->setExclusive(false);
        if (fileGroup != actionGroup && optionsGroup != actionGroup) {
            connect(m_appWin, &QC_ApplicationWindow::windowsChanged, actionGroup, &LC_ActionGroup::setEnabled);
        }
    }

    /*auto toolGroups = agm->toolGroups();
    for (auto actionGroup : toolGroups) {
        connect(actionGroup, &LC_ActionGroup::triggered, m_appWin, &QC_ApplicationWindow::relayAction);
    }*/
}

void LC_ActionFactory::createEntityLayerActions(QMap<QString, QAction*>& map, LC_ActionGroup* group) const {
    createSpecialAction(map, group, LC_ActionNames::ActionEntityLayerActivate, tr("Activate Entity's Layer (Context Action)"), ":/icons/select_entity.lci", tr("Activates the layer of the currently clicked or selected entity."));

    createActionHandlerActions(map, group,{
        {"EntityLayerActivate",     RS2::ActionLayerEntityActivate,          tr("Activate Entity's Layer"),           ":/icons/select_entity.lci",      tr("Switches the active layer to match the layer of the selected entity.")},
        {"EntityLayerView",         RS2::ActionLayerEntityToggleView,        tr("Hide Entity's Layer"),               ":/icons/not_visible.lci",        tr("Toggles visibility of the layer belonging to the selected entity.")},
        {"EntityLayerHideOthers",   RS2::ActionLayerEntityHideOthers,        tr("Hide Other Layers than Entity's"),   ":/icons/not_visible_all.lci",    tr("Hides all layers except the one belonging to the selected entity.")},
        {"EntityLayerLock",         RS2::ActionLayerEntityToggleLock,        tr("Lock Entity's Layer"),               ":/icons/locked.lci",             tr("Toggles locking of the layer belonging to the selected entity.")},
        {"EntityLayerConstruction", RS2::ActionLayerEntityToggleConstruction,tr("Toggle Entity's Layer Construction"),":/icons/construction_layer.lci", tr("Toggles construction mode for the layer belonging to the selected entity.")},
        {"EntityLayerPrint",        RS2::ActionLayerEntityTogglePrint,       tr("Toggle Entity's Layer Printing"),    ":/icons/print.lci",              tr("Toggles printing output for the layer belonging to the selected entity.")}
 });
}

// todo - add explanations for commands for actions (probably mix with commandItems) as it was mentioned in issue #570

void LC_ActionFactory::setupDefaultShortcutsAndCompleteInit(LC_ActionGroupManager* agm) {
    QMap<QString, QAction *> &actionMap = agm->getActionsMap();
    setupCreatedActions(actionMap);
    setDefaultShortcuts(actionMap, agm);

    // todo - may we report errors somehow there?
    markNotEditableActionsShortcuts(actionMap);

    agm->completeInit();

    fillActionLists(actionMap);
    addActionsToMainWindow(actionMap);

    prepareActionsToDisableInPrintPreview(m_appWin->m_actionsToDisableInPrintPreviewList, actionMap);
}

void LC_ActionFactory::fillActionContainer(LC_ActionGroupManager* agm, const bool useTheme){
    m_usingTheme = useTheme;
    QMap<QString, QAction *> &actionMap = agm->getActionsMap();
    createSelectActions(actionMap, agm->getGroupByName("select"));
    createDrawLineActions(actionMap, agm->getGroupByName("line"));
    createDrawPointsActions(actionMap, agm->getGroupByName("point"));
    createDrawShapeActions(actionMap, agm->getGroupByName("shape"));
    createDrawCircleActions(actionMap, agm->getGroupByName("circle"));
    createDrawCurveActions(actionMap, agm->getGroupByName("curve"));
    createDrawSplineActions(actionMap, agm->getGroupByName("spline"));
    createDrawEllipseActions(actionMap, agm->getGroupByName("ellipse"));
    createDrawPolylineActions(actionMap, agm->getGroupByName("polyline"));
    createDrawOtherActions(actionMap, agm->getGroupByName("other"));
    createDrawDimensionsActions(actionMap, agm->getGroupByName("dimension"));
    createModifyActions(actionMap, agm->getGroupByName("modify"));
    createPenActions(actionMap, agm->getGroupByName("pen"));
    createInfoActions(actionMap, agm->getGroupByName("info"));
    createViewActions(actionMap, agm->getGroupByName("view"));
    createDockAreasActions(actionMap, agm->getGroupByName("dock_areas"));
    createToolbarAreasActions(actionMap, agm->getGroupByName("tb_areas"));
    createFileActions(actionMap, agm->getGroupByName("file"));

    createSnapActions(actionMap, agm->getGroupByName("snap"));
    createRelativeInputActions(actionMap, agm->getGroupByName("relative_input"));
    createInfoCursorActions(actionMap, agm->getGroupByName("infoCursor"));
    createSnapExtraActions(actionMap, agm->getGroupByName("snap_extras"));
    createRestrictActions(actionMap, agm->getGroupByName("restriction"));
    createRelZeroActions(actionMap, agm->getGroupByName("other"));
    createUCSActions(actionMap, agm->getGroupByName("ucs"));
    createEditActions(actionMap, agm->getGroupByName("edit"));

    createEntityLayerActions(actionMap, agm->getGroupByName("entity_layer"));
    createInteractivePickActions(actionMap, agm->getGroupByName("interactive_pick"));

    for (QAction* value: std::as_const(actionMap)){
        if (value != nullptr) {
            value->setCheckable(true);
        }
    }

    // not checkable actions
    createPenActionsUncheckable(actionMap, agm->getGroupByName("pen"));
    createOrderActionsUncheckable(actionMap, agm->getGroupByName("order"));
    createLayerActionsUncheckable(actionMap, agm->getGroupByName("layer"));
    createBlockActionsUncheckable(actionMap, agm->getGroupByName("block"));
    createOptionsActionsUncheckable(actionMap, agm->getGroupByName("options"));
    createSelectActionsUncheckable(actionMap, agm->getGroupByName("select"));
    createFileActionsUncheckable(actionMap, agm->getGroupByName("file"));
    createViewActionsUncheckable(actionMap, agm->getGroupByName("view"));
    createNamedViewActionsUncheckable(actionMap, agm->getGroupByName("namedViews"));
    createWorkspacesActionsUncheckable(actionMap, agm->getGroupByName("workspaces"));
    createEditActionsUncheckable(actionMap, agm->getGroupByName("edit"));
    createDrawDimensionsUncheckable(actionMap, agm->getGroupByName("dimension"));
    createHelpActionsUncheckable(actionMap, agm->getGroupByName("help"));
}

void LC_ActionFactory::createDrawShapeActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group, {
        {"DrawLineRectangle",       RS2::ActionDrawLineRectangle,       tr("Rectangle"),           ":/icons/line_rectangle.lci",        tr("Draws rectangular polygons from two opposite corner points.")},
        {"DrawLineRectangle1Point", RS2::ActionDrawRectangle1Point,     tr("Rectangle (1 Point)"), ":/icons/rectangle_1_point.lci",     tr("Draws rectangles defined by size, placement point, and corner.")},
        {"DrawLineRectangle2Points",RS2::ActionDrawRectangle2Points,    tr("Rectangle (2 Points)"),":/icons/rectangle_2_points.lci",    tr("Draws aligned rectangular polygons from two diagonal points.")},
        {"DrawLineRectangle3Points",RS2::ActionDrawRectangle3Points,    tr("Rectangle (3 Points)"),":/icons/rectangle_3_points.lci",    tr("Draws rotated rectangles defined by baseline endpoints and height.")},
        {"DrawLinePolygonCenCor",   RS2::ActionDrawLinePolygonCenCor,   tr("Pol&ygon (Cen,Cor)"),  ":/icons/line_polygon_cen_cor.lci",  tr("Draws regular polygons defined by center point and vertex corner.")},
        {"DrawLinePolygonCenTan",   RS2::ActionDrawLinePolygonCenTan,   tr("Pol&ygon (Cen,Tan)"),  ":/icons/line_polygon_cen_tan.lci",  tr("Draws regular polygons defined by center point and tangent midpoint.")},
        {"DrawLinePolygonCorCor",   RS2::ActionDrawLinePolygonCorCor,   tr("Polygo&n (Cor,Cor)"),  ":/icons/line_polygon_cor_cor.lci",  tr("Draws regular polygons defined by two opposite corner points.")},
        {"DrawLinePolygonSideSide", RS2::ActionDrawLinePolygonSideSide, tr("Polygo&n (Tan,Tan)"),  ":/icons/line_polygon_size_size.lci",tr("Draws regular polygons defined by two side midpoint vertices.")},
        {"DrawStar",                RS2::ActionDrawStar,                tr("Star"),                ":/icons/line_polygon_star.lci",     tr("Draws star-shaped polygons with configurable arms and radii.")}
    });
}

void LC_ActionFactory::createDrawLineActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group,{
        {"DrawLine",                 RS2::ActionDrawLine,                tr("&2 Points"),             ":/icons/line_2p.lci",                   tr("Draws straight lines between two or more points.")},
        {"DrawLineAngle",            RS2::ActionDrawLineAngle,           tr("&Angle"),                ":/icons/line_angle.lci",                tr("Draws a line at a defined angle and length from a base point.")},
        {"DrawLineHorizontal",       RS2::ActionDrawLineHorizontal,      tr("&Horizontal"),           ":/icons/line_horizontal.lci",           tr("Draws strictly horizontal lines through picked points.")},
        {"DrawLineVertical",         RS2::ActionDrawLineVertical,        tr("Vertical"),              ":/icons/line_vertical.lci",             tr("Draws strictly vertical lines through picked points.")},
        {"DrawLineFree",             RS2::ActionDrawLineFreehand,        tr("&Freehand Line"),        ":/icons/line_freehand.lci",             tr("Draws freehand sketch curves following the cursor trajectory.")},
        {"DrawLineParallel",         RS2::ActionDrawLineParallel,        tr("&Parallel"),             ":/icons/line_parallel.lci",             tr("Draws parallel concentric entities at a specified offset distance.")},
        {"DrawLineParallelThrough",  RS2::ActionDrawLineParallelThrough, tr("Parallel through point"),":/icons/line_parallel_p.lci",           tr("Draws parallel entities passing through a specified point.")},
        {"DrawLineBisector",         RS2::ActionDrawLineBisector,        tr("Bisector"),              ":/icons/line_bisector.lci",             tr("Draws angle bisectors between two intersecting lines.")},
        {"DrawLineTangent1",         RS2::ActionDrawLineTangent1,        tr("Tangent (P,C)"),         ":/icons/line_tangent_pc.lci",           tr("Draws lines tangent to an arc or circle from a point.")},
        {"DrawLineTangent2",         RS2::ActionDrawLineTangent2,        tr("Tangent (C,C)"),         ":/icons/line_tangent_cc.lci",           tr("Draws lines tangent to two existing circles or arcs.")},
        {"DrawLineOrthTan",          RS2::ActionDrawLineOrthTan,         tr("Tangent &Orthogonal"),   ":/icons/line_tangent_perpendicular.lci",tr("Draws lines tangent to a circle and perpendicular to a line.")},
        {"DrawLineOrthogonal",       RS2::ActionDrawLineOrthogonal,      tr("Orthogonal"),            ":/icons/line_perpendicular.lci",        tr("Draws lines perpendicular to an existing entity.")},
        {"DrawLineRelAngle",         RS2::ActionDrawLineRelAngle,        tr("Relative angle"),        ":/icons/line_relative_angle.lci",       tr("Draws lines at a relative angle to an existing entity.")},
        {"DrawLineRel",              RS2::ActionDrawSnakeLine,           tr("Snake"),                 ":/icons/line_rel.lci",                  tr("Draws continuous connected perpendicular snake lines.")},
        {"DrawLineRelX",             RS2::ActionDrawSnakeLineX,          tr("Snake (X)"),             ":/icons/line_rel_x.lci",                tr("Draws snake lines aligned along the X axis.")},
        {"DrawLineRelY",             RS2::ActionDrawSnakeLineY,          tr("Snake (Y)"),             ":/icons/line_rel_y.lci",                tr("Draws snake lines aligned along the Y axis.")},
        {"DrawLineAngleRel",         RS2::ActionDrawLineAngleRel,        tr("Angle From Line"),       ":/icons/line_angle_rel.lci",            tr("Draws lines at a relative angle from an existing reference line.")},
        {"DrawLineOrthogonalRel",    RS2::ActionDrawLineOrthogonalRel,   tr("Orthogonal From Line"),  ":/icons/line_ortho_rel.lci",            tr("Draws perpendicular lines starting from a relative offset point.")},
        {"DrawLineFromPointToLine",  RS2::ActionDrawLineFromPointToLine, tr("From Point To Line"),    ":/icons/line_to_ortho.lci",             tr("Draws perpendicular dropped lines from a point to a target line.")},
        {"DrawLineMiddle",           RS2::ActionDrawCenterLine,          tr("Centerline"),            ":/icons/line_middle.lci",               tr("Draws centerline entities midway between two lines.")},
        {"DrawCross",                RS2::ActionDrawCenterMark,          tr("Center Mark"),           ":/icons/cross_circle1.lci",             tr("Draws center mark crosshairs for circles and arcs.")},
        {"DrawSliceDivideLine",      RS2::ActionDrawSliceDivideLine,     tr("Slice/Divide Line"),     ":/icons/slice_divide.lci",              tr("Divides and cuts a line into equal segmented lengths.")},
        {"DrawSliceDivideCircle",    RS2::ActionDrawSliceDivideCircle,   tr("Slice/Divide Circle"),   ":/icons/slice_divide_circle.lci",       tr("Divides and segments circles or arcs into equal angle sections.")},
        {"DrawLineRadiant",          RS2::ActionDrawLineRadiant,         tr("Radiant Line"),          ":/icons/line_far_point.lci",            tr("Draws radiant construction rays emanating from a center point.")}
    });
}

void LC_ActionFactory::createDrawPointsActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group,{
        {"DrawPoint",        RS2::ActionDrawPoint,        tr("&Points"),          ":/icons/points.lci",         tr("Places single point marker entities in the drawing.")},
        {"DrawLinePoints",   RS2::ActionDrawPointsLine,   tr("Line of Points"),   ":/icons/line_points.lci",    tr("Places multiple equally spaced point entities along a line.")},
        {"DrawPointsMiddle", RS2::ActionDrawPointsMiddle, tr("Middle Points"),    ":/icons/points_middle.lci",  tr("Places point entities at the midpoint between clicked locations.")},
        {"DrawPointLattice", RS2::ActionDrawPointsLattice,tr("Lattice of Points"),":/icons/points_lattice.lci", tr("Places a 2D matrix lattice array of point markers.")},
        {"SelectPoints",     RS2::ActionSelectPoints,     tr("Select Points"),    ":/icons/select_points.lci",  tr("Selects point markers located within the current viewport.")},
        {"PasteToPoints",    RS2::ActionPasteToPoints,    tr("Paste to Points"),  ":/icons/paste_to_points.lci",tr("Pastes clipboard contents repeatedly onto all selected point entities.")}
    });
}

void LC_ActionFactory::createSelectActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group,{
        {"SelectSingle",        RS2::ActionSelectSingle,       tr("Select Entity"),                ":/icons/select_entity.lci",                tr("Selects or deselects individual clicked entities.")},
        {"SelectWindow",        RS2::ActionSelectWindow,       tr("Select Window"),                ":/icons/select_window.lci",                tr("Selects entities fully enclosed within a rectangle window.")},
        {"DeselectWindow",      RS2::ActionDeselectWindow,     tr("Deselect Window"),              ":/icons/deselect_window.lci",              tr("Deselects entities enclosed within a drawn rectangle window.")},
        {"SelectContour",       RS2::ActionSelectContour,      tr("(De-)Select &Contour"),         ":/icons/deselect_contour.lci",             tr("Selects all continuously connected entities forming a contour.")},
        {"SelectIntersected",   RS2::ActionSelectIntersected,  tr("Select Intersected Entities"),  ":/icons/select_intersected_entities.lci",  tr("Selects entities intersected by a crossing fence line.")},
        {"DeselectIntersected", RS2::ActionDeselectIntersected,tr("Deselect Intersected Entities"),":/icons/deselect_intersected_entities.lci",tr("Deselects entities intersected by a crossing fence line.")},
        {"SelectLayer",         RS2::ActionSelectLayer,        tr("(De-)Select Layer"),            ":/icons/deselect_layer.lci",               tr("Selects all entities belonging to the active or picked layer.")},
        {"SelectQuick",         RS2::ActionSelectQuick,        tr("Select Quick"),                 ":/icons/select_conditional.lci",           tr("Selects entities matching specific property criteria.")},
        {"SelectionModeToggle", RS2::ActionSelectModeToggle,   tr("Additive/Exclusive Selection"), ":/icons/select_mode_add.lci",              tr("Toggles between additive selection and single selection modes.")},
        {"SelectionGeneric",    RS2::ActionSelectGeneric,      tr("Select Entities"),              ":/icons/select.lci",                       tr("Selects entities using default interactive picker.")}
    });
}

void LC_ActionFactory::createDrawCircleActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group,{
        {"DrawCircle",         RS2::ActionDrawCircleCenterPoint,         tr("Center, &Point"),                 ":/icons/circle_center_point.lci",                 tr("Draws circles defined by center point and perimeter radius point.")},
        {"DrawCircleByArc",    RS2::ActionDrawCircleByArc,               tr("By Arc"),                         ":/icons/circle_by_arc.lci",                       tr("Converts an arc into a complete 360-degree circle entity.")},
        {"DrawCircleCR",       RS2::ActionDrawCircleCenterRadius,        tr("Center, &Radius"),                ":/icons/circle_center_radius.lci",                tr("Draws circles with a fixed numerical radius from a center point.")},
        {"DrawCircle2P",       RS2::ActionDrawCircle2Points,             tr("2 Points"),                       ":/icons/circle_2_points.lci",                     tr("Draws circles passing through two diametrically opposite points.")},
        {"DrawCircle2PR",      RS2::ActionDrawCircle2PointsRadius,       tr("2 Points, Radius"),               ":/icons/circle_2_points_radius.lci",              tr("Draws circles passing through two points with a given radius.")},
        {"DrawCircle3P",       RS2::ActionDrawCircle3Points,             tr("3 Points"),                       ":/icons/circle_3_points.lci",                     tr("Draws circles passing through three perimeter points.")},
        {"DrawCircleParallel", RS2::ActionDrawCircleParallel,            tr("&Concentric"),                    ":/icons/circle_concentric.lci",                   tr("Draws concentric offset circles through a point or by distance.")},
        {"DrawCircleInscribe", RS2::ActionDrawCircleInscribe,            tr("Circle &Inscribed"),              ":/icons/circle_inscribed.lci",                    tr("Draws circles inscribed inside triangular boundaries.")},
        {"DrawCircleTan2",     RS2::ActionDrawCircleTan2EntitiesRadius,  tr("Tangential 2 Entities, Radius"),  ":/icons/circle_tangential_2circles_radius.lci",   tr("Draws circles tangent to two entities with a specified radius.")},
        {"DrawCircleTan2_1P",  RS2::ActionDrawCircleTangental2Entities1Point, tr("Tangential 2 Entities, 1 Point"),":/icons/circle_tangential_2circles_point.lci",tr("Draws circles tangent to two entities and passing through one point.")},
        {"DrawCircleTan3",     RS2::ActionDrawCircleTan3Entities,        tr("Tangential &3 Entities"),         ":/icons/circle_tangential_3entities.lci",         tr("Draws circles tangent to three entities.")},
        {"DrawCircleTan1_2P",  RS2::ActionDrawCircleTangental1Entity2Points, tr("Tangential, 2 P&oints"),      ":/icons/circle_tangential_2points.lci",           tr("Draws circles tangent to one entity and passing through two points.")}
    });
}

void LC_ActionFactory::createDrawCurveActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group, {
        {"DrawArc",           RS2::ActionDrawArc,          tr("&Center, Point, Angles"),   ":/icons/arc_center_point_angle.lci",   tr("Draws circular arcs defined by center point, radius, and angles.")},
        {"DrawArcChord",      RS2::ActionDrawArcChord,     tr("&Center, Point, Chord"),    ":/icons/arc_center_point_chord.lci",   tr("Draws circular arcs defined by chord length and angle.")},
        {"DrawArcAngleLen",   RS2::ActionDrawArcAngleLen,  tr("&Center, Point, Arc Angle"),":/icons/arc_center_point_anglelen.lci",tr("Draws circular arcs defined by sweep angle and perimeter length.")},
        {"DrawArc3P",         RS2::ActionDrawArc3P,        tr("&3 Points"),                ":/icons/arc_3_points.lci",             tr("Draws circular arcs passing through three points.")},
        {"DrawArc2PAngle",    RS2::ActionDrawArc2PAngle,   tr("&2 Points, Angle"),         ":/icons/arc_2p_angle.lci",             tr("Draws circular arcs passing through two points with an included angle.")},
        {"DrawArc2PRadius",   RS2::ActionDrawArc2PRadius,  tr("&2 Points, Radius"),        ":/icons/arc_2p_radius.lci",            tr("Draws circular arcs passing through two points with a specified radius.")},
        {"DrawArc2PLength",   RS2::ActionDrawArc2PLength,  tr("&2 Points, Length"),        ":/icons/arc_2p_length.lci",            tr("Draws circular arcs passing through two points with a specified arc length.")},
        {"DrawArc2PHeight",   RS2::ActionDrawArc2PHeight,  tr("&2 Points, Height"),        ":/icons/arc_2p_height.lci",            tr("Draws circular arcs passing through two points with a specified sagitta height.")},
        {"DrawArcParallel",   RS2::ActionDrawArcParallel,  tr("&Concentric"),              ":/icons/arc_concentric.lci",           tr("Draws concentric offset arcs through a point or by distance.")},
        {"DrawArcTangential", RS2::ActionDrawArcTangential,tr("Arc &Tangential"),          ":/icons/arc_continuation.lci",         tr("Draws circular arcs tangential to the endpoint of an entity.")},
        {"DrawDualCurve",     RS2::ActionDrawDual,         tr("&Dual Curve"),              ":/icons/i",                            tr("Draws paired dual curve geometries.")}
    });
}

void LC_ActionFactory::createDrawSplineActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group, {
        {"DrawParabola4Points",    RS2::ActionDrawParabola4Points,      tr("Para&bola 4 points"),        ":/icons/parabola_4_points.lci",       tr("Draws parabolic curves passing through four defined points.")},
        {"DrawParabolaFD",         RS2::ActionDrawParabolaFocusDiretrix,tr("Parabola &Focus Directrix"), ":/icons/parabola_focus_directrix.lci",tr("Draws parabolic curves defined by focus point and directrix line.")},
        {"DrawHyperbolaFP",        RS2::ActionDrawHyperbolaFoci2Points, tr("&Hyperbola &Foci Point"),    ":/icons/hyperbola_foci_point.lci",    tr("Draws hyperbolic curves defined by focal points and a curve point.")},
        {"DrawSpline",             RS2::ActionDrawSpline,               tr("&Spline"),                   ":/icons/spline.lci",                  tr("Draws smooth cubic B-splines defined by control vertices.")},
        {"DrawSplinePoints",       RS2::ActionDrawSplinePoints,         tr("&Spline through points"),    ":/icons/spline_points.lci",           tr("Draws smooth spline curves passing directly through picked points.")},
        {"DrawSplinePointsAppend", RS2::ActionDrawSplinePointAppend,    tr("&Append spline point"),      ":/icons/spline_points_add.lci",       tr("Appends new fit points to an existing spline.")},
        {"DrawSplinePointsRemove", RS2::ActionDrawSplinePointRemove,    tr("&Remove spline points"),     ":/icons/spline_points_remove.lci",    tr("Removes a fit point from an existing spline.")},
        {"DrawSplinePointsAdd",    RS2::ActionDrawSplinePointAdd,       tr("&Insert spline points"),     ":/icons/spline_points_insert.lci",    tr("Inserts a new fit point into an existing spline.")},
        {"DrawSplineExplode",      RS2::ActionDrawSplineExplode,        tr("&Explode spline to lines"),  ":/icons/spline_explode.lci",          tr("Decomposes a B-spline curve into connected polylines or arcs.")},
        {"DrawSplineFromPolyline", RS2::ActionDrawSplineFromPolyline,   tr("&Spline from polyline"),     ":/icons/spline_from_polyline.lci",    tr("Fits and generates a smooth B-spline from a polyline contour.")},
        {"DrawSplinePointsDelTwo", RS2::ActionDrawSplinePointDelTwo,    tr("&Remove between two points"),":/icons/spline_points_remove_two.lci",tr("Deletes all spline segments located between two picked points.")}
    });
}

void LC_ActionFactory::createDrawEllipseActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"DrawEllipse1Point",       RS2::ActionDrawEllipse1Point,       tr("&Ellipse (1 Point)"),          ":/icons/ellipse_1_point.lci",        tr("Draws ellipses defined by center point and a perimeter point.")},
        {"DrawEllipseArc1Point",    RS2::ActionDrawEllipseArc1Point,    tr("&Ellipse Arc (1 Point)"),      ":/icons/ellipse_arc_1_point.lci",    tr("Draws elliptical arcs passing through a center point and boundary point.")},
        {"DrawEllipseAxis",         RS2::ActionDrawEllipseAxis,         tr("&Ellipse (Axis)"),             ":/icons/ellipse_axis.lci",           tr("Draws full ellipses defined by center point and major/minor axes.")},
        {"DrawEllipseArcAxis",      RS2::ActionDrawEllipseArcAxis,      tr("Ellipse &Arc (Axis)"),         ":/icons/ellipse_arc_axis.lci",       tr("Draws elliptical arcs defined by major/minor axes and angles.")},
        {"DrawEllipseFociPoint",    RS2::ActionDrawEllipseFociPoint,    tr("Ellipse &Foci Point"),         ":/icons/ellipse_foci_point.lci",     tr("Draws ellipses defined by two focal points and a perimeter point.")},
        {"DrawEllipse4Points",      RS2::ActionDrawEllipse4Points,      tr("Ellipse &4 Point"),            ":/icons/ellipse_4_points.lci",       tr("Draws ellipses passing through four points.")},
        {"DrawEllipseCenter3Points",RS2::ActionDrawEllipseCenter3Points,tr("Ellipse Center and &3 Points"),":/icons/ellipse_center_3_points.lci",tr("Draws ellipses defined by center point and three perimeter points.")},
        {"DrawEllipseInscribe",     RS2::ActionDrawEllipseInscribe,     tr("Ellipse &Inscribed"),          ":/icons/ellipse_inscribed.lci",      tr("Draws ellipses inscribed inside a quadrilateral boundary.")}
    });
}

void LC_ActionFactory::createDrawPolylineActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group,{
        {"DrawPolyline",       RS2::ActionDrawPolyline,       tr("&Polyline"),                       ":/icons/polylines_polyline.lci",               tr("Draws continuous sequences of connected line and arc segments.")},
        {"PolylineAdd",        RS2::ActionPolylineAdd,        tr("&Add node"),                       ":/icons/insert_node.lci",                      tr("Inserts a new vertex node into an existing polyline.")},
        {"PolylineAppend",     RS2::ActionPolylineAppend,     tr("A&ppend node"),                    ":/icons/append_node.lci",                      tr("Appends new segment nodes to the end of an existing polyline.")},
        {"PolylineDel",        RS2::ActionPolylineDel,        tr("&Delete node"),                    ":/icons/delete_node.lci",                      tr("Deletes an individual vertex node from a polyline.")},
        {"PolylineDelBetween", RS2::ActionPolylineDelBetween, tr("Delete &between two nodes"),       ":/icons/delete_between_nodes.lci",             tr("Deletes all polyline segments situated between two selected nodes.")},
        {"PolylineTrim",       RS2::ActionPolylineTrim,       tr("&Trim segments"),                  ":/icons/trim.lci",                             tr("Trims or extends polyline segments to a boundary entity.")},
        {"PolylineEquidistant",RS2::ActionPolylineEquidistant,tr("Create &Equidistant Polylines"),   ":/icons/create_equidistant_polyline.lci",      tr("Creates parallel equidistant offset polylines.")},
        {"PolylineSegment",    RS2::ActionPolylineSegment,    tr("Polyline from Existing &Segments"),":/icons/create_polyline_from_existing_segments.lci", tr("Converts connected individual line and arc entities into a polyline.")},
        {"PolylineArcToLines", RS2::ActionPolylineArcsToLines,tr("Polyline Arcs to Chords"),         ":/icons/polyline_arc_to_lines.lci",            tr("Segments curved polyline arcs into straight chord approximations.")},
        {"PolylineSegmentType", RS2::ActionPolylineChangeSegmentType,    tr("Polyline Change Segment Type"),           ":/icons/polyline_segment_type.lci",           tr("Toggles polyline segments between straight lines and curved arcs.")}
    });
}

void LC_ActionFactory::createDrawOtherActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"ZoomPan",        RS2::ActionZoomPan,        tr("Zoom &Panning"),":/icons/zoom_pan.lci",    tr("Pans the visual viewport dynamically following cursor drag.")},
        {"DrawMText",      RS2::ActionDrawMText,      tr("&MText"),       ":/icons/mtext.lci",       tr("Creates formatted multi-line paragraphs using vector fonts.")},
        {"DrawText",       RS2::ActionDrawText,       tr("&Text"),        ":/icons/text.lci",        tr("Creates single-line or multi-line vector text annotations.")},
        {"DrawHatch",      RS2::ActionDrawHatch,      tr("&Hatch"),       ":/icons/hatch.lci",       tr("Fills enclosed closed boundaries with associative pattern hatching.")},
        {"DrawImage",      RS2::ActionDrawImage,      tr("Insert &Image"),":/icons/camera.lci",      tr("Inserts and positions external bitmap images into the CAD drawing.")},
        {"DrawBoundingBox",RS2::ActionDrawBoundingBox,tr("Bounding &Box"),":/icons/bounding_box.lci",tr("Calculates and draws the rectangular boundary box of entities.")}
    });
}

void LC_ActionFactory::createDrawDimensionsActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group,{
        {"DimAligned",        RS2::ActionDimAligned,          tr("&Aligned"),                ":/icons/dim_aligned.lci",            tr("Draws aligned linear dimensions parallel to two measurement points.")},
        {"DimLinear",         RS2::ActionDimLinear,           tr("&Linear"),                 ":/icons/dim_linear.lci",             tr("Draws orthogonal linear dimensions at a specified angle.")},
        {"DimLinearHor",      RS2::ActionDimLinearHor,        tr("&Horizontal"),             ":/icons/dim_horizontal.lci",         tr("Draws strictly horizontal linear dimension entities.")},
        {"DimLinearVer",      RS2::ActionDimLinearVer,        tr("&Vertical"),               ":/icons/dim_vertical.lci",           tr("Draws strictly vertical linear dimension entities.")},
        {"DimRadial",         RS2::ActionDimRadial,           tr("&Radial"),                 ":/icons/dim_radial.lci",             tr("Draws radius annotations with leader arrows for arcs and circles.")},
        {"DimDiametric",      RS2::ActionDimDiametric,        tr("&Diametric"),              ":/icons/dim_diametric.lci",          tr("Draws diameter annotations with leader arrows for circles and arcs.")},
        {"DimAngular",        RS2::ActionDimAngular,          tr("&Angular"),                ":/icons/dim_angular.lci",            tr("Measures and annotates angles between two intersecting lines.")},
        {"DimArc",            RS2::ActionDimArc,              tr("&Arc"),                    ":/icons/dim_arc.lci",                tr("Annotates the circumferential curve length of circular arcs.")},
        {"DimLeader",         RS2::ActionDimLeader,           tr("&Leader"),                 ":/icons/dim_leader.lci",             tr("Draws annotation callout leaders with customizable text arrows.")},
        {"DimBaseline",       RS2::ActionDimBaseline,         tr("&Baseline"),               ":/icons/dim_baseline.lci",           tr("Creates stacked baseline dimensions continuing from a previous base.")},
        {"DimContinue",       RS2::ActionDimContinue,         tr("&Continue"),               ":/icons/dim_continue.lci",           tr("Creates continuous chained dimensions extending from a previous line.")},
        {"DimOrdinate",       RS2::ActionDimOrdinate,         tr("&Ordinate"),               ":/icons/dim_ordinate.lci",           tr("Draws datum ordinate X or Y coordinate dimensions.")},
        {"DimOrdinateForBase",RS2::ActionDimOrdByOriginSelect,tr("Select Ordinates by base"),":/icons/dim_ordinate_by_origin.lci", tr("Designates a base reference origin for datum ordinate dimensions.")},
        {"DimOrdinateReBase", RS2::ActionDimOrdRebase,        tr("Ordinates Re-base"),       ":/icons/dim_ordinate_rebase.lci",    tr("Re-bases an existing ordinate dimension to a new reference origin.")},
        {"DimPickApply",      RS2::ActionDimStyleApply,       tr("Copy Style"),              ":/icons/dim_apply_style.lci",        tr("Picks dimension style variables and applies them to other entities.")},
        {"DimModify",         RS2::ActionDimModify,           tr("Modify Style"),            ":/icons/dim_modify_style.lci",       tr("Modifies existing dimension style variables.")},
        {"GTDFeatureFrame",   RS2::ActionGTDFCFrame,          tr("Feature Control Frame"),   ":/icons/gdt_featurecontrolframe.lci",tr("Creates GTD Feature Frame")}
    });
}


void LC_ActionFactory::createModifyActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group,{
        {"ModifyAttributes",     RS2::ActionModifyAttributes,     tr("&Attributes"),               ":/icons/attributes.lci",             tr("Modifies layer, color, line type, and pen width attributes.")},
        {"ModifyDelete",         RS2::ActionModifyDelete,         tr("&Delete"),                   ":/icons/delete.lci",                 tr("Permanently removes selected entities from the drawing.")},
        {"ModifyMove",           RS2::ActionModifyMove,           tr("&Move / Copy"),              ":/icons/move_copy.lci",              tr("Moves or duplicates selected entities from a base point.")},
        {"ModifyRevertDirection",RS2::ActionModifyRevertDirection,tr("Re&vert direction"),         ":/icons/revert_direction.lci",       tr("Inverts the start and end orientation of lines, arcs, and polylines.")},
        {"ModifyRotate",         RS2::ActionModifyRotate,         tr("&Rotate"),                   ":/icons/rotate.lci",                 tr("Rotates selected entities around a reference center point.")},
        {"ModifyScale",          RS2::ActionModifyScale,          tr("&Scale"),                    ":/icons/scale.lci",                  tr("Scales selected entities larger or smaller around a reference point.")},
        {"ModifyMirror",         RS2::ActionModifyMirror,         tr("&Mirror"),                   ":/icons/mirror.lci",                 tr("Reflects and mirrors selected entities across a defined axis.")},
        {"ModifyMoveRotate",     RS2::ActionModifyMoveRotate,     tr("Mo&ve and Rotate"),          ":/icons/move_rotate.lci",            tr("Moves and rotates selected entities in a single combined operation.")},
        {"ModifyRotate2",        RS2::ActionModifyRotateTwice,    tr("Rotate T&wo"),               ":/icons/rotate2.lci",                tr("Rotates entities simultaneously around two distinct center points.")},
        {"ModifyEntity",         RS2::ActionModifyEntity,         tr("&Properties"),               ":/icons/properties.lci",             tr("Opens the geometry properties editor for the clicked entity.")},
        {"ModifyTrim",           RS2::ActionModifyTrim,           tr("&Trim"),                     ":/icons/trim.lci",                   tr("Trims or extends an entity up to an intersecting boundary line.")},
        {"ModifyTrim2",          RS2::ActionModifyTrim2,          tr("Tr&im Two"),                 ":/icons/trim2.lci",                  tr("Trims two intersecting entities to form a clean joint corner.")},
        {"ModifyTrimAmount",     RS2::ActionModifyTrimAmount,     tr("&Lengthen"),                 ":/icons/trim_value.lci",             tr("Extends or shortens entities by a specified numerical length.")},
        {"ModifyOffset",         RS2::ActionModifyOffset,         tr("O&ffset"),                   ":/icons/offset.lci",                 tr("Generates parallel or concentric offset curves at a fixed distance.")},
        {"ModifyCut",            RS2::ActionModifyCut,            tr("&Divide"),                   ":/icons/divide.lci",                 tr("Divides an entity into two separate segments at a picked point.")},
        {"ModifyStretch",        RS2::ActionModifyStretch,        tr("&Stretch"),                  ":/icons/stretch.lci",                tr("Stretches shapes and moves vertices enclosed in a crossing window.")},
        {"ModifyBevel",          RS2::ActionModifyBevel,          tr("&Bevel"),                    ":/icons/bevel.lci",                  tr("Creates a beveled chamfer corner between two intersecting lines.")},
        {"ModifyRound",          RS2::ActionModifyRound,          tr("&Fillet"),                   ":/icons/fillet.lci",                 tr("Creates a rounded fillet curve between two intersecting entities.")},
        {"ModifyExplodeText",    RS2::ActionModifyExplodeText,    tr("&Explode Text into Letters"),":/icons/explode_text_to_letters.lci",tr("Decomposes text strings into editable line and arc contours.")},
        {"BlocksExplode",        RS2::ActionBlocksExplode,        tr("Ex&plode"),                  ":/icons/explode.lci",                tr("Explodes block references and complex objects into basic entities.")},
        {"ModifyBreakDivide",    RS2::ActionModifyBreakDivide,    tr("Break/Divide"),              ":/icons/break_out_trim.lci",         tr("Breaks or divides an entity between two picked boundary points.")},
        {"ModifyLineGap",        RS2::ActionModifyLineGap,        tr("Line Gap"),                  ":/icons/line_gap.lci",               tr("Creates a gap opening of defined width in a line entity.")},
        {"ModifyLineJoin",       RS2::ActionModifyLineJoin,       tr("Line Join"),                 ":/icons/line_join.lci",              tr("Merges collinear segments into a single continuous entity.")},
        {"ModifyDuplicate",      RS2::ActionModifyDuplicate,      tr("Duplicate"),                 ":/icons/duplicate.lci",              tr("Creates fast in-place duplicates of selected entities.")},
        {"ModifyAlign",          RS2::ActionModifyAlign,          tr("Align"),                     ":/icons/halign_left.lci",            tr("Aligns selected entities using two source and destination point pairs.")},
        {"ModifyAlignOne",       RS2::ActionModifyAlignOne,       tr("Align One"),                 ":/icons/align_one.lci",              tr("Aligns selected entities to a target axis using one reference point.")},
        {"ModifyAlignRef",       RS2::ActionModifyAlignRef,       tr("Align Reference Points"),    ":/icons/align_ref.lci",              tr("Aligns entities matching a reference angle and displacement.")}
    });
}

void LC_ActionFactory::createPenActionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group, {
        {"PenSyncFromLayer", RS2::ActionPenSyncFromLayer, tr("Update Current Pen by Active Layer' Pen"), ":/icons/back.lci", tr("Synchronizes current pen attributes to match the active layer.")}
    });
}
void LC_ActionFactory::createPenActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createSpecialAction(map, group, LC_ActionNames::WidgetPenControls, tr("Pen Controls Bar (Widget)"), ":/icons/pen_apply.lci");

    createActionHandlerActions(map, group, {
        {"PenPick",        RS2::ActionPenPick,        tr("&Pick Pen From Entity"),           ":/icons/pen_pick_entity.lci",  tr("Sets active pen color, width, and line type matching clicked entity.")},
        {"PenPickResolved",RS2::ActionPenPickResolved,tr("&Pick Pen From Entity (Resolved)"),":/icons/pen_pick_resolved.lci",tr("Picks resolved layer attributes of an entity as active pen.")},
        {"PenApply",       RS2::ActionPenApply,       tr("Apply Pen to Entity"),             ":/icons/pen_apply.lci",        tr("Applies current active pen attributes to clicked entities.")},
        {"PenCopy",        RS2::ActionPenCopy,        tr("Copy Pen"),                        ":/icons/pen_copy.lci",         tr("Copies pen attributes from one entity directly to another.")}
    });
}

void LC_ActionFactory::createInfoCursorActions(QMap<QString, QAction *> &map, QActionGroup *group) {
    createMainWindowActions(map, group, {
        {"EntityDescriptionInfo", &QC_ApplicationWindow::slotShowEntityDescriptionOnHover, tr("Show Entity Description"), ":/icons/entity_description_info.lci", tr("Displays entity description and layer information in cursor overlay.")}
    });
    createActions(map, group, {
        {"InfoCursorEnable",       tr("Enable Info Cursor"),":/icons/info_cursor_enable.lci",      tr("Toggles visibility of the interactive info cursor overlay.")},
        {"InfoCursorAbs",          tr("Absolute Pos"),      ":/icons/info_cursor_zone1.lci",       tr("Displays absolute Cartesian coordinates on the cursor overlay.")},
        {"InfoCursorSnap",         tr("Snap"),              ":/icons/info_cursor_zone2.lci",       tr("Displays active magnetic snap mode details on the cursor overlay.")},
        {"InfoCursorRel",          tr("Relative"),          ":/icons/info_cursor_zone3.lci",       tr("Displays relative coordinates from relative zero on cursor overlay.")},
        {"InfoCursorPrompt",       tr("Prompt"),            ":/icons/info_cursor_zone4.lci",       tr("Displays action instructions and prompts beside the cursor.")},
        {"InfoCursorCatchedEntity",tr("Caught Entity"),     ":/icons/info_cursor_zone2_entity.lci",tr("Displays hovered entity properties and type on cursor overlay.")}
    });
}

void LC_ActionFactory::createSnapActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createSpecialAction(map, group, LC_ActionNames::WidgetSnapControls, tr("Snap Controls Bar (Widget)"), ":/icons/snap_intersection.lci", tr("Toolbar widget for configuring magnetic entity snaps."));

    createActions(map, group, {
        {"SnapVisual",               tr("Snap Visual"),                  ":/icons/snap_visual.lci",                     tr("Enables visual snap guides and dynamic alignment rays.")},
        {"SnapVisualLock",           tr("Toggle Snap Visual Lock"),      ":/icons/snap_visual_lock.lci",                tr("Locks visual snap reference lines and points.")},
        {"SnapVisualDynDistance",    tr("Show Dynamic Distance Circles"),":/icons/snap_visual_dynamic_distance.lci",    tr("Displays dynamic distance circle guides from snapped vertices.")},
        {"SnapVisualDistanceTan",    tr("Show Distance Tangentials"),    ":/icons/snap_visual_dynamic_distance_tangent.lci", tr("Displays tangent guide circles from snapped vertices.")},
        {"SnapVisualAngleSnap",      tr("Show Angle Rays"),              ":/icons/snap_visual_angle_snap.lci",          tr("Displays angle snap ray guides from snapped vertices.")},
        {"SnapVisualRelAngleSnap",   tr("Show Angle Relative Rays"),     ":/icons/snap_visual_rel_angle_snap.lci",      tr("Displays relative angle rays aligned with existing segments.")},
        {"SnapVisualAutoAddSnap",    tr("Add Snap Points"),              ":/icons/snap_visual_add_snaps_auto.lci",      tr("Automatically adds snap points as references.")},
        {"SnapVisualAutoAddSnapLast",tr("Add Last Snap Only"),           ":/icons/snap_visual_add_snaps_last.lci",      tr("Limits visual guide points to the most recently snapped vertex.")},
        {"SnapVisualShowFarGuides",  tr("Show Far Guides"),              ":/icons/snap_visual_show_far_guides.lci",     tr("Displays extended visual projection lines across the viewport.")},
        {"SnapGrid",                 tr("Snap on Grid"),                 ":/icons/snap_grid.lci",                       tr("Snaps cursor to the nearest drawing grid intersection.")},
        {"SnapMiddleManual",         tr("Snap Middle Manual"),           ":/icons/snap_middle_manual.lci",              tr("Snaps to the calculated midpoint between two picked points.")},
        {"SnapEnd",                  tr("Snap on Endpoints"),            ":/icons/snap_endpoints_short.lci",            tr("Snaps cursor to endpoints of lines, arcs, and polylines.")},
        {"SnapEntity",               tr("Snap on Entity"),               ":/icons/snap_entity_short.lci",               tr("Snaps cursor anywhere directly along an entity contour.")},
        {"SnapCenter",               tr("Snap Center"),                  ":/icons/snap_center_short.lci",               tr("Snaps cursor to center points of circles, arcs, and ellipses.")},
        {"SnapMiddle",               tr("Snap Middle"),                  ":/icons/snap_middle_short.lci",               tr("Snaps cursor to midpoints of lines, arcs, and segments.")},
        {"SnapDistance",             tr("Snap Distance"),                ":/icons/snap_distance_short.lci",             tr("Snaps cursor at a specified distance from entity endpoints.")},
        {"SnapIntersection",         tr("Snap Intersection"),            ":/icons/snap_intersection.lci",               tr("Snaps cursor to intersection points between entities.")}
   });
}

void LC_ActionFactory::createRelativeInputActions(QMap<QString, QAction*>& map, QActionGroup* group) {
    createActions(map, group, {
       {"RelativeInputLengh",tr("Relative By Distance"),":/icons/relative_len.lci",  tr("Inputs next point using distance and angle from relative zero.")},
       {"RelativeInputAngle",tr("Relative By Angle"),   ":/icons/relative_angle.lci",tr("Inputs next point using fixed angle relative to previous point.")},
       {"RelativeInputDX",   tr("Relative By X Offset"),":/icons/relative_dx.lci",   tr("Inputs next point using relative X delta coordinate.")},
       {"RelativeInputDY",   tr("Relative By Y Offset"),":/icons/relative_dy.lci",   tr("Inputs next point using relative Y delta coordinate.")},
       {"RelativeInputX",    tr("Relative With X"),     ":/icons/relative_x.lci",    tr("Constrains next point to a fixed absolute X coordinate.")},
       {"RelativeInputY",    tr("Relative With Y"),     ":/icons/relative_y.lci",    tr("Constrains next point to a fixed absolute Y coordinate.")},
       {"RelativeAddLine",   tr("Add Guiding Line"),    ":/icons/guiding_line.lci",  tr("Creates an interactive guiding construction line.")},
       {"RelativeAddCircle", tr("Add Guiding Circle"),  ":/icons/guiding_circle.lci",tr("Creates an interactive guiding construction circle.")},
       {"RelativeAddPoint",  tr("Add Guiding Point"),   ":/icons/guiding_point.lci", tr("Creates an interactive guiding construction point.")}
    });

    auto tryShowRelativeInput = [](const QC_ApplicationWindow* appWin, RS2::RelativePointParam type) {
        LC_ActionContext* ctx = appWin->getActionContext();
        RS_ActionInterface* currentAction = ctx->getCurrentAction();
        if (currentAction != nullptr) {
            currentAction->tryShowRelativeInput(type);
        }
    };

    connect(map["RelativeInputLengh"], &QAction::triggered, this, [this, tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_LENGTH);
    });
    connect(map["RelativeInputAngle"], &QAction::triggered, this, [this,tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_ANGLE);
    });
    connect(map["RelativeInputDX"], &QAction::triggered, this, [this,tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_DX);
    });
    connect(map["RelativeInputDY"], &QAction::triggered, this, [this,tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_DY);
    });
    connect(map["RelativeInputX"], &QAction::triggered, this, [this,tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_X);
    });
    connect(map["RelativeInputY"], &QAction::triggered, this, [this,tryShowRelativeInput](bool) {
        tryShowRelativeInput(m_appWin, RS2::REL_POINT_Y);
    });

    auto tryAddVisualGuide = [](const QC_ApplicationWindow* appWin, bool hasLength, bool hasAngle, bool hasDx, bool hasDy, bool hasNormal) {
        LC_ActionContext* ctx = appWin->getActionContext();
        const auto currentAction = dynamic_cast<RS_PreviewActionInterface*>(ctx->getCurrentAction());
        if (currentAction != nullptr) {
            currentAction->tryAddVisualGuidingPointForCurrentPoint(hasLength, hasAngle, hasDx, hasDy, hasNormal);
        }
    };

    connect(map["RelativeAddLine"], &QAction::triggered, this, [this, tryAddVisualGuide](bool) {
       tryAddVisualGuide(m_appWin, false, true, false, false, true);
    });
    connect(map["RelativeAddCircle"], &QAction::triggered, this, [this,tryAddVisualGuide](bool) {
        tryAddVisualGuide(m_appWin, true, false, false, false, false);
    });
    connect(map["RelativeAddPoint"], &QAction::triggered, this, [this,tryAddVisualGuide](bool) {
        tryAddVisualGuide(m_appWin, true, true, true, true, true);
    });
}

void LC_ActionFactory::createRestrictActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActions(map, group, {
        {"RestrictHorizontal", tr("Restrict Horizontal"), ":/icons/restr_hor.lci",         tr("Constrains cursor movement strictly to horizontal axis.")},
        {"RestrictVertical",   tr("Restrict Vertical"),   ":/icons/restr_ver.lci",         tr("Constrains cursor movement strictly to vertical axis.")},
        {"RestrictOrthogonal", tr("Restrict Orthogonal"), ":/icons/restr_ortho.lci",       tr("Constrains cursor movement to 90-degree orthogonal axes.")},
        {"RestrictNothing",    tr("Restrict Nothing"),    ":/icons/restrict_nothing.lci",  tr("Disables directional cursor movement constraints.")}
    });
}

void LC_ActionFactory::createUCSActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createSpecialActions(map, group, {
       {LC_ActionNames::WidgetUCSSelector,tr("UCS Selector (Widget)"),             ":/icons/ucs_set_wcs.lci",tr("User coordinate system selector dropdown widget.")},
       {LC_ActionNames::MenuUCSList,      tr("User Coordinate Systems (Sub-Menu)"),":/icons/ucs_ucs.lci",    tr("List of saved user coordinate systems.")}
   });

    createActions(map, group, {
        {"UCSSetWCS", RS2::ActionUCSCreate, tr("To WCS"), ":/icons/ucs_set_wcs.lci", tr("Resets active coordinate system to World Coordinate System (WCS).")}
    });

    createActionHandlerActions(map, group, {
        {"UCSCreate",          RS2::ActionUCSCreate,          tr("Create UCS"),                   ":/icons/ucs_add.lci",               tr("Defines a new User Coordinate System (UCS) by origin and axes.")},
        {"UCSSetByDimOrdinate",RS2::ActionUCSSetByDimOrdinate,tr("Set UCS By Ordinate Dimension"),":/icons/ucs_set_by_dimordinate.lci",tr("Aligns UCS origin with an ordinate dimension base point.")}
    });
}

void LC_ActionFactory::createWorkspacesActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group){
   createSpecialActions(map, group, {
        {LC_ActionNames::WidgetWorkspaceSelector, tr("Workspace Selector (Widget)"),   ":/icons/workspace.lci",         tr("Workspace selector dropdown widget.")},
        {LC_ActionNames::MenuWorkspacesList,      tr("Saved Workspaces (Sub-Menu)"),   ":/icons/workspace.lci",         tr("List of saved workspace navigation layouts.")},
        {LC_ActionNames::MenuDrawings,            tr("Drawings & Windows (Sub-Menu)"), ":/icons/workspace.lci",         tr("List of currently opened drawing windows.")},
        {LC_ActionNames::MenuDockWidgets,         tr("Tool Windows (Sub-Menu)"),       ":/icons/dockwidgets_left.lci",  tr("Sub-menu to toggle individual dock widgets.")},
        {LC_ActionNames::MenuCadDockWidgets,      tr("CAD Tool Windows (Sub-Menu)"),   ":/icons/dockwidgets_left.lci",  tr("Sub-menu to toggle CAD tool dock widgets.")},
        {LC_ActionNames::MenuToolbars,            tr("Toolbars (Sub-Menu)"),           ":/icons/create_toolbar.lci",    tr("Sub-menu to toggle application toolbars.")},
        {LC_ActionNames::MenuCadToolbars,         tr("CAD Toolbars (Sub-Menu)"),       ":/icons/create_toolbar.lci",    tr("Sub-menu to toggle CAD toolbars.")}
    });

    createMainWindowActions(map, group, {
        {"WorkspaceCreate",     &QC_ApplicationWindow::saveWorkspace,       tr("Save Workspace"),        ":/icons/workspace_save.lci",    tr("Saves current window and dock configuration as a workspace layout.")},
        {"WorkspaceRemove",     &QC_ApplicationWindow::removeWorkspace,     tr("Remove Workspace"),      ":/icons/workspace_remove.lci",  tr("Deletes a saved workspace layout from disk.")},
        {"WorkspaceRestore",    &QC_ApplicationWindow::restoreWorkspace,    tr("Restore Workspace"),     ":/icons/workspace.lci",         tr("Restores a saved workspace window and dock configuration.")},
        {"RedockWidgets",       &QC_ApplicationWindow::slotRedockWidgets,   tr("Re-dock Tool Windows"),  nullptr,                         tr("Re-docks all floating tools back to default dock areas.")},
        {"InvokeMenuCreator",   &QC_ApplicationWindow::invokeMenuCreator,   tr("Custom Menu Creator"),   ":/icons/create_menu.lci",       tr("Opens custom context menu editor dialog.")},
        {"InvokeToolbarCreator",&QC_ApplicationWindow::invokeToolbarCreator,tr("Custom Toolbar Creator"),":/icons/create_toolbar.lci",    tr("Opens main menu and toolbars customization dialog.")}
    });
}

void LC_ActionFactory::createRelZeroActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"SetRelativeZero",  RS2::ActionSetRelativeZero, tr("Set relative zero position"),  ":/icons/set_rel_zero.lci",  tr("Sets relative zero origin to a clicked point.")}
    });
    createActions(map, group, {
        {"LockRelativeZero",   tr("Lock relative zero position"), ":/icons/lock_rel_zero.lci", tr("Prevents relative zero origin from automatically updating on clicks.")}
        // todo - add action for hiding/showing related zero
       //{"RestrictOrthogonal", tr("Restrict Orthogonal"),         ":/icons/restr_ortho.lci"}
    });
}

void LC_ActionFactory::createSnapExtraActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActions(map, group, {
        {"ExclusiveSnapMode",tr("Exclusive Snap Mode"),":/icons/exclusive.lci",tr("Toggles exclusive snapping: activates single snap mode, disabling others.")},
        {"SnapFree",         tr("Free Snap"),          ":/icons/snap_free.lci",tr("Positions cursor freely without magnetic snapping.")}
    });
}

void LC_ActionFactory::createOrderActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"OrderBottom",RS2::ActionOrderBottom,tr("Move to Bottom"),    ":/icons/downmost.lci",tr("Sends selected entities to the very bottom of the visual draw order.")},
        {"OrderLower", RS2::ActionOrderLower, tr("Lower After Entity"),":/icons/down.lci",    tr("Lowers selected entities one step backward in visual draw order.")},
        {"OrderRaise", RS2::ActionOrderRaise, tr("Raise Over Entity"), ":/icons/up.lci",      tr("Raises selected entities one step forward in visual draw order.")},
        {"OrderTop",   RS2::ActionOrderTop,   tr("Move to Top"),       ":/icons/upmost.lci",  tr("Brings selected entities to the very top of the visual draw order.")}
    });
}

void LC_ActionFactory::createInfoActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"InfoPoint",       RS2::ActionInfoPoint,           tr("&Point Coordinates"),                ":/icons/info_point.lci",                 tr("Queries absolute, relative, and polar coordinates of a clicked point.")},
        {"InfoDist",        RS2::ActionInfoDistPoint2Point, tr("&Distance Point to Point"),          ":/icons/distance_point_to_point.lci",    tr("Measures linear, horizontal, and vertical distances between two points.")},
        {"InfoDist2",       RS2::ActionInfoDistEntity2Point,tr("Distance &Entity to Point"),         ":/icons/distance_entity_to_point.lci",   tr("Measures perpendicular distance from a point to a target entity.")},
        {"InfoDist3",       RS2::ActionInfoDistPoint2Entity,tr("Distance &Point to Entity"),         ":/icons/distance_point_to_entity.lci",   tr("Measures clearance distance between two parallel entities.")},
        {"InfoAngle",       RS2::ActionInfoAngle,           tr("An&gle between two lines"),          ":/icons/angle_line_to_line.lci",         tr("Measures the angle between two selected lines.")},
        {"InfoTotalLength", RS2::ActionInfoTotalLength,     tr("Total &length of selected entities"),":/icons/total_length_selected_entities.lci", tr("Calculates the cumulative perimeter length of selected entities.")},
        {"InfoArea",        RS2::ActionInfoArea,            tr("Polygonal &Area"),                   ":/icons/polygonal_area.lci",             tr("Calculates the enclosed area and perimeter circumference of a polygon.")},
        {"EntityInfo",      RS2::ActionInfoProperties,      tr("Entity Pro&perties"),                ":/icons/entity_properties_select.lci",   tr("Displays detailed geometry and layer metrics for a clicked entity.")},
        {"PickCoordinates", RS2::ActionInfoPickCoordinates, tr("Collect &Coordinates"),              ":/icons/pick_coordinates.lci",           tr("Collects and logs coordinates sequentially from clicked locations.")},
        {"InfoAngle3Points",RS2::ActionInfoAngle3Points,    tr("Ang&le between 3 points"),           ":/icons/angle_3_points.lci",             tr("Measures the interior angle defined by vertex and two ray points.")}
    });
}

void LC_ActionFactory::createViewActions(QMap<QString, QAction*>& map, QActionGroup* group) {
    createActionHandlerActions(map, group, {
        {"ZoomWindow", RS2::ActionZoomWindow, tr("&Window Zoom"), ":/icons/zoom_window.lci", tr("Zooms into a rectangular window specified with cursor.")}
    });

    createMainWindowActions(map, group, {
        {"MainMenu",         &QC_ApplicationWindow::toggleMainMenu,       tr("&Main Menu"),            ":/icons/main_menu.lci",         tr("Toggles visibility of the top menu bar.")},
        {"Fullscreen",       &QC_ApplicationWindow::toggleFullscreen,     tr("&Fullscreen"),           ":/icons/fullscreen.lci",        tr("Toggles full screen application display mode.")},
        {"ViewGrid",         &QC_ApplicationWindow::slotViewGrid,         tr("&Grid"),                 ":/icons/grid.lci",              tr("Toggles visibility of the drawing grid.")},
        {"ViewDraft",        &QC_ApplicationWindow::slotViewDraft,        tr("&Draft"),                ":/icons/draft.lci",             tr("Toggles draft mode: hides line widths for maximum redraw speed.")},
        {"ViewLinesDraft",   &QC_ApplicationWindow::slotViewDraftLines,   tr("&Draft Lines"),          ":/icons/draftLineWidth.lci",    tr("Toggles draft mode for zero-width hair lines.")},
        {"ViewAntialiasing", &QC_ApplicationWindow::slotViewAntialiasing, tr("&Antialiasing"),         ":/icons/anti_aliasing.lci",     tr("Toggles smooth anti-aliased rendering for graphics.")},
        {"ViewStatusBar",    &QC_ApplicationWindow::slotViewStatusBar,    tr("&Statusbar"),            nullptr,                         tr("Toggles visibility of the bottom status bar.")},
        {"ViewGridOrtho",    &QC_ApplicationWindow::slotViewGridOrtho,    tr("&Orthogonal Grid"),      ":/icons/grid_ortho.lci",        tr("Switches drawing grid to standard Cartesian orthogonal grid.")},
        {"ViewGridIsoLeft",  &QC_ApplicationWindow::slotViewGridIsoLeft,  tr("&Isometric Left Grid"),  ":/icons/grid_iso_left.lci",     tr("Switches drawing grid to isometric left plane.")},
        {"ViewGridIsoTop",   &QC_ApplicationWindow::slotViewGridIsoTop,   tr("&Isometric Top Grid"),   ":/icons/grid_iso_top.lci",      tr("Switches drawing grid to isometric top plane.")},
        {"ViewGridIsoRight", &QC_ApplicationWindow::slotViewGridIsoRight, tr("&Isometric Right Grid"), ":/icons/grid_iso_right.lci",    tr("Switches drawing grid to isometric right plane.")}
    }, true);
}


void LC_ActionFactory::createLayerActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"LayersDefreezeAll",       RS2::ActionLayersDefreezeAll,       tr("&Show all layers"),          ":/icons/visible_all.lci",          tr("Makes all drawing layers visible.")},
        {"LayersFreezeAll",         RS2::ActionLayersFreezeAll,         tr("&Hide all layers"),          ":/icons/not_visible_all.lci",      tr("Hides all drawing layers except active layer.")},
        {"LayersUnlockAll",         RS2::ActionLayersUnlockAll,         tr("&Unlock all"),               ":/icons/unlocked.lci",             tr("Unlocks all drawing layers for editing.")},
        {"LayersLockAll",           RS2::ActionLayersLockAll,           tr("&Lock all"),                 ":/icons/locked.lci",               tr("Locks all drawing layers to prevent accidental modification.")},
        {"LayersAdd",               RS2::ActionLayersAdd,               tr("&Add Layer"),                ":/icons/add.lci",                  tr("Creates a new layer in the drawing.")},
        {"LayersRemove",            RS2::ActionLayersRemove,            tr("&Remove Layer"),             ":/icons/remove.lci",               tr("Deletes selected layer from drawing.")},
        {"LayersEdit",              RS2::ActionLayersEdit,              tr("&Edit Layer"),               ":/icons/attributes.lci",           tr("Modifies properties, color, line type, and width of selected layer.")},
        {"LayersToggleLock",        RS2::ActionLayersToggleLock,        tr("Toggle Layer Loc&k"),        ":/icons/locked.lci",               tr("Toggles locked state of active layer.")},
        {"LayersToggleView",        RS2::ActionLayersToggleView,        tr("&Toggle Layer Visibility"),  ":/icons/visible.lci",              tr("Toggles visibility of active layer.")},
        {"LayersTogglePrint",       RS2::ActionLayersTogglePrint,       tr("Toggle Layer &Print"),       ":/icons/print.lci",                tr("Toggles print output inclusion for active layer.")},
        {"LayersToggleConstruction",RS2::ActionLayersToggleConstruction,tr("Toggle &Construction Layer"),":/icons/construction_layer.lci",   tr("Toggles construction line mode for active layer.")},
        {"LayersExportSelected",    RS2::ActionLayersExportSelected,    tr("&Export Selected Layer(s)"), ":/icons/layer_export_selected.lci",tr("Exports selected layer entities to a new DXF file.")},
        {"LayersExportVisible",     RS2::ActionLayersExportVisible,     tr("Export &Visible Layer(s)"),  ":/icons/layer_export_visible.lci", tr("Exports all visible layer entities to a new DXF file.")}
    });
}

void LC_ActionFactory::createBlockActionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createSpecialAction(map, group, LC_ActionNames::ActionEditBlock, tr("Edit Block (Context Action)"), ":/icons/properties.lci", tr("Opens selected block in the block editor window."));

    createActionHandlerActions(map, group, {
        {"BlocksDefreezeAll",RS2::ActionBlocksDefreezeAll,tr("&Show all blocks"),        ":/icons/visible_all.lci",        tr("Makes all block entities visible in the drawing.")},
        {"BlocksFreezeAll",  RS2::ActionBlocksFreezeAll,  tr("&Hide all blocks"),        ":/icons/not_visible_all.lci",    tr("Hides all block entities in the drawing.")},
        {"BlocksAdd",        RS2::ActionBlocksAdd,        tr("&Add Block"),              ":/icons/add.lci",                tr("Creates an empty block definition.")},
        {"BlocksRemove",     RS2::ActionBlocksRemove,     tr("&Remove Block"),           ":/icons/remove.lci",             tr("Deletes selected block definition and all its insert references.")},
        {"BlocksAttributes", RS2::ActionBlocksAttributes, tr("&Rename Block"),           ":/icons/rename_active_block.lci",tr("Renames active block definition.")},
        {"BlocksEdit",       RS2::ActionBlocksEdit,       tr("&Edit Block"),             ":/icons/properties.lci",         tr("Opens active block in the block editor.")},
        {"BlocksSave",       RS2::ActionBlocksSave,       tr("&Save Block"),             ":/icons/save.lci",               tr("Saves modifications to active block.")},
        {"BlocksInsert",     RS2::ActionBlocksInsert,     tr("&Insert Block"),           ":/icons/insert_active_block.lci",tr("Inserts an instance reference of a block into the drawing.")},
        {"BlocksToggleView", RS2::ActionBlocksToggleView, tr("Toggle Block &Visibility"),":/icons/visible.lci",            tr("Toggles visibility of selected block.")},
        {"BlocksCreate",     RS2::ActionBlocksCreate,     tr("&Create Block"),           ":/icons/create_block.lci",       tr("Creates a new block definition from selected entities.")}
    });
}

void LC_ActionFactory::createHelpActionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group) {
    auto* actAbout = justCreateAction(map, "HelpAbout", tr("&About"), ":/images/librecad.png", group, tr("About LibreCAD"));
    if (m_appWin != nullptr) {
        connect(actAbout, &QAction::triggered, m_appWin, &QC_ApplicationWindow::showAboutWindow);
    }

    auto* actLicense = justCreateAction(map, "HelpLicense", tr("&License"), "",  group, tr("Application's License information"));
    if (m_appWin != nullptr) {
        connect(actLicense, &QAction::triggered, m_appWin, &QC_ApplicationWindow::invokeLicenseWindow);
    }

    auto addUrlAction = [this, &map, group](const char* key, const QString& title, const char* icon, const char* url, const QString& desc) {
        auto* action = justCreateAction(map, key, title, icon, group, desc);
        connect(action, &QAction::triggered, this, [url]() {
            QDesktopServices::openUrl(QUrl(url));
        });
    };

    addUrlAction("HelpWiki",          tr("&Wiki"),           "",                           "https://dokuwiki.librecad.org/",               tr("Opens the LibreCAD online documentation wiki."));
    addUrlAction("HelpManual",        tr("User's &Manual"),  ":/icons/help.lci",           "https://librecad.readthedocs.io/",             tr("Opens the LibreCAD user manual."));
    addUrlAction("HelpCommands",      tr("&Commands"),       "",                           "https://librecad.readthedocs.io/en/latest/ref/tools.html", tr("Opens the command-line tools reference guide."));
    addUrlAction("HelpStyleSheets",   tr("&Style Sheets"),   "",                           "https://librecad.readthedocs.io/en/latest/ref/customize.html#style-sheets", tr("Opens the custom style sheets documentation."));
    addUrlAction("HelpWidgets",       tr("Wid&gets"),        "",                           "https://librecad.readthedocs.io/en/latest/ref/menu.html#widgets", tr("Opens the user interface widgets documentation."));
    addUrlAction("HelpForum",         tr("&Forum"),          "",                           "https://discourse.librecad.org/",              tr("Opens the LibreCAD community support forum."));
    addUrlAction("HelpZulip",         tr("Zulip &Chat"),     "",                           "https://librecad.zulipchat.com/",              tr("Opens the LibreCAD developer chat on Zulip."));
    addUrlAction("HelpSubmitError",   tr("&Submit Error"),   "",                           "https://github.com/LibreCAD/LibreCAD/issues/new", tr("Opens GitHub issues to report a bug."));
    addUrlAction("HelpRequestFeature",tr("&Request Feature"),"",                           "https://github.com/LibreCAD/LibreCAD/issues",   tr("Opens GitHub issues to request a new feature."));
    addUrlAction("HelpReleases",      tr("&Releases Page"),  ":/icons/feature_request.lci","https://github.com/LibreCAD/LibreCAD/releases", tr("Opens the GitHub releases page for version updates."));
    addUrlAction("HelpDonate",        tr("&Donate"),         "",                           "https://librecad.org/donate.html",              tr("Opens the donation page to support LibreCAD development."));
    addUrlAction("HelpGitHub",        tr("&GitHub Page"),    ":/icons/github.lci",         "https://github.com/LibreCAD/LibreCAD",          tr("Opens the LibreCAD source code repository on GitHub."));
    addUrlAction("HelpHome",          tr("librecad.org"),    "",                           "https://librecad.org",                          tr("Opens the official LibreCAD homepage."));
}

void LC_ActionFactory::createOptionsActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group){
    createActionHandlerActions(map, group, {
        {"OptionsDrawing", RS2::ActionOptionsDrawing, tr("Current &Drawing Preferences"), ":/icons/drawing_settings.lci", tr("Configures units, grid, dimensions, and paper for active drawing.")}
    });

    createMainWindowActions(map, group, {
        {"OptionsGeneral",      &QC_ApplicationWindow::slotOptionsGeneral,      tr("&Application Preferences"),":/icons/settings.lci",          tr("Configures global application settings, paths, and defaults.")},
        {"OptionsCustomization",&QC_ApplicationWindow::slotOptionsCustomization,tr("&Customization"),          ":/icons/options_customize.lci", tr("Customizes keyboard shortcuts, command aliases, menus, and toolbars.")},
        {"OptionsStyling",      &QC_ApplicationWindow::stylingOptionsDialog,    tr("&UI Styling"),             ":/icons/options_styling.lci",   tr("Configures themes, colors, and visual styling for the user interface.")},
        {"ShortcutsOptions",    &QC_ApplicationWindow::slotOptionsShortcuts,    tr("Keyboard Shortcuts"),      ":/icons/shortcuts_settings.lci",tr("Opens keyboard shortcuts customization page.")},
        {"DeviceOptions",       &QC_ApplicationWindow::showDeviceOptions,       tr("Device Options"),          nullptr,                         tr("Configures pointing devices, mice, and digitizer tablets.")},
        {"ReloadStyleSheet",    &QC_ApplicationWindow::reloadStyleSheet,        tr("Reload Style Sheet"),      nullptr,                         tr("Reloads active CSS style sheet from disk.")}
    });
}

void LC_ActionFactory::createFileActions(QMap<QString, QAction*>& map, QActionGroup* group) {
    createMainWindowActions(map, group, {
        {"FilePrintPreview", &QC_ApplicationWindow::slotFilePrintPreview, tr("Print Pre&view"), ":/icons/print_preview.lci", tr("Opens interactive paper scale and plot print preview layout.")}
    });
}

void LC_ActionFactory::createFileActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group){
    createActions(map, group, {
        {"FileClose", tr("&Close"), ":/icons/close.lci", tr("Closes the active drawing window.")}
    });

    createSpecialAction(map, group, LC_ActionNames::MenuRecentFiles, tr("Recent Files (Sub-Menu)"), ":/icons/fileopen.lci", tr("List of recently opened drawing files."));

    createMainWindowActions(map, group, {
        {"FileExport",     &QC_ApplicationWindow::slotFileExport,                tr("&Export as image"),  ":/icons/export.lci",             tr("Exports active drawing graphics to bitmap image formats.")},
        {"FileCloseAll",   &QC_ApplicationWindow::slotFileCloseAll,              tr("Close All"),         ":/icons/close_all.lci",          tr("Closes all currently opened drawing subwindows.")},
        {"FilePrintPDF",   &QC_ApplicationWindow::slotFilePrintPDF,              tr("Export as PDF"),     ":/icons/export_pdf.lci",         tr("Exports active drawing or preview layout directly to PDF.")},
        {"BlocksImport",   &QC_ApplicationWindow::slotImportBlock,               tr("&Block"),            ":/icons/insert_active_block.lci",tr("Imports block geometry from external drawing files.")},
        {"FileNew",        &QC_ApplicationWindow::slotFileNewFromDefaultTemplate,tr("&New"),              ":/icons/new.lci",                tr("Creates a blank untitled drawing document.")},
        {"FileNewTemplate",&QC_ApplicationWindow::slotFileNewFromTemplate,       tr("New From &Template"),":/icons/new_from_template.lci",  tr("Creates a new drawing document based on a selected template.")},
        {"FileOpen",       &QC_ApplicationWindow::slotFileOpen,                  tr("&Open..."),          ":/icons/open.lci",               tr("Opens an existing DXF, DWG, or drawing file from disk.")},
        {"FileSave",       &QC_ApplicationWindow::slotFileSave,                  tr("&Save"),             ":/icons/save.lci",               tr("Saves modifications in active drawing document to disk.")},
        {"FileSaveAs",     &QC_ApplicationWindow::slotFileSaveAs,                tr("Save &as..."),       ":/icons/save_as.lci",            tr("Saves active drawing document under a new file path.")},
        {"FileSaveAll",    &QC_ApplicationWindow::slotFileSaveAll,               tr("Save A&ll..."),      ":/icons/save_all.lci",           tr("Saves all currently opened drawing documents to disk.")},
        {"FilePrint",      &QC_ApplicationWindow::slotFilePrint,                 tr("&Print..."),         ":/icons/print.lci",              tr("Sends active drawing or print preview layout to printer.")},
        {"FileQuit",       &QC_ApplicationWindow::slotFileQuit,                  tr("&Quit"),             ":/icons/quit.lci",               tr("Exits the LibreCAD application.")},
        {"BackupExport",   &QC_ApplicationWindow::slotBackupExport,              tr("Export Configuration..."), ":/icons/save.lci",  tr("Creates an archive of application settings and custom presets.")},
        {"BackupRestore",  &QC_ApplicationWindow::slotBackupRestore,             tr("Import Configuration..."),":/icons/open.lci",  tr("Restores application settings and custom presets from an archive.")},
    });

    createAction_AH("FileExportMakerCam", RS2::ActionFileExportMakerCam, tr("Export as CA&M/plain SVG..."), nullptr, group, map, tr("Exports drawing paths to CAM/SVG format for CNC and laser machining."));
}

void LC_ActionFactory::createDockAreasActions(QMap<QString, QAction*>& map, QActionGroup* group) {
    createMainWindowActions(map, group, {
        {"LeftDockAreaToggle",       &QC_ApplicationWindow::toggleLeftDockArea,       tr("Left Dock Area"),     ":/icons/dockwidgets_left.lci",    tr("Toggles visibility of tool windows in the left dock area.")},
        {"RightDockAreaToggle",      &QC_ApplicationWindow::toggleRightDockArea,      tr("Right Dock Area"),    ":/icons/dockwidgets_right.lci",   tr("Toggles visibility of tool windows in the right dock area.")},
        {"TopDockAreaToggle",        &QC_ApplicationWindow::toggleTopDockArea,        tr("Top Dock Area"),      ":/icons/dockwidgets_top.lci",     tr("Toggles visibility of tool windows in the top dock area.")},
        {"BottomDockAreaToggle",     &QC_ApplicationWindow::toggleBottomDockArea,     tr("Bottom Dock Area"),   ":/icons/dockwidgets_bottom.lci",  tr("Toggles visibility of tool windows in the bottom dock area.")},
        {"FloatingDockwidgetsToggle",&QC_ApplicationWindow::toggleFloatingDockwidgets,tr("Floating Tools"),     ":/icons/dockwidgets_floating.lci",tr("Toggles visibility of all floating tool windows.")}
    }, true);
}

void LC_ActionFactory::createToolbarAreasActions(QMap<QString, QAction*>& map, QActionGroup* group) {
    createMainWindowActions(map, group, {
        {"LeftTBAreaToggle",   &QC_ApplicationWindow::toggleLeftToolbarArea,   tr("Left Toolbar Area"),   ":/icons/dockwidgets_left.lci",   tr("Toggles visibility of toolbars in the left dock area.")},
        {"RightTBAreaToggle",  &QC_ApplicationWindow::toggleRightToolbarArea,  tr("Right Toolbar Area"),  ":/icons/dockwidgets_right.lci",  tr("Toggles visibility of toolbars in the right dock area.")},
        {"TopTBAreaToggle",    &QC_ApplicationWindow::toggleTopToolbarArea,    tr("Top Toolbar Area"),    ":/icons/dockwidgets_top.lci",    tr("Toggles visibility of toolbars in the top dock area.")},
        {"BottomTBAreaToggle", &QC_ApplicationWindow::toggleBottomToolbarArea, tr("Bottom Toolbar Area"), ":/icons/dockwidgets_bottom.lci", tr("Toggles visibility of toolbars in the bottom dock area.")}
    }, true);
}

void LC_ActionFactory::createViewActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) {
    createSpecialAction(map, group, LC_ActionNames::MenuRecentActions, tr("Recent Actions (Sub-Menu)"), ":/icons/zoom_in.lci");

    createMainWindowActions(map, group, {
        {"FocusCommand", &QC_ApplicationWindow::slotFocusCommandLine,   tr("Focus on &Command Line"),   ":/icons/editclear.lci",       tr("Transfers keyboard focus directly to the command line input.")},
        {"FocusOptions", &QC_ApplicationWindow::slotFocusOptionsWidget, tr("Focus on &Options Widget"), ":/icons/drawing_settings.lci",tr("Transfers keyboard focus directly to the active tool options bar.")}
    });

    createActionHandlerActions(map, group, {
        {"ZoomIn",      RS2::ActionZoomIn,      tr("Zoom &In"),      ":/icons/zoom_in.lci",      tr("Zooms viewport closer in to magnify drawing details.")},
        {"ZoomOut",     RS2::ActionZoomOut,     tr("Zoom &Out"),     ":/icons/zoom_out.lci",     tr("Zooms viewport further out to view a larger drawing area.")},
        {"ZoomAuto",    RS2::ActionZoomAuto,    tr("&Auto Zoom"),    ":/icons/zoom_auto.lci",    tr("Fits and centers all drawing entities within the viewport window.")},
        {"ZoomPrevious",RS2::ActionZoomPrevious,tr("Previous &View"),":/icons/zoom_previous.lci",tr("Restores the previous viewport zoom magnification and pan center.")},
        {"ZoomRedraw",  RS2::ActionZoomRedraw,  tr("&Redraw"),       ":/icons/redraw.lci",       tr("Forces complete redraw and graphics cache refresh.")}
    });
}

void LC_ActionFactory::createNamedViewActionsUncheckable(QMap<QString, QAction*>& map, QActionGroup* group) {
    createSpecialActions(map, group, {
        {LC_ActionNames::WidgetNamedViewsSelector,tr("Named Views Selector (Widget)"),":/icons/nview_visible.lci",tr("Named views selector dropdown widget.")},
        {LC_ActionNames::MenuNamedViewsList,      tr("Saved Views (Sub-Menu)"),       ":/icons/visible_all.lci",  tr("List of saved viewport views.")}
    });

    createMainWindowActions(map, group, {
        {"ZoomViewSave",    &QC_ApplicationWindow::saveNamedView,          tr("&Save View"),          ":/icons/nview_add.lci",    tr("Saves current viewport center and zoom level as a named view.")},
        {"ZoomViewRestore", &QC_ApplicationWindow::restoreNamedViewCurrent,tr("Restore Current View"),":/icons/nview_visible.lci",tr("Restores currently selected named view in viewport.")},
        {"ZoomViewRestore1",&QC_ApplicationWindow::restoreNamedView1,      tr("Restore View 1"),      ":/icons/nview_visible.lci",tr("Restores named view #1 in viewport.")},
        {"ZoomViewRestore2",&QC_ApplicationWindow::restoreNamedView2,      tr("Restore View 2"),      ":/icons/nview_visible.lci",tr("Restores named view #2 in viewport.")},
        {"ZoomViewRestore3",&QC_ApplicationWindow::restoreNamedView3,      tr("Restore View 3"),      ":/icons/nview_visible.lci",tr("Restores named view #3 in viewport.")},
        {"ZoomViewRestore4",&QC_ApplicationWindow::restoreNamedView4,      tr("Restore View 4"),      ":/icons/nview_visible.lci",tr("Restores named view #4 in viewport.")},
        {"ZoomViewRestore5",&QC_ApplicationWindow::restoreNamedView5,      tr("Restore View 5"),      ":/icons/nview_visible.lci",tr("Restores named view #5 in viewport.")}
    });
}


void LC_ActionFactory::createSelectActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"SelectAll",   RS2::ActionSelectAll,   tr("Select &All"),     ":/icons/select_all.lci",     tr("Selects all visible and unlocked entities in the active drawing.")},
        {"DeselectAll", RS2::ActionDeselectAll, tr("Deselect &all"),   ":/icons/deselect_all.lci",   tr("Clears selection across all entities in the active drawing.")},
        {"SelectInvert",RS2::ActionSelectInvert,tr("Invert Selection"),":/icons/select_inverted.lci",tr("Inverts the selection: unselects selected and selects unselected.")}
    });
}

void LC_ActionFactory::createEditActionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"EditUndo",          RS2::ActionEditUndo,          tr("&Undo"),             ":/icons/undo.lci",  tr("Reverts the most recent drawing or editing operation.")},
        {"EditRedo",          RS2::ActionEditRedo,          tr("&Redo"),             ":/icons/redo.lci",  tr("Re-applies the most recently reverted drawing operation.")},
        {"ModifyDeleteQuick", RS2::ActionModifyDeleteQuick, tr("&Delete Selected"),  ":/icons/delete.lci",tr("Erases currently selected entities instantly.")},
        {"EditKillAllActions",RS2::ActionEditKillAllActions,tr("&Selection Pointer"),":/icons/cursor.lci",tr("Terminates all active drawing actions and returns to selection pointer.")}
    });
}

void LC_ActionFactory::createDrawDimensionsUncheckable(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"DimRegenerate",RS2::ActionDimRegenerate,tr("Regenerate Dimensions"),":/icons/dim_regenerate.lci",tr("Recalculates and redraws all dimension entity geometries.")}
    });

    const auto dimSettingsAction = justCreateAction(map,"DimStyles",tr( "&Dimension Styles"),
                                                    ":/icons/dim_style_manager.lci", group,
                                                    tr("Opens dimension formatting and styles configuration editor."));

    connect(dimSettingsAction, &QAction::triggered, this, [this](bool){
        m_appWin->changeDrawingOptions(LC_SettingsPagesDrawing::Dimensions);
    });
}

void LC_ActionFactory::createInteractivePickActions(QMap<QString, QAction *> &map, QActionGroup *group) const {
    createActionHandlerActions(map, group, {
        {"PickPoint",   RS2::ActionInteractivePickPoint,  tr("Pick Point"),   ":/icons/interactive_pick_point.lci",   tr("Picks a point interactively in drawing for an options field.")},
        {"PickPointX",  RS2::ActionInteractivePickPoint_X,tr("Pick Point X"), ":/icons/interactive_pick_point_x.lci", tr("Picks an X coordinate interactively for an options field.")},
        {"PickPointY",  RS2::ActionInteractivePickPoint_Y,tr("Pick Point Y"), ":/icons/interactive_pick_point_y.lci", tr("Picks a Y coordinate interactively for an options field.")},
        {"PickDistance",RS2::ActionInteractivePickLength, tr("Pick Distance"),":/icons/interactive_pick_distance.lci",tr("Measures a distance interactively between two points for an options field.")},
        {"PickAngle",   RS2::ActionInteractivePickAngle,  tr("Pick Angle"),   ":/icons/interactive_pick_angle.lci",   tr("Measures an angle interactively between two points for an options field.")}
    });
}

void LC_ActionFactory::createEditActions(QMap<QString, QAction*>& map, QActionGroup* group) const {
    createActionHandlerActions(map, group, {
        {"EditCut",           RS2::ActionEditCut,           tr("Cu&t"),            ":/icons/cut.lci",            tr("Cuts selected entities to clipboard with a base reference point.")},
        {"EditCutQuick",      RS2::ActionEditCutQuick,      tr("Cut Quic&k"),      ":/icons/cut.lci",            tr("Cuts selected entities to clipboard instantly without base point.")},
        {"EditCopy",          RS2::ActionEditCopy,          tr("&Copy"),           ":/icons/copy.lci",           tr("Copies selected entities to clipboard with a base reference point.")},
        {"EditCopyQuick",     RS2::ActionEditCopyQuick,     tr("Copy &Quick"),     ":/icons/copy.lci",           tr("Copies selected entities to clipboard instantly without base point.")},
        {"EditPaste",         RS2::ActionEditPaste,         tr("&Paste"),          ":/icons/paste.lci",          tr("Pastes clipboard entities into the drawing at a picked position.")},
        {"EditPasteTransform",RS2::ActionEditPasteTransform,tr("Paste &Transform"),":/icons/paste_transform.lci",tr("Pastes clipboard entities with scale, rotation, and mirror options.")}
    });
}

void LC_ActionFactory::updateSnapActionsBySettings(const QMap<QString, QAction*>& map) {
    using namespace CFG_VisualSnap;
    map["SnapVisualAngleSnap"]->setChecked(o_VSAngleSnapStepRaysVertexes);
    map["SnapVisualRelAngleSnap"]->setChecked(o_VSAngleSnapStepRaysRelative);
    map["SnapVisualDynDistance"]->setChecked(o_VSVertexVertexDistanceCircles);
    map["SnapVisualDistanceTan"]->setChecked(o_VSVertexVertexDistanceTangents);
    map["SnapVisualShowFarGuides"]->setChecked(o_VSShowNotSnappableGuides);

    const bool autoAddSnapPoints = o_VSSnapAutoAddSnapPoint;
    const auto actionAddSnapAuto = map["SnapVisualAutoAddSnap"];
    actionAddSnapAuto->setChecked(autoAddSnapPoints);
    const auto actionAddLastSnapOnly = map["SnapVisualAutoAddSnapLast"];
    actionAddLastSnapOnly->setEnabled(autoAddSnapPoints);
    actionAddLastSnapOnly->setChecked(o_VSSnapAutoAddLastSnapPointOnly);
}

void LC_ActionFactory::setupActionInfoSettingAction(QMap<QString, QAction*>& map, const std::unique_ptr<LC_InfoCursorSettingsManager>::pointer infoCursorSettingsManager, const char* actionName, int tag) {
    auto actionInfoCursorAbs = map[actionName];
    actionInfoCursorAbs->setProperty("InfoCursorActionTag", tag);
    connect(actionInfoCursorAbs, &QAction::triggered, infoCursorSettingsManager, &LC_InfoCursorSettingsManager::slotInfoCursorSetting);
    connect(infoCursorSettingsManager, &LC_InfoCursorSettingsManager::showInfoCursorSettingChanged, actionInfoCursorAbs, &QAction::setEnabled);
}

void LC_ActionFactory::setupCreatedActions(QMap<QString, QAction *> &map) {
    map["ZoomPrevious"]->setEnabled(false);
    map["RightDockAreaToggle"]->setChecked(true);
    {
        using namespace CFG_Appearance;
        const bool statusBarVisible = o_StatusBarVisible;
        const bool mainMenuVisible = o_MainMenuVisible;
        const bool fullScreenMode = o_FullscreenMode;
        map["ViewStatusBar"]->setChecked(statusBarVisible);
        map["MainMenu"]->setChecked(mainMenuVisible);
        map["Fullscreen"]->setChecked(fullScreenMode);
        map["OptionsGeneral"]->setMenuRole(QAction::NoRole);
    }
    const bool additiveSelection = CFG_Selection::o_Additivity;
    map["SelectionModeToggle"]->setChecked(additiveSelection);

    connect(m_appWin, &QC_ApplicationWindow::printPreviewChanged, map["FilePrint"], &QAction::setChecked);
    connect(m_appWin, &QC_ApplicationWindow::printPreviewChanged, map["FilePrintPreview"], &QAction::setChecked);
    connect(m_appWin, &QC_ApplicationWindow::gridChanged, map["ViewGrid"], &QAction::setChecked);
    connect(m_appWin, &QC_ApplicationWindow::draftChanged, map["ViewDraft"], &QAction::setChecked);
    connect(m_appWin, &QC_ApplicationWindow::draftChanged, map["ViewLinesDraft"], &QAction::setDisabled);
    connect(m_appWin, &QC_ApplicationWindow::antialiasingChanged, map["ViewAntialiasing"], &QAction::setChecked);
    connect(m_appWin, &QC_ApplicationWindow::windowsChanged, map["OptionsDrawing"], &QAction::setEnabled);

    QAction *&entityInfoAction = map["EntityDescriptionInfo"];
    connect(m_appWin, &QC_ApplicationWindow::showEntityDescriptionOnHoverChanged, entityInfoAction, &QAction::setChecked);

    const auto infoCursorSettingsManager = m_appWin->m_infoCursorSettingsManager.get();
    connect(infoCursorSettingsManager, &LC_InfoCursorSettingsManager::showInfoCursorSettingChanged, entityInfoAction, &QAction::setVisible);

    auto action = map["InfoCursorEnable"];
    action->setProperty("InfoCursorActionTag", 0);
    connect(action, &QAction::triggered, m_appWin->m_infoCursorSettingsManager.get(), &LC_InfoCursorSettingsManager::slotInfoCursorSetting);

    setupActionInfoSettingAction(map, infoCursorSettingsManager, "InfoCursorAbs", 1);
    setupActionInfoSettingAction(map, infoCursorSettingsManager, "InfoCursorSnap", 2);
    setupActionInfoSettingAction(map, infoCursorSettingsManager, "InfoCursorRel", 3);
    setupActionInfoSettingAction(map, infoCursorSettingsManager, "InfoCursorPrompt", 4);
    setupActionInfoSettingAction(map, infoCursorSettingsManager, "InfoCursorCatchedEntity", 5);
    // setupActionInfoSettingAction(map, infoCursorSettingsManager, "EntityDescriptionInfo", 6);
    auto actionInfoCursorAbs = map["EntityDescriptionInfo"];
    actionInfoCursorAbs->setProperty("InfoCursorActionTag", 6);

    // with this setting, the action's icon will not be set as current item in action's options bar and status bar (in QC_ApplicationWindow::relayAction())
    map["DimStyles"]->setProperty("_SetAsCurrentActionInView", false);
    map["LockRelativeZero"]->setProperty("_SetAsCurrentActionInView", false);
    map["SelectionModeToggle"]->setProperty("_SetAsCurrentActionInView", false);

    connect(RS_SETTINGS, &RS_Settings::optionChanged, [map](const QString& groupName, const QString &propertyName, [[maybe_unused]]QVariant oldValue, const QVariant& newValue) -> void {
        if (groupName == CFG_Selection::Group.groupName() && propertyName == CFG_Selection::o_Additivity.key()) {
            const auto action = map["SelectionModeToggle"];
            const bool value = newValue.toBool();
            action->setChecked(value);
        }
    });

    updateSnapActionsBySettings(map);
    QAction* actionSnapVisual = map["SnapVisual"];

    connect(map["SnapVisualLock"], &QAction::toggled, [this, actionSnapVisual](bool toggled) {
        const auto currentAction = m_actionHandler->getCurrentAction();
        if (currentAction != nullptr) {
            currentAction->lockVisualSnap(toggled);
        }

        if (toggled) {
            actionSnapVisual->setIcon(QIcon(":/icons/snap_visual_lock.lci"));
        }
        else {
            actionSnapVisual->setIcon(QIcon(":/icons/snap_visual.lci"));
        }
    });

    connect(RS_SETTINGS, &RS_Settings::optionsChanged, [this, &map]() {
        updateSnapActionsBySettings(map);
    });

    auto actionAddLastSnapOnly = map["SnapVisualAutoAddSnapLast"];
    connect(map["SnapVisualAutoAddSnap"], &QAction::toggled, [actionAddLastSnapOnly](bool checked) {
           actionAddLastSnapOnly->setEnabled(checked);
        });
    LC_GROUP_END();

    makeActionsExcludedFromRecent(map, {
         // Navigation & Zoom
         "ZoomPan",
         "ZoomWindow",
         "ZoomIn",
         "ZoomOut",
         "ZoomAuto",
         "ZoomPrevious",
         "ZoomRedraw",

         // Selection
         "EditKillAllActions",
         "SelectAll",
         "DeselectAll",
         "SelectInvert",
         "SelectionModeToggle",
         "SelectSingle",
         "SelectWindow",
         "DeselectWindow",
         "SelectContour",
         "SelectIntersected",
         "DeselectIntersected",
         "SelectLayer",
         "SelectQuick",
         "SelectionGeneric",

         // Clipboard & Undo
         "EditUndo",
         "EditRedo",
         "ModifyDeleteQuick",
         "EditCut",
         "EditCutQuick",
         "EditCopy",
         "EditCopyQuick",
         "EditPaste",
         "EditPasteTransform",

         // Layer Management
         "LayersDefreezeAll",
         "LayersFreezeAll",
         "LayersUnlockAll",
         "LayersLockAll",
         "LayersAdd",
         "LayersRemove",
         "LayersEdit",
         "LayersToggleLock",
         "LayersToggleView",
         "LayersTogglePrint",
         "LayersToggleConstruction",
         "LayersExportSelected",
         "LayersExportVisible",
         "EntityLayerActivate",
         "EntityLayerView",
         "EntityLayerHideOthers",
         "EntityLayerLock",
         "EntityLayerConstruction",
         "EntityLayerPrint",

         // Block Management
         "BlocksDefreezeAll",
         "BlocksFreezeAll",
         "BlocksToggleView",
         "BlocksRemove",
         "BlocksSave",

         // Sub-Pickers, Utility & Export
         "PickPoint",
         "PickPointX",
         "PickPointY",
         "PickDistance",
         "PickAngle",
         "SetRelativeZero",
         "PenSyncFromLayer",
         "FileExportMakerCam"
     });
}

void LC_ActionFactory::setDefaultShortcuts(QMap<QString, QAction*>& map, const LC_ActionGroupManager* agm) {
    QList<QKeySequence> commandLineShortcuts;
    commandLineShortcuts << QKeySequence(Qt::CTRL | Qt::Key_M) << QKeySequence(Qt::Key_Colon);
    if (CFG_Keyboard::o_ToggleFreeSnapOnSpace) {
        commandLineShortcuts << QKeySequence(Qt::Key_Space);
    }

    std::vector<LC_ShortcutInfo> shortcutsList = {
        {"ModifyRevertDirection", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R)},
        {"ModifyDuplicate",QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D)},
        {"OrderBottom", QKeySequence(Qt::Key_End)},
        {"OrderLower", QKeySequence(Qt::Key_PageDown)},
        {"OrderRaise", QKeySequence(Qt::Key_PageUp)},
        {"OrderTop", QKeySequence(Qt::Key_Home)},
        {"SelectAll", QKeySequence::SelectAll},
        // RVT April 29, 2011 - Added esc key to de-select all entities
        {"DeselectAll", QKeySequence(tr("Ctrl+K"))},
        {"EditUndo", QKeySequence::Undo},
        {"EditRedo", QKeySequence::Redo},
        {"EditCut", QKeySequence::Cut},
        {"EditCutQuick", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X)},
        {"EditCopy", QKeySequence::Copy},
        {"EditCopyQuick", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C)},
        {"EditPaste", QKeySequence::Paste},
        {"EditPasteTransform", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V)},
        {"ZoomIn", QKeySequence::ZoomIn},
        {"ZoomOut", QKeySequence::ZoomOut},
        {"ZoomAuto", QKeySequence(Qt::CTRL | Qt::Key_F)},
        {"ZoomRedraw", QKeySequence::Refresh},
        {"OptionsDrawing", QKeySequence::Preferences},
        {"ReloadStyleSheet", QKeySequence("Ctrl+T")},
        {"FileClose", QKeySequence::Close},
        {"FileCloseAll", QKeySequence("Shift+" + QKeySequence(QKeySequence::Close).toString())},
        {"ViewGrid", QKeySequence(tr("Ctrl+G", "Toggle Grid"))},
        // todo - it's better to replace to something different.... ctrl+d is rather for duplicate
        {"ViewDraft", QKeySequence(tr("Ctrl+D","Toggle Draft Mode"))},
        {"ViewStatusBar", QKeySequence(tr("Ctrl+I", "Hide Statusbar"))},
        {"ModifyDeleteQuick",QKeySequence::Delete  /*QList<QKeySequence>() << QKeySequence::Delete << QKeySequence(Qt::Key_Backspace)*/},
        {"FileNew", QKeySequence::New},
        {"FileOpen", QKeySequence::Open},
        {"FileSave", QKeySequence::Save},
        {"FileSaveAs", QKeySequence::SaveAs},
        {"FilePrint", QKeySequence::Print},
        {"FileQuit", QKeySequence::Quit},
        {"FocusCommand", QKeySequence(Qt::CTRL | Qt::Key_M)},
        // commandLineShortcuts}, // fixme - restore shortcuts for focus command line!!!
#if defined(Q_OS_LINUX)
        {"Fullscreen", QKeySequence("F11")},
#else
        {"Fullscreen", QKeySequence::FullScreen},
        {"ExclusiveSnapMode", QKeySequence(Qt::ALT | Qt::Key_X)},
#endif
        {"MainMenu", QKeySequence("F10")},
        {"RelativeInputLengh", QKeySequence(Qt::SHIFT | Qt::Key_D)},
        {"RelativeInputAngle", QKeySequence(Qt::SHIFT | Qt::Key_A)},
        {"RelativeInputDX", QKeySequence(Qt::SHIFT | Qt::Key_X)},
        {"RelativeInputDY", QKeySequence(Qt::SHIFT | Qt::Key_Z)},
        {"RelativeAddLine", QKeySequence(Qt::SHIFT | Qt::Key_L)},
        {"RelativeAddCircle", QKeySequence(Qt::SHIFT | Qt::Key_C)},
        {"RelativeAddPoint", QKeySequence(Qt::SHIFT | Qt::Key_P)},
        {"RestrictOrthogonal", QKeySequence(Qt::Key_F8)}, // Issue #2526: default shortcut for snapping: restriction orthogonal
        {LC_ActionNames::ToggleDockProperties, QKeySequence(Qt::CTRL | Qt::Key_1)}, // AutoCAD standard Ctrl+1
        {LC_ActionNames::ToggleDockCommandLine, QKeySequence(Qt::CTRL | Qt::Key_9)}  // AutoCAD standard Ctrl+9
    };

    map["FileClose"]->setShortcutContext(Qt::WidgetShortcut);

    QKeySequence shortcut = QKeySequence::SaveAs; //(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    // only define this shortcut for platforms not already using it for save as
    if (shortcut != QKeySequence::SaveAs){
        shortcutsList.emplace_back("FileSaveAll", shortcut);
    }

    LC_ShortcutsManager::assignShortcutsToActions(map, shortcutsList);
}

void LC_ActionFactory::markNotEditableActionsShortcuts(const QMap<QString, QAction *> &map) {
    // placeholder for exclusion of some actions (by name) from editing in shortcuts mapping dialog
    makeActionsShortcutsNonEditable(map, {
        "RestrictNothing",
        "PickPoint",
        "PickPointX",
        "PickPointY",
        "PickDistance",
        "PickAngle"
    });
}

void LC_ActionFactory::fillActionLists(const QMap<QString, QAction *> &map){
    file_Actions = {"FileNew", "FileNewTemplate", "FileOpen", "FileSave", "FileSaveAs", "FileSaveAll"};

    shapeActions = {
        "DrawLineRectangle",
        "DrawLineRectangle1Point",
        "DrawLineRectangle2Points",
        "DrawLineRectangle3Points",
        "DrawLinePolygonCenCor",
        "DrawLinePolygonCenTan",
        "DrawLinePolygonCorCor",
        "DrawLinePolygonSideSide",
        "DrawStar"
    };

    lineActions = {
        "DrawLine",
        "DrawLineAngle",
        "DrawLineHorizontal",
        "DrawLineVertical",
        "DrawLineParallelThrough",
        "DrawLineParallel",
        "DrawLineBisector",
        "DrawLineTangent1",
        "DrawLineTangent2",
        "DrawLineOrthTan",
        "DrawLineOrthogonal",
        "DrawLineRelAngle",
        "DrawLineRel",
        "DrawLineRelX",
        "DrawLineRelY",
        "DrawLineAngleRel",
        "DrawLineOrthogonalRel",
        "DrawLineFromPointToLine",
        "DrawSliceDivideLine",
        "DrawSliceDivideCircle",
        "DrawCross",
        "DrawLineMiddle",
        "DrawLineRadiant"
    };

    pointActions = {
        "DrawPoint",
        "DrawLinePoints",
        "DrawPointsMiddle",
        "DrawPointLattice",
        "SelectPoints",
        "PasteToPoints"};

    circleActions = {
        "DrawCircle",
        "DrawCircle2P",
        "DrawCircle2PR",
        "DrawCircle3P",
        "DrawCircleCR",
        "DrawCircleTan2_1P",
        "DrawCircleTan1_2P",
        "DrawCircleTan2",
        "DrawCircleTan3",
        "DrawCircleInscribe",
        "DrawCircleParallel",
        "DrawCircleByArc"
    };

    curveActions = {
        "DrawArc",
        "DrawArcChord",
        "DrawArcAngleLen",
        "DrawArc3P",
        "DrawArc2PAngle",
        "DrawArc2PRadius",
        "DrawArc2PLength",
        "DrawArc2PHeight",
        "DrawArcTangential",
        "DrawEllipseArcAxis",
        "DrawEllipseArc1Point"
    };

    splineActions = {
        "DrawSpline",
        "DrawSplinePoints",
        "DrawSplineFromPolyline",
        "DrawSplinePointsAppend",
        "DrawSplinePointsAdd",
        "DrawSplinePointsRemove",
        "DrawSplinePointsDelTwo",
        "DrawSplineExplode",
        "DrawLineFree",
        "DrawParabola4Points",
        "DrawParabolaFD",
        "DrawHyperbolaFP"
    };

    ellipseActions = {
        "DrawEllipse1Point",
        "DrawEllipseAxis",
        "DrawEllipseFociPoint",
        "DrawEllipse4Points",
        "DrawEllipseCenter3Points",
        "DrawEllipseInscribe"
    };

    polylineActions = {
        "DrawPolyline",
        "PolylineAdd",
        "PolylineAppend",
        "PolylineDel",
        "PolylineDelBetween",
        "PolylineTrim",
        "PolylineEquidistant",
        "PolylineSegment",
        "PolylineArcToLines",
        "PolylineSegmentType"
    };

    textActions = {
        "DrawText", "DrawMText", "ModifyExplodeText"
    };

    selectActions = {
        "DeselectAll",
        "SelectAll",
        "SelectSingle",
        "SelectContour",
        "SelectWindow",
        "DeselectWindow",
        "SelectIntersected",
        "DeselectIntersected",
        "SelectLayer",
        "SelectPoints",
        "SelectInvert",
        "SelectQuick"
    };

    dimensionActions = {
        "DimAligned",
        "DimLinear",
        "DimLinearHor",
        "DimLinearVer",
        "DimBaseline",
        "DimContinue",
        "DimRadial",
        "DimDiametric",
        "DimAngular",
        "DimArc",
        "DimLeader",
        "DimOrdinate",
        "DimOrdinateForBase",
        "DimOrdinateReBase",
        "DimPickApply",
        "DimRegenerate",
        "DimStyles"
    };

    otherDrawingActions = {
        "DrawHatch",
        "DrawImage",
        "DrawBoundingBox"
    };

    modifyActions = {
        "ModifyMove",
        "ModifyDuplicate",
        "ModifyAlign",
        "ModifyAlignOne",
        "ModifyAlignRef",
        "ModifyRotate",
        "ModifyScale",
        "ModifyMirror",
        "ModifyMoveRotate",
        "ModifyRotate2",
        "ModifyRevertDirection",
        "ModifyTrim",
        "ModifyTrim2",
        "ModifyTrimAmount",
        "ModifyLineJoin",
        "ModifyCut",
        "ModifyBreakDivide",
        "ModifyLineGap",
        "ModifyOffset",
        "ModifyBevel",
        "ModifyRound",
        "ModifyStretch",
        "ModifyEntity",
        "ModifyAttributes",
        "ModifyExplodeText",
        "BlocksExplode",
        "ModifyDelete"
    };

    orderActions = {
        "OrderTop",
        "OrderBottom",
        "OrderRaise",
        "OrderLower"};

    infoActions = {
        "InfoPoint",
        "InfoDist",
        "InfoDist2",
        "InfoDist3",
        "InfoAngle",
        "InfoAngle3Points",
        "InfoTotalLength",
        "InfoArea",
        "EntityInfo"
    };

    layerActions = {
        "LayersDefreezeAll",
        "LayersFreezeAll",
        "LayersUnlockAll",
        "LayersLockAll",
        "LayersAdd",
        "LayersRemove",
        "LayersEdit",
        "LayersToggleLock",
        "LayersToggleView",
        "LayersTogglePrint",
        "LayersToggleConstruction",
        "LayersExportSelected",
        "LayersExportVisible"
    };

    blockActions = {
        "BlocksDefreezeAll",
        "BlocksFreezeAll",
        "BlocksToggleView",
        "BlocksAdd",
        "BlocksRemove",
        "BlocksAttributes",
        "BlocksInsert",
        "BlocksEdit",
        "BlocksSave",
        "BlocksCreate",
        "BlocksExplode"
    };

    penActions = {
        "PenSyncFromLayer",
        "PenPick",
        "PenPickResolved",
        "PenApply",
        "PenCopy"};

    entityLayerActions = {
        "EntityLayerView",
        "EntityLayerHideOthers",
        "EntityLayerLock",
        "EntityLayerConstruction",
        "EntityLayerPrint",
        "LayersDefreezeAll"
    };
}

void LC_ActionFactory::prepareActionsToDisableInPrintPreview(QList<QAction*>& actionsList, const QMap<QString, QAction *> &map) const {
    fillActionsList(actionsList, QList<QString>{
        "EditCut",
        "EditCutQuick",
        "EditCopy",
        "EditCopyQuick",
        "EditPaste",
        "EditPasteTransform",
        "ViewGrid",
        "ViewDraft",
        "ViewLinesDraft",
        "ViewAntialiasing",
        "ModifyDeleteQuick",
        "EditKillAllActions",
        "ZoomIn",
        "ZoomOut",
        "ZoomAuto",
        "ZoomPrevious",
        "ZoomWindow",
        "ZoomPan",
        "OptionsDrawing",
        "ViewGridOrtho",
        "ViewGridIsoLeft",
        "ViewGridIsoTop",
        "ViewGridIsoRight",
        "UCSSetWCS",
        "UCSCreate",
        "UCSSetByDimOrdinate",
    }, map);

    fillActionsList(actionsList,lineActions, map);
    fillActionsList(actionsList,pointActions, map);
    fillActionsList(actionsList,shapeActions, map);
    fillActionsList(actionsList,circleActions, map);
    fillActionsList(actionsList,curveActions, map);
    fillActionsList(actionsList,splineActions, map);
    fillActionsList(actionsList,ellipseActions, map);
    fillActionsList(actionsList,polylineActions, map);
    fillActionsList(actionsList,selectActions, map);
    fillActionsList(actionsList,dimensionActions, map);
    fillActionsList(actionsList,otherDrawingActions, map);
    fillActionsList(actionsList,modifyActions, map);
    fillActionsList(actionsList,orderActions, map);
    fillActionsList(actionsList,infoActions, map);
    fillActionsList(actionsList,blockActions, map);
    fillActionsList(actionsList,penActions, map);
    fillActionsList(actionsList,layerActions, map);
    fillActionsList(actionsList,entityLayerActions, map);
}
