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

#ifndef LC_SHORTCUTS_TREE_MODEL_H
#define LC_SHORTCUTS_TREE_MODEL_H

#include <QKeySequence>
#include <QMap>

#include "lc_action_mapping_tree_model_base.h"
#include "lc_shortcut_info.h"

class LC_ActionGroupManager;
class LC_ShortcutTreeItem;

class LC_ShortcutsTreeModel : public LC_ActionMappingTreeModelBase {
    Q_OBJECT
public:
    enum Column {
        ICON = 0,
        NAME = 0,
        SHORTCUT = 1,
        LAST
    };

    explicit LC_ShortcutsTreeModel(QObject* parent = nullptr,
                                   QColor filteredItemColor = QColor(189, 99, 19),
                                   QColor conflictItemColor = QColor(218, 54, 51));
    ~LC_ShortcutsTreeModel() override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    LC_ShortcutTreeItem* getItemForIndex(const QModelIndex& index) const;

    void rebuildModel(LC_ActionGroupManager* pManager);
    void resetAllToDefault();
    bool checkForCollisions(LC_ShortcutInfo* shortcutInfo = nullptr, QString* outFirstConflict = nullptr);
    void applyShortcuts(const QMap<QString, QKeySequence>& map, bool replace);
    bool isModified() const;
    void commitBaseline();

    void setFilteringShortcut(const QKeySequence& sequence);
    void clearFilteringShortcut();
    bool hasShortcutFilter() const;

    const QMap<QString, LC_ShortcutInfo*>& getShortcuts() const;

private:
    QMap<QString, LC_ShortcutInfo*> m_shortcuts;
    QKeySequence m_filteringShortcut;
    bool m_hasShortcutFilter{false};
};

#endif
