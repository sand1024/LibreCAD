/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_SHORTCUT_TREE_ITEM_H
#define LC_SHORTCUT_TREE_ITEM_H

#include "lc_action_mapping_item_base.h"

class QAction;
class LC_ShortcutInfo;

class LC_ShortcutTreeItem : public LC_ActionMappingItemBase {
public:
    LC_ShortcutTreeItem(LC_ShortcutTreeItem* parent, const QAction* action, LC_ShortcutInfo* shortcutInfo);
    LC_ShortcutTreeItem(LC_ShortcutTreeItem* parent, const QIcon& ic, const QString& name, const QString& desc);
    ~LC_ShortcutTreeItem() override = default;

    QString getName() const { return name(); }
    QIcon getIcon() const { return icon(); }
    QString getDescription() const { return m_description; }

    void clearShortcut() const;
    void resetShortcutToDefault() const;
    LC_ShortcutInfo* getShortcutInfo() const;
    QString getShortcutViewString() const;

    LC_ShortcutTreeItem* addChild(const QAction* action, LC_ShortcutInfo* shortcutInfo);

    LC_ShortcutTreeItem* child(int row) const {
        return static_cast<LC_ShortcutTreeItem*>(LC_AbstractTreeItem::child(row));
    }
    LC_ShortcutTreeItem* parent() const {
        return static_cast<LC_ShortcutTreeItem*>(LC_AbstractTreeItem::parent());
    }

    QString identifier() const;

private:
    QString m_description;
    LC_ShortcutInfo* m_shortcutInfo{nullptr};
};

#endif
