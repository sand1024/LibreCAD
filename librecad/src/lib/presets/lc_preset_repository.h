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

#ifndef LC_PRESET_REPOSITORY_H
#define LC_PRESET_REPOSITORY_H

#include <QString>
#include <QStringList>

#include "lc_preset_error.h"

class LC_PresetRepository {
public:
    virtual ~LC_PresetRepository() = default;
    virtual bool exists(const QString& name) const = 0;
    virtual bool removeByKey(const QString& name) = 0;
    virtual QStringList getAvailableNames() = 0;
    virtual QList<QPair<QString, QString>> getPresetChoices() const = 0;

    // Capabilities and storage control
    virtual QString getFileExtension() const = 0;
    virtual QString fileIdentifier() const = 0;
    virtual bool isReady() const = 0;
    virtual bool setConfigDir(const QString& newConfigDir) = 0;
    virtual LC_PresetError lastError() const = 0;
};

#endif
