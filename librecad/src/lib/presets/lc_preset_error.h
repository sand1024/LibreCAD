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

#ifndef LC_PRESET_ERROR_H
#define LC_PRESET_ERROR_H

#include <QString>
#include <QObject>

enum class LC_PresetErrorCode {
    None = 0,
    DirectoryCreationFailed,
    DirectoryNotWritable,
    FileNotFound,
    FileReadFailed,
    FileWriteFailed,
    InvalidJsonSyntax,
    IncompatibleFormat,
    UnsupportedVersion,
    ReadOnlyPreset,
    CorruptedData,
    StorageUnavailable
};

struct LC_PresetError {
    LC_PresetErrorCode code = LC_PresetErrorCode::None;
    QString technicalDetails;
    QString localizedMessage;

    bool isOk() const {
        return code == LC_PresetErrorCode::None;
    }

    void clear() {
        code = LC_PresetErrorCode::None;
        technicalDetails.clear();
        localizedMessage.clear();
    }

    static LC_PresetError ok() {
        return LC_PresetError();
    }

    static LC_PresetError fromCode(LC_PresetErrorCode errCode, const QString& message, const QString& details = QString()) {
        LC_PresetError err;
        err.code = errCode;
        err.localizedMessage = message;
        err.technicalDetails = details;
        return err;
    }
};

#endif
