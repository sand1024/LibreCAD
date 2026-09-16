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

#ifndef LC_ABSTRACT_TREE_ITEM_H
#define LC_ABSTRACT_TREE_ITEM_H

#include <QList>

class LC_AbstractTreeItem {
public:
    explicit LC_AbstractTreeItem(LC_AbstractTreeItem* parent = nullptr)
        : m_parent(parent) {
    }

    virtual ~LC_AbstractTreeItem() {
        qDeleteAll(m_children);
    }

    void appendChild(LC_AbstractTreeItem* child) {
        if (child != nullptr) {
            m_children.append(child);
        }
    }

    LC_AbstractTreeItem* child(int row) const {
        if (row >= 0 && row < m_children.size()) {
            return m_children.at(row);
        }
        return nullptr;
    }

    int childCount() const {
        return m_children.size();
    }

    LC_AbstractTreeItem* parent() const {
        return m_parent;
    }

    int row() const {
        if (m_parent != nullptr) {
            return m_parent->m_children.indexOf(const_cast<LC_AbstractTreeItem*>(this));
        }
        return 0;
    }

    const QList<LC_AbstractTreeItem*>& children() const {
        return m_children;
    }

protected:
    LC_AbstractTreeItem* m_parent{nullptr};
    QList<LC_AbstractTreeItem*> m_children;
};

#endif
