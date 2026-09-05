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

LC_AbstractPresetManager::LC_AbstractPresetManager(const QString& initialKey, QObject* parent)
    : QObject(parent)
    , m_activeKey(initialKey.isEmpty() ? DEFAULT_THEME_KEY : initialKey)
    , m_originalActiveKey(m_activeKey) {
}

QString LC_AbstractPresetManager::getActivePresetKey() const {
    return m_activeKey;
}

QString LC_AbstractPresetManager::getAppliedPresetKey() const {
    return m_originalActiveKey;
}

bool LC_AbstractPresetManager::isReadOnlyDefault() const {
    return (m_activeKey == DEFAULT_THEME_KEY || m_activeKey.isEmpty());
}

QString LC_AbstractPresetManager::defaultPresetDisplayName() const {
    return presetStrings().defaultPresetName;
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

bool LC_AbstractPresetManager::isGated() const {
    return isReadOnlyDefault();
}

QString LC_AbstractPresetManager::gatedMessage() const {
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    return QString();
}

QString LC_AbstractPresetManager::gatedActionText() const {
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_AbstractPresetManager::gatedActionCallback() const {
    return nullptr;
}


// --- Transaction Acceptance / Rejection ---
bool LC_AbstractPresetManager::onDialogAccept(QWidget* parentDialog) {
    // 1. If modified, prompt user (Save / Discard / Cancel)
    if (!handlePromptSaveOnAccept(parentDialog)) {
        return false; // Abort accept, keep dialog open
    }

    // 2. Commit the active key to the system
    applyActiveConfigToSystem(m_activeKey);
    return true;
}

bool LC_AbstractPresetManager::onDialogReject(QWidget* parentDialog) {
    return handlePromptDiscardOnReject(parentDialog);
}
