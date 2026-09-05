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
#include "../lc_custom_style_helper.h"

#include <QApplication>
#include <QFile>
#include <QStyleFactory>

#include "lc_settings_app_styling.h"
#include "rs_settings.h"

void LC_CustomStyleHelper::loadFromSettings() {
    using namespace CFG_AppStyling;

    const bool allowStyle = o_AllowStyle;
    if (allowStyle) {
        const QString style = o_Style;
        QApplication::setStyle(QStyleFactory::create(style));
    }

    const QString sheet_path = o_StyleSheet;
    loadStyleSheet(sheet_path);
}

bool LC_CustomStyleHelper::loadStyleSheet(const QString& path) {
    if (!path.isEmpty() && QFile::exists(path)) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            auto content = file.readAll();
            auto contentString = QString::fromUtf8(content);
            qApp->setStyleSheet(contentString);
            return true;
        }
    }
    else {
        qApp->setStyleSheet("");
    }
    return false;
}
