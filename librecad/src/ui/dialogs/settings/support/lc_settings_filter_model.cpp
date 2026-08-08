/*******************************************************************************
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

#include "lc_settings_filter_model.h"

LC_SettingsFilterModel::LC_SettingsFilterModel(QObject* parent, const QMap<QString, QStringList>& index)
    : QSortFilterProxyModel(parent), m_searchIndex(index) {}

void LC_SettingsFilterModel::setFilterText(const QString& text) {
    m_filterText = text.trimmed();
    invalidateFilter();
}

bool LC_SettingsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString pageId = idx.data(Qt::UserRole).toString();

    if (m_filterText.isEmpty()) {
        return true;
    }

    const QString name = idx.data(Qt::DisplayRole).toString();
    if (name.contains(m_filterText, Qt::CaseInsensitive)) return true;

    if (m_searchIndex.contains(pageId)) {
        for (const QString& kw : m_searchIndex[pageId]) {
            if (kw.contains(m_filterText, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }

    const int childCount = sourceModel()->rowCount(idx);
    for (int r = 0; r < childCount; ++r) {
        if (filterAcceptsRow(r, idx)) {
            return true;
        }
    }

    return false;
}
