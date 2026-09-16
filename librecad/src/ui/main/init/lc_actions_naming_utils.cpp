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
#include "lc_actions_naming_utils.h"

#include <QSet>

#include "lc_action_group.h"
#include "lc_action_group_manager.h"

namespace {
    inline constexpr const char* PREFIX_MENU = "Menu:";
    inline constexpr int PREFIX_MENU_LEN = 5;
}

LC_ActionNamingService::LC_ActionNamingService(LC_ActionGroupManager* agm)
    : m_groupManager(agm) {
}

bool LC_ActionNamingService::isSystemToken(const QString& token) const {
    return token.startsWith(PREFIX_MENU);
}

QString LC_ActionNamingService::normalizeKeyToGroupName(const QString& key) const {
    const QString lower = key.toLower();

    if (lower == "line" || lower == "lines")
        return "line";
    if (lower == "point" || lower == "points")
        return "point";
    if (lower == "circle" || lower == "circles")
        return "circle";
    if (lower == "arc" || lower == "arcs" || lower == "curve")
        return "curve";
    if (lower == "polygon" || lower == "polygons" || lower == "shape")
        return "shape";
    if (lower == "spline" || lower == "splines")
        return "spline";
    if (lower == "ellipse" || lower == "ellipses")
        return "ellipse";
    if (lower == "polyline" || lower == "polylines")
        return "polyline";
    if (lower == "dimension" || lower == "dimensions")
        return "dimension";
    if (lower == "modify" || lower == "modify generic")
        return "modify";
    if (lower == "info" || lower == "measure")
        return "info";
    if (lower == "select")
        return "select";
    if (lower == "order")
        return "order";
    if (lower == "pen")
        return "pen";
    if (lower == "file")
        return "file";
    if (lower == "edit")
        return "edit";
    if (lower == "view")
        return "view";
    if (lower == "options" || lower == "tools")
        return "options";
    if (lower == "other")
        return "other";
    if (lower == "block" || lower == "blocks")
        return "block";
    if (lower == "layer" || lower == "layers")
        return "layer";
    if (lower == "workspace" || lower == "workspaces")
        return "workspaces";
    if (lower == "help")
        return "help";
    if (lower == "ucs")
        return "ucs";

    return lower;
}

QString LC_ActionNamingService::canonicalToken(const LC_ActionGroup* group) const {
    if (group == nullptr) {
        return QString();
    }
    return canonicalToken(group->getName());
}

QString LC_ActionNamingService::canonicalToken(const QString& groupName) const {
    if (groupName == "line")
        return "Menu:Line";
    if (groupName == "point")
        return "Menu:Point";
    if (groupName == "circle")
        return "Menu:Circle";
    if (groupName == "curve")
        return "Menu:Arc";
    if (groupName == "shape")
        return "Menu:Polygon";
    if (groupName == "spline")
        return "Menu:Spline";
    if (groupName == "ellipse")
        return "Menu:Ellipse";
    if (groupName == "polyline")
        return "Menu:Polyline";
    if (groupName == "dimension")
        return "Menu:Dimension";
    if (groupName == "modify")
        return "Menu:Modify";
    if (groupName == "info")
        return "Menu:Info";
    if (groupName == "select")
        return "Menu:Select";
    if (groupName == "order")
        return "Menu:Order";
    if (groupName == "pen")
        return "Menu:Pen";
    if (groupName == "file")
        return "Menu:File";
    if (groupName == "edit")
        return "Menu:Edit";
    if (groupName == "view")
        return "Menu:View";
    if (groupName == "options")
        return "Menu:Options";
    if (groupName == "other")
        return "Menu:Other";
    if (groupName == "block")
        return "Menu:Blocks";
    if (groupName == "layer")
        return "Menu:Layers";
    if (groupName == "workspaces")
        return "Menu:Workspace";
    if (groupName == "help")
        return "Menu:Help";
    if (groupName == "ucs")
        return "Menu:UCS";
    if (groupName == "namedViews")
        return "Menu:ViewsRestore";

    if (!groupName.isEmpty()) {
        QString cap = groupName;
        cap[0] = cap[0].toUpper();
        return PREFIX_MENU + cap;
    }
    return QString();
}

QString LC_ActionNamingService::iconPath(const QString& token) const {
    if (!token.startsWith(PREFIX_MENU)) {
        return QString();
    }

    const QString key = token.mid(PREFIX_MENU_LEN);

    // 1. Query LC_ActionGroup via ActionGroupManager as the single source of truth
    if (m_groupManager != nullptr) {
        const QString groupName = normalizeKeyToGroupName(key);
        const auto* group = m_groupManager->getActionGroup(groupName);
        if (group != nullptr && !group->getIconPath().isEmpty()) {
            return group->getIconPath();
        }
    }

    // 2. Structural menu containers (not corresponding to an LC_ActionGroup)
    if (key == "Import")
        return ":/icons/import.lci";
    if (key == "Export")
        return ":/icons/export.lci";
    if (key == "ViewsRestore")
        return ":/icons/nview_visible.lci";
    if (key == "OnlineDocs" || key == "Plugins")
        return ":/icons/help.lci";
    if (key == "DockAreas" || key == "ToolbarAreas")
        return ":/icons/dockwidgets_left.lci";
    if (key == "Align")
        return ":/icons/align_one.lci";

    return QString();
}

QString LC_ActionNamingService::displayName(const QString& token, bool stripAmpersand) const {
    QString title = token;

    if (token.startsWith(PREFIX_MENU)) {
        const QString key = token.mid(PREFIX_MENU_LEN);

        // 1. Structural menu containers
        if (key == "Import")
            title = tr("Import");
        else if (key == "Export")
            title = tr("Export");
        else if (key == "ViewsRestore")
            title = tr("&Views Restore");
        else if (key == "OnlineDocs")
            title = tr("On&line Docs");
        else if (key == "Plugins")
            title = tr("Pl&ugins");
        else if (key == "DockAreas")
            title = tr("Dock Areas");
        else if (key == "ToolbarAreas")
            title = tr("Toolbar Areas");
        else if (key == "Align")
            title = tr("Align");
        else if (key == "Draw Line")
            title = tr("Draw Line");
        else if (key == "Draw Circle")
            title = tr("Draw Circle");
        else if (key == "Draw Other")
            title = tr("Draw Other");
        else if (key == "Modify Generic")
            title = tr("Modify Generic");
            // Plural toolbar forms
        else if (key == "Lines")
            title = tr("Lines");
        else if (key == "Points")
            title = tr("Points");
        else if (key == "Circles")
            title = tr("Circles");
        else if (key == "Arcs")
            title = tr("Arcs");
        else if (key == "Polygons")
            title = tr("Polygons");
        else if (key == "Splines")
            title = tr("Splines");
        else if (key == "Ellipses")
            title = tr("Ellipses");
        else if (key == "PolyLines")
            title = tr("PolyLines");
        else if (key == "Dimensions")
            title = tr("&Dimensions");
        else {
            // 2. Query LC_ActionGroup via ActionGroupManager
            if (m_groupManager != nullptr) {
                const QString groupName = normalizeKeyToGroupName(key);
                const auto* group = m_groupManager->getActionGroup(groupName);
                if (group != nullptr) {
                    title = group->getTitle();
                }
                else {
                    title = key;
                }
            }
            else {
                title = key;
            }
        }
    }

    if (stripAmpersand) {
        title.remove('&');
    }
    return title;
}

QString LC_ActionNamingService::toolbarTitle(const QString& toolbarKey) const {
    if (toolbarKey == "File")
        return tr("File");
    if (toolbarKey == "Edit")
        return tr("Edit");
    if (toolbarKey == "View")
        return tr("View");
    if (toolbarKey == "Pen")
        return tr("Pen");
    if (toolbarKey == "Preferences")
        return tr("Preferences");
    if (toolbarKey == "Categories")
        return tr("Categories");
    if (toolbarKey == "Order")
        return tr("Order");
    if (toolbarKey == "Named Views")
        return tr("Named Views");
    if (toolbarKey == "UCS")
        return tr("UCS");
    if (toolbarKey == "Workspaces")
        return tr("Workspaces");
    if (toolbarKey == "Dock Areas")
        return tr("Dock Areas");
    if (toolbarKey == "Creators")
        return tr("Creators");
    if (toolbarKey == "Entity's Layer")
        return tr("Entity's Layer");
    if (toolbarKey == "Info Cursor")
        return tr("Info Cursor");

    // Canonical CAD Dock Widget Names
    if (toolbarKey == "dock_cad_mega")
        return tr("CAD Tools Matrix");
    if (toolbarKey == "dock_line")
        return tr("Line (Dock Widget)");
    if (toolbarKey == "dock_point")
        return tr("Point (Dock Widget)");
    if (toolbarKey == "dock_shape")
        return tr("Polygon (Dock Widget)");
    if (toolbarKey == "dock_circle")
        return tr("Circle (Dock Widget)");
    if (toolbarKey == "dock_curve")
        return tr("Arc (Dock Widget)");
    if (toolbarKey == "dock_spline")
        return tr("Spline (Dock Widget)");
    if (toolbarKey == "dock_ellipse")
        return tr("Ellipse (Dock Widget)");
    if (toolbarKey == "dock_polyline")
        return tr("Polyline (Dock Widget)");
    if (toolbarKey == "dock_select")
        return tr("Select (Dock Widget)");
    if (toolbarKey == "dock_dimension")
        return tr("Dimension (Dock Widget)");
    if (toolbarKey == "dock_other")
        return tr("Other (Dock Widget)");
    if (toolbarKey == "dock_modify")
        return tr("Modify (Dock Widget)");
    if (toolbarKey == "dock_info")
        return tr("Info (Dock Widget)");
    if (toolbarKey == "dock_order")
        return tr("Order (Dock Widget)");

    return toolbarKey;
}

QList<QPair<QString, QString>> LC_ActionNamingService::predefinedCategories() const {
    QList<QPair<QString, QString>> categories;
    QSet<QString> addedTokens;

    // 1. Dynamic groups from ActionGroupManager
    if (m_groupManager != nullptr) {
        for (const auto* group : m_groupManager->allGroupsList()) {
            if (group != nullptr && group->isToolbarMenuConfigurable()) {
                const QString token = canonicalToken(group->getName());
                if (!addedTokens.contains(token)) {
                    addedTokens.insert(token);
                    const QString label = displayName(token, /*stripAmpersand=*/true);
                    categories.append(qMakePair(token, label));
                }
            }
        }
    }

    // 2. Predefined structural menu containers
    static const QStringList structuralKeys = {
        "Import",
        "Export",
        "Align",
        "Draw Line",
        "Draw Circle",
        "Draw Other",
        "Modify Generic",
        "OnlineDocs",
        "Plugins",
        "ViewsRestore",
        "DockAreas",
        "ToolbarAreas"
    };

    for (const auto& key : structuralKeys) {
        const QString token = PREFIX_MENU + key;
        if (!addedTokens.contains(token)) {
            addedTokens.insert(token);
            const QString label = displayName(token, /*stripAmpersand=*/true);
            categories.append(qMakePair(token, label));
        }
    }

    std::sort(categories.begin(), categories.end(), [](const QPair<QString, QString>& a, const QPair<QString, QString>& b) {
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    return categories;
}
