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

#include "lc_shortcuts_manager.h"

#include <QApplication>
#include <QFile>

#include "lc_action_tooltip_builder.h"
#include "lc_repository_shortcuts.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_state.h"
#include "lc_settings_paths.h"
#include "lc_shortcuts_storage.h"
#include "rs_debug.h"

LC_ShortcutsManager::LC_ShortcutsManager() = default;

int LC_ShortcutsManager::saveShortcuts(
    QMap<QString, LC_ShortcutInfo *> &shortcuts, QMap<QString, QAction *> &actionsMap) const {

    applyShortcutsMapToActionsMap(shortcuts, actionsMap);
    updateActionTooltips(actionsMap);

    const QString defaultShortcutsFileName = getDefaultShortcutsFileName();

    const int saveResult = LC_ShortcutsStorage::saveShortcuts(defaultShortcutsFileName, shortcuts.values(), false);
    return saveResult;
}

int LC_ShortcutsManager::loadShortcuts(QMap<QString, QAction *> &actionsMap) const {
    const QString defaultFileName = getDefaultShortcutsFileName();
    auto shortcuts = QMap<QString, QKeySequence>();
    const int loadResult = loadShortcuts(defaultFileName, &shortcuts);
    if (loadResult == LC_ShortcutsStorage::OK){
        applyKeySequencesMapToActionsMap(shortcuts, actionsMap);
    }
    updateActionTooltips(actionsMap);
    return loadResult;
}

int LC_ShortcutsManager::loadActiveScheme(QMap<QString, QAction*>& actionsMap) {
    init();

    const QString activeScheme = CFG_AppState::o_ActiveShortcutsScheme;
    if (activeScheme.isEmpty() || activeScheme == DEFAULT_THEME_KEY) {
        // Fallback to native hardcoded QAction defaults
        updateActionTooltips(actionsMap);
        return LC_ShortcutsStorage::OK;
    }

    if (m_repository != nullptr) {
        ShortcutsConfig config;
        if (m_repository->loadByKey(activeScheme, config)) {
            applyKeySequencesMapToActionsMap(config.shortcuts, actionsMap);
            updateActionTooltips(actionsMap);
            return LC_ShortcutsStorage::OK;
        }
    }

    updateActionTooltips(actionsMap);
    return LC_ShortcutsStorage::OK;
}

int LC_ShortcutsManager::saveShortcuts(const QString &fileName, const QList<LC_ShortcutInfo *> &shortcutsList) const {
    const int result = LC_ShortcutsStorage::saveShortcuts(fileName, shortcutsList);
    return result;
}

int LC_ShortcutsManager::loadShortcuts(const QString &filename, QMap<QString, QKeySequence> *result) const{
   return LC_ShortcutsStorage::loadShortcuts(filename, result);
}

void LC_ShortcutsManager::updateActionTooltips(const QMap<QString, QAction *> &actionsMap) const {
    LC_ActionTooltipBuilder::updateAllTooltips(actionsMap);
}

void LC_ShortcutsManager::init() const {
    const QString baseFolder = getShortcutsMappingsFolder();
    auto* self = const_cast<LC_ShortcutsManager*>(this);
    if (self->m_repository == nullptr) {
        self->m_repository = std::make_unique<LC_RepositoryShortcuts>(baseFolder + "/shortcuts");
    }
    self->m_repository->migrateLegacyShortcutsIfNeeded(baseFolder);
}


void LC_ShortcutsManager::applyShortcutsMapToActionsMap(QMap<QString, LC_ShortcutInfo*> &shortcuts, QMap<QString, QAction *> &actionsMap) const{
    for (auto [key, shortcut] : shortcuts.asKeyValueRange()){
        QAction* action = actionsMap[key];
        if (action != nullptr){
            auto keySequence = shortcut->getKey();
            action->setShortcut(keySequence);
        }
        // else {
            // LC_ERR << "Action is not found for applyShortcutsMapToActionsMap(). Key:" << key;
        // }
    }
}

LC_RepositoryShortcuts* LC_ShortcutsManager::getRepository() const {
    return m_repository.get();
}

void LC_ShortcutsManager::applyKeySequencesMapToActionsMap(QMap<QString, QKeySequence> &shortcuts, QMap<QString, QAction *> &actionsMap) const{
    for (auto [name, shortcut] : shortcuts.asKeyValueRange()){
        QAction* action = actionsMap[name];
        if (action != nullptr){
            action->setShortcut(shortcut);
            action->setShortcutContext(Qt::ApplicationShortcut);
        }
    }
}

void LC_ShortcutsManager::assignShortcutsToActions(const QMap<QString, QAction *> &map, const std::vector<LC_ShortcutInfo> &shortcutsList) const {
    for (const LC_ShortcutInfo &a: shortcutsList){
        QAction* createdAction = map[a.getName()];
        if (createdAction != nullptr){
            const QList<QKeySequence> &list = a.getKeysList();
            if (list.isEmpty()) {
                const QKeySequence &sequence = a.getKey();
                if (sequence != QKeySequence::UnknownKey) {
                    createdAction->setShortcut(sequence);
                }
            } else { // todo - support for future
                createdAction->setShortcuts(list);
            }
        }
    }
}

QString LC_ShortcutsManager::getPlainActionToolTip(const QAction* action){
    if (action != nullptr) {
        if (!action->shortcut().isEmpty()) {
            QString tooltip = action->property(PROPERTY_SHORTCUT_BACKUP).toString();
            if (tooltip.isEmpty()) {
                tooltip = action->toolTip();
            }
            return tooltip;
        }
        return action->toolTip();
    }
    return "";
}


/* guesses a descriptive text from a text suited for a menu entry
   This is equivalent to QActions internal qt_strippedText()
*/
QString LC_ShortcutsManager::strippedActionText(QString s) const{
    s.remove( QString::fromLatin1("..."));
    for (int i = 0; i < s.size(); ++i) {
        if (s.at(i) == QLatin1Char('&')) {
            s.remove(i, 1);
        }
    }
    return s.trimmed();
}

QString LC_ShortcutsManager::getShortcutsMappingsFolder() const {
    QString settingsDir = CFG_Paths::o_OtherSettingsDir;
    return settingsDir;
}

QString LC_ShortcutsManager::getDefaultShortcutsFileName() const {
    const QString path =  getShortcutsMappingsFolder() + "/shortcuts.lcsc";
    return QDir::toNativeSeparators(path);
}
