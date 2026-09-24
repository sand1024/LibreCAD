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

#ifndef LC_SHORTCUTS_MANAGER_H
#define LC_SHORTCUTS_MANAGER_H

#include "lc_shortcut_info.h"
#include "lc_repository_shortcuts.h"

class LC_ShortcutsManager {
public:
    LC_ShortcutsManager(LC_RepositoryKeymaps* repository);
    ~LC_ShortcutsManager() = default;
    static void assignShortcutsToActions(const QMap<QString, QAction *> &map, const std::vector<LC_ShortcutInfo> &shortcutsList);
    void updateActionTooltips(const QMap<QString, QAction *> &actionsMap) const;
    void applyShortcutsMapToActionsMap(QMap<QString, LC_ShortcutInfo *> &shortcuts, QMap<QString, QAction *> &actionsMap) const;  // fixme - temporary, review
    int loadActiveScheme(QMap<QString, QAction*>& actionsMap);
    LC_RepositoryKeymaps* getRepository() const;
protected:
    LC_RepositoryKeymaps* m_repository {nullptr};
    [[deprecated]]
    QString strippedActionText(QString s) const;
    void applyKeySequencesMapToActionsMap(QMap<QString, QKeySequence> &shortcuts, QMap<QString, QAction *> &actionsMap) const;
};
#endif
