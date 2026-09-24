
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

#ifndef LC_DEFAULT_CONTEXT_MENUS_BUILDER_H
#define LC_DEFAULT_CONTEXT_MENUS_BUILDER_H

#include <QCoreApplication>
#include "lc_action_node.h"

class LC_ActionFactory;
class LC_ActionGroupManager;

class LC_DefaultContextMenusBuilder {
    Q_DECLARE_TR_FUNCTIONS(LC_DefaultContextMenusBuilder)
public:
    static ContextMenusConfig createDefaultConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
    static ContextMenusConfig createExtendedConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
    static bool isBuiltInContextMenu(const ContextMenuDef& menuDef);

private:
    static ActionNode makeSubMenu(const QString& title, const QString& groupNameForIcon,
                                  const QList<ActionNode>& children, const LC_ActionGroupManager* agm);

    static QList<ActionNode> commonEditActions();
    static QList<ActionNode> commonSelectActions();
    static QList<ActionNode> commonModifyActions();
    static QList<ActionNode> commonAlignActions();
    static QList<ActionNode> commonOrderActions();
    static QList<ActionNode> commonLayerActions();
    static QList<ActionNode> commonInfoActions();
    static QList<ActionNode> commonViewActions();
    static QList<ActionNode> commonFileActions();
    static QList<ActionNode> commonOptionActions();

    static QList<ActionNode> commonEntityHeader();
    static QList<ActionNode> commonGeometryTail(const LC_ActionGroupManager* agm);
    static QList<ActionNode> commonDimensionTail(const LC_ActionGroupManager* agm);
    static QList<ActionNode> commonExtendedFooter();

    static ContextMenuDef assembleGeometryMenu(const QString& title, const LC_MenuActivator& activator,
                                               const QList<ActionNode>& specificNodes, const LC_ActionGroupManager* agm);
    static ContextMenuDef assembleDimensionMenu(const QString& title, const LC_MenuActivator& activator,
                                                const QList<ActionNode>& specificNodes, const LC_ActionGroupManager* agm);
    static ContextMenuDef extendEntityMenu(const ContextMenuDef& compactDef);

    static ContextMenuDef menuEmptySpace(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
    static ContextMenuDef menuEmptySpaceExtended(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);

    static ContextMenuDef menuAnyEntity(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuLine(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuCircle(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuArc(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuPolyline(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuSpline(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuSplinePoints(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuEllipse(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuPoint(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuParabola(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuText(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuMText(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuInsert(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuImage(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuHatch(const LC_ActionGroupManager* agm);

    static ContextMenuDef menuDimLinear(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimAligned(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimRadial(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimDiametric(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimArc(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimOrdinate(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimLeader(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuDimAngular(const LC_ActionGroupManager* agm);
    static ContextMenuDef menuAutoZoom();
};
#endif
