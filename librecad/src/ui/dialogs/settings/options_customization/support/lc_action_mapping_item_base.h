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

#ifndef LC_ACTION_MAPPING_ITEM_BASE_H
#define LC_ACTION_MAPPING_ITEM_BASE_H

#include <QIcon>
#include <QString>
#include "lc_abstract_tree_item.h"

class LC_ActionMappingItemBase : public LC_AbstractTreeItem {
public:
    LC_ActionMappingItemBase(LC_AbstractTreeItem* parent,
                             const QString& name,
                             const QIcon& icon,
                             bool isGroup = false)
        : LC_AbstractTreeItem(parent)
        , m_name(name)
        , m_icon(icon)
        , m_isGroup(isGroup) {
    }

    ~LC_ActionMappingItemBase() override = default;

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon& icon) { m_icon = icon; }

    bool isGroup() const { return m_isGroup; }
    void setGroup(bool group) { m_isGroup = group; }

    bool isModified() const { return m_isModified; }
    void setModified(bool modified) { m_isModified = modified; }

    bool hasCollision() const { return m_hasCollision; }
    void setCollision(bool collision) { m_hasCollision = collision; }

    bool isMatched() const { return m_isMatched; }
    void setMatched(bool matched) { m_isMatched = matched; }

protected:
    QString m_name;
    QIcon m_icon;
    bool m_isGroup{false};
    bool m_isModified{false};
    bool m_hasCollision{false};
    bool m_isMatched{false};
};

#endif
