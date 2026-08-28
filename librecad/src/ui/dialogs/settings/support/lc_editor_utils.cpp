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


#include "lc_editor_utils.h"

#include <QComboBox>

#include "lc_ui_style_manager.h"

void LC_EditorUtils::updatePresetComboFonts(QComboBox* combobox, const QString& activeKey) {
    QString keyToSearch = activeKey;
    if (keyToSearch.isEmpty()) {
        keyToSearch = DEFAULT_THEME_KEY;
    }

    QFont normalFont = combobox->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    QAbstractItemModel* model = combobox->model();
    if (!model) return;

    for (int i = 0; i < combobox->count(); ++i) {
        QModelIndex idx = model->index(i, 0);
        QString itemData = combobox->itemData(i).toString();


        if (keyToSearch == itemData) {
            model->setData(idx, boldFont, Qt::FontRole);
        } else {
            model->setData(idx, normalFont, Qt::FontRole);
        }
    }
}




void LC_EditorUtils::fillNamesFromCombobox(QComboBox* cb, QStringList& list) {
    int themeCount = cb->count();
    for (int i = 1; i < themeCount; i++) {
        QString themeName = cb->itemText(i);
        list << themeName;
    }
}
