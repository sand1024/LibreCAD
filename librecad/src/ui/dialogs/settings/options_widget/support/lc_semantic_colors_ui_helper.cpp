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

#include "lc_semantic_colors_ui_helper.h"
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "lc_color_button.h"

QString LC_SemanticColorsUiHelper::getRoleDisplayName(LC_SemanticColors role) {
    switch (role) {
        case LC_SemanticColors::FilteredItem:  return tr("Item with Filter Match");
        case LC_SemanticColors::ConflictingItem:   return tr("Item with Conflict (or Error)");
        case LC_SemanticColors::SearchResultItem:  return tr("Search Result Item");
    }
    return QString();
}

QString LC_SemanticColorsUiHelper::getRoleTooltip(LC_SemanticColors role) {
    switch (role) {
        case LC_SemanticColors::FilteredItem:
            return tr("Active filter match item's color (Default: Blue). Applicable where items are filtered.");
        case LC_SemanticColors::ConflictingItem:
            return tr("Item with conflicts - like shortcut key conflicts, duplicate entries, and validation errors (Default: Red).");
        case LC_SemanticColors::SearchResultItem:
            return tr("Search match text labels and filter result badges in dialogs (Default: Amber).");
    }
    return QString();
}

void LC_SemanticColorsUiHelper::setupTable(QTableWidget* table) {
    if (table == nullptr) {
        return;
    }

    table->setColumnCount(2);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Role")));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Color")));

    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(26);

    const int totalRoles = sizeof(SEMANTIC_COLOR_DEFS) / sizeof(SEMANTIC_COLOR_DEFS[0]);
    table->setRowCount(totalRoles);

    for (int row = 0; row < totalRoles; ++row) {
        const auto role = SEMANTIC_COLOR_DEFS[row].role;
        const QString displayName = getRoleDisplayName(role);
        const QString tooltipText  = getRoleTooltip(role);

        auto* itemKey = new QTableWidgetItem(displayName);
        itemKey->setData(Qt::UserRole, QLatin1String(SEMANTIC_COLOR_DEFS[row].key));
        itemKey->setToolTip(tooltipText);
        itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
        table->setItem(row, 0, itemKey);

        auto* btn = new LC_ColorButton(table);
        btn->setToolTip(tooltipText);
        btn->setDialogTitle(tr("Select Color for: %1").arg(displayName));
        table->setCellWidget(row, 1, btn);
    }
}

void LC_SemanticColorsUiHelper::populateTable(QTableWidget* table, const QMap<QString, QColor>& colors, bool isDark, bool isReadOnly) {
    if (table == nullptr) {
        return;
    }

    table->blockSignals(true);
    const int totalRoles = sizeof(SEMANTIC_COLOR_DEFS) / sizeof(SEMANTIC_COLOR_DEFS[0]);

    for (int row = 0; row < totalRoles; ++row) {
        const auto& def = SEMANTIC_COLOR_DEFS[row];
        auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, 1));
        if (btn != nullptr) {
            btn->blockSignals(true);
            btn->setLocked(isReadOnly);

            const QColor defaultColor(isDark ? def.defaultDarkHex : def.defaultLightHex);
            const QColor col = colors.value(QLatin1String(def.key), defaultColor);
            btn->setColor(col);
            btn->blockSignals(false);
        }
    }

    table->blockSignals(false);
}

void LC_SemanticColorsUiHelper::extractColors(const QTableWidget* table, QMap<QString, QColor>& outColors) {
    if (table == nullptr) {
        return;
    }

    for (int row = 0; row < table->rowCount(); ++row) {
        const QString key = table->item(row, 0)->data(Qt::UserRole).toString();
        const auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, 1));
        if (btn != nullptr) {
            outColors[key] = btn->color();
        }
    }
}

void LC_SemanticColorsUiHelper::connectChanged(QTableWidget* table, QObject* receiver, const std::function<void()>& callback) {
    if (table == nullptr || receiver == nullptr) {
        return;
    }

    for (int row = 0; row < table->rowCount(); ++row) {
        auto* btn = qobject_cast<LC_ColorButton*>(table->cellWidget(row, 1));
        if (btn != nullptr) {
            connect(btn, &LC_ColorButton::colorChanged, receiver, [callback]() {
                callback();
            });
        }
    }
}
