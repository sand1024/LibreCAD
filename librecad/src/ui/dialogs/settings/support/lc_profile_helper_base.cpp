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

#include "lc_profile_helper_base.h"

#include <QFile>
#include <QJsonDocument>

LC_ProfileExchangeHelperBase::LC_ProfileExchangeHelperBase(const QString& fileIdentifier)
    : m_fileFormatIdentifier(fileIdentifier) {
}

bool LC_ProfileExchangeHelperBase::writeJsonFileCompressed(const QString& filePath, const QJsonObject& root, int compressionLevel = 1) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QByteArray uncompressedData = QJsonDocument(root).toJson(QJsonDocument::Compact);

    QByteArray compressedData = qCompress(uncompressedData, compressionLevel);

    if (compressedData.isEmpty()) {
        return false;
    }

    file.write(compressedData);
    return true;
}

bool LC_ProfileExchangeHelperBase::writeJsonFile(const QString& filePath, const QJsonObject& root) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool LC_ProfileExchangeHelperBase::readJsonFile(const QString& filePath, QJsonObject& outRoot) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return false;
    }

    outRoot = doc.object();
    return validateIdentifier(outRoot);
}


// fixme - sand - more detailed reporting of errors
bool LC_ProfileExchangeHelperBase::readJsonFileCompressed(const QString& filePath, QJsonObject& outRoot) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray compressedData = file.readAll();
    file.close();

    if (compressedData.isEmpty()) {
        return false;
    }


    QByteArray uncompressedData = qUncompress(compressedData);
    if (uncompressedData.isEmpty()) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(uncompressedData);

    if (!doc.isObject()) {
        return false;
    }

    outRoot = doc.object();
    return validateIdentifier(outRoot);
}

bool LC_ProfileExchangeHelperBase::validateIdentifier(const QJsonObject& root) const {
    QString fileType = root.value("lc_file_type").toString();
    if (fileType.isEmpty()) {
        fileType = root.value("file_identifier").toString(); // Backward compatibility
    }
    return (fileType == m_fileFormatIdentifier);
}
