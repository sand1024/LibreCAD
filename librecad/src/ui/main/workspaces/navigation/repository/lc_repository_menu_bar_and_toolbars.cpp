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

#include "lc_repository_menu_bar_and_toolbars.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

LC_RepositoryMenuBarAndToolbars::LC_RepositoryMenuBarAndToolbars(const QString& configDir)
    : LC_PresetRepositoryBase<NavigationLayoutConfig>(configDir, MENUS_TOOLBARS_EXTENSION, MENUS_TOOLBARS_FILE_IDENTIFIER, "toolbars_menus_index.lcix") {
}

QJsonArray LC_RepositoryMenuBarAndToolbars::serializeNodes(const QList<ActionNode>& nodes) {
    QJsonArray arr;
    for (const auto& node : nodes) {
        QJsonObject obj;
        obj["type"] = static_cast<int>(node.type);
        if (node.type == ActionNodeType::Action) {
            obj["actionName"] = node.actionName;
        }
        else if (node.type == ActionNodeType::Group) {
            obj["groupTitle"] = node.groupTitle;
            obj["groupIcon"] = node.groupIcon;
            obj["popupMode"] = static_cast<int>(node.popupMode);
            obj["children"] = serializeNodes(node.children);
        }
        arr.append(obj);
    }
    return arr;
}

QList<ActionNode> LC_RepositoryMenuBarAndToolbars::deserializeNodes(const QJsonArray& arr) {
    QList<ActionNode> nodes;
    for (const auto& val : arr) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject obj = val.toObject();
        ActionNode node;
        node.type = static_cast<ActionNodeType>(obj.value("type").toInt());
        if (node.type == ActionNodeType::Action) {
            node.actionName = obj.value("actionName").toString();
        }
        else if (node.type == ActionNodeType::Group) {
            node.groupTitle = obj.value("groupTitle").toString();
            node.groupIcon = obj.value("groupIcon").toString();
            node.popupMode = static_cast<ToolButtonPopupMode>(obj.value("popupMode").toInt());
            node.children = deserializeNodes(obj.value("children").toArray());
        }
        nodes.append(node);
    }
    return nodes;
}

QJsonObject LC_RepositoryMenuBarAndToolbars::configToJson(const NavigationLayoutConfig& config) const {
    QJsonObject root;
    root["name"] = config.name;
    root["activeMenuVariant"] = config.activeMenuVariant;
    root["menuCompact"] = serializeNodes(config.menuCompact);
    root["menuCompactTools"] = serializeNodes(config.menuCompactTools);
    root["menuExtended"] = serializeNodes(config.menuExtended);

    QJsonArray tbArray;
    for (const auto& tb : config.toolbars) {
        QJsonObject tbObj;
        tbObj["name"] = tb.name;
        tbObj["icon"] = tb.icon;
        tbObj["area"] = tb.area;
        tbObj["visible"] = tb.visible;
        tbObj["kind"] = static_cast<int>(tb.kind);
        tbObj["nodes"] = serializeNodes(tb.nodes);
        tbArray.append(tbObj);
    }
    root["toolbars"] = tbArray;
    return root;
}

bool LC_RepositoryMenuBarAndToolbars::configFromJson(const QJsonObject& json, NavigationLayoutConfig& config) const {
    if (!json.contains("name")) {
        return false;
    }

    config.name = json.value("name").toString();
    config.activeMenuVariant = json.value("activeMenuVariant").toInt(0);
    config.menuCompact = deserializeNodes(json.value("menuCompact").toArray());
    config.menuCompactTools = deserializeNodes(json.value("menuCompactTools").toArray());
    config.menuExtended = deserializeNodes(json.value("menuExtended").toArray());

    config.toolbars.clear();
    const QJsonArray tbArray = json.value("toolbars").toArray();
    for (const auto& val : tbArray) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject tbObj = val.toObject();
        ToolbarDef tb;
        tb.name = tbObj.value("name").toString();
        tb.icon = tbObj.value("icon").toString();
        tb.area = static_cast<Qt::ToolBarArea>(tbObj.value("area").toInt(static_cast<int>(Qt::TopToolBarArea)));
        tb.visible = tbObj.value("visible").toBool(true);
        tb.kind = static_cast<ToolbarKind>(tbObj.value("kind").toInt(static_cast<int>(ToolbarKind::Standard)));
        tb.nodes = deserializeNodes(tbObj.value("nodes").toArray());
        config.toolbars.append(tb);
    }
    return true;
}

void LC_RepositoryMenuBarAndToolbars::migrateLegacyToolbarsIfNeeded(NavigationLayoutConfig& config) {
    QDir dir(m_configDir);
    const QStringList legacyFiles = dir.entryList(QStringList() << "*.lctb", QDir::Files);

    for (const QString& fileName : legacyFiles) {
        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) {
            continue;
        }

        const QJsonArray tbArray = doc.object().value("toolbars").toArray();
        for (const auto& val : tbArray) {
            if (!val.isObject()) {
                continue;
            }
            QJsonObject tbObj = val.toObject();
            ToolbarDef tb;
            tb.name = tbObj.value("name").toString();
            tbObj["area"] = static_cast<int>(tb.area);
            tb.visible = tbObj.value("visible").toBool(true);
            tb.kind = ToolbarKind::Custom;
            tb.nodes = deserializeNodes(tbObj.value("nodes").toArray());

            bool exists = false;
            for (const auto& existing : config.toolbars) {
                if (existing.name == tb.name) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                config.toolbars.append(tb);
            }
        }
    }
}
