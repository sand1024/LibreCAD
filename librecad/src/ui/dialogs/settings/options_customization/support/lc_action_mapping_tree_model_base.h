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

#ifndef LC_ACTION_MAPPING_TREE_MODEL_BASE_H
#define LC_ACTION_MAPPING_TREE_MODEL_BASE_H

#include <QAbstractItemModel>
#include <QColor>
#include <QRegularExpression>
#include <memory>

#include "lc_action_mapping_item_base.h"

class LC_ActionMappingTreeModelBase : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit LC_ActionMappingTreeModelBase(QObject* parent = nullptr,
                                           const QColor& filterColor = QColor(189, 99, 19),
                                           const QColor& conflictColor = QColor(218, 54, 51));
    ~LC_ActionMappingTreeModelBase() override = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    LC_ActionMappingItemBase* getItemForIndex(const QModelIndex& index) const;

    void setFilteringRegexp(const QString& pattern, bool highlightMode = false);
    void setFilterForConflicts(bool filterOnly);
    bool isFilterForConflicts() const { return m_filterForConflicts; }
    bool isFiltered() const { return m_hasFilter || m_filterForConflicts; }
protected:
    std::unique_ptr<LC_ActionMappingItemBase> m_rootItem;
    QRegularExpression m_filterRegex;
    bool m_hasFilter{false};
    bool m_highlightOnly{false};
    bool m_filterForConflicts{false};
    QColor m_filterColor;
    QColor m_conflictColor;
};

#endif
