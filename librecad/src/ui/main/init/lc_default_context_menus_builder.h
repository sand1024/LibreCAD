
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

class LC_DefaultContextMenusBuilder {
    Q_DECLARE_TR_FUNCTIONS(LC_DefaultContextMenusBuilder)
public:
    static ContextMenusConfig createDefaultConfig();
    static bool isBuiltInContextMenu(const ContextMenuDef& menuDef);
private:
    static ContextMenuDef menuEmptySpace();
    static ContextMenuDef menuAnyEntity();
    static ContextMenuDef menuLine();
    static ContextMenuDef menuCircle();
    static ContextMenuDef menuArc();
    static ContextMenuDef menuPolyline();
    static ContextMenuDef menuSpline();
    static ContextMenuDef menuSplinePoints();
    static ContextMenuDef menuEllipse();
    static ContextMenuDef menuPoint();
    static ContextMenuDef menuParabola();
    static ContextMenuDef menuText();
    static ContextMenuDef menuMText();
    static ContextMenuDef menuInsert();
    static ContextMenuDef menuImage();
    static ContextMenuDef menuHatch();
    static ContextMenuDef menuDimLinear();
    static ContextMenuDef menuDimAligned();
    static ContextMenuDef menuDimRadial();
    static ContextMenuDef menuDimDiametric();
    static ContextMenuDef menuDimArc();
    static ContextMenuDef menuDimOrdinate();
    static ContextMenuDef menuDimLeader();
    static ContextMenuDef menuAutoZoom();

    static QList<ActionNode> commonModifyActions();
    static QList<ActionNode> commonAlignActions();
    static QList<ActionNode> commonOrderActions();
    static QList<ActionNode> commonLayerActions();
    static QList<ActionNode> commonInfoActions();
    static QList<ActionNode> commonViewActions();
    static QList<ActionNode> commonFileActions();
    static QList<ActionNode> commonOptionActions();
};

#endif
