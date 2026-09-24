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

#ifndef LC_CUSTOM_ACTION_NODE_H
#define LC_CUSTOM_ACTION_NODE_H

#include <QList>
#include <QString>

#include "lc_menu_activator.h"

enum class ActionNodeType {
    Action,    // Clickable action
    Separator, // Visual divider line
    Group      // Flyout group in toolbars or sub-menu in menus
};

enum class ToolButtonPopupMode {
    InstantPopup,       // Clicking the button directly opens the drop-down menu
    SplitFirstAction,   // Main button executes 1st action; side arrow opens menu
    SplitLastUsedAction // Main button executes last-used action (sticky)
};

struct ActionNode {
    ActionNodeType type = ActionNodeType::Separator;
    QString actionName;
    QString customLabel;
    QString groupTitle;
    QString groupIcon;
    ToolButtonPopupMode popupMode = ToolButtonPopupMode::InstantPopup;
    QList<ActionNode> children;

    ActionNode() = default;

    ActionNode(const QString& name) {
        if (name.isEmpty() || name == "-") {
            type = ActionNodeType::Separator;
        }
        else {
            type = ActionNodeType::Action;
            actionName = name;
        }
    }

    ActionNode(const char* name) {
        if (name == nullptr || *name == '\0' || QString::fromUtf8(name) == "-") {
            type = ActionNodeType::Separator;
        }
        else {
            type = ActionNodeType::Action;
            actionName = QString::fromUtf8(name);
        }
    }

    ActionNode(const char* name, const QString& label) {
        if (name == nullptr || *name == '\0' || QString::fromUtf8(name) == "-") {
            type = ActionNodeType::Separator;
        }
        else {
            type = ActionNodeType::Action;
            actionName = QString::fromUtf8(name);
        }
        customLabel = label;
    }

    ActionNode(const QString& title, const QString& icon, const QList<ActionNode>& childNodes,
               ToolButtonPopupMode mode = ToolButtonPopupMode::InstantPopup)
        : type(ActionNodeType::Group)
        , groupTitle(title)
        , groupIcon(icon)
        , popupMode(mode)
        , children(childNodes) {}
};

enum class ToolbarKind {
    Standard      = 0, // General application toolbars (File, Edit, View, Pen, etc.)
    Cad           = 1, // Floating/docked CAD toolbars (Line, Circle, etc.)
    CadDockWidget = 2, // Individual CAD dock widgets (Line, Circle, etc.)
    CadMatrix     = 3, // The Mega CAD Tools Matrix dock widget
    Host          = 4, // Pre-instantiated container toolbars (Tool Options, Snap, etc.)
    Custom        = 5  // User-created custom toolbars
};

struct ToolbarDef {
    QString name;
    QString icon;
    Qt::ToolBarArea area = Qt::TopToolBarArea;
    bool visible = true;
    bool lineBreak = false;
    ToolbarKind kind = ToolbarKind::Standard;
    QList<ActionNode> nodes;

    bool isBuiltIn() const {
        return kind != ToolbarKind::Custom;
    }

    bool isActionDriven() const {
        return kind == ToolbarKind::Standard || kind == ToolbarKind::Cad || kind == ToolbarKind::Custom;
    }
};

struct NavigationLayoutConfig {
    QString name;
    QList<ActionNode> menuMinimal;
    QList<ActionNode> menuCompact;
    QList<ActionNode> menuExtended;
    QList<ToolbarDef> toolbars;
};

struct ContextMenuDef {
    QString name;
    LC_MenuActivator activator;
    QList<ActionNode> nodes;
    bool isBuiltIn = false;

    ContextMenuDef() = default;

    ContextMenuDef(const QString& menuName, const LC_MenuActivator& act, const QList<ActionNode>& childNodes = {}, bool builtIn = false)
        : name(menuName), activator(act), nodes(childNodes), isBuiltIn(builtIn) {
    }
};

struct ContextMenusConfig {
    QString name;
    QList<ContextMenuDef> menus;
};

#endif
