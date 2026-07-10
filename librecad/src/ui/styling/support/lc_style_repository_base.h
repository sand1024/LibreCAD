
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

#ifndef LC_StyleRepositoryBase_H
#define LC_StyleRepositoryBase_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QSet>

#include "lc_icons_style_shared.h"
#include "lc_palette_editor_shared.h"
#include "lc_style_index_helper.h"

template <typename ConfigType>
class LC_StyleRepositoryBase : public LC_RepositoryBase {
public:
    LC_StyleRepositoryBase(const QString& configDir,
                           const QString& extension,
                           const QString& fileIdentifier,
                           const QString& indexFileName)
        : m_configDir(configDir)
        , m_extension(extension)
        , m_fileIdentifier(fileIdentifier)
        , m_indexFileName(indexFileName)
        , m_isIndexLoaded(false)
    {
        QDir().mkpath(m_configDir);
        m_indexFilePath = m_configDir + "/" + m_indexFileName;
    }

    virtual ~LC_StyleRepositoryBase() override = default;

    QString configDir() const { return m_configDir; }
    QString extension() const { return m_extension; }
    QString fileIdentifier() const { return m_fileIdentifier; }

    QString getFilePathForName(const QString& name) const {
        QString safeFilename = name.toLower().replace(" ", "_");
        safeFilename.remove(QRegularExpression("[^a-z0-9_]"));
        return m_configDir + "/" + safeFilename + m_extension;
    }

    // Resolves filepaths directly by their unique filenames (keys) on Load/Delete
    QString getFilePathForKey(const QString& key) const {
        return m_configDir + "/" + key;
    }

    bool exists(const QString& key) const override {
        if (key == DEFAULT_THEME_KEY) {
            return true;
        }
        return QFile::exists(getFilePathForKey(key));
    }

    bool isCompatible(const QJsonObject& json) const {
        QString fileId = json["file_identifier"].toString();
        if (fileId.isEmpty()) {
            fileId = json["type"].toString(); // Fallback for older formats
        }
        return (fileId == m_fileIdentifier);
    }

    // Checks if a file on disk matches this repository's format signature
    bool isCompatibleFile(const QString& filepath) const {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        return isCompatible(doc.object());
    }

    bool deleteByKey(const QString& key) {
        if (key == DEFAULT_THEME_KEY) {
            return false; // Guard: virtual defaults cannot be deleted
        }
        QString filepath = getFilePathForKey(key);
        bool removed = QFile::remove(filepath);
        if (removed) {
            ensureIndexLoaded();
            if (m_cachedIndex.contains(key)) {
                m_cachedIndex.remove(key);
                LC_StyleIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
            }
        }
        return removed;
    }

    bool removeByKey(const QString& key) override {
        return deleteByKey(key);
    }


    void beginScanning() {
        m_pendingScanFiles.clear();
        QDir dir(m_configDir);
        QStringList filters;
        filters << QString("*") + m_extension;
        m_pendingScanFiles = dir.entryList(filters, QDir::Files);
    }

    bool getNextPreset(QString &name, QString &filePath) {
        while (!m_pendingScanFiles.isEmpty()) {
            QString filename = m_pendingScanFiles.takeFirst();
            QString path = m_configDir + "/" + filename;
            QFile file(path);
            if (readNameFromFile(file, name)) {
                filePath = path;
                return true;
            }
        }
        return false;
    }

    bool loadByKey(const QString& key, ConfigType& config) const {
        if (key == DEFAULT_THEME_KEY) {
            return false; // Handled by editor-level defaults initialization
        }
        return loadFile(getFilePathForKey(key), config);
    }

    bool save(const QString& name, const ConfigType& config, QString& outKey) {
        QString filepath = getFilePathForName(name);
        QString filename = QFileInfo(filepath).fileName();
        outKey = filename; // Returns the stable key to the caller

        ConfigType configCopy = config;
        configCopy.name = name;

        if (saveFile(filepath, configCopy)) {
            ensureIndexLoaded();
            IndexEntry entry;
            entry.displayName = name;
            entry.lastModified = QFileInfo(filepath).lastModified().toString(Qt::ISODate);

            m_cachedIndex.insert(filename, entry);
            LC_StyleIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
            return true;
        }
        return false;
    }

    // High-performance index initialization
    void initializeIndex() {
        ensureIndexLoaded();
        StyleIndexMap updatedIndex;
        bool indexChanged = false;

        QStringList outOfDateFiles = LC_StyleIndexHelper::getOutOfDateFiles(
            m_configDir, m_extension, m_cachedIndex, updatedIndex, indexChanged
        );

        for (const QString& filename : outOfDateFiles) {
            QString filepath = m_configDir + "/" + filename;
            QFile file(filepath);
            QString displayName;
            if (readNameFromFile(file, displayName)) {
                updatedIndex[filename].displayName = displayName;
                indexChanged = true;
            } else {
                updatedIndex.remove(filename);
                indexChanged = true;
            }
        }

        if (indexChanged) {
            m_cachedIndex = updatedIndex;
            LC_StyleIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
        }
    }

    QStringList getAvailableNames() override {
        ensureIndexLoaded();
        QStringList names;
        for (auto it = m_cachedIndex.begin(); it != m_cachedIndex.end(); ++it) {
            if (!it.value().displayName.isEmpty()) {
                names << it.value().displayName;
            }
        }
        names.sort(Qt::CaseInsensitive);
        return names;
    }


    QList<QPair<QString, QString>> getPresetChoices() const override {
        ensureIndexLoaded();
        QList<QPair<QString, QString>> choices;
        for (auto it = m_cachedIndex.begin(); it != m_cachedIndex.end(); ++it) {
            choices.append(qMakePair(it.value().displayName, it.key()));
        }
        // Sort by visual translated text name
        std::sort(choices.begin(), choices.end(), [](const QPair<QString, QString>& a, const QPair<QString, QString>& b) {
            return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
        });
        return choices;
    }

    QString getFilePathForDisplayName(const QString& displayName)  {
        ensureIndexLoaded();
        for (auto it = m_cachedIndex.begin(); it != m_cachedIndex.end(); ++it) {
            if (it.value().displayName == displayName) {
                return getFilePathForKey(it.key());
            }
        }
        return QString();
    }

    virtual QJsonObject configToJson(const ConfigType& config) const = 0;
    virtual bool configFromJson(const QJsonObject& json, ConfigType& config) const = 0;

protected:
    bool loadFile(const QString& filepath, ConfigType& config) const {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly)) return false;

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        const QJsonObject root = doc.object();

        if (!isCompatible(root)) {
            return false;
        }

        config.name = root["name"].toString();
        return configFromJson(root, config);
    }


    bool saveFile(const QString& filepath, const ConfigType& config) const {
        QJsonObject root = configToJson(config);
        root["file_identifier"] = m_fileIdentifier;
        root["name"] = config.name;

        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly)) return false;

        file.write(QJsonDocument(root).toJson());
        return true;
    }

    virtual bool readNameFromFile(QFile &file, QString &name) const {
        if (!file.open(QIODevice::ReadOnly)) return false;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        QJsonObject root = doc.object();
        QString fileId = root["file_identifier"].toString();
        if (fileId.isEmpty()) {
            fileId = root["type"].toString(); // Support older icons style formats
        }
        if (fileId != m_fileIdentifier) return false;

        name = root["name"].toString();
        return !name.isEmpty();
    }

    void ensureIndexLoaded() const {
        if (!m_isIndexLoaded) {
            m_cachedIndex = LC_StyleIndexHelper::loadIndex(m_indexFilePath);
            m_isIndexLoaded = true;
        }
    }

    QString m_configDir;
    QString m_extension;
    QString m_fileIdentifier;
    QString m_indexFileName;
    QString m_indexFilePath;

    mutable StyleIndexMap m_cachedIndex;
    mutable bool m_isIndexLoaded;
    QStringList m_pendingScanFiles;
};


#endif
