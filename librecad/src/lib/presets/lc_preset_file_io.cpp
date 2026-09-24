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

#include "lc_preset_file_io.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include "rs_debug.h"

namespace LC_PresetFileIO {
    LC_PresetError readJsonFile(const QString& filePath, QJsonObject& outObject) {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            const QString msg = QObject::tr("Preset file '%1' does not exist.").arg(filePath);
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::FileNotFound, msg, filePath);
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString msg = QObject::tr("Cannot open preset file '%1' for reading: %2").arg(filePath, file.errorString());
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::FileReadFailed, msg, file.errorString());
        }

        const QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            const QString msg = QObject::tr("Syntax error in preset file '%1': %2").arg(filePath, parseError.errorString());
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::InvalidJsonSyntax, msg, parseError.errorString());
        }

        if (!doc.isObject()) {
            const QString msg = QObject::tr("Preset file '%1' root content is not a valid JSON object.").arg(filePath);
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::CorruptedData, msg, filePath);
        }

        outObject = doc.object();
        return LC_PresetError::ok();
    }

    LC_PresetError writeJsonFile(const QString& filePath, const QJsonObject& object) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            const QString msg = QObject::tr("Cannot open preset file '%1' for writing: %2").arg(filePath, file.errorString());
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::FileWriteFailed, msg, file.errorString());
        }

        const QJsonDocument doc(object);
        const qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
        file.close();

        if (bytesWritten == -1) {
            const QString msg = QObject::tr("Failed to write preset data to '%1': %2").arg(filePath, file.errorString());
            LC_ERR << "LC_PresetFileIO: " << msg;
            return LC_PresetError::fromCode(LC_PresetErrorCode::FileWriteFailed, msg, file.errorString());
        }

        return LC_PresetError::ok();
    }
}
