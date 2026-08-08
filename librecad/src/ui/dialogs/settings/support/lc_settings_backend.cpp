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

#include "lc_settings_backend.h"

#include "rs_debug.h"
#include "rs_settings.h"

LC_LibreCADSettingsBackend::LC_LibreCADSettingsBackend(const LC_SettingsGroupBase& group)
    : m_defaultGroup(group.groupName()) {}

void LC_LibreCADSettingsBackend::parseKey(const QString& key, QString& outGroup, QString& outKey) const {
    if (key.contains('/')) {
        const int idx = key.lastIndexOf('/');
        outGroup = key.left(idx);
        outKey = key.mid(idx + 1);
    } else {
        outGroup = m_defaultGroup;
        outKey = key;
    }
}

QVariant LC_LibreCADSettingsBackend::value(const QString& key, const QVariant& defaultValue) const {
    QString group, targetKey;
    parseKey(key, group, targetKey);

    // Handle Colors (Fixing the "QColor(ARGB..." bug)
    if (defaultValue.userType() == QMetaType::QColor) {
        const QString defHex = defaultValue.value<QColor>().name();
        const QString resultHex = RS_Settings::instance()->readStrSingle(group, targetKey, defHex);

        // We return a QVariant(QColor) so LC_Setting::get() can cast it correctly
        return QVariant::fromValue(QColor::fromString(resultHex));
    }

    // Handle Doubles (Existing logic is okay, but let's be safe)
    if (defaultValue.typeId() == QMetaType::Double) {
        bool ok = false;
        const QString valStr = RS_Settings::instance()->readStrSingle(group, targetKey, QString::number(defaultValue.toDouble()));
        double val = valStr.toDouble(&ok);
        return ok ? val : defaultValue;
    }

    // Handle Bools/Ints (Symmetrical with RS_Settings specialized methods)
    if (defaultValue.typeId() == QMetaType::Bool) {
        return RS_Settings::instance()->readBoolSingle(group, targetKey, defaultValue.toBool());
    }
    if (defaultValue.typeId() == QMetaType::Int) {
        return RS_Settings::instance()->readIntSingle(group, targetKey, defaultValue.toInt());
    }

    return RS_Settings::instance()->readStrSingle(group, targetKey, defaultValue.toString());
}

void LC_LibreCADSettingsBackend::setValue(const QString& key, const QVariant& val) {
    QString group, targetKey;
    parseKey(key, group, targetKey);

    QVariant toStore = val;
    // Normalize QColor to Hex String so RS_Settings stays clean
    if (val.userType() == QMetaType::QColor) {
        toStore = val.value<QColor>().name();
    }

    RS_Settings::instance()->writeEntrySingle(group, targetKey, toStore);
}

bool LC_LibreCADSettingsBackend::sync() {
    return true;
}
