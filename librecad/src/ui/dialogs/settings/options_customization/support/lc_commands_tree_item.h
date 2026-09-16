
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

#ifndef LC_COMMANDS_TREE_ITEM_H
#define LC_COMMANDS_TREE_ITEM_H

#include "lc_action_mapping_item_base.h"

class LC_CommandsTreeItem : public LC_ActionMappingItemBase {
public:
    LC_CommandsTreeItem(LC_CommandsTreeItem* parent, const QString& title, const QIcon& icon);
    LC_CommandsTreeItem(LC_CommandsTreeItem* parent, const QString& name, const QIcon& icon,
                        const QString& actionOrKeyName, bool isKeyword);
    ~LC_CommandsTreeItem() override = default;

    QString identifier() const { return m_identifier; }
    bool isKeyword() const { return m_isKeyword; }

    QString getMainCommand() const { return m_mainCommand; }
    QString getKeyCode() const { return m_keycode; }
    QString getAlias() const { return m_alias; }

    void setMainCommand(const QString& cmd) { m_mainCommand = cmd; }
    void setKeyCode(const QString& a1) { m_keycode = a1; }
    void setAlias(const QString& a2) { m_alias = a2; }

    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    LC_CommandsTreeItem* child(int row) const {
        return static_cast<LC_CommandsTreeItem*>(LC_AbstractTreeItem::child(row));
    }
    LC_CommandsTreeItem* parent() const {
        return static_cast<LC_CommandsTreeItem*>(LC_AbstractTreeItem::parent());
    }

    const QStringList& effectiveTriggers() const { return m_effectiveTriggers; }
    void setEffectiveTriggers(const QStringList& triggers) { m_effectiveTriggers = triggers; }

private:
    QString m_identifier;
    QString m_mainCommand;
    QString m_keycode;
    QString m_alias;
    QStringList m_effectiveTriggers;
    bool m_isKeyword{false};
    QString m_description;
};

#endif
