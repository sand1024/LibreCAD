
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

#ifndef LC_PRESET_MANAGER_CONFIG_BASE_H
#define LC_PRESET_MANAGER_CONFIG_BASE_H

#include "lc_abstract_preset_manager.h"
#include "lc_preset_file_io.h"

template <typename TConfig, typename TRepo>
class LC_PresetManagerConfigBase : public LC_AbstractPresetManager {
public:
    explicit LC_PresetManagerConfigBase(TRepo* repo, const QString& initialKey, QObject* parent = nullptr)
        : LC_AbstractPresetManager(initialKey, parent)
        , m_repository(repo) {
        if (m_repository != nullptr && !this->isDefaultPreset(this->m_activeKey) && !m_repository->exists(this->m_activeKey)) {
            this->setActivePresetKeyDefault();
            this->m_originalActiveKey = this->m_activeKey;
        }
    }

    ~LC_PresetManagerConfigBase() override = default;

    bool isStorageAvailable() const override {
        return (m_repository != nullptr && m_repository->isReady());
    }

    QList<QPair<QString, QString>> getAvailablePresets() const override {
        QList<QPair<QString, QString>> choices;
        prependDefaultPresets(choices);
        if (m_repository != nullptr) {
            choices.append(m_repository->getPresetChoices());
        }
        return choices;
    }

    bool saveCurrentPreset() override {
        this->clearLastError();
        if (this->isReadOnlyDefault()) {
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::ReadOnlyPreset,
                tr("Cannot overwrite read-only default preset.")));
            return false;
        }
        if (!isStorageAvailable() || m_repository == nullptr) {
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::StorageUnavailable,
                tr("Preset storage is unavailable.")));
            return false;
        }

        m_workingConfig.name = this->currentPresetDisplayName();
        if (this->m_saveCommitCallback != nullptr) {
            this->m_saveCommitCallback();
        }

        prepareWorkingConfigForSave();

        QString outKey;
        if (m_repository->save(m_workingConfig.name, m_workingConfig, outKey)) {
            this->setActivePresetKey(outKey);
            this->setDirtyState(false);
            onPostSavePreset(outKey);
            return true;
        }

        this->setLastError(m_repository->lastError());
        return false;
    }

    bool savePresetAs(const QString& name, QString& outKey) override {
        this->clearLastError();
        if (!isStorageAvailable() || m_repository == nullptr) {
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::StorageUnavailable,
                tr("Preset storage is unavailable.")));
            return false;
        }

        m_workingConfig.name = name;
        if (this->m_saveCommitCallback != nullptr) {
            this->m_saveCommitCallback();
        }

        prepareWorkingConfigForSave();

        if (m_repository->save(name, m_workingConfig, outKey)) {
            this->setActivePresetKey(outKey);
            this->setDirtyState(false);
            onPostSavePreset(outKey);
            return true;
        }

        this->setLastError(m_repository->lastError());
        return false;
    }

    bool importPresetFromFile(const QString& filePath, [[maybe_unused]] QWidget* parent) override {
        this->clearLastError();
        QJsonObject root;
        const LC_PresetError readErr = LC_PresetFileIO::readJsonFile(filePath, root);
        if (!readErr.isOk()) {
            this->setLastError(readErr);
            return false;
        }

        if (m_repository == nullptr || !m_repository->isCompatible(root)) {
            const QString msg = tr("File '%1' has an incompatible format for this preset.").arg(filePath);
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::IncompatibleFormat, msg));
            return false;
        }

        if (!m_repository->configFromJson(root, m_workingConfig)) {
            const QString msg = tr("Failed to parse preset configuration from '%1'.").arg(filePath);
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::CorruptedData, msg));
            return false;
        }

        this->setDirtyState(true);
        return true;
    }

    bool exportPresetToFile([[maybe_unused]] const QString& key, const QString& filePath, [[maybe_unused]] QWidget* parent) override {
        this->clearLastError();
        if (m_repository == nullptr) {
            this->setLastError(LC_PresetError::fromCode(LC_PresetErrorCode::StorageUnavailable,
                tr("Preset repository is unavailable.")));
            return false;
        }

        prepareWorkingConfigForSave();

        const QJsonObject obj = m_repository->createJSON(m_workingConfig);
        const LC_PresetError writeErr = LC_PresetFileIO::writeJsonFile(filePath, obj);
        if (!writeErr.isOk()) {
            this->setLastError(writeErr);
            return false;
        }
        return true;
    }

    const TConfig& workingConfig() const { return m_workingConfig; }
    TConfig& workingConfig() { return m_workingConfig; }

protected:

    virtual void prepareWorkingConfigForSave() {}
    virtual void onPostSavePreset([[maybe_unused]] const QString& outKey) {}


    virtual void prependDefaultPresets(QList<QPair<QString, QString>> &choices) const {
        choices.prepend(qMakePair(this->defaultPresetDisplayName(), CFG_AppState::DEFAULT_THEME_KEY));
    }

    bool doDeletePreset(const QString& key) override {
        if (m_repository == nullptr) {
            return false;
        }
        const bool ok = m_repository->removeByKey(key);
        if (!ok) {
            this->setLastError(m_repository->lastError());
        }
        return ok;
    }

    TConfig m_workingConfig;
    TRepo* m_repository = nullptr;
};

#endif
