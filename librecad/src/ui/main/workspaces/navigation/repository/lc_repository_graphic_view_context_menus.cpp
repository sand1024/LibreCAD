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

#include "lc_repository_graphic_view_context_menus.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>

namespace {
    QJsonObject serializeMenuNode(const ActionNode& node) {
        QJsonObject obj;
        obj["type"] = static_cast<int>(node.type);

        if (node.type == ActionNodeType::Action) {
            obj["action"] = node.actionName;
        } else if (node.type == ActionNodeType::Group) {
            obj["title"] = node.groupTitle;
            obj["icon"] = node.groupIcon;

            QJsonArray childrenArr;
            for (const auto& child : node.children) {
                childrenArr.append(serializeMenuNode(child));
            }
            obj["children"] = childrenArr;
        }
        return obj;
    }

    ActionNode deserializeMenuNode(const QJsonObject& obj) {
        ActionNode node;
        node.type = static_cast<ActionNodeType>(obj["type"].toInt(static_cast<int>(ActionNodeType::Action)));

        if (node.type == ActionNodeType::Action) {
            node.actionName = obj["action"].toString();
        } else if (node.type == ActionNodeType::Group) {
            node.groupTitle = obj["title"].toString();
            node.groupIcon = obj["icon"].toString();

            const QJsonArray childrenArr = obj["children"].toArray();
            for (const auto& val : childrenArr) {
                node.children.append(deserializeMenuNode(val.toObject()));
            }
        }
        return node;
    }

    inline const QString MENUS_EXTENSION = ".lccm";
    inline const QString MENUS_FILE_IDENTIFIER = "LibreCAD Config: Context Menus";
}

LC_RepositoryGraphicViewContextMenus::LC_RepositoryGraphicViewContextMenus(const QString& configDir)
    : LC_PresetRepositoryBase<ContextMenusConfig>(
          configDir, MENUS_EXTENSION, MENUS_FILE_IDENTIFIER, "menus_index.lcix") {
}

QJsonObject LC_RepositoryGraphicViewContextMenus::configToJson(const ContextMenusConfig& config) const {
    QJsonObject root;
    QJsonArray menusArr;

    for (const auto& m : config.menus) {
        QJsonObject mObj;
        mObj["name"] = m.name;
        mObj["activator"] = m.activator.getShortcut();
        mObj["builtIn"] = m.isBuiltIn;
        QJsonArray nodesArr;
        for (const auto& node : m.nodes) {
            nodesArr.append(serializeMenuNode(node));
        }
        mObj["nodes"] = nodesArr;
        menusArr.append(mObj);
    }

    root["menus"] = menusArr;
    return root;
}

bool LC_RepositoryGraphicViewContextMenus::configFromJson(const QJsonObject& json, ContextMenusConfig& config) const {
    config.menus.clear();
    const QJsonArray menusArr = json["menus"].toArray();

    for (const auto& val : menusArr) {
        const QJsonObject mObj = val.toObject();
        ContextMenuDef m;
        m.name = mObj["name"].toString();
        m.isBuiltIn = mObj["builtIn"].toBool(false);

        const QString actStr = mObj["activator"].toString();
        auto* parsedAct = LC_MenuActivator::fromShortcut(actStr);
        if (parsedAct != nullptr) {
            m.activator = *parsedAct;
            delete parsedAct;
        }

        const QJsonArray nodesArr = mObj["nodes"].toArray();
        for (const auto& nVal : nodesArr) {
            m.nodes.append(deserializeMenuNode(nVal.toObject()));
        }
        config.menus.append(m);
    }
    return true;
}

void LC_RepositoryGraphicViewContextMenus::migrateLegacyMenusIfNeeded() {
    if (!getAvailableNames().isEmpty()) return;

    QSettings settings;
    settings.beginGroup("CustomMenus");
    const QStringList keys = settings.childKeys();
    if (keys.isEmpty()) {
        settings.endGroup();
        return;
    }

    QMap<QString, QString> menuToShortcut;
    settings.beginGroup("../Activators");
    for (const auto& aKey : settings.childKeys()) {
        menuToShortcut.insert(settings.value(aKey).toString(), aKey);
    }
    settings.endGroup();

    ContextMenusConfig migrated;
    migrated.name = QObject::tr("Imported Menus");

    settings.beginGroup("CustomMenus");
    for (const auto& key : keys) {
        ContextMenuDef m;
        m.name = key;
        const QString actStr = menuToShortcut.value(key, "");
        auto* parsedAct = LC_MenuActivator::fromShortcut(actStr);
        if (parsedAct != nullptr) {
            m.activator = *parsedAct;
            delete parsedAct;
        }

        const QStringList flat = settings.value(key).toStringList();
        for (const auto& act : flat) {
            ActionNode node;
            node.type = act.isEmpty() ? ActionNodeType::Separator : ActionNodeType::Action;
            node.actionName = act;
            m.nodes.append(node);
        }
        migrated.menus.append(m);
    }
    settings.endGroup();

    QString outKey;
    save(migrated.name, migrated, outKey);
    initializeIndex();
}
