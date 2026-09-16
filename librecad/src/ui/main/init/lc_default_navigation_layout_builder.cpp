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

#include "lc_default_navigation_layout_builder.h"

#include "lc_action_factory.h"

ToolbarDef LC_DefaultNavigationLayoutBuilder::makeStandardToolbar(const QString& name, const QString& icon,
                                                                  Qt::ToolBarArea area, const QList<ActionNode>& nodes,
                                                                  bool visible) {
    ToolbarDef tb;
    tb.name = name;
    tb.icon = icon;
    tb.area = area;
    tb.visible = visible;
    tb.kind = ToolbarKind::Standard;
    tb.nodes = nodes;
    return tb;
}

ToolbarDef LC_DefaultNavigationLayoutBuilder::makeCadToolbar(const QString& name, const QString& icon,
                                                            const QList<ActionNode>& actions, bool visible) {
    ToolbarDef tb;
    tb.name = name;
    tb.icon = icon;
    tb.area = Qt::BottomToolBarArea;
    tb.visible = visible;
    tb.kind = ToolbarKind::Cad;
    tb.nodes = actions;
    return tb;
}

ToolbarDef LC_DefaultNavigationLayoutBuilder::makeCadDockWidget(const QString& name, const QString& icon,
                                                                const QList<ActionNode>& actions) {
    ToolbarDef tb;
    tb.name = name;
    tb.icon = icon;
    tb.area = Qt::LeftToolBarArea;
    tb.visible = true;
    tb.kind = ToolbarKind::CadDockWidget;
    tb.nodes = actions;
    return tb;
}

ToolbarDef LC_DefaultNavigationLayoutBuilder::makeCadMatrix(const QString& name, const QString& icon,
                                                          const QList<ActionNode>& groups) {
    ToolbarDef tb;
    tb.name = name;
    tb.icon = icon;
    tb.area = Qt::LeftToolBarArea;
    tb.visible = true;
    tb.kind = ToolbarKind::CadMatrix;
    tb.nodes = groups;
    return tb;
}

QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildCadDockWidgets() {
    QList<ToolbarDef> dockWidgets;

    // 1. CAD Tools Matrix
    QList<ActionNode> matrixCategories = {
        ActionNode("Menu:Line",      ":/icons/line.lci",                 lineActions()),
        ActionNode("Menu:Point",     ":/icons/points.lci",               pointActions()),
        ActionNode("Menu:Polygon",   ":/icons/rectangle_2_points.lci",   shapeActions()),
        ActionNode("Menu:Circle",    ":/icons/circle.lci",               circleActions()),
        ActionNode("Menu:Arc",       ":/icons/arc_center_point_angle.lci", curveActions()),
        ActionNode("Menu:Spline",    ":/icons/spline_points.lci",        splineActions()),
        ActionNode("Menu:Ellipse",   ":/icons/ellipses.lci",             ellipseActions()),
        ActionNode("Menu:Polyline",  ":/icons/polylines.lci",            polylineActions()),
        ActionNode("Menu:Select",    ":/icons/select.lci",               selectActions()),
        ActionNode("Menu:Modify",    ":/icons/move_rotate.lci",          modifyActions()),
        ActionNode("Menu:Dimension", ":/icons/dim_horizontal.lci",       dimensionActions()),
        ActionNode("Menu:Info",      ":/icons/measure.lci",              infoActions()),
        ActionNode("Menu:Other",     ":/icons/text.lci",                 otherDrawingActions()),
        ActionNode("Menu:Order",     ":/icons/order.lci",                orderActions())
    };

    dockWidgets.append(makeCadMatrix(tr("CAD Tools Matrix"), ":/icons/line_polygon_star.lci", matrixCategories));

    // 2. Individual CAD Dock Widgets with clean display names
    dockWidgets.append({
        makeCadDockWidget(tr("Line (Dock)"),      ":/icons/line.lci",                 lineActions()),
        makeCadDockWidget(tr("Point (Dock)"),     ":/icons/points.lci",               pointActions()),
        makeCadDockWidget(tr("Polygon (Dock)"),   ":/icons/rectangle_2_points.lci",   shapeActions()),
        makeCadDockWidget(tr("Circle (Dock)"),    ":/icons/circle.lci",               circleActions()),
        makeCadDockWidget(tr("Arc (Dock)"),       ":/icons/arc_center_point_angle.lci", curveActions()),
        makeCadDockWidget(tr("Spline (Dock)"),    ":/icons/spline_points.lci",        splineActions()),
        makeCadDockWidget(tr("Ellipse (Dock)"),   ":/icons/ellipses.lci",             ellipseActions()),
        makeCadDockWidget(tr("Polyline (Dock)"),  ":/icons/polylines.lci",            polylineActions()),
        makeCadDockWidget(tr("Dimension (Dock)"), ":/icons/dim_horizontal.lci",       dimensionActions()),
        makeCadDockWidget(tr("Other (Dock)"),     ":/icons/text.lci",                 otherDrawingActions()),
        makeCadDockWidget(tr("Modify (Dock)"),    ":/icons/move_rotate.lci",          modifyActions()),
        makeCadDockWidget(tr("Info (Dock)"),      ":/icons/measure.lci",              infoActions()),
        makeCadDockWidget(tr("Order (Dock)"),     ":/icons/order.lci",                orderActions())
    });

    return dockWidgets;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::lineActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::shapeActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::pointActions() {
    static const QList<ActionNode> list = {
        "DrawPoint",
        "DrawLinePoints",
        "DrawPointsMiddle",
        "DrawPointLattice",
        "SelectPoints",
        "PasteToPoints"
    };
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::circleActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::curveActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::splineActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::ellipseActions() {
    static const QList<ActionNode> list = {
        "DrawEllipse1Point",
        "DrawEllipseAxis",
        "DrawEllipseFociPoint",
        "DrawEllipse4Points",
        "DrawEllipseCenter3Points",
        "DrawEllipseInscribe"
    };
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::polylineActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::selectActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::dimensionActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::otherDrawingActions() {
    static const QList<ActionNode> list = {"DrawText", "DrawMText", "DrawHatch", "DrawImage", "DrawBoundingBox"};
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::modifyActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::orderActions() {
    static const QList<ActionNode> list = {"OrderTop", "OrderBottom", "OrderRaise", "OrderLower"};
    return list;
}

const QList<ActionNode>& LC_DefaultNavigationLayoutBuilder::infoActions() {
    static const QList<ActionNode> list = {
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
    return list;
}

QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuCompact(const LC_ActionFactory*, const LC_ActionGroupManager*) {
    QList<ActionNode> menus = {
        // File
        {
            "Menu:File",
            ":/icons/save.lci",
            {
                "FileNew",
                "FileNewTemplate",
                "FileOpen",
                "SpecialMenu:RecentFiles",
                "-",
                "FileSave",
                "FileSaveAs",
                "FileSaveAll",
                "-",
                ActionNode{"Menu:Import", ":/icons/import.lci", {"DrawImage", "BlocksImport"}},
                ActionNode{"Menu:Export", ":/icons/export.lci", {"FileExportMakerCam", "FilePrintPDF", "FileExport"}},
                "-",
                "FilePrint",
                "FilePrintPreview",
                "-",
                "FileClose",
                "FileCloseAll",
                "FileQuit"
            }
        },

        // Options
        {
            "Menu:Options",
            ":/icons/settings.lci",
            {"OptionsGeneral", "OptionsCustomization", "OptionsStyling", "DeviceOptions", "ReloadStyleSheet", "-", "OptionsDrawing"}
        },

        // Edit
        {
            "Menu:Edit",
            ":/icons/rename_active_block.lci",
            {
                "EditKillAllActions",
                "-",
                "EditUndo",
                "EditRedo",
                "-",
                "EditCut",
                "EditCopy",
                "EditPaste",
                "EditPasteTransform",
                "PasteToPoints",
                "-",
                "EditCutQuick",
                "EditCopyQuick",
                "ModifyDeleteQuick"
            }
        },

        // View
        {
            "Menu:View",
            ":/icons/zoom_in.lci",
            {
                "ViewGrid",
                "ViewDraft",
                "ViewLinesDraft",
                "ViewAntialiasing",
                "-",
                "ViewGridOrtho",
                "ViewGridIsoLeft",
                "ViewGridIsoTop",
                "ViewGridIsoRight",
                "-",
                "ZoomRedraw",
                "ZoomIn",
                "ZoomOut",
                "ZoomAuto",
                "ZoomPrevious",
                "ZoomWindow",
                "ZoomPan",
                "-",
                "ZoomViewSave",
                ActionNode{
                    "Menu:ViewsRestore",
                    ":/icons/nview_visible.lci",
                    {"ZoomViewRestore1", "ZoomViewRestore2", "ZoomViewRestore3", "ZoomViewRestore4", "ZoomViewRestore5"}
                }
            }
        },

        // Tools (Combined)
        {
            "Menu:Tools",
            ":/icons/settings.lci",
            {
                {"Menu:Line", ":/icons/line.lci", lineActions()},
                {"Menu:Point", ":/icons/points.lci", pointActions()},
                {"Menu:Circle", ":/icons/circle.lci", circleActions()},
                {"Menu:Arc", ":/icons/arc_center_point_angle.lci", curveActions()},
                {"Menu:Polygon", ":/icons/rectangle_1_point.lci", shapeActions()},
                {"Menu:Spline", ":/icons/spline_points.lci", splineActions()},
                {"Menu:Ellipse", ":/icons/ellipses.lci", ellipseActions()},
                {"Menu:Polyline", ":/icons/polylines_polyline.lci", polylineActions()},
                {"Menu:Select", ":/icons/select.lci", selectActions()},
                {"Menu:Dimension", ":/icons/dim_horizontal.lci", dimensionActions()},
                {"Menu:Other", ":/icons/text.lci", otherDrawingActions()},
                {"Menu:Modify", ":/icons/move_rotate.lci", modifyActions()},
                {"Menu:Info", ":/icons/measure.lci", infoActions()},
                {"Menu:Order", ":/icons/order.lci", orderActions()}
            }
        },

        // Workspace
        {
            "Menu:Workspace",
            ":/icons/workspace.lci",
            {
                "Fullscreen",
                "MainMenu",
                "ViewStatusBar",
                "-",
                ActionNode{
                    "Menu:DockAreas",
                    "",
                    {"LeftDockAreaToggle", "RightDockAreaToggle", "TopDockAreaToggle", "BottomDockAreaToggle", "FloatingDockwidgetsToggle"}
                },
                ActionNode{"Menu:ToolbarAreas", "", {"LeftTBAreaToggle", "RightTBAreaToggle", "TopTBAreaToggle", "BottomTBAreaToggle"}},
                LC_ActionNames::MenuDockWidgets,
                LC_ActionNames::MenuCadDockWidgets,
                LC_ActionNames::MenuToolbars,
                LC_ActionNames::MenuCadToolbars,
                "-",
                "RedockWidgets",
                "-",
                "WorkspaceCreate",
                LC_ActionNames::MenuWorkspacesList,
                "-",
                "InvokeMenuCreator",
                "InvokeToolbarCreator",
                "-",
                LC_ActionNames::MenuDrawings
            }
        },

        // Help
        {
            "Menu:Help",
            ":/icons/help.lci",
            {
                ActionNode{
                    "Menu:OnlineDocs",
                    ":/icons/help.lci",
                    {"HelpWiki", "HelpManual", "HelpCommands", "HelpStyleSheets", "HelpWidgets"}
                },
                "-",
                "HelpForum",
                "HelpZulip",
                "-",
                "HelpGitHub",
                "HelpSubmitError",
                "HelpRequestFeature",
                "HelpReleases",
                "-",
                "HelpHome",
                "HelpAbout",
                "HelpLicense",
                "-",
                "HelpDonate"
            }
        }
    };

    return menus;
}

QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuCompactTools(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    QList<ActionNode> menus = buildDefaultMenuCompact(af, agm);

    menus.removeAt(4); // Remove combined Tools

    QList<ActionNode> drawSubmenus = {
        {"Menu:Line", ":/icons/line.lci", lineActions()},
        {"Menu:Point", ":/icons/points.lci", pointActions()},
        {"Menu:Circle", ":/icons/circle.lci", circleActions()},
        {"Menu:Arc", ":/icons/arc_center_point_angle.lci", curveActions()},
        {"Menu:Polygon", ":/icons/rectangle_1_point.lci", shapeActions()},
        {"Menu:Spline", ":/icons/spline_points.lci", splineActions()},
        {"Menu:Ellipse", ":/icons/ellipses.lci", ellipseActions()},
        {"Menu:Polyline", ":/icons/polylines_polyline.lci", polylineActions()},
        {"Menu:Other", ":/icons/text.lci", otherDrawingActions()}
    };

    menus.insert(4, {"Menu:Draw", ":/icons/line.lci", drawSubmenus});
    menus.insert(5, {"Menu:Modify", ":/icons/move_rotate.lci", modifyActions()});
    menus.insert(6, {"Menu:Dimensions", ":/icons/dim_horizontal.lci", dimensionActions()});
    menus.insert(7, {"Menu:Info", ":/icons/measure.lci", infoActions()});

    return menus;
}

QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuExtended(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    QList<ActionNode> menus = buildDefaultMenuCompact(af, agm);

    menus.removeAt(4); // Remove combined Tools

    int idx = 4;
    menus.insert(idx++, {"Menu:Line", ":/icons/line.lci", lineActions()});
    menus.insert(idx++, {"Menu:Point", ":/icons/points.lci", pointActions()});
    menus.insert(idx++, {"Menu:Circle", ":/icons/circle.lci", circleActions()});
    menus.insert(idx++, {"Menu:Arc", ":/icons/arc_center_point_angle.lci", curveActions()});
    menus.insert(idx++, {"Menu:Polygon", ":/icons/rectangle_1_point.lci", shapeActions()});
    menus.insert(idx++, {"Menu:Spline", ":/icons/spline_points.lci", splineActions()});
    menus.insert(idx++, {"Menu:Ellipse", ":/icons/ellipses.lci", ellipseActions()});
    menus.insert(idx++, {"Menu:Polyline", ":/icons/polylines_polyline.lci", polylineActions()});
    menus.insert(idx++, {"Menu:Other", ":/icons/text.lci", otherDrawingActions()});
    menus.insert(idx++, {"Menu:Modify", ":/icons/move_rotate.lci", modifyActions()});
    menus.insert(idx++, {"Menu:Dimensions", ":/icons/dim_horizontal.lci", dimensionActions()});
    menus.insert(idx++, {"Menu:Info", ":/icons/measure.lci", infoActions()});

    return menus;
}

QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildStandardToolbars() {
    return {
        makeStandardToolbar(tr("Info Cursor"), ":/icons/info_cursor_enable.lci", Qt::TopToolBarArea, {
            ActionNode{"Menu:InfoCursor", ":/icons/info_cursor_enable.lci", {
                "InfoCursorEnable",
                "InfoCursorAbs",
                "InfoCursorSnap",
                "InfoCursorRel",
                "InfoCursorPrompt",
                "InfoCursorCatchedEntity",
                "EntityDescriptionInfo"
            }, ToolButtonPopupMode::SplitFirstAction}
        }),

        // --- Top Area Toolbars ---
        makeStandardToolbar(tr("File"), ":/icons/fileopen.lci", Qt::TopToolBarArea, {
            "FileNew",
            "FileNewTemplate",
            "FileOpen",
            "FileSave",
            "FileSaveAs",
            "FileSaveAll",
            "FilePrint",
            "FilePrintPreview"
        }),

        makeStandardToolbar(tr("Edit"), ":/icons/copy.lci", Qt::TopToolBarArea, {
            "EditKillAllActions",
            "SelectionModeToggle",
            "EntityDescriptionInfo",
            "-",
            "EditUndo",
            "EditRedo",
            "-",
            "EditCut",
            "EditCopy",
            "EditPaste",
            "EditPasteTransform"
        }),

        makeStandardToolbar(tr("View"), ":/icons/draft.lci", Qt::TopToolBarArea, {
            "ViewGrid",
            "ViewDraft",
            "ViewLinesDraft",
            "ViewAntialiasing",
            "-",
            "ZoomRedraw",
            "ZoomIn",
            "ZoomOut",
            "ZoomAuto",
            "ZoomPrevious",
            "ZoomWindow",
            "ZoomPan"
        }),

        makeStandardToolbar(tr("Named Views"), ":/icons/nview_visible.lci", Qt::TopToolBarArea, {
            "ZoomViewSave",
            LC_ActionNames::WidgetNamedViewsSelector
        }),

        makeStandardToolbar(tr("UCS"), ":/icons/ucs_set_wcs.lci", Qt::TopToolBarArea, {
            "UCSCreate",
            LC_ActionNames::WidgetUCSSelector
        }),

        makeStandardToolbar(tr("Workspaces"), ":/icons/workspace.lci", Qt::TopToolBarArea, {
            ActionNode{"Menu:WorkspaceActions", ":/icons/workspace.lci", {"WorkspaceCreate", "WorkspaceRemove"}, ToolButtonPopupMode::SplitFirstAction},
            LC_ActionNames::WidgetWorkspaceSelector
        }),

        makeStandardToolbar(tr("Preferences"), ":/icons/settings.lci", Qt::TopToolBarArea, {
            "OptionsGeneral",
            "OptionsStyling",
            "OptionsCustomization",
            "OptionsDrawing"
        }),

        // Pen toolbar (line break before; variable actions only)
        makeStandardToolbar(tr("Pen"), ":/icons/pen_apply.lci", Qt::TopToolBarArea, {
            "PenSyncFromLayer",
            "PenPick",
            "PenPickResolved",
            "PenApply",
            "PenCopy"
        }, /*visible=*/true),

        makeStandardToolbar(tr("Entity's Layer"), ":/icons/select_entity.lci", Qt::TopToolBarArea, {
            ActionNode{"Menu:EntityLayers", ":/icons/not_visible.lci", {
                "EntityLayerView",
                "EntityLayerHideOthers",
                "EntityLayerLock",
                "EntityLayerConstruction",
                "EntityLayerPrint",
                "LayersDefreezeAll"
            }, ToolButtonPopupMode::SplitFirstAction},
            "EntityLayerActivate"
        }),

        // --- Left Area Toolbars ---
        makeStandardToolbar(tr("Categories"), ":/icons/line_polygon_star.lci", Qt::LeftToolBarArea, {
            ActionNode{"Menu:Lines", ":/icons/line.lci", lineActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Points", ":/icons/points.lci", pointActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Circles", ":/icons/circle.lci", circleActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Arcs", ":/icons/arc_center_point_angle.lci", curveActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Splines", ":/icons/spline_points.lci", splineActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Polygons", ":/icons/rectangle_2_points.lci", shapeActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Ellipses", ":/icons/ellipses.lci", ellipseActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:PolyLines", ":/icons/polylines.lci", polylineActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Select", ":/icons/select.lci", selectActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Dimension", ":/icons/dim_horizontal.lci", dimensionActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Other", ":/icons/text.lci", otherDrawingActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Modify", ":/icons/move_rotate.lci", modifyActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Measure", ":/icons/measure.lci", infoActions(), ToolButtonPopupMode::InstantPopup},
            ActionNode{"Menu:Order", ":/icons/order.lci", orderActions(), ToolButtonPopupMode::InstantPopup}
        }),

        makeStandardToolbar(tr("Order"), ":/icons/order.lci", Qt::LeftToolBarArea, orderActions(), /*visible=*/false),

        // --- Bottom Area Toolbars ---
        makeStandardToolbar(tr("Dock Areas"), ":/icons/dockwidgets_left.lci", Qt::BottomToolBarArea, {
            "LeftDockAreaToggle",
            "RightDockAreaToggle",
            "TopDockAreaToggle",
            "BottomDockAreaToggle",
            "FloatingDockwidgetsToggle"
        }),

        makeStandardToolbar(tr("Creators"), ":/icons/create_toolbar.lci", Qt::BottomToolBarArea, {
            "InvokeMenuCreator",
            "InvokeToolbarCreator"
        })
    };
}

QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildCadToolbars() {
    return {
        makeCadToolbar(tr("Line"),      ":/icons/line.lci", lineActions()),
        makeCadToolbar(tr("Point"),     ":/icons/points.lci", pointActions()),
        makeCadToolbar(tr("Polygon"),   ":/icons/rectangle_2_points.lci", shapeActions()),
        makeCadToolbar(tr("Circle"),    ":/icons/circle.lci", circleActions()),
        makeCadToolbar(tr("Arc"),       ":/icons/arc_center_point_angle.lci", curveActions()),
        makeCadToolbar(tr("Spline"),    ":/icons/spline_points.lci", splineActions()),
        makeCadToolbar(tr("Ellipse"),   ":/icons/ellipses.lci", ellipseActions()),
        makeCadToolbar(tr("Polyline"),  ":/icons/polylines.lci", polylineActions()),
        makeCadToolbar(tr("Dimension"), ":/icons/dim_horizontal.lci", dimensionActions()),
        makeCadToolbar(tr("Other"),     ":/icons/text.lci", otherDrawingActions()),
        makeCadToolbar(tr("Modify"),    ":/icons/move_rotate.lci", modifyActions()),
        makeCadToolbar(tr("Info"),      ":/icons/measure.lci", infoActions()),
        makeCadToolbar(tr("Select"),    ":/icons/select.lci", selectActions())
    };
}


NavigationLayoutConfig LC_DefaultNavigationLayoutBuilder::createDefaultConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    NavigationLayoutConfig config;
    config.name = tr("Default Workspace");
    config.activeMenuVariant = 0;
    config.menuCompact = buildDefaultMenuCompact(af, agm);
    config.menuCompactTools = buildDefaultMenuCompactTools(af, agm);
    config.menuExtended = buildDefaultMenuExtended(af, agm);

    config.toolbars.clear();
    config.toolbars.append(buildStandardToolbars());
    config.toolbars.append(buildCadToolbars());
    config.toolbars.append(buildCadDockWidgets());

    return config;
}
