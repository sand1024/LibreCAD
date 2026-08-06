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
        int idx = key.lastIndexOf('/');
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

    // LC_ERR << "LC_SettingsBackend::value: Parsed Group:" << group << "Parsed Key:" << targetKey << "Default:" << defaultValue.toString();

    QVariant result;
    if (defaultValue.typeId() == QMetaType::Bool) {
        result = QVariant(RS_Settings::instance()->readBoolSingle(group, targetKey, defaultValue.toBool()));
    } else if (defaultValue.typeId() == QMetaType::Int) {
        result = QVariant(RS_Settings::instance()->readIntSingle(group, targetKey, defaultValue.toInt()));
    } else if (defaultValue.typeId() == QMetaType::Double) {
        bool ok = false;
        QString valStr = RS_Settings::instance()->readStrSingle(group, targetKey, QString::number(defaultValue.toDouble()));
        double val = valStr.toDouble(&ok);
        result = QVariant(ok ? val : defaultValue);
    } else {
        result = QVariant(RS_Settings::instance()->readStrSingle(group, targetKey, defaultValue.toString()));
    }
    if (!result.isValid() || result.toString().isEmpty()) {
        return defaultValue;
    }

    // LC_ERR << "LC_SettingsBackend::value: RS_Settings returned:" << result.toString();
    return result;
}

void LC_LibreCADSettingsBackend::setValue(const QString& key, const QVariant& val) {
    QString group, targetKey;
    parseKey(key, group, targetKey);
    RS_Settings::instance()->writeEntrySingle(group, targetKey, val);
}

bool LC_LibreCADSettingsBackend::sync() {
    return true;
}
