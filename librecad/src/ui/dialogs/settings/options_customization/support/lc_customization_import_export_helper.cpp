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

#include "lc_customization_import_export_helper.h"

#include <QCoreApplication>
#include <QJsonObject>

LC_CustomizationImportExportHelper::LC_CustomizationImportExportHelper(
    LC_RepositoryKeymaps* shortcutsRepo,
    LC_RepositoryCommands* commandsRepo,
    LC_RepositoryMenuBarAndToolbars* navRepo,
    LC_RepositoryGraphicViewContextMenus* menusRepo)
    : LC_ProfileExchangeHelperBase(CUSTOMIZATION_PROFILE_FILE_IDENTIFIER)
    , m_shortcutsRepo(shortcutsRepo)
    , m_commandsRepo(commandsRepo)
    , m_navRepo(navRepo)
    , m_menusRepo(menusRepo) {
}

bool LC_CustomizationImportExportHelper::exportProfile(const QString& exportFilePath,
                                                       const QString& profileName,
                                                       const ShortcutsConfig* shortcuts,
                                                       const CommandsConfig* commands,
                                                       const NavigationLayoutConfig* navigation,
                                                       const ContextMenusConfig* contextMenus) const {
    QJsonObject root;
    root["lc_file_format"]    = m_fileFormatIdentifier;
    root["lc_file_format_version"] = "1.0";
    root["profile_name"]    = profileName;

    if (shortcuts != nullptr && m_shortcutsRepo != nullptr) {
        QJsonObject obj = m_shortcutsRepo->configToJson(*shortcuts);
        obj["name"] = shortcuts->name;
        root["inlined_shortcuts"] = obj;
    }
    if (commands != nullptr && m_commandsRepo != nullptr) {
        QJsonObject obj = m_commandsRepo->configToJson(*commands);
        obj["name"] = commands->name;
        root["inlined_commands"] = obj;
    }
    if (navigation != nullptr && m_navRepo != nullptr) {
        QJsonObject obj = m_navRepo->configToJson(*navigation);
        obj["name"] = navigation->name;
        root["inlined_navigation"] = obj;
    }
    if (contextMenus != nullptr && m_menusRepo != nullptr) {
        QJsonObject obj = m_menusRepo->configToJson(*contextMenus);
        obj["name"] = contextMenus->name;
        root["inlined_context_menus"] = obj;
    }

    return writeJsonFile(exportFilePath, root);
}

bool LC_CustomizationImportExportHelper::inspectProfile(const QString& importFilePath,
                                                        QString& outProfileName,
                                                        QList<LC_ChecklistChoice>& outChoices) const {
    QJsonObject root;
    if (!readJsonFile(importFilePath, root)) {
        return false;
    }

    outProfileName = root.value("profile_name").toString();
    outChoices.clear();

    if (root.contains("inlined_shortcuts")) {
        outChoices.append({"shortcuts", QCoreApplication::translate("LC_CustomizationProfile", "Keyboard Shortcuts"),
                           QCoreApplication::translate("LC_CustomizationProfile", "Keymap scheme bindings"), true, true});
    }
    if (root.contains("inlined_commands")) {
        outChoices.append({"commands", QCoreApplication::translate("LC_CustomizationProfile", "Command Aliases"),
                           QCoreApplication::translate("LC_CustomizationProfile", "Command-line aliases and keycodes"), true, true});
    }
    if (root.contains("inlined_navigation")) {
        outChoices.append({"navigation", QCoreApplication::translate("LC_CustomizationProfile", "Menus & Toolbars"),
                           QCoreApplication::translate("LC_CustomizationProfile", "Main menu bar and toolbar arrangements"), true, true});
    }
    if (root.contains("inlined_context_menus")) {
        outChoices.append({"context_menus", QCoreApplication::translate("LC_CustomizationProfile", "Context Menus"),
                           QCoreApplication::translate("LC_CustomizationProfile", "Drawing area popup context menus"), true, true});
    }

    return !outChoices.isEmpty();
}

bool LC_CustomizationImportExportHelper::importProfile(const QString& importFilePath,
                                                       const QList<LC_ChecklistChoice>& selectedChoices,
                                                       QString& outProfileName,
                                                       QString& outShortcutsKey,
                                                       QString& outCommandsKey,
                                                       QString& outNavigationKey,
                                                       QString& outMenusKey) const {
    QJsonObject root;
    if (!readJsonFile(importFilePath, root)) {
        return false;
    }

    outProfileName = root.value("profile_name").toString();

    auto isSelected = [&selectedChoices](const QString& id) -> bool {
        for (const auto& c : selectedChoices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    // 1. Shortcuts
    if (isSelected("shortcuts") && m_shortcutsRepo != nullptr && root.contains("inlined_shortcuts")) {
        ShortcutsConfig config;
        const QJsonObject obj = root.value("inlined_shortcuts").toObject();
        if (m_shortcutsRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_shortcutsRepo->save(config.name, config, outKey)) {
                outShortcutsKey = outKey;
            }
        }
    }

    // 2. Commands
    if (isSelected("commands") && m_commandsRepo != nullptr && root.contains("inlined_commands")) {
        CommandsConfig config;
        const QJsonObject obj = root.value("inlined_commands").toObject();
        if (m_commandsRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_commandsRepo->save(config.name, config, outKey)) {
                outCommandsKey = outKey;
            }
        }
    }

    // 3. Navigation
    if (isSelected("navigation") && m_navRepo != nullptr && root.contains("inlined_navigation")) {
        NavigationLayoutConfig config;
        const QJsonObject obj = root.value("inlined_navigation").toObject();
        if (m_navRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_navRepo->save(config.name, config, outKey)) {
                outNavigationKey = outKey;
            }
        }
    }

    // 4. Context Menus
    if (isSelected("context_menus") && m_menusRepo != nullptr && root.contains("inlined_context_menus")) {
        ContextMenusConfig config;
        const QJsonObject obj = root.value("inlined_context_menus").toObject();
        if (m_menusRepo->configFromJson(obj, config)) {
            QString outKey;
            if (m_menusRepo->save(config.name, config, outKey)) {
                outMenusKey = outKey;
            }
        }
    }

    return true;
}
