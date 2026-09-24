
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

#ifndef LC_STYLEREPOSITORYBASE_H
#define LC_STYLEREPOSITORYBASE_H

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "lc_presets_index_helper.h"
#include "lc_preset_error.h"
#include "lc_preset_file_io.h"
#include "lc_preset_repository.h"
#include "lc_settings_app_state.h"
#include "rs_debug.h"

template <typename ConfigType>
class LC_PresetRepositoryBase : public LC_PresetRepository {
public:
    using ConfigurationType = ConfigType;

    LC_PresetRepositoryBase(const QString& configDir,
                           const QString& extension,
                           const QString& fileIdentifier,
                           const QString& indexFileName)
        : m_extension(extension)
        , m_fileFormatIdentifier(fileIdentifier)
        , m_indexFileName(indexFileName)
        , m_isIndexLoaded(false) {
        if (!configDir.isEmpty()) {
            setConfigDir(configDir);
        }
    }

    ~LC_PresetRepositoryBase() override = default;

    QString getConfigDir() const { return m_configDir; }
    QString getFileExtension() const override { return m_extension; }
    QString fileIdentifier() const override { return m_fileFormatIdentifier; }
    QString fileVersion() const { return m_fileFormatVersion; }
    void setFileVersion(const QString& version) { m_fileFormatVersion = version; }

    bool isReady() const override {
        return m_isReady;
    }

    LC_PresetError lastError() const override {
        return m_lastError;
    }

    bool setConfigDir(const QString& newConfigDir) override {
        m_lastError.clear();
        QDir dir(newConfigDir);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                const QString msg = QObject::tr("Cannot create preset storage directory '%1'.").arg(newConfigDir);
                LC_ERR << "LC_PresetRepositoryBase: " << msg;
                m_lastError = LC_PresetError::fromCode(LC_PresetErrorCode::DirectoryCreationFailed, msg, newConfigDir);
                m_isReady = false;
                return false;
            }
        }

        m_configDir = newConfigDir;
        m_indexFilePath = m_configDir + "/" + m_indexFileName;
        m_isIndexLoaded = false;
        m_cachedIndex.clear();
        m_isReady = true;

        initializeIndex();
        return true;
    }

    bool exists(const QString& key) const override {
        if (isDefaultKey(key)) {
            return true;
        }
        return QFile::exists(getFilePathForKey(key));
    }

    bool removeByKey(const QString& key) override {
        m_lastError.clear();
        if (isDefaultKey(key)) {
            m_lastError = LC_PresetError::fromCode(LC_PresetErrorCode::ReadOnlyPreset,
                QObject::tr("Cannot delete the default template preset."));
            return false;
        }

        const QString filepath = getFilePathForKey(key);
        if (!QFile::remove(filepath)) {
            const QString msg = QObject::tr("Could not delete preset file '%1'.").arg(filepath);
            LC_ERR << "LC_PresetRepositoryBase: " << msg;
            m_lastError = LC_PresetError::fromCode(LC_PresetErrorCode::FileWriteFailed, msg, filepath);
            return false;
        }

        ensureIndexLoaded();
        const QString filenameWithExt = QFileInfo(filepath).fileName();
        const QString baseKey = QFileInfo(filepath).completeBaseName();

        if (m_cachedIndex.contains(filenameWithExt)) {
            m_cachedIndex.remove(filenameWithExt);
            LC_PresetsIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
        }
        else if (m_cachedIndex.contains(baseKey)) {
            m_cachedIndex.remove(baseKey);
            LC_PresetsIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
        }
        return true;
    }

    // Alias for backward compatibility
    bool deleteByKey(const QString& key) {
        return removeByKey(key);
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
            QString cleanKey = it.key();
            if (cleanKey.endsWith(m_extension, Qt::CaseInsensitive)) {
                cleanKey.chop(m_extension.length());
            }
            choices.append(qMakePair(it.value().displayName, cleanKey));
        }
        std::sort(choices.begin(), choices.end(), [](const QPair<QString, QString>& a, const QPair<QString, QString>& b) {
            return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
        });
        return choices;
    }

    bool loadByKey(const QString& key, ConfigType& config) const {
        if (isDefaultKey(key)) {
            return false;
        }
        return loadFile(getFilePathForKey(key), config);
    }

    bool save(const QString& name, const ConfigType& config, QString& outKey) {
        if (!m_isReady) {
            m_lastError = LC_PresetError::fromCode(LC_PresetErrorCode::StorageUnavailable,
                QObject::tr("Preset storage is unavailable."));
            return false;
        }

        const QString filepath = getFilePathForName(name);
        const QString filenameWithExt = QFileInfo(filepath).fileName();
        outKey = QFileInfo(filepath).completeBaseName();

        ConfigType configCopy = config;
        configCopy.name = name;

        if (saveFile(filepath, configCopy)) {
            ensureIndexLoaded();
            IndexEntry entry;
            entry.displayName = name;
            entry.lastModified = QFileInfo(filepath).lastModified().toString(Qt::ISODate);

            m_cachedIndex.insert(filenameWithExt, entry);
            LC_PresetsIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
            return true;
        }
        return false;
    }

    virtual bool isDefaultKey(const QString& key) const {
        return (key == CFG_AppState::DEFAULT_THEME_KEY);
    }

    void initializeIndex() {
        if (!m_isReady) {
            return;
        }
        ensureIndexLoaded();
        PresetsIndexMap updatedIndex;
        bool indexChanged = false;

        const QStringList outOfDateFiles = LC_PresetsIndexHelper::getOutOfDateFiles(
            m_configDir, m_extension, m_cachedIndex, updatedIndex, indexChanged
        );

        for (const QString& filename : outOfDateFiles) {
            const QString filepath = m_configDir + "/" + filename;
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
            LC_PresetsIndexHelper::saveIndex(m_indexFilePath, m_cachedIndex);
        }
    }

    virtual QJsonObject configToJson(const ConfigType& config) const = 0;
    virtual bool configFromJson(const QJsonObject& json, ConfigType& config) const = 0;

    bool isCompatible(const QJsonObject& json) const {
        QString fileId = json["lc_file_format"].toString();
        if (fileId.isEmpty()) {
            fileId = json["file_identifier"].toString();
        }
        if (fileId.isEmpty()) {
            fileId = json["type"].toString();
        }
        return (fileId == m_fileFormatIdentifier);
    }

    bool byteArrayToJson(ConfigType& config, const QByteArray& jsonBytes) const {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            return false;
        }

        const QJsonObject root = doc.object();
        if (!isCompatible(root)) {
            return false;
        }

        config.name = root["name"].toString();
        return configFromJson(root, config);
    }

    bool loadFile(const QString& filepath, ConfigType& config) const {
        QJsonObject root;
        const LC_PresetError err = LC_PresetFileIO::readJsonFile(filepath, root);
        if (!err.isOk()) {
            m_lastError = err;
            return false;
        }

        if (!isCompatible(root)) {
            const QString msg = QObject::tr("File '%1' is not compatible with format '%2'.")
                                    .arg(filepath, m_fileFormatIdentifier);
            LC_ERR << "LC_PresetRepositoryBase: " << msg;
            m_lastError = LC_PresetError::fromCode(LC_PresetErrorCode::IncompatibleFormat, msg);
            return false;
        }

        config.name = root["name"].toString();
        return configFromJson(root, config);
    }

    QJsonObject createJSON(const ConfigType& config) const {
        QJsonObject root = configToJson(config);
        root["lc_file_format"] = m_fileFormatIdentifier;
        root["lc_file_format_version"] = m_fileFormatVersion;
        root["name"] = config.name;
        return root;
    }

    bool saveFile(const QString& filepath, const ConfigType& config) const {
        const QJsonObject root = createJSON(config);
        const LC_PresetError err = LC_PresetFileIO::writeJsonFile(filepath, root);
        if (!err.isOk()) {
            m_lastError = err;
            return false;
        }
        return true;
    }

protected:
    static QString sanitizeFileName(const QString& input) {
        if (input.isEmpty()) {
            return QString("unnamed");
        }

        static const QRegularExpression illegalChars(R"([<>:"/\\|?*\x00-\x1F\x7F])");
        QString sanitized = input;
        sanitized.replace(illegalChars, QString());
        sanitized = sanitized.replace(' ', '_').trimmed();

        while (sanitized.endsWith('.') || sanitized.endsWith(' ') || sanitized.endsWith('_')) {
            sanitized.chop(1);
        }

        return sanitized.isEmpty() ? QString("unnamed") : sanitized;
    }

    QString getFilePathForName(const QString& name) const {
        const QString safeFilename = sanitizeFileName(name).toLower();
        return m_configDir + "/" + safeFilename + m_extension;
    }

    QString getFilePathForKey(const QString& key) const {
        if (key.endsWith(m_extension, Qt::CaseInsensitive)) {
            return m_configDir + "/" + key;
        }
        return m_configDir + "/" + key + m_extension;
    }

    virtual bool readNameFromFile(QFile& file, QString& name) const {
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) {
            return false;
        }
        const QJsonObject root = doc.object();
        if (!isCompatible(root)) {
            return false;
        }

        name = root["name"].toString();
        return !name.isEmpty();
    }

    void ensureIndexLoaded() const {
        if (!m_isIndexLoaded && m_isReady) {
            m_cachedIndex = LC_PresetsIndexHelper::loadIndex(m_indexFilePath);
            m_isIndexLoaded = true;
        }
    }

    QString m_configDir;
    QString m_extension;
    QString m_fileFormatIdentifier;
    QString m_fileFormatVersion = "1.0";
    QString m_indexFileName;
    QString m_indexFilePath;

    mutable PresetsIndexMap m_cachedIndex;
    mutable bool m_isIndexLoaded = false;
    bool m_isReady = false;
    mutable LC_PresetError m_lastError;
};

#endif
