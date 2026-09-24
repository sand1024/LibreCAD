
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

#ifndef LC_PROFILE_HELPER_BASE_H
#define LC_PROFILE_HELPER_BASE_H

#include <QJsonObject>
#include <QString>

class LC_ProfileExchangeHelperBase {
public:
    explicit LC_ProfileExchangeHelperBase(const QString& fileIdentifier);
    bool writeJsonFileCompressed(const QString& filePath, const QJsonObject& root, int compressionLevel) const;
    virtual ~LC_ProfileExchangeHelperBase() = default;

    QString fileIdentifier() const { return m_fileFormatIdentifier; }

    bool writeJsonFile(const QString& filePath, const QJsonObject& root) const;
    bool readJsonFile(const QString& filePath, QJsonObject& outRoot) const;
    bool readJsonFileCompressed(const QString& filePath, QJsonObject& outRoot) const;
    bool validateIdentifier(const QJsonObject& root) const;

protected:
    QString m_fileFormatIdentifier;
};

#endif
