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

#include "lc_action_mapping_tree_model_base.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

LC_ActionMappingTreeModelBase::LC_ActionMappingTreeModelBase(QObject* parent,
                                                             const QColor& filterColor,
                                                             const QColor& conflictColor)
    : QAbstractItemModel(parent)
    , m_filterColor(filterColor)
    , m_conflictColor(conflictColor) {
    m_filterRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
}

QModelIndex LC_ActionMappingTreeModelBase::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    const LC_ActionMappingItemBase* parentItem = parent.isValid()
        ? static_cast<const LC_ActionMappingItemBase*>(parent.internalPointer())
        : m_rootItem.get();

    if (parentItem == nullptr) {
        return QModelIndex();
    }

    auto* childItem = static_cast<LC_ActionMappingItemBase*>(parentItem->child(row));
    if (childItem != nullptr) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex LC_ActionMappingTreeModelBase::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    const auto* childItem = static_cast<const LC_ActionMappingItemBase*>(child.internalPointer());
    auto* parentItem = (childItem != nullptr) ? static_cast<LC_ActionMappingItemBase*>(childItem->parent()) : nullptr;

    if (parentItem == nullptr || parentItem == m_rootItem.get()) {
        return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int LC_ActionMappingTreeModelBase::rowCount(const QModelIndex& parent) const {
    const LC_ActionMappingItemBase* parentItem = parent.isValid()
        ? static_cast<const LC_ActionMappingItemBase*>(parent.internalPointer())
        : m_rootItem.get();

    return (parentItem != nullptr) ? parentItem->childCount() : 0;
}

LC_ActionMappingItemBase* LC_ActionMappingTreeModelBase::getItemForIndex(const QModelIndex& index) const {
    if (index.isValid()) {
        return static_cast<LC_ActionMappingItemBase*>(index.internalPointer());
    }
    return nullptr;
}

QVariant LC_ActionMappingTreeModelBase::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto* item = getItemForIndex(index);
    if (item == nullptr) {
        return QVariant();
    }

    if (role == Qt::BackgroundRole && item->isGroup()) {
        return QApplication::palette().color(QPalette::AlternateBase);
    }

    if (role == Qt::FontRole) {
        QFont font;
        if (item->isModified()) {
            font.setItalic(true);
        }
        if (item->hasCollision()) {
            font.setBold(true);
        }
        return font;
    }

    if (role == Qt::ForegroundRole) {
        if (item->hasCollision()) {
            return m_conflictColor;
        }
        if (item->isMatched()) {
            return m_filterColor;
        }
    }

    return QVariant();
}

void LC_ActionMappingTreeModelBase::setFilteringRegexp(const QString& pattern, bool highlightMode) {
    m_filterRegex.setPattern(pattern.trimmed());
    m_hasFilter = !pattern.trimmed().isEmpty();
    m_highlightOnly = highlightMode;
}

void LC_ActionMappingTreeModelBase::setFilterForConflicts(bool filterOnly) {
    m_filterForConflicts = filterOnly;
}
