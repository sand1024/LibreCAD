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

#include "lc_default_context_menus_builder.h"

#include "lc_action_factory.h"
#include "lc_action_group_manager.h"
#include "lc_default_navigation_layout_builder.h"
namespace {
    QList<ActionNode> toNodes(const QList<QString>& names) {
        QList<ActionNode> nodes;
        nodes.reserve(names.size());
        for (const auto& name : names) {
            nodes.append(ActionNode(name));
        }
        return nodes;
    }
}

ActionNode LC_DefaultContextMenusBuilder::makeSubMenu(const QString& title, const QString& groupNameForIcon,
                                                      const QList<ActionNode>& children, const LC_ActionGroupManager* agm) {
    const QString icon = (agm != nullptr) ? agm->iconPath(groupNameForIcon) : QString();
    return ActionNode(title, icon, children);
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonEntityHeader() {
    return {
        ActionNode("SpecialMenu:RecentActions"),
        ActionNode("EditKillAllActions"),
        ActionNode("Menu:Edit", ":/icons/rename_active_block.lci", commonEditActions())
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonGeometryTail(const LC_ActionGroupManager* agm) {
    return {
        makeSubMenu(tr("Modify Generic"), "modify", commonModifyActions(), agm),
        makeSubMenu(tr("Align"), "align", commonAlignActions(), agm),
        makeSubMenu(tr("Order"), "order", commonOrderActions(), agm),
        makeSubMenu(tr("Layers"), "layer", commonLayerActions(), agm),
        makeSubMenu(tr("Info"), "info", commonInfoActions(), agm),
        ActionNode("EntityInfo")
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonDimensionTail(const LC_ActionGroupManager* agm) {
    return {
        makeSubMenu(tr("Align"), "align", commonAlignActions(), agm),
        makeSubMenu(tr("Order"), "order", commonOrderActions(), agm),
        makeSubMenu(tr("Layers"), "layer", commonLayerActions(), agm),
        makeSubMenu(tr("Info"), "info", commonInfoActions(), agm),
        ActionNode("EntityInfo")
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonExtendedFooter() {
    return {
        ActionNode("-"),
        ActionNode("Menu:View", ":/icons/zoom_in.lci", commonViewActions()),
        ActionNode("Menu:File", ":/icons/save.lci", commonFileActions()),
        ActionNode("SpecialMenu:WorkspacesRescue"),
        ActionNode("Menu:Options", ":/icons/settings.lci", commonOptionActions())
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::assembleGeometryMenu(const QString& title, const LC_MenuActivator& activator,
                                                                   const QList<ActionNode>& specificNodes,
                                                                   const LC_ActionGroupManager* agm) {
    QList<ActionNode> nodes = commonEntityHeader();
    nodes.append(specificNodes);
    nodes.append(commonGeometryTail(agm));
    return ContextMenuDef(title, activator, nodes, true);
}

ContextMenuDef LC_DefaultContextMenusBuilder::assembleDimensionMenu(const QString& title, const LC_MenuActivator& activator,
                                                                    const QList<ActionNode>& specificNodes,
                                                                    const LC_ActionGroupManager* agm) {
    QList<ActionNode> nodes = commonEntityHeader();
    nodes.append(specificNodes);
    nodes.append(commonDimensionTail(agm));
    return ContextMenuDef(title, activator, nodes, true);
}

ContextMenuDef LC_DefaultContextMenusBuilder::extendEntityMenu(const ContextMenuDef& compactDef) {
    ContextMenuDef ext = compactDef;
    ext.nodes.clear();

    ext.nodes.append(ActionNode("SpecialMenu:RecentActions"));
    ext.nodes.append(ActionNode("EditKillAllActions"));
    ext.nodes.append(ActionNode("Menu:Select", ":/icons/select.lci", commonSelectActions()));
    ext.nodes.append(ActionNode("Menu:Edit", ":/icons/rename_active_block.lci", commonEditActions()));

    for (const auto& node : compactDef.nodes) {
        if (node.actionName == "SpecialMenu:RecentActions" || node.actionName == "EditKillAllActions" || node.actionName == "Menu:Edit" ||
            node.groupTitle == "Menu:Edit") {
            continue;
        }
        ext.nodes.append(node);
    }

    ext.nodes.append(commonExtendedFooter());
    return ext;
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuEmptySpace(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    Q_UNUSED(af);
    return {
        tr("Empty Space (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, false, RS2::EntityUnknown},
        {
            ActionNode("SpecialMenu:RecentActions"),
            ActionNode("EditKillAllActions"),
            makeSubMenu(tr("Select"), "select", commonSelectActions(), agm),
            makeSubMenu(tr("Edit"), "edit", commonEditActions(), agm),
            {
                "Menu:Draw",
                ":/icons/line_2p.lci",
                {
                    makeSubMenu(tr("Line"), "line", {
                                    {"DrawLine"},
                                    {"DrawLineAngle"},
                                    {"DrawLineHorizontal"},
                                    {"DrawLineVertical"},
                                    {"DrawLineParallel"}
                                }, agm),
                    makeSubMenu(tr("Circle"), "circle", {
                        {"DrawCircle"},
                        {"DrawCircle2P"},
{"DrawCircleCR"},
{"DrawCircle3P"}
                    }, agm),
                    makeSubMenu(tr("Arc"), "curve", {{"DrawArc"}, {"DrawArc3P"}, {"DrawArcTangential"}}, agm),
                    makeSubMenu(tr("Polyline"), "polyline", {{"DrawPolyline"}, {"PolylineAdd"}, {"PolylineAppend"}, {"PolylineTrim"}}, agm),
                    makeSubMenu(tr("Spline"), "spline", {{"DrawSpline"}, {"DrawSplinePoints"}}, agm),
                    makeSubMenu(tr("Polygon"), "shape", {{"DrawLineRectangle"}, {"DrawLinePolygonCenCor"}}, agm),
                    makeSubMenu(tr("Other"), "other", {{"DrawText"}, {"DrawMText"}, {"DrawHatch"}, {"DrawImage"}}, agm)
                }
            },
            makeSubMenu(tr("Modify"), "modify", commonModifyActions(), agm),
            makeSubMenu(tr("Align"), "align", commonAlignActions(), agm),
            makeSubMenu(tr("Order"), "order", commonOrderActions(), agm),
            makeSubMenu(tr("Layers"), "layer", {{"LayersDefreezeAll"}}, agm),
            makeSubMenu(tr("Info"), "info", commonInfoActions(), agm),
            ActionNode("-"),
            makeSubMenu(tr("View"), "view", commonViewActions(), agm),
            makeSubMenu(tr("File"), "file", commonFileActions(), agm),
            ActionNode("SpecialMenu:WorkspacesRescue"),
            makeSubMenu(tr("Options"), "options", commonOptionActions(), agm)
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuEmptySpaceExtended(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    QList<ActionNode> lineNodes =  toNodes(af->lineActions);
    QList<ActionNode> polyNodes = toNodes(af->polylineActions);
    QList<ActionNode> pointNodes = toNodes(af->pointActions);
    QList<ActionNode> circleNodes =  toNodes(af->circleActions);
    QList<ActionNode> curveNodes =  toNodes(af->curveActions);
    QList<ActionNode> shapeNodes = toNodes(af->shapeActions);
    QList<ActionNode> splineNodes = toNodes(af->splineActions);
    QList<ActionNode> ellipseNodes = toNodes(af->ellipseActions);
    QList<ActionNode> otherNodes = toNodes(af->otherDrawingActions);
    QList<ActionNode> modifyNodes = toNodes(af->modifyActions);
    QList<ActionNode> dimNodes = toNodes(af->dimensionActions);

    return {
        tr("Empty Space (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, false, RS2::EntityUnknown},
        {
            ActionNode("SpecialMenu:RecentActions"),
            ActionNode("EditKillAllActions"),
            makeSubMenu(tr("Select"), "select", commonSelectActions(), agm),
            makeSubMenu(tr("Edit"), "edit", commonEditActions(), agm),
            {
                "Menu:Draw",
                ":/icons/line_2p.lci",
                {
                    makeSubMenu(tr("Line"), "line", lineNodes, agm),
                    makeSubMenu(tr("Polyline"), "polyline", polyNodes, agm),
                    makeSubMenu(tr("Point"), "point", pointNodes, agm),
                    makeSubMenu(tr("Circle"), "circle", circleNodes, agm),
                    makeSubMenu(tr("Arc"), "curve", curveNodes, agm),
                    makeSubMenu(tr("Polygon"), "shape", shapeNodes, agm),
                    makeSubMenu(tr("Spline"), "spline", splineNodes, agm),
                    makeSubMenu(tr("Ellipse"), "ellipse", ellipseNodes, agm),
                    makeSubMenu(tr("Other"), "other", otherNodes, agm)
                }
            },
            makeSubMenu(tr("Modify"), "modify", modifyNodes, agm),
            makeSubMenu(tr("Dimensions"), "dimension", dimNodes, agm),
            makeSubMenu(tr("Align"), "align", commonAlignActions(), agm),
            makeSubMenu(tr("Order"), "order", commonOrderActions(), agm),
            makeSubMenu(tr("Layers"), "layer", {{"LayersDefreezeAll"}}, agm),
            makeSubMenu(tr("Info"), "info", commonInfoActions(), agm),
            ActionNode("-"),
            makeSubMenu(tr("View"), "view", commonViewActions(), agm),
            makeSubMenu(tr("File"), "file", commonFileActions(), agm),
            ActionNode("SpecialMenu:WorkspacesRescue"),
            makeSubMenu(tr("Options"), "options", commonOptionActions(), agm)
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuAnyEntity(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Any Entity Fallback (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityUnknown}, {}, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuArc(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Arc (Right-Click)"), {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityArc}, {
                                    makeSubMenu(tr("Modify Arc"), "modify", {
                                                    {"DrawSliceDivideCircle"},
                                                    {"ModifyBreakDivide"},
                                                    {"ModifyCut"},
                                                    {"ModifyTrimAmount"},
                                                    {"ModifyTrim"},
                                                    {"ModifyTrim2"},
                                                    {"ModifyOffset"},
                                                    {"ModifyRevertDirection"},
                                                    {"-"},
                                                    {"ModifyRound"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Circle"), "circle", {
                                                    {"DrawCircleByArc"},
                                                    {"DrawCircleTan1_2P"},
                                                    {"DrawCircleTan2"},
                                                    {"DrawCircleTan2_1P"},
                                                    {"DrawCircleTan3"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Line"), "line", {
                                                    {"DrawLineOrthTan"},
                                                    {"DrawLineTangent1"},
                                                    {"DrawLineTangent2"},
                                                    {"DrawLineOrthogonal"},
                                                    {"DrawLineRelAngle"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {
                                                    {"DrawArcTangential"},
                                                    {"DrawCross"},
                                                    {"DrawCircleParallel"},
                                                    {"DrawLineParallelThrough"},
                                                    {"DrawBoundingBox"},
                                                    {"PolylineSegment"}
                                                }, agm),
                                    makeSubMenu(tr("Dimensions"), "dimension", {
                                                    {"DimRadial"},
                                                    {"DimDiametric"},
                                                    {"DimArc"},
                                                    {"DimLeader"},
                                                    {"DimOrdinate"},
                                                    {"-"},
                                                    {"DimStyles"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuPolyline(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Polyline (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityPolyline}, {
                                    makeSubMenu(tr("Modify Polyline"), "modify", {
                                                    {"PolylineAdd"},
                                                    {"PolylineAppend"},
                                                    {"PolylineDel"},
                                                    {"PolylineDelBetween"},
                                                    {"PolylineTrim"},
                                                    {"PolylineSegmentType"},
                                                    {"PolylineArcToLines"},
                                                    {"PolylineSegment"},
                                                    {"PolylineEquidistant"},
                                                    {"BlocksExplode"},
                                                    {"ModifyRevertDirection"},
                                                    {"ModifyOffset"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Line"), "line", {
                                                    {"DrawLineBisector"},
                                                    {"DrawLineOrthTan"},
                                                    {"DrawLineTangent1"},
                                                    {"DrawLineTangent2"},
                                                    {"DrawLineOrthogonal"},
                                                    {"DrawLineRelAngle"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {
                                                    {"PolylineEquidistant"},
                                                    {"DrawLineParallelThrough"},
                                                    {"DrawSplineFromPolyline"},
                                                    {"DrawBoundingBox"}
                                                }, agm),
                                    makeSubMenu(tr("Dimensions"), "dimension", {
                                                    {"DimAligned"},
                                                    {"DimLinear"},
                                                    {"DimLinearHor"},
                                                    {"DimLinearVer"},
                                                    {"DimAngular"},
                                                    {"DimLeader"},
                                                    {"DimOrdinate"},
                                                    {"-"},
                                                    {"DimStyles"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuSpline(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Spline (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntitySpline}, {
                                    makeSubMenu(tr("Modify Spline"), "modify", {
                                                    {"DrawSplinePointsAdd"},
                                                    {"DrawSplinePointsAppend"},
                                                    {"DrawSplinePointsRemove"},
                                                    {"DrawSplineExplode"},
                                                    {"DrawSplinePointsDelTwo"},
                                                    {"BlocksExplode"},
                                                    {"ModifyRevertDirection"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuSplinePoints(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Spline by Points (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntitySplinePoints}, {
                                    makeSubMenu(tr("Modify Spline Points"), "modify", {
                                                    {"DrawSplinePointsAdd"},
                                                    {"DrawSplinePointsAppend"},
                                                    {"DrawSplinePointsRemove"},
                                                    {"DrawSplineExplode"},
                                                    {"DrawSplinePointsDelTwo"},
                                                    {"ModifyCut"},
                                                    {"ModifyRevertDirection"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {{"DrawLineTangent1"}, {"DrawBoundingBox"}}, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuEllipse(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Ellipse (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityEllipse}, {
                                    ActionNode("ModifyCut"),
                                    ActionNode("ModifyRound"),
                                    makeSubMenu(tr("Draw Line"), "line", {
                                                    {"DrawLineOrthTan"},
                                                    {"DrawLineOrthogonal"},
                                                    {"DrawLineTangent1"},
                                                    {"DrawLineTangent2"},
                                                    {"DrawLineRelAngle"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {
                                                    {"DrawCross"},
                                                    {"DrawBoundingBox"},
                                                    {"DrawArcTangential"},
                                                    {"ModifyRevertDirection"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuPoint(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Point (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityPoint}, {
                                    ActionNode("SelectPoints"),
                                    ActionNode("PasteToPoints"),
                                    ActionNode("DrawPointsMiddle"),
                                    ActionNode("DrawLinePoints"),
                                    ActionNode("DrawPointLattice")
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuParabola(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Parabola (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityParabola}, {
                                    ActionNode("DrawLineOrthTan"),
                                    ActionNode("DrawLineTangent1"),
                                    ActionNode("DrawLineTangent2"),
                                    ActionNode("ModifyCut"),
                                    ActionNode("DrawBoundingBox")
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuText(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Text (Right-Click)"), {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityText},
                                {ActionNode("ModifyExplodeText"), ActionNode("BlocksExplode"), ActionNode("DrawBoundingBox")}, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuMText(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("MText (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityMText}, {
                                    ActionNode("ModifyExplodeText"),
                                    ActionNode("BlocksExplode"),
                                    ActionNode("DrawBoundingBox")
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuInsert(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Block Insert (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityInsert}, {
                                    ActionNode(LC_ActionNames::ActionEditBlock),
                                    ActionNode("EntityInfo"),
                                    ActionNode("BlocksExplode")
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuImage(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Image (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityImage}, {}, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuHatch(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Hatch (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityHatch}, {}, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimAligned(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Aligned (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimAligned}, {
                                     {"DimPickApply"},
                                     {"DimBaseline"},
                                     {"DimContinue"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimRadial(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Radial (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimRadial}, {
                                     {"DimPickApply"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimDiametric(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Diametric (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimDiametric}, {
                                     {"DimPickApply"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimArc(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Arc (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimArc}, {
                                     {"DimPickApply"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimOrdinate(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Ordinate (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimOrdinate}, {
                                     {"DimPickApply"},
                                     {"DimOrdinateForBase"},
                                     {"DimOrdinateReBase"},
                                     {"UCSSetByDimOrdinate"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimLeader(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Leader (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimLeader}, {
                                     {"DimPickApply"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuAutoZoom() {
    return {
        tr("AutoZoom (Middle Double-Click)"),
        {LC_MenuActivator::MIDDLE, LC_MenuActivator::DBL_CLICK, false, RS2::EntityUnknown},
        {ActionNode("ZoomAuto")}
    };
}

bool LC_DefaultContextMenusBuilder::isBuiltInContextMenu(const ContextMenuDef& menuDef) {
    return menuDef.isBuiltIn;
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuLine(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Line (Right-Click)"), {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityLine},
                                {
                                    makeSubMenu(tr("Modify Line"), "modify", {
                                                    {"DrawSliceDivideLine"},
                                                    {"ModifyCut"},
                                                    {"ModifyBreakDivide"},
                                                    {"ModifyTrimAmount"},
                                                    {"ModifyLineJoin"},
                                                    {"ModifyTrim"},
                                                    {"ModifyTrim2"},
                                                    {"ModifyLineGap"},
                                                    {"ModifyOffset"},
                                                    {"ModifyRevertDirection"},
                                                    {"-"},
                                                    {"ModifyRound"},
                                                    {"ModifyBevel"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Line"), "line", {
                                                    {"DrawLineParallelThrough"},
                                                    {"DrawLineOrthogonalRel"},
                                                    {"DrawLineOrthogonal"},
                                                    {"DrawLineParallel"},
                                                    {"DrawLineRel"},
                                                    {"DrawLineRelAngle"},
                                                    {"DrawLineAngleRel"},
                                                    {"DrawLineOrthTan"},
                                                    {"DrawLineBisector"},
                                                    {"DrawLineFree"},
                                                    {"DrawLineMiddle"},
                                                    {"DrawLineFromPointToLine"},
                                                    {"DrawLineRadiant"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Circle"), "circle", {
                                                    {"DrawCircleTan1_2P"},
                                                    {"DrawCircleTan2"},
                                                    {"DrawCircleTan2_1P"},
                                                    {"DrawCircleTan3"},
                                                    {"DrawCircleInscribe"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {
                                                    {"DrawArcTangential"},
                                                    {"DrawEllipseInscribe"},
                                                    {"DrawBoundingBox"},
                                                    {"PolylineSegment"}
                                                }, agm),
                                    makeSubMenu(tr("Dimensions"), "dimension", {
                                                    {"DimAligned"},
                                                    {"DimLinear"},
                                                    {"DimLinearHor"},
                                                    {"DimLinearVer"},
                                                    {"DimAngular"},
                                                    {"DimLeader"},
                                                    {"DimOrdinate"},
                                                    {"-"},
                                                    {"DimStyles"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuCircle(const LC_ActionGroupManager* agm) {
    return assembleGeometryMenu(tr("Circle (Right-Click)"),
                                {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityCircle}, {
                                    makeSubMenu(tr("Modify Circle"), "modify", {
                                                    {"DrawSliceDivideCircle"},
                                                    {"ModifyBreakDivide"},
                                                    {"ModifyCut"},
                                                    {"ModifyTrim"},
                                                    {"ModifyTrim2"},
                                                    {"ModifyOffset"},
                                                    {"-"},
                                                    {"ModifyRound"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Circle"), "circle", {
                                                    {"DrawCircleTan1_2P"},
                                                    {"DrawCircleTan2"},
                                                    {"DrawCircleTan2_1P"},
                                                    {"DrawCircleTan3"},
                                                    {"DrawCircleParallel"},
                                                    {"DrawLineParallelThrough"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Line"), "line", {
                                                    {"DrawLineOrthTan"},
                                                    {"DrawLineTangent1"},
                                                    {"DrawLineTangent2"},
                                                    {"DrawLineRelAngle"},
                                                    {"DrawLineOrthogonal"}
                                                }, agm),
                                    makeSubMenu(tr("Draw Other"), "other", {{"DrawCross"}, {"DrawBoundingBox"}}, agm),
                                    makeSubMenu(tr("Dimensions"), "dimension", {
                                                    {"DimRadial"},
                                                    {"DimDiametric"},
                                                    {"DimLeader"},
                                                    {"DimOrdinate"},
                                                    {"-"},
                                                    {"DimStyles"}
                                                }, agm)
                                }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimLinear(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Linear (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimLinear}, {
                                     {"DimPickApply"},
                                     {"DimBaseline"},
                                     {"DimContinue"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimAngular(const LC_ActionGroupManager* agm) {
    return assembleDimensionMenu(tr("Dimension Angular (Right-Click)"),
                                 {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimAngular}, {
                                     {"DimPickApply"},
                                     {"DimBaseline"},
                                     {"DimContinue"},
                                     {"DimRegenerate"},
                                     {"-"},
                                     {"DimStyles"},
                                     {"-"},
                                     {"EntityInfo"}
                                 }, agm);
}

ContextMenusConfig LC_DefaultContextMenusBuilder::createDefaultConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    ContextMenusConfig config;
    config.name = tr("Default - Compact");
    config.menus = {
        menuEmptySpace(af, agm),
        menuAnyEntity(agm),
        menuLine(agm),
        menuCircle(agm),
        menuArc(agm),
        menuPolyline(agm),
        menuSpline(agm),
        menuSplinePoints(agm),
        menuEllipse(agm),
        menuPoint(agm),
        menuParabola(agm),
        menuText(agm),
        menuMText(agm),
        menuInsert(agm),
        menuImage(agm),
        menuHatch(agm),
        menuDimLinear(agm),
        menuDimAligned(agm),
        menuDimRadial(agm),
        menuDimDiametric(agm),
        menuDimArc(agm),
        menuDimOrdinate(agm),
        menuDimLeader(agm),
        menuDimAngular(agm),
        menuAutoZoom()
    };
    for (auto& m : config.menus) {
        if (m.name != tr("AutoZoom (Middle Double-Click)")) {
            m.isBuiltIn = true;
        }
    }
    return config;
}

ContextMenusConfig LC_DefaultContextMenusBuilder::createExtendedConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    ContextMenusConfig config;
    config.name = tr("Default - Extended");
    config.menus = {
        menuEmptySpaceExtended(af, agm),
        extendEntityMenu(menuAnyEntity(agm)),
        extendEntityMenu(menuLine(agm)),
        extendEntityMenu(menuCircle(agm)),
        extendEntityMenu(menuArc(agm)),
        extendEntityMenu(menuPolyline(agm)),
        extendEntityMenu(menuSpline(agm)),
        extendEntityMenu(menuSplinePoints(agm)),
        extendEntityMenu(menuEllipse(agm)),
        extendEntityMenu(menuPoint(agm)),
        extendEntityMenu(menuParabola(agm)),
        extendEntityMenu(menuText(agm)),
        extendEntityMenu(menuMText(agm)),
        extendEntityMenu(menuInsert(agm)),
        extendEntityMenu(menuImage(agm)),
        extendEntityMenu(menuHatch(agm)),
        extendEntityMenu(menuDimLinear(agm)),
        extendEntityMenu(menuDimAligned(agm)),
        extendEntityMenu(menuDimRadial(agm)),
        extendEntityMenu(menuDimDiametric(agm)),
        extendEntityMenu(menuDimArc(agm)),
        extendEntityMenu(menuDimOrdinate(agm)),
        extendEntityMenu(menuDimLeader(agm)),
        extendEntityMenu(menuDimAngular(agm)),
        menuAutoZoom()
    };
    for (auto& m : config.menus) {
        if (m.name != tr("AutoZoom (Middle Double-Click)")) {
            m.isBuiltIn = true;
        }
    }
    return config;
}

// ------

QList<ActionNode> LC_DefaultContextMenusBuilder::commonEditActions() {
    return {
        {"EditUndo"},
        {"EditRedo"},
        {"-"},
        {"EditCopy"},
        {"EditCopyQuick"},
        {"-"},
        {"EditPaste"},
        {"EditPasteTransform"},
        {"PasteToPoints"},
        {"-"},
        {"EditCut"},
        {"EditCutQuick"},
        {"-"},
        {"PenPick"},
        {"PenPickResolved"},
        {"PenApply"},
        {"PenCopy"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonSelectActions() {
    return {
        {"SelectSingle"},
        {"SelectContour"},
        {"SelectIntersected"},
        {"DeselectIntersected"},
        {"SelectLayer"},
        {"SelectPoints"},
        {"SelectWindow"},
        {"DeselectWindow"},
        {"SelectAll"},
        {"DeselectAll"},
        {"SelectInvert"},
        {"SelectQuick"},
        {"SelectionModeToggle"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonModifyActions() {
    return {
        {"ModifyMove"},
        {"ModifyDuplicate"},
        {"ModifyRotate"},
        {"ModifyMirror"},
        {"ModifyScale"},
        {"ModifyStretch"},
        {"ModifyMoveRotate"},
        {"ModifyRotate2"},
        {"-"},
        {"ModifyDelete"},
        {"EntityInfo"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonAlignActions() {
    return {{"ModifyAlign"}, {"ModifyAlignOne"}, {"ModifyAlignRef"}};
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonOrderActions() {
    return {{"OrderBottom"}, {"OrderLower"}, {"-"}, {"OrderTop"}, {"OrderRaise"}};
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonLayerActions() {
    return {
        {"SpecialAction:EntityLayerActivate"},
        {"EntityLayerView"},
        {"EntityLayerLock"},
        {"EntityLayerPrint"},
        {"EntityLayerConstruction"},
        {"EntityLayerHideOthers"},
        {"LayersDefreezeAll"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonInfoActions() {
    return {
        {"EntityInfo"},
        {"InfoPoint"},
        {"InfoDist2"},
        {"InfoDist"},
        {"InfoDist3"},
        {"InfoAngle"},
        {"InfoAngle3Points"},
        {"InfoTotalLength"},
        {"InfoArea"},
        {"PickCoordinates"},
        {"-"},
        {"EntityDescriptionInfo"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonViewActions() {
    return {
        {"Fullscreen"},
        {"ViewGrid"},
        {"ViewDraft"},
        {"ViewLinesDraft"},
        {"ViewAntialiasing"},
        {"-"},
        {"ViewGridOrtho"},
        {"ViewGridIsoLeft"},
        {"ViewGridIsoTop"},
        {"ViewGridIsoRight"},
        {"-"},
        {"ZoomRedraw"},
        {"ZoomIn"},
        {"ZoomOut"},
        {"ZoomAuto"},
        {"ZoomPrevious"},
        {"ZoomWindow"},
        {"ZoomPan"},
        {"-"},
        {"ZoomViewSave"},
        {
            "Menu:ViewsRestore",
            ":/icons/nview_visible.lci",
            {{"ZoomViewRestore1"}, {"ZoomViewRestore2"}, {"ZoomViewRestore3"}, {"ZoomViewRestore4"}, {"ZoomViewRestore5"}}
        },
        {"-"},
        {"UCSCreate"},
        {"UCSSetWCS"},
        {"UCSSetByDimOrdinate"},
        {LC_ActionNames::MenuUCSList}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonFileActions() {
    return {
        {"FileNew"},
        {"FileNewTemplate"},
        {"FileOpen"},
        {"SpecialMenu:RecentFiles"},
        {"-"},
        {"FileSave"},
        {"FileSaveAs"},
        {"FileSaveAll"},
        {"-"},
        {"Menu:Import", ":/icons/import.lci", {{"DrawImage"}, {"BlocksImport"}}},
        {"Menu:Export", ":/icons/export.lci", {{"FileExportMakerCam"}, {"FilePrintPDF"}, {"FileExport"}}},
        {"-"},
        {"FilePrint"},
        {"FilePrintPreview"},
        {"-"},
        {"FileClose"},
        {"FileCloseAll"},
        {"FileQuit"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonOptionActions() {
    return {{"OptionsDrawing"}, {"OptionsGeneral"}, {"OptionsStyling"}, {"OptionsCustomization"}};
}
