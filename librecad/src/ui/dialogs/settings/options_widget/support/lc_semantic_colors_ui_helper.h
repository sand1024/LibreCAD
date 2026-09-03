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

#ifndef LC_SEMANTIC_COLORS_UI_HELPER_H
#define LC_SEMANTIC_COLORS_UI_HELPER_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QString>
#include <functional>
#include "lc_settings_colors_semantics.h"

class QTableWidget;

class LC_SemanticColorsUiHelper : public QObject {
    Q_OBJECT
public:
    static QString getRoleDisplayName(LC_SemanticColors role);
    static QString getRoleTooltip(LC_SemanticColors role);

    static void setupTable(QTableWidget* table);
    static void populateTable(QTableWidget* table, const QMap<QString, QColor>& colors, bool isDark, bool isReadOnly = false);
    static void extractColors(const QTableWidget* table, QMap<QString, QColor>& outColors);
    static void connectChanged(QTableWidget* table, QObject* receiver, const std::function<void()>& callback);
};

#endif // LC_SEMANTIC_COLORS_UI_HELPER_H
