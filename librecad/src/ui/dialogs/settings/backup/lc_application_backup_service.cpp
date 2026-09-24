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

#include "lc_application_backup_service.h"

#include <QDateTime>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>

#include "lc_action_group_manager.h"
#include "lc_command_manager.h"
#include "lc_graphic_view_context_menu_provider.h"
#include "lc_navigation_creator.h"
#include "lc_repository_commands.h"
#include "lc_repository_graphic_view_context_menus.h"
#include "lc_repository_menu_bar_and_toolbars.h"
#include "lc_repository_shortcuts.h"
#include "lc_repository_viewport_theme.h"
#include "lc_settings_app_state.h"
#include "lc_shortcuts_manager.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"

namespace {
namespace {
    const QStringList PORTABLE_SETTINGS_GROUPS = {
        "Grid", "Snap", "VisualSnap", "Render", "Colors",
        "Appearance", "Defaults", "CADPreferences", "WindowOptions", "CommandsPromotion"
    };

    const QString G_STRING_LIST_START = "SL$";
    const QString G_STRING_LIST_SEP   = "|";

    template <typename RepoType, typename ConfigType>
    void exportRepoHelper(RepoType* repo, const QString& activeKey, const QString& inlinedKey, QJsonObject& targetObj, bool isFullScope) {
        if (repo == nullptr) {
            return;
        }

        if (isFullScope) {
            QJsonArray arr;
            for (const auto& pair : repo->getPresetChoices()) {
                ConfigType cfg;
                if (repo->loadByKey(pair.second, cfg)) {
                    QJsonObject o = repo->configToJson(cfg);
                    o["name"] = cfg.name;
                    arr.append(o);
                }
            }
            targetObj.insert(inlinedKey, arr);
        }
        else if (!activeKey.isEmpty() && !repo->isDefaultKey(activeKey)) {
            ConfigType cfg;
            if (repo->loadByKey(activeKey, cfg)) {
                QJsonObject o = repo->configToJson(cfg);
                o["name"] = cfg.name;
                targetObj.insert(inlinedKey, o);
            }
        }
    }

    template <typename RepoType, typename ConfigType>
    void importRepoHelper(RepoType* repo, const QJsonValue& val, const std::function<QString(const QString&, const QStringList&)>& nameResolver) {
        if (repo == nullptr) {
            return;
        }

        auto importSingleObj = [repo, &nameResolver](const QJsonObject& obj) {
            ConfigType cfg;
            if (repo->configFromJson(obj, cfg)) {
                const QString uniqueName = nameResolver(cfg.name, repo->getAvailableNames());
                cfg.name = uniqueName;
                QString outKey;
                repo->save(cfg.name, cfg, outKey);
            }
        };

        if (val.isArray()) {
            for (const auto& item : val.toArray()) {
                importSingleObj(item.toObject());
            }
        }
        else if (val.isObject()) {
            importSingleObj(val.toObject());
        }
    }
}
}

LC_ApplicationBackupService::LC_ApplicationBackupService(QC_ApplicationWindow* appWin)
    : LC_ProfileExchangeHelperBase(APPLICATION_BACKUP_FILE_IDENTIFIER)
    , m_appWin(appWin) {
}

QString LC_ApplicationBackupService::makeUniquePresetName(const QString& baseName, const QStringList& existingNames) {
    if (!existingNames.contains(baseName, Qt::CaseInsensitive)) {
        return baseName;
    }
    const QString candidate = baseName + "_imported";
    if (!existingNames.contains(candidate, Qt::CaseInsensitive)) {
        return candidate;
    }
    int counter = 1;
    while (existingNames.contains(candidate + "_" + QString::number(counter), Qt::CaseInsensitive)) {
        counter++;
    }
    return candidate + "_" + QString::number(counter);
}

QJsonObject LC_ApplicationBackupService::exportPortableSettings() {
    QJsonObject groupsObj;
    QSettings* settings = RS_SETTINGS->getSettings();
    if (settings == nullptr) {
        return groupsObj;
    }

    for (const QString& group : PORTABLE_SETTINGS_GROUPS) {
        settings->beginGroup(group);
        QJsonObject valuesObj;
        for (const QString& key : settings->childKeys()) {
            const QVariant val = settings->value(key);
            if (val.userType() == QMetaType::QStringList) {
                QString encoded = G_STRING_LIST_START;
                for (const auto& item : val.toStringList()) {
                    encoded += G_STRING_LIST_SEP + item;
                }
                valuesObj.insert(key, encoded);
            }
            else {
                valuesObj.insert(key, val.toString());
            }
        }
        settings->endGroup();
        if (!valuesObj.isEmpty()) {
            groupsObj.insert(group, valuesObj);
        }
    }
    return groupsObj;
}

void LC_ApplicationBackupService::importPortableSettings(const QJsonObject& groupsObj) {
    for (const QString& group : groupsObj.keys()) {
        if (!PORTABLE_SETTINGS_GROUPS.contains(group)) {
            continue;
        }

        const QJsonObject valuesObj = groupsObj.value(group).toObject();
        LC_GROUP(group);
        {
            for (const QString& key : valuesObj.keys()) {
                const QString strVal = valuesObj.value(key).toString();
                if (strVal.startsWith(G_STRING_LIST_START)) {
                    QStringList parts = strVal.split(G_STRING_LIST_SEP, Qt::KeepEmptyParts);
                    QStringList items;
                    for (int i = 1; i < parts.size(); ++i) {
                        items.append(parts.at(i));
                    }
                    LC_SET(key, items);
                }
                else {
                    LC_SET(key, strVal);
                }
            }
        }
        LC_GROUP_END();
    }
}

bool LC_ApplicationBackupService::exportBackup(QWidget* parent) {
    if (m_appWin == nullptr) {
        return false;
    }

    QList<LC_ChecklistChoice> choices = {
        {"customization", tr("User Customization"),     tr("Shortcuts, aliases, toolbars, and context menus"), true, true},
        {"viewport",      tr("Drawing Area Themes"),    tr("Visual styles and viewport color schemes"),        true, true},
        {"styling",       tr("Application Styling"),    tr("Palettes, widget skins, icons, and typography"),    true, true},
        {"preferences",   tr("Application Preferences"),tr("Portable snapping, grid, and rendering settings"),  true, true}
    };

    QString archiveName;
    bool isFullScope = true;
    const bool accepted = LC_DlgChecklistSelection::selectChoicesWithInputAndScope(
        parent,
        tr("Export Configuration"),
        tr("Configure options and select components to package:"),
        tr("Archive Name:"),
        tr("LibreCAD Configuration %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")),
        tr("Package Scope:"),
        tr("Full Library (All custom saved presets across repositories)"),
        tr("Active Setup Only (Only currently active configurations)"),
        isFullScope,
        archiveName,
        choices);

    if (!accepted || archiveName.trimmed().isEmpty()) {
        return false;
    }

    archiveName = archiveName.trimmed();
    QString safeFilename = archiveName.toLower();
    safeFilename.replace(" ", "_");
    safeFilename.remove(QRegularExpression(R"([^a-z0-9_\-\.])"));
    safeFilename += ".lccfg";

    const QString filePath = QFileDialog::getSaveFileName(
        parent, tr("Export Configuration"), safeFilename, tr("LibreCAD Configuration (*.lccfg);;All Files (*.*)"));
    if (filePath.isEmpty()) {
        return false;
    }

    auto isChecked = [&choices](const QString& id) -> bool {
        for (const auto& c : choices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    QJsonObject root;
    root["lc_file_format"]     = m_fileFormatIdentifier;
    root["lc_file_format_version"]  = "1.0";
    root["archive_name"]     = archiveName;
    root["created"]          = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["scope_full"]       = isFullScope;

    QJsonObject activePointersObj;
    activePointersObj["shortcuts"]        = CFG_AppState::o_ActiveShortcutsScheme.get();
    activePointersObj["commands"]         = CFG_AppState::o_ActiveCommandsScheme.get();
    activePointersObj["navigation"]       = CFG_AppState::o_ActiveNavigationLayoutScheme.get();
    activePointersObj["context_menus"]    = CFG_AppState::o_ActiveContextMenusScheme.get();
    root["active_pointers"] = activePointersObj;

    // 1. Customization Repositories
    if (isChecked("customization")) {
        QJsonObject custObj;
        auto shortcutsManager = m_appWin->getShortcutsManager();
        auto* shortcutsRepo = shortcutsManager->getRepository();

        auto* cmdMgr = m_appWin->getCommandManager();
        auto* cmdRepo = (cmdMgr != nullptr) ? cmdMgr->getRepository() : nullptr;

        auto* navCreator = m_appWin->getCreatorInvoker();
        auto* navRepo = (navCreator != nullptr) ? navCreator->getRepository() : nullptr;

        auto* ctxProvider = m_appWin->getContextMenuProvider();
        auto* menusRepo = (ctxProvider != nullptr) ? ctxProvider->getMenusRepository() : nullptr;

        exportRepoHelper<LC_RepositoryKeymaps, ShortcutsConfig>(
            shortcutsRepo, CFG_AppState::o_ActiveShortcutsScheme, "shortcuts", custObj, isFullScope);

        exportRepoHelper<LC_RepositoryCommands, CommandsConfig>(
            cmdRepo, CFG_AppState::o_ActiveCommandsScheme, "commands", custObj, isFullScope);

        exportRepoHelper<LC_RepositoryMenuBarAndToolbars, NavigationLayoutConfig>(
            navRepo, CFG_AppState::o_ActiveNavigationLayoutScheme, "navigation", custObj, isFullScope);

        exportRepoHelper<LC_RepositoryGraphicViewContextMenus, ContextMenusConfig>(
            menusRepo, CFG_AppState::o_ActiveContextMenusScheme, "context_menus", custObj, isFullScope);

        root["customization"] = custObj;
    }
    // 2. Viewport Themes
    if (isChecked("viewport")) {

        LC_RepositoryViewportTheme* vpRepo = m_appWin->getUiStyleManager()->getViewportStylingRepository();
        QJsonArray vpArray;
        for (const auto& pair : vpRepo->getPresetChoices()) {
            LC_ViewportThemeConfig cfg;
            if (vpRepo->loadByKey(pair.second, cfg)) {
                QJsonObject o = vpRepo->configToJson(cfg);
                o["name"] = cfg.name;
                vpArray.append(o);
            }
        }
        root["viewport_themes"] = vpArray;
    }

    // 3. Portable Preferences
    if (isChecked("preferences")) {
        root["portable_preferences"] = exportPortableSettings();
    }

    bool compressedFile = false; // fixme - sand - decide later whether it will be binary or not.
    bool success = compressedFile ? writeJsonFileCompressed(filePath, root, 5) : writeJsonFile(filePath, root);
    if (success) {
        QMessageBox::information(parent, tr("Congifuration Exported"),
                                 tr("Application configuration '%1' has been successfully created.").arg(archiveName));
    }
    else {
        QMessageBox::critical(parent, tr("Export Error"), tr("Could not write backup file to disk."));
    }
    return success;
}

bool LC_ApplicationBackupService::importBackup(QWidget* parent) {
    if (m_appWin == nullptr) {
        return false;
    }

    const QString filePath = QFileDialog::getOpenFileName(
        parent, tr("Import Configuration"), QString(), tr("LibreCAD Configuration (*.lccfg);;All Files (*.*)"));
    if (filePath.isEmpty()) {
        return false;
    }

    QJsonObject root;
    if (!readJsonFile(filePath, root)) {
        QMessageBox::critical(parent, tr("Import Error"), tr("Invalid or corrupted backup archive."));
        return false;
    }

    const QString archiveName = root.value("archive_name").toString(tr("Unnamed Archive"));
    QList<LC_ChecklistChoice> availableChoices;

    if (root.contains("customization")) {
        availableChoices.append({"customization", tr("User Customization"),
                                 tr("Restore keymaps, aliases, toolbars, and context menus"), true, true});
    }
    if (root.contains("viewport_themes")) {
        availableChoices.append({"viewport", tr("Drawing Area Themes"),
                                 tr("Restore visual themes and viewport schemes"), true, true});
    }
    if (root.contains("portable_preferences")) {
        availableChoices.append({"preferences", tr("Application Preferences"),
                                 tr("Restore general snapping, grid, and rendering settings"), true, true});
    }

    const QString promptMsg = tr("Backup archive '%1' contains the following categories. Select which to restore:").arg(archiveName);
    if (!LC_DlgChecklistSelection::selectChoices(parent, tr("Restore Application Backup"), promptMsg, availableChoices)) {
        return false;
    }

    auto isSelected = [&availableChoices](const QString& id) -> bool {
        for (const auto& c : availableChoices) {
            if (c.id == id) {
                return c.checked;
            }
        }
        return false;
    };

    // 1. Restore Customization Repositories with Auto-Renaming
    if (isSelected("customization") && root.contains("customization")) {
        const QJsonObject custObj = root.value("customization").toObject();

        auto shortcutsManager = m_appWin->getShortcutsManager();
        auto* shortcutsRepo = shortcutsManager->getRepository();

        auto* cmdMgr = m_appWin->getCommandManager();
        auto* cmdRepo = (cmdMgr != nullptr) ? cmdMgr->getRepository() : nullptr;

        auto* navCreator = m_appWin->getCreatorInvoker();
        auto* navRepo = (navCreator != nullptr) ? navCreator->getRepository() : nullptr;

        auto* ctxProvider = m_appWin->getContextMenuProvider();
        auto* menusRepo = (ctxProvider != nullptr) ? ctxProvider->getMenusRepository() : nullptr;

        auto nameResolver = [](const QString& baseName, const QStringList& existingNames) {
            return makeUniquePresetName(baseName, existingNames);
        };

        importRepoHelper<LC_RepositoryKeymaps, ShortcutsConfig>(
            shortcutsRepo, custObj.value("shortcuts"), nameResolver);

        importRepoHelper<LC_RepositoryCommands, CommandsConfig>(
            cmdRepo, custObj.value("commands"), nameResolver);

        importRepoHelper<LC_RepositoryMenuBarAndToolbars, NavigationLayoutConfig>(
            navRepo, custObj.value("navigation"), nameResolver);

        importRepoHelper<LC_RepositoryGraphicViewContextMenus, ContextMenusConfig>(
            menusRepo, custObj.value("context_menus"), nameResolver);
    }

    // 2. Restore Viewport Themes with Auto-Renaming
    if (isSelected("viewport") && root.contains("viewport_themes")) {
        LC_RepositoryViewportTheme* vpRepo = m_appWin->getUiStyleManager()->getViewportStylingRepository();
        const QJsonArray vpArr = root.value("viewport_themes").toArray();
        for (const auto& item : vpArr) {
            const QJsonObject obj = item.toObject();
            LC_ViewportThemeConfig cfg;
            if (vpRepo->configFromJson(obj, cfg)) {
                cfg.name = makeUniquePresetName(cfg.name, vpRepo->getAvailableNames());
                QString outKey;
                vpRepo->save(cfg.name, cfg, outKey);
            }
        }
    }

    // 3. Restore Portable Preferences
    if (isSelected("preferences") && root.contains("portable_preferences")) {
        importPortableSettings(root.value("portable_preferences").toObject());
        RS_SETTINGS->emitOptionsChanged();
    }

    QMessageBox::information(parent, tr("Configuration Restored"),
                             tr("Configuration archive '%1' has been successfully restored.").arg(archiveName));
    return true;
}
