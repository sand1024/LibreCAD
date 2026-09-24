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

#include "lc_abstract_preset_manager.h"

#include "lc_settings_app_state.h"

LC_AbstractPresetManager::LC_AbstractPresetManager(const QString& initialKey, QObject* parent)
    : QObject(parent)
    , m_activeKey(initialKey.isEmpty() ? CFG_AppState::DEFAULT_THEME_KEY : initialKey)
    , m_originalActiveKey(m_activeKey)
    , m_isDirty(false) {
}

void LC_AbstractPresetManager::setActivePresetKey(const QString& key) {
    m_activeKey = key;
}

void LC_AbstractPresetManager::setActivePresetKeyDefault() {
    m_activeKey = CFG_AppState::DEFAULT_THEME_KEY;
}

QString LC_AbstractPresetManager::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_AbstractPresetManager::getAppliedPresetKey() const {
    return m_originalActiveKey;
}

bool LC_AbstractPresetManager::isReadOnlyDefault() const {
    return isDefaultPreset(m_activeKey);
}

QString LC_AbstractPresetManager::defaultPresetDisplayName() const {
    const QString name = presetStrings().defaultPresetName;
    if (name.isEmpty()) {
        return tr("Default");
    }
    return name;
}

void LC_AbstractPresetManager::rollbackState() {
    loadPreset(m_originalActiveKey);
}

void LC_AbstractPresetManager::setChangedCallback(std::function<void(bool)> callback) {
    m_changedCallback = std::move(callback);
}

void LC_AbstractPresetManager::setResetCallback(std::function<void()> callback) {
    m_resetCallback = std::move(callback);
}

void LC_AbstractPresetManager::setSaveCommitCallback(std::function<void()> callback) {
    m_saveCommitCallback = std::move(callback);
}

void LC_AbstractPresetManager::setDirtyState(bool dirty) {
    m_isDirty = dirty;
    if (m_changedCallback != nullptr) {
        m_changedCallback(dirty);
    }
}

void LC_AbstractPresetManager::applyCurrentPreset() {
    if (m_saveCommitCallback != nullptr) {
        m_saveCommitCallback();
    }

    applyActiveConfigToSystem(m_activeKey);
    onPostApplyPreset();
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}

bool LC_AbstractPresetManager::deletePreset(const QString& key) {
    clearLastError();

    if (isDefaultPreset(key)) {
        setLastError(LC_PresetError::fromCode(
            LC_PresetErrorCode::ReadOnlyPreset,
            tr("The default template preset cannot be deleted.")
        ));
        return false;
    }

    if (!isStorageAvailable()) {
        setLastError(LC_PresetError::fromCode(
            LC_PresetErrorCode::StorageUnavailable,
            tr("Preset storage is unavailable.")
        ));
        return false;
    }

    if (!doDeletePreset(key)) {
        if (m_lastError.isOk()) {
            setLastError(LC_PresetError::fromCode(
                LC_PresetErrorCode::FileWriteFailed,
                tr("Failed to delete preset '%1'.").arg(key)
            ));
        }
        return false;
    }

    if (m_activeKey == key) {
        setActivePresetKeyDefault();
        loadPreset(CFG_AppState::DEFAULT_THEME_KEY);
    }

    if (m_originalActiveKey == key) {
        m_originalActiveKey = CFG_AppState::DEFAULT_THEME_KEY;
        applyActiveConfigToSystem(CFG_AppState::DEFAULT_THEME_KEY);
    }

    setDirtyState(false);
    return true;
}

bool LC_AbstractPresetManager::isGated() const {
    if (!isStorageAvailable()) {
        return true;
    }
    return isReadOnlyDefault();
}

QString LC_AbstractPresetManager::gatedMessage() const {
    if (!isStorageAvailable()) {
        if (!m_lastError.localizedMessage.isEmpty()) {
            return tr("Preset storage is unavailable or read-only (%1). Presets cannot be modified or saved.")
                .arg(m_lastError.localizedMessage);
        }
        return tr("Preset storage is unavailable or read-only. Presets cannot be modified or saved.");
    }
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    return QString();
}

QString LC_AbstractPresetManager::gatedIcon() const {
    if (!isStorageAvailable()) {
        return QString();
    }
    if (isReadOnlyDefault()) {
        return ":/icons/copy.lci";
    }
    return QString();
}

QString LC_AbstractPresetManager::gatedActionText() const {
    if (!isStorageAvailable()) {
        return QString();
    }
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_AbstractPresetManager::gatedActionCallback() const {
    return nullptr;
}

bool LC_AbstractPresetManager::onDialogAccept(QWidget* parentDialog) {
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false;
    }
    applyActiveConfigToSystem(m_activeKey);
    return true;
}

bool LC_AbstractPresetManager::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}
