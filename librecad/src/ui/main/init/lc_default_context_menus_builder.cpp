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
    return {
        {"ModifyAlign"},
        {"ModifyAlignOne"},
        {"ModifyAlignRef"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonOrderActions() {
    return {
        {"OrderBottom"},
        {"OrderLower"},
        {"-"},
        {"OrderTop"},
        {"OrderRaise"}
    };
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
        {"ZoomViewSave"}
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
        {"FilePrint"},
        {"FilePrintPreview"},
        {"-"},
        {"FileClose"},
        {"FileCloseAll"},
        {"FileQuit"}
    };
}

QList<ActionNode> LC_DefaultContextMenusBuilder::commonOptionActions() {
    return {
        {"OptionsDrawing"},
        {"OptionsGeneral"},
        {"OptionsStyling"},
        {"OptionsCustomization"}
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuEmptySpace() {
    return {
        tr("Empty Space (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, false, RS2::EntityUnknown},
        {
            {"SpecialMenu:RecentActions"},
            {"EditKillAllActions"},
            {
                "Menu:Select", ":/icons/select.lci",
                {
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
                }
            },
            {
                "Menu:Edit", ":/icons/rename_active_block.lci",
                {
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
                }
            },
            {
                "Menu:Draw", ":/icons/line_2p.lci",
                {
                    {"Menu:Line", ":/icons/line.lci", {{"DrawLine"}, {"DrawLineAngle"}, {"DrawLineHorizontal"}, {"DrawLineVertical"}, {"DrawLineParallel"}}},
                    {"Menu:Circle", ":/icons/circle.lci", {{"DrawCircle"}, {"DrawCircle2P"}, {"DrawCircleCR"}, {"DrawCircle3P"}}},
                    {"Menu:Arc", ":/icons/arc_center_point_angle.lci", {{"DrawArc"}, {"DrawArc3P"}, {"DrawArcTangential"}}},
                    {"Menu:Polyline", ":/icons/polylines_polyline.lci", {{"DrawPolyline"}, {"PolylineAdd"}, {"PolylineAppend"}, {"PolylineTrim"}}},
                    {"Menu:Spline", ":/icons/spline_points.lci", {{"DrawSpline"}, {"DrawSplinePoints"}}},
                    {"Menu:Polygon", ":/icons/rectangle_1_point.lci", {{"DrawLineRectangle"}, {"DrawLinePolygonCenCor"}}},
                    {"Menu:Other", ":/icons/text.lci", {{"DrawText"}, {"DrawMText"}, {"DrawHatch"}, {"DrawImage"}}}
                }
            },
            {"Menu:Modify", ":/icons/move_rotate.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", {{"LayersDefreezeAll"}}},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"-"},
            {"Menu:View", ":/icons/zoom_in.lci", commonViewActions()},
            {"Menu:File", ":/icons/save.lci", commonFileActions()},
            {"SpecialMenu:WorkspacesRescue"},
            {"Menu:Options", ":/icons/settings.lci", commonOptionActions()}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuLine() {
    return {
        tr("Line (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityLine},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
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
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {
                "Menu:Draw Line", ":/icons/line_parallel.lci",
                {
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
                }
            },
            {
                "Menu:Draw Circle", ":/icons/circle_center_point.lci",
                {
                    {"DrawCircleTan1_2P"},
                    {"DrawCircleTan2"},
                    {"DrawCircleTan2_1P"},
                    {"DrawCircleTan3"},
                    {"DrawCircleInscribe"}
                }
            },
            {
                "Menu:Draw Other", ":/icons/arc_continuation.lci",
                {
                    {"DrawArcTangential"},
                    {"DrawEllipseInscribe"},
                    {"DrawBoundingBox"},
                    {"PolylineSegment"}
                }
            },
            {
                "Menu:Dimensions", ":/icons/dim_aligned.lci",
                {
                    {"DimAligned"},
                    {"DimLinear"},
                    {"DimLinearHor"},
                    {"DimLinearVer"},
                    {"DimAngular"},
                    {"DimLeader"},
                    {"DimOrdinate"},
                    {"-"},
                    {"DimStyles"}
                }
            },
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuCircle() {
    return {
        tr("Circle (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityCircle},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
                    {"DrawSliceDivideCircle"},
                    {"ModifyBreakDivide"},
                    {"ModifyCut"},
                    {"ModifyTrim"},
                    {"ModifyTrim2"},
                    {"ModifyOffset"},
                    {"-"},
                    {"ModifyRound"}
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {
                "Menu:Draw Circle", ":/icons/circle_center_point.lci",
                {
                    {"DrawCircleTan1_2P"},
                    {"DrawCircleTan2"},
                    {"DrawCircleTan2_1P"},
                    {"DrawCircleTan3"},
                    {"DrawCircleParallel"},
                    {"DrawLineParallelThrough"}
                }
            },
            {
                "Menu:Draw Line", ":/icons/line_parallel.lci",
                {
                    {"DrawLineOrthTan"},
                    {"DrawLineTangent1"},
                    {"DrawLineTangent2"},
                    {"DrawLineRelAngle"},
                    {"DrawLineOrthogonal"}
                }
            },
            {"Menu:Draw Other", ":/icons/arc_continuation.lci", {{"DrawCross"}, {"DrawBoundingBox"}}},
            {
                "Menu:Dimensions", ":/icons/dim_aligned.lci",
                {
                    {"DimRadial"},
                    {"DimDiametric"},
                    {"DimLeader"},
                    {"DimOrdinate"},
                    {"-"},
                    {"DimStyles"}
                }
            },
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuArc() {
    return {
        tr("Arc (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityArc},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
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
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {
                "Menu:Draw Circle", ":/icons/circle_center_point.lci",
                {
                    {"DrawCircleByArc"},
                    {"DrawCircleTan1_2P"},
                    {"DrawCircleTan2"},
                    {"DrawCircleTan2_1P"},
                    {"DrawCircleTan3"}
                }
            },
            {
                "Menu:Draw Line", ":/icons/line_parallel.lci",
                {
                    {"DrawLineOrthTan"},
                    {"DrawLineTangent1"},
                    {"DrawLineTangent2"},
                    {"DrawLineOrthogonal"},
                    {"DrawLineRelAngle"}
                }
            },
            {
                "Menu:Draw Other", ":/icons/arc_continuation.lci",
                {
                    {"DrawArcTangential"},
                    {"DrawCross"},
                    {"DrawCircleParallel"},
                    {"DrawLineParallelThrough"},
                    {"DrawBoundingBox"},
                    {"PolylineSegment"}
                }
            },
            {
                "Menu:Dimensions", ":/icons/dim_aligned.lci",
                {
                    {"DimRadial"},
                    {"DimDiametric"},
                    {"DimArc"},
                    {"DimLeader"},
                    {"DimOrdinate"},
                    {"-"},
                    {"DimStyles"}
                }
            },
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuPolyline() {
    return {
        tr("Polyline (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityPolyline},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
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
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {
                "Menu:Draw Line", ":/icons/line_parallel.lci",
                {
                    {"DrawLineBisector"},
                    {"DrawLineOrthTan"},
                    {"DrawLineTangent1"},
                    {"DrawLineTangent2"},
                    {"DrawLineOrthogonal"},
                    {"DrawLineRelAngle"}
                }
            },
            {
                "Menu:Draw Other", ":/icons/arc_continuation.lci",
                {
                    {"PolylineEquidistant"},
                    {"DrawLineParallelThrough"},
                    {"DrawSplineFromPolyline"},
                    {"DrawBoundingBox"}
                }
            },
            {
                "Menu:Dimensions", ":/icons/dim_aligned.lci",
                {
                    {"DimAligned"},
                    {"DimLinear"},
                    {"DimLinearHor"},
                    {"DimLinearVer"},
                    {"DimAngular"},
                    {"DimLeader"},
                    {"DimOrdinate"},
                    {"-"},
                    {"DimStyles"}
                }
            },
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuSpline() {
    return {
        tr("Spline (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntitySpline},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
                    {"DrawSplinePointsAdd"},
                    {"DrawSplinePointsAppend"},
                    {"DrawSplinePointsRemove"},
                    {"DrawSplineExplode"},
                    {"DrawSplinePointsDelTwo"},
                    {"BlocksExplode"},
                    {"ModifyRevertDirection"}
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuSplinePoints() {
    return {
        tr("Spline by Points (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntitySplinePoints},
        {
            {
                "Menu:Modify", ":/icons/attributes.lci",
                {
                    {"DrawSplinePointsAdd"},
                    {"DrawSplinePointsAppend"},
                    {"DrawSplinePointsRemove"},
                    {"DrawSplineExplode"},
                    {"DrawSplinePointsDelTwo"},
                    {"ModifyCut"},
                    {"ModifyRevertDirection"}
                }
            },
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Draw Other", ":/icons/arc_continuation.lci", {{"DrawLineTangent1"}, {"DrawBoundingBox"}}},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuEllipse() {
    return {
        tr("Ellipse (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityEllipse},
        {
            {"ModifyCut"},
            {"ModifyRound"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {
                "Menu:Draw Line", ":/icons/line_parallel.lci",
                {
                    {"DrawLineOrthTan"},
                    {"DrawLineOrthogonal"},
                    {"DrawLineTangent1"},
                    {"DrawLineTangent2"},
                    {"DrawLineRelAngle"}
                }
            },
            {
                "Menu:Draw Other", ":/icons/arc_continuation.lci",
                {
                    {"DrawCross"},
                    {"DrawBoundingBox"},
                    {"DrawArcTangential"},
                    {"ModifyRevertDirection"}
                }
            },
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuPoint() {
    return {
        tr("Point (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityPoint},
        {
            {"SelectPoints"},
            {"PasteToPoints"},
            {"DrawPointsMiddle"},
            {"DrawLinePoints"},
            {"DrawPointLattice"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuParabola() {
    return {
        tr("Parabola (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityParabola},
        {
            {"DrawLineOrthTan"},
            {"DrawLineTangent1"},
            {"DrawLineTangent2"},
            {"ModifyCut"},
            {"DrawBoundingBox"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuText() {
    return {
        tr("Text (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityText},
        {
            {"ModifyExplodeText"},
            {"BlocksExplode"},
            {"DrawBoundingBox"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuMText() {
    return {
        tr("MText (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityMText},
        {
            {"ModifyExplodeText"},
            {"BlocksExplode"},
            {"DrawBoundingBox"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuInsert() {
    return {
        tr("Block Insert (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityInsert},
        {
            {"SpecialAction:EditBlock"},
            {"EntityInfo"},
            {"BlocksExplode"},
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuImage() {
    return {
        tr("Image (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityImage},
        {
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuHatch() {
    return {
        tr("Hatch (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityHatch},
        {
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimLinear() {
    return {
        tr("Dimension Linear (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimLinear},
        {
            {"DimPickApply"},
            {"DimBaseline"},
            {"DimContinue"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimAligned() {
    return {
        tr("Dimension Aligned (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimAligned},
        {
            {"DimPickApply"},
            {"DimBaseline"},
            {"DimContinue"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimRadial() {
    return {
        tr("Dimension Radial (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimRadial},
        {
            {"DimPickApply"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimDiametric() {
    return {
        tr("Dimension Diametric (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimDiametric},
        {
            {"DimPickApply"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimArc() {
    return {
        tr("Dimension Arc (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimArc},
        {
            {"DimPickApply"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimOrdinate() {
    return {
        tr("Dimension Ordinate (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimOrdinate},
        {
            {"DimPickApply"},
            {"DimOrdinateForBase"},
            {"DimOrdinateReBase"},
            {"UCSSetByDimOrdinate"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuDimLeader() {
    return {
        tr("Dimension Leader (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityDimLeader},
        {
            {"DimPickApply"},
            {"DimRegenerate"},
            {"-"},
            {"DimStyles"},
            {"-"},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuAnyEntity() {
    return {
        tr("Any Entity Fallback (Right-Click)"),
        {LC_MenuActivator::RIGHT, LC_MenuActivator::CLICK_RELEASE, true, RS2::EntityUnknown},
        {
            {"Menu:Modify Generic", ":/icons/move_copy.lci", commonModifyActions()},
            {"Menu:Align", ":/icons/align_one.lci", commonAlignActions()},
            {"Menu:Order", ":/icons/order.lci", commonOrderActions()},
            {"Menu:Layers", ":/icons/layer_list.lci", commonLayerActions()},
            {"Menu:Info", ":/icons/measure.lci", commonInfoActions()},
            {"EntityInfo"}
        }
    };
}

ContextMenuDef LC_DefaultContextMenusBuilder::menuAutoZoom() {
    return {
        tr("AutoZoom (Middle Double-Click)"),
        {LC_MenuActivator::MIDDLE, LC_MenuActivator::DBL_CLICK, false, RS2::EntityUnknown},
        {{"ZoomAuto"}}
    };
}

ContextMenusConfig LC_DefaultContextMenusBuilder::createDefaultConfig() {
    ContextMenusConfig config;
    config.name = tr("Default Menus Suite");
    config.menus = {
        menuEmptySpace(),
        menuAnyEntity(),
        menuLine(),
        menuCircle(),
        menuArc(),
        menuPolyline(),
        menuSpline(),
        menuSplinePoints(),
        menuEllipse(),
        menuPoint(),
        menuParabola(),
        menuText(),
        menuMText(),
        menuInsert(),
        menuImage(),
        menuHatch(),
        menuDimLinear(),
        menuDimAligned(),
        menuDimRadial(),
        menuDimDiametric(),
        menuDimArc(),
        menuDimOrdinate(),
        menuDimLeader(),
        menuAutoZoom()
    };
    for (auto& m : config.menus) {
        // AutoZoom is a gesture popup and can be customized/deleted by user
        if (m.name != tr("AutoZoom (Middle Double-Click)")) {
            m.isBuiltIn = true;
        }
    }
    return config;
}

bool LC_DefaultContextMenusBuilder::isBuiltInContextMenu(const ContextMenuDef& menuDef) {
    return menuDef.isBuiltIn;
}
