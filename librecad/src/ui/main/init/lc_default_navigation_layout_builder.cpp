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
#include "lc_action_group_manager.h"
#include "rs_debug.h"

namespace {


    QList<ActionNode> toNodes(const QList<QString>& names) {
        QList<ActionNode> nodes;
        nodes.reserve(names.size());
        for (const auto& name : names) {
            nodes.append(ActionNode(name));
        }
        return nodes;
    }

    ActionNode makeCategoryNode(const QString& groupName, const LC_ActionGroupManager* agm, const QList<ActionNode>& actions,
                                ToolButtonPopupMode popupMode = ToolButtonPopupMode::InstantPopup) {
        const auto* group = (agm != nullptr) ? agm->getActionGroup(groupName) : nullptr;
        QString token;
        QString icon;
        if (group != nullptr) {
            token = group->token();
            icon = group->getIconPath();
            if (icon.isEmpty()) {
                LC_ERR << "Icon empty: " << groupName;
            }
        }
        else {
            token = (QString("Menu:") + groupName);
            icon = QString();
            LC_ERR << "Icon empty no group: " << groupName;
        }
        ActionNode node(token, icon, actions);
        node.popupMode = popupMode;
        return node;
    }

    ActionNode makeCategoryNode(const QString& groupName, const LC_ActionGroupManager* agm,  const QList<QString>& names,
                                ToolButtonPopupMode popupMode = ToolButtonPopupMode::InstantPopup) {
        auto actionNodes = toNodes(names);
        return makeCategoryNode(groupName, agm, actionNodes, popupMode);
    }

    ToolbarDef makeStandardToolbar(const QString& name, const QString& icon, Qt::ToolBarArea area, const QList<ActionNode>& nodes,
                                   bool visible = true, bool lineBreak = false) {
        ToolbarDef tb;
        const QString cleanName = name.startsWith("tb_s_") ? name.mid(5) : name;
        tb.name = "tb_s_" + cleanName;
        tb.icon = icon;
        tb.area = area;
        tb.visible = visible;
        tb.kind = ToolbarKind::Standard;
        tb.nodes = nodes;
        tb.lineBreak = lineBreak;
        return tb;
    }

    ToolbarDef makeHostToolbar(const QString& name, const QString& icon, Qt::ToolBarArea area,
                               bool visible = true, bool lineBreak = false) {
        ToolbarDef tb;
        const QString cleanName = name.startsWith("tb_s_") ? name.mid(5) : name;
        tb.name = "tb_s_" + cleanName;
        tb.icon = icon;
        tb.area = area;
        tb.visible = visible;
        tb.lineBreak = lineBreak;
        tb.kind = ToolbarKind::Host;
        return tb;
    }

    ToolbarDef makeCadToolbar(const QString& groupName, const LC_ActionGroupManager* agm,
                              const QList<ActionNode>& actions, bool visible = false) {
        const QString cleanGroup = groupName.startsWith("tb_cad_") ? groupName.mid(7)
                                 : groupName.startsWith("cad_")    ? groupName.mid(4)
                                 : groupName;
        const auto* group = (agm != nullptr) ? agm->getActionGroup(cleanGroup) : nullptr;

        ToolbarDef tb;
        tb.name = "tb_cad_" + cleanGroup;
        tb.icon = (group != nullptr) ? group->getIconPath() : QString();
        tb.area = Qt::BottomToolBarArea;
        tb.visible = visible;
        tb.kind = ToolbarKind::Cad;
        tb.nodes = actions;
        return tb;
    }

    ToolbarDef makeCadToolbar(const QString& groupName, const LC_ActionGroupManager* agm,
                              const QList<QString>& names, bool visible = false) {
        auto actionNodes = toNodes(names);
        return makeCadToolbar(groupName, agm, actionNodes, visible);
    }

    ToolbarDef makeCadDockWidget(const QString& groupName, const LC_ActionGroupManager* agm,
                                 const QList<ActionNode>& actions, bool visible = false) {
        const QString cleanGroup = groupName.startsWith("dock_cad_") ? groupName.mid(9)
                                 : groupName.startsWith("cad_")      ? groupName.mid(4)
                                 : groupName;
        const auto* group = (agm != nullptr) ? agm->getActionGroup(cleanGroup) : nullptr;

        ToolbarDef tb;
        tb.name = "dock_cad_" + cleanGroup;
        tb.icon = (group != nullptr) ? group->getIconPath() : QString();
        tb.area = Qt::LeftToolBarArea;
        tb.visible = visible;
        tb.kind = ToolbarKind::CadDockWidget;
        tb.nodes = actions;
        return tb;
    }

    ToolbarDef makeCadDockWidget(const QString& groupName, const LC_ActionGroupManager* agm,
                                 const QList<QString>& names, bool visible = false) {
        auto actionNodes = toNodes(names);
        return makeCadDockWidget(groupName, agm, actionNodes, visible);
    }

    ToolbarDef makeCadMatrix(const QString& name, const QString& icon, const QList<ActionNode>& groups) {
        ToolbarDef tb;
        tb.name = "dock_cad_mega";
        tb.icon = icon;
        tb.area = Qt::LeftToolBarArea;
        tb.visible = false;
        tb.kind = ToolbarKind::CadMatrix;
        tb.nodes = groups;
        return tb;
    }
}


QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildCadDockWidgets(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    if (af == nullptr) {
        return {};
    }

    QList<ActionNode> matrixCategories = {
        makeCategoryNode("line",      agm, af->lineActions),
         makeCategoryNode("point",     agm, af->pointActions),
         makeCategoryNode("shape",     agm, af->shapeActions),
         makeCategoryNode("circle",    agm, af->circleActions),
         makeCategoryNode("curve",     agm, af->curveActions),
         makeCategoryNode("spline",    agm, af->splineActions),
         makeCategoryNode("ellipse",   agm, af->ellipseActions),
         makeCategoryNode("polyline",  agm, af->polylineActions),
         makeCategoryNode("select",    agm, af->selectActions),
         makeCategoryNode("modify",    agm, af->modifyActions),
         makeCategoryNode("dimension", agm, af->dimensionActions),
         makeCategoryNode("info",      agm, af->infoActions),
         makeCategoryNode("other",     agm, af->otherDrawingActions),
         makeCategoryNode("order",     agm, af->orderActions)
    };

    QList<ToolbarDef> dockWidgets;
    dockWidgets.append(makeCadMatrix(tr("CAD Tools Matrix"), ":/icons/line_polygon_star.lci", matrixCategories));

    dockWidgets.append({
       makeCadDockWidget("line",      agm, af->lineActions),
       makeCadDockWidget("point",     agm, af->pointActions),
       makeCadDockWidget("shape",     agm, af->shapeActions),
       makeCadDockWidget("circle",    agm, af->circleActions),
       makeCadDockWidget("curve",     agm, af->curveActions),
       makeCadDockWidget("spline",    agm, af->splineActions),
       makeCadDockWidget("ellipse",   agm, af->ellipseActions),
       makeCadDockWidget("polyline",  agm, af->polylineActions),
       makeCadDockWidget("select",    agm, af->selectActions),
       makeCadDockWidget("dimension", agm, af->dimensionActions),
       makeCadDockWidget("other",     agm, af->otherDrawingActions),
       makeCadDockWidget("modify",    agm, af->modifyActions),
       makeCadDockWidget("info",      agm, af->infoActions),
       makeCadDockWidget("order",     agm, af->orderActions)
    });

    return dockWidgets;
}

QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuMinimal(const LC_ActionFactory* af,
                                                                             const LC_ActionGroupManager* agm) {
    if (af == nullptr || agm == nullptr) {
        return {};
    }

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
                ActionNode{"Menu:Import", ":/icons/import.lci", {
                    "DrawImage",
                    "BlocksImport",
                    "-",
                    "BackupRestore"
                }},
                ActionNode{"Menu:Export", ":/icons/export.lci", {
                    "FileExportMakerCam",
                    "FilePrintPDF",
                    "FileExport",
                    "-",
                    "BackupExport"
                }},
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
            {"OptionsGeneral", "OptionsCustomization", "OptionsStyling", "ShortcutsOptions", "DeviceOptions", "ReloadStyleSheet", "-", "OptionsDrawing"}
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
                {"Menu:ViewsRestore", ":/icons/nview_visible.lci", {
                    "ZoomViewRestore1",
                    "ZoomViewRestore2",
                    "ZoomViewRestore3",
                    "ZoomViewRestore4",
                    "ZoomViewRestore5"
                }},
                "-",
                "UCSCreate",
                "UCSSetWCS",
                "UCSSetByDimOrdinate",
                LC_ActionNames::MenuUCSList
            }
        },

        // Plugins
        LC_ActionNames::MenuPlugins,

        // Tools (Combined minimal form)
        {
            "Menu:Tools",
            ":/icons/line_polygon_star.lci",
            {
                makeCategoryNode("line",      agm, af->lineActions),
                makeCategoryNode("point",     agm, af->pointActions),
                makeCategoryNode("circle",    agm, af->circleActions),
                makeCategoryNode("curve",     agm, af->curveActions),
                makeCategoryNode("shape",     agm, af->shapeActions),
                makeCategoryNode("spline",    agm, af->splineActions),
                makeCategoryNode("ellipse",   agm, af->ellipseActions),
                makeCategoryNode("polyline",  agm, af->polylineActions),
                makeCategoryNode("text",      agm, af->textActions),
                makeCategoryNode("select",    agm, af->selectActions),
                makeCategoryNode("dimension", agm, af->dimensionActions),
                makeCategoryNode("other",     agm, af->otherDrawingActions),
                makeCategoryNode("modify",    agm, af->modifyActions),
                makeCategoryNode("info",      agm, af->infoActions),
                makeCategoryNode("order",     agm, af->orderActions)
            }
        },

        // Workspace
        {
            "Menu:Workspace", ":/icons/workspace.lci",
            {
                "Fullscreen",
                "MainMenu",
                "ViewStatusBar",
                "-",
                {"Menu:DockAreas", "", {
                    "LeftDockAreaToggle",
                    "RightDockAreaToggle",
                    "TopDockAreaToggle",
                    "BottomDockAreaToggle",
                    "FloatingDockwidgetsToggle"
                }},
                {"Menu:ToolbarAreas", "", {
                    "LeftTBAreaToggle",
                    "RightTBAreaToggle",
                    "TopTBAreaToggle",
                    "BottomTBAreaToggle"
                }},
                "-",
                LC_ActionNames::MenuDockWidgets,
                LC_ActionNames::MenuCadDockWidgets,
                "-",
                LC_ActionNames::MenuToolbars,
                LC_ActionNames::MenuCadToolbars,
                "-",
                "RedockWidgets",
                "-",
                "WorkspaceCreate",
                "WorkspaceRestore",
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
                    {
                        "HelpWiki",
                        "HelpManual",
                        "HelpCommands",
                        "HelpStyleSheets",
                        "HelpWidgets"
                    }
                },
                "-",
                "HelpForum",
                "HelpZulip",
                "-",
                "HelpSubmitError",
                "HelpRequestFeature",
                "HelpReleases",
                "-",
                "HelpAbout",
                "HelpLicense",
                "-",
                "HelpDonate"
            }
        }
    };

    return menus;
}


QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuCompact(const LC_ActionFactory* af,
                                                                             const LC_ActionGroupManager* agm) {
    QList<ActionNode> menus = buildDefaultMenuMinimal(af, agm);

    int toolsIdx = -1;
    for (int i = 0; i < menus.size(); ++i) {
        if (menus[i].actionName == "Menu:Tools" || menus[i].groupTitle == "Menu:Tools") {
            toolsIdx = i;
            break;
        }
    }

    if (toolsIdx >= 0) {
        menus.removeAt(toolsIdx);
    }
    else {
        toolsIdx = menus.size();
    }

    QList<ActionNode> modifyWithOrder = toNodes(af->modifyActions);
    modifyWithOrder.append(ActionNode("-"));
    modifyWithOrder.append(makeCategoryNode("order", agm, toNodes(af->orderActions)));

    QList<ActionNode> drawSubmenus = {
        makeCategoryNode("line",     agm, af->lineActions),
        makeCategoryNode("point",    agm, af->pointActions),
        makeCategoryNode("circle",   agm, af->circleActions),
        makeCategoryNode("curve",    agm, af->curveActions),
        makeCategoryNode("shape",    agm, af->shapeActions),
        makeCategoryNode("spline",   agm, af->splineActions),
        makeCategoryNode("ellipse",  agm, af->ellipseActions),
        makeCategoryNode("polyline", agm, af->polylineActions),
        makeCategoryNode("text",     agm, af->textActions),
        makeCategoryNode("other",    agm, af->otherDrawingActions)
    };

    int insertIdx = toolsIdx;
    menus.insert(insertIdx++, makeCategoryNode("select", agm, af->selectActions));
    menus.insert(insertIdx++, ActionNode{"Menu:Draw", ":/icons/line.lci", drawSubmenus});
    menus.insert(insertIdx++, ActionNode{"Menu:Modify", ":/icons/move_rotate.lci", modifyWithOrder});
    menus.insert(insertIdx++, makeCategoryNode("dimension", agm, af->dimensionActions));
    menus.insert(insertIdx++, makeCategoryNode("info", agm, af->infoActions));

    return menus;
}


QList<ActionNode> LC_DefaultNavigationLayoutBuilder::buildDefaultMenuExtended(const LC_ActionFactory* af,
                                                                              const LC_ActionGroupManager* agm) {
    QList<ActionNode> menus = buildDefaultMenuMinimal(af, agm);

    int toolsIdx = -1;
    for (int i = 0; i < menus.size(); ++i) {
        if (menus[i].actionName == "Menu:Tools" || menus[i].groupTitle == "Menu:Tools") {
            toolsIdx = i;
            break;
        }
    }

    if (toolsIdx >= 0) {
        menus.removeAt(toolsIdx);
    }
    else {
        toolsIdx = menus.size();
    }

    QList<ActionNode> modifyWithOrder = toNodes(af->modifyActions);
    modifyWithOrder.append(ActionNode("-"));
    modifyWithOrder.append(makeCategoryNode("order", agm, af->orderActions));

    int insertIdx = toolsIdx;
    menus.insert(insertIdx++, makeCategoryNode("select",    agm, af->selectActions));
    menus.insert(insertIdx++, makeCategoryNode("line",      agm, af->lineActions));
    menus.insert(insertIdx++, makeCategoryNode("point",     agm, af->pointActions));
    menus.insert(insertIdx++, makeCategoryNode("circle",    agm, af->circleActions));
    menus.insert(insertIdx++, makeCategoryNode("curve",     agm, af->curveActions));
    menus.insert(insertIdx++, makeCategoryNode("shape",     agm, af->shapeActions));
    menus.insert(insertIdx++, makeCategoryNode("spline",    agm, af->splineActions));
    menus.insert(insertIdx++, makeCategoryNode("ellipse",   agm, af->ellipseActions));
    menus.insert(insertIdx++, makeCategoryNode("polyline",  agm, af->polylineActions));
    menus.insert(insertIdx++, makeCategoryNode("text",      agm, af->textActions));
    menus.insert(insertIdx++, makeCategoryNode("other",     agm, af->otherDrawingActions));
    menus.insert(insertIdx++, {"Menu:Modify", ":/icons/move_rotate.lci", modifyWithOrder});
    menus.insert(insertIdx++, makeCategoryNode("dimension", agm, af->dimensionActions));
    menus.insert(insertIdx++, makeCategoryNode("info",      agm, af->infoActions));

    return menus;
}

QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildStandardToolbars(const LC_ActionFactory* af,
                                                                          const LC_ActionGroupManager* agm) {
    if (af == nullptr || agm == nullptr) {
        return {};
    }

    return {
        // --- Top Area: Row 1 ---
        makeStandardToolbar("info_cursor", ":/icons/info_cursor_enable.lci", Qt::TopToolBarArea, {
            {"Menu:InfoCursor", ":/icons/info_cursor_enable.lci", {
                "InfoCursorEnable",
                "InfoCursorAbs",
                "InfoCursorSnap",
                "InfoCursorRel",
                "InfoCursorPrompt",
                "InfoCursorCatchedEntity",
                "EntityDescriptionInfo"
            }, ToolButtonPopupMode::SplitFirstAction}
        }),

        makeStandardToolbar("file", ":/icons/fileopen.lci", Qt::TopToolBarArea, toNodes(af->file_Actions)),

        makeStandardToolbar("edit", ":/icons/copy.lci", Qt::TopToolBarArea, {
            {"select", ":/icons/cursor.lci", toNodes(af->selectActions), ToolButtonPopupMode::SplitFirstAction},
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

        makeStandardToolbar("view", ":/icons/draft.lci", Qt::TopToolBarArea, {
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

        makeStandardToolbar("workspaces", ":/icons/workspace.lci", Qt::TopToolBarArea, {
            {"Menu:WorkspaceActions", ":/icons/workspace.lci", {
                "WorkspaceCreate",
                "WorkspaceRemove"
            }, ToolButtonPopupMode::SplitFirstAction},
            LC_ActionNames::WidgetWorkspaceSelector
        }),

        makeStandardToolbar("named_views", ":/icons/nview_visible.lci", Qt::TopToolBarArea, {
            "ZoomViewSave",
            LC_ActionNames::WidgetNamedViewsSelector
        }),

        makeStandardToolbar("ucs", ":/icons/ucs_set_wcs.lci", Qt::TopToolBarArea, {
            "UCSCreate",
            LC_ActionNames::WidgetUCSSelector
        }),

        makeStandardToolbar("options", ":/icons/settings.lci", Qt::TopToolBarArea, {
            "OptionsGeneral",
            "OptionsStyling",
            "OptionsCustomization",
            "OptionsDrawing"
        }),

        // --- Top Area: Row 2 (lineBreak = true on pen starts the second row) ---
        makeStandardToolbar("pen", ":/icons/pen_apply.lci", Qt::TopToolBarArea, toNodes(af->penActions),true, true),

        makeStandardToolbar("entity_layer", ":/icons/select_entity.lci", Qt::TopToolBarArea, {
            "EntityLayerActivate",
            {"Menu:EntityLayers", ":/icons/not_visible.lci", toNodes(af->entityLayerActions), ToolButtonPopupMode::SplitFirstAction}
        }),

        makeHostToolbar("tool_options", ":/icons/drawing_settings.lci", Qt::TopToolBarArea, true),

        // --- Left Area ---
        makeStandardToolbar("categories", ":/icons/line_polygon_star.lci", Qt::LeftToolBarArea, {
            makeCategoryNode("line",      agm, af->lineActions),
            makeCategoryNode("point",     agm, af->pointActions),
            makeCategoryNode("circle",    agm, af->circleActions),
            makeCategoryNode("curve",     agm, af->curveActions),
            makeCategoryNode("spline",    agm, af->splineActions),
            makeCategoryNode("shape",     agm, af->shapeActions),
            makeCategoryNode("ellipse",   agm, af->ellipseActions),
            makeCategoryNode("polyline",  agm, af->polylineActions),
            makeCategoryNode("text",      agm, af->textActions),
            makeCategoryNode("select",    agm, af->selectActions),
            makeCategoryNode("dimension", agm, af->dimensionActions),
            makeCategoryNode("other",     agm, af->otherDrawingActions),
            makeCategoryNode("modify",    agm, af->modifyActions),
            makeCategoryNode("info",      agm, af->infoActions),
            makeCategoryNode("order",     agm, af->orderActions)
        }),

        makeStandardToolbar("order", ":/icons/order.lci", Qt::LeftToolBarArea, toNodes(af->orderActions), false),

        // --- Bottom Area ---
        makeHostToolbar("snap", ":/icons/snap_visual.lci", Qt::BottomToolBarArea, true),

        makeStandardToolbar("dock_areas", ":/icons/dockwidgets_left.lci", Qt::BottomToolBarArea, {
            "LeftDockAreaToggle",
            "RightDockAreaToggle",
            "TopDockAreaToggle",
            "BottomDockAreaToggle",
            "FloatingDockwidgetsToggle"
        }),


        makeStandardToolbar("creators", ":/icons/create_toolbar.lci", Qt::BottomToolBarArea, {
            "InvokeMenuCreator",
            "InvokeToolbarCreator"
        })
    };
}


QList<ToolbarDef> LC_DefaultNavigationLayoutBuilder::buildCadToolbars(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    if (af == nullptr) {
        return {};
    }
    return {
       makeCadToolbar("line",      agm, af->lineActions),
       makeCadToolbar("point",     agm, af->pointActions),
       makeCadToolbar("shape",     agm, af->shapeActions),
       makeCadToolbar("circle",    agm, af->circleActions),
       makeCadToolbar("curve",     agm, af->curveActions),
       makeCadToolbar("spline",    agm, af->splineActions),
       makeCadToolbar("ellipse",   agm, af->ellipseActions),
       makeCadToolbar("polyline",  agm, af->polylineActions),
       makeCadToolbar("dimension", agm, af->dimensionActions),
       makeCadToolbar("other",     agm, af->otherDrawingActions),
       makeCadToolbar("modify",    agm, af->modifyActions),
       makeCadToolbar("info",      agm, af->infoActions),
       makeCadToolbar("select",    agm, af->selectActions),
       makeCadToolbar("order",     agm, af->orderActions)
    };
}

NavigationLayoutConfig LC_DefaultNavigationLayoutBuilder::createDefaultConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm) {
    NavigationLayoutConfig config;
    config.name = tr("Default Workspace");
    config.menuMinimal = buildDefaultMenuMinimal(af, agm);
    config.menuCompact = buildDefaultMenuCompact(af, agm);
    config.menuExtended = buildDefaultMenuExtended(af, agm);

    config.toolbars.clear();
    config.toolbars.append(buildStandardToolbars(af, agm));
    config.toolbars.append(buildCadToolbars(af, agm));
    config.toolbars.append(buildCadDockWidgets(af, agm));

    return config;
}
