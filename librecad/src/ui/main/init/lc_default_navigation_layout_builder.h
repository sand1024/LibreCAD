
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

#ifndef LC_DEFAULT_LAYOUT_BUILDER_H
#define LC_DEFAULT_LAYOUT_BUILDER_H

#include <QCoreApplication>
#include "lc_action_node.h"

class LC_ActionFactory;
class LC_ActionGroupManager;

class LC_DefaultNavigationLayoutBuilder {
    Q_DECLARE_TR_FUNCTIONS(LC_DefaultLayoutBuilder)
public:
    static NavigationLayoutConfig createDefaultConfig(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
private:
    static QList<ActionNode> buildDefaultMenuCompact(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
    static QList<ActionNode> buildDefaultMenuCompactTools(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);
    static QList<ActionNode> buildDefaultMenuExtended(const LC_ActionFactory* af, const LC_ActionGroupManager* agm);

    static QList<ToolbarDef> buildStandardToolbars();
    static QList<ToolbarDef> buildCadToolbars();

    static ToolbarDef makeStandardToolbar(const QString& name, const QString& icon, Qt::ToolBarArea area,
                                          const QList<ActionNode>& nodes, bool visible = true);
    static ToolbarDef makeCadToolbar(const QString& name, const QString& icon, const QList<ActionNode>& actions,
                                     bool visible = false);

    static ToolbarDef makeCadDockWidget(const QString& name, const QString& icon, const QList<ActionNode>& actions);
    static ToolbarDef makeCadMatrix(const QString& name, const QString& icon, const QList<ActionNode>& groups);
    static QList<ToolbarDef> buildCadDockWidgets();

    static const QList<ActionNode>& lineActions();
    static const QList<ActionNode>& shapeActions();
    static const QList<ActionNode>& pointActions();
    static const QList<ActionNode>& circleActions();
    static const QList<ActionNode>& curveActions();
    static const QList<ActionNode>& splineActions();
    static const QList<ActionNode>& ellipseActions();
    static const QList<ActionNode>& polylineActions();
    static const QList<ActionNode>& selectActions();
    static const QList<ActionNode>& dimensionActions();
    static const QList<ActionNode>& otherDrawingActions();
    static const QList<ActionNode>& modifyActions();
    static const QList<ActionNode>& orderActions();
    static const QList<ActionNode>& infoActions();

};

#endif
