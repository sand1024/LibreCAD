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

#include "lc_style_index_helper.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#define COMPRESS_INDEX

// Loads the index map from a given index filepath on disk
StyleIndexMap LC_StyleIndexHelper::loadIndex(const QString& indexFilePath) {
    StyleIndexMap indexMap;
    QFile file(indexFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return indexMap; // Return empty map if no index exists yet
    }

#ifdef COMPRESS_INDEX
    const QJsonDocument doc = QJsonDocument::fromJson(qUncompress(file.readAll()));
#    else
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
#    endif
    const QJsonObject root = doc.object();
    const QJsonObject filesObj = root["files"].toObject();

    for (auto it = filesObj.begin(); it != filesObj.end(); ++it) {
        QJsonObject entryObj = it.value().toObject();
        IndexEntry entry;
        entry.displayName = entryObj["name"].toString();
        entry.lastModified = entryObj["modified"].toString();
        indexMap.insert(it.key(), entry);
    }
    return indexMap;
}

// Writes the index map back to disk
bool LC_StyleIndexHelper::saveIndex(const QString& indexFilePath, const StyleIndexMap& indexMap) {
    QJsonObject root;
    QJsonObject filesObj;

    for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
        QJsonObject entryObj;
        entryObj["name"] = it.value().displayName;
        entryObj["modified"] = it.value().lastModified;
        filesObj.insert(it.key(), entryObj);
    }
    root["files"] = filesObj;

    QFile file(indexFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
#ifdef COMPRESS_INDEX
    file.write(qCompress(QJsonDocument(root).toJson(QJsonDocument::Compact)));
#    elif
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
#endif
    return true;
}

// High-performance handshake: checks directory structure and returns files requiring a parse
QStringList LC_StyleIndexHelper::getOutOfDateFiles(const QString& directoryPath, const QString& extension, const StyleIndexMap& indexMap,
                                                   StyleIndexMap& outUpdatedIndex, bool& indexChanged) {
    QStringList outOfDateFiles;
    outUpdatedIndex = indexMap;
    indexChanged = false;

    QDir dir(directoryPath);
    QStringList filters;
    filters << "*" + extension;
    const QFileInfoList filesList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    QSet<QString> existingFileNames;

    for (const QFileInfo& info : filesList) {
        QString filename = info.fileName();
        existingFileNames.insert(filename);

        QString currentModified = info.lastModified().toString(Qt::ISODate);

        if (!indexMap.contains(filename) || indexMap[filename].lastModified != currentModified) {
            outOfDateFiles << filename;
            indexChanged = true;
            // Temporarily store empty display name; calling repository will parse the file and update it
            IndexEntry entry;
            entry.displayName = "";
            entry.lastModified = currentModified;
            outUpdatedIndex.insert(filename, entry);
        }
    }

    // Symmetrically prune deleted files from the cache index
    for (auto it = outUpdatedIndex.begin(); it != outUpdatedIndex.end();) {
        if (!existingFileNames.contains(it.key())) {
            it = outUpdatedIndex.erase(it);
            indexChanged = true;
        }
        else {
            ++it;
        }
    }

    return outOfDateFiles;
}
