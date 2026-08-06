
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

#ifndef LC_SETTINGS_BACKEND_H
#define LC_SETTINGS_BACKEND_H

#include <QString>
#include <QVariant>

#include "lc_setting.h"

class LC_SettingsBackend {
public:
    virtual ~LC_SettingsBackend() = default;

    virtual QVariant value(const QString& key, const QVariant& defaultValue) const = 0;
    virtual void setValue(const QString& key, const QVariant& val) = 0;
    virtual bool sync() = 0;
};

class LC_LibreCADSettingsBackend : public LC_SettingsBackend {
public:
    explicit LC_LibreCADSettingsBackend(const LC_SettingsGroupBase& group);
    ~LC_LibreCADSettingsBackend() override = default;

    QVariant value(const QString& key, const QVariant& defaultValue) const override;
    void setValue(const QString& key, const QVariant& val) override;
    bool sync() override;

private:
    QString m_defaultGroup;
    void parseKey(const QString& key, QString& outGroup, QString& outKey) const;
};

#endif
