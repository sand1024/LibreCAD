
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

#ifndef LC_ABSTRACT_PRESET_MANAGER_H
#define LC_ABSTRACT_PRESET_MANAGER_H

#include <QObject>
#include <QString>
#include <functional>
#include "lc_palette_editor_shared.h"
#include "lc_preset_manager_interface.h"

class LC_AbstractPresetManager : public QObject, public LC_PresetManagerInterface {
    Q_OBJECT
public:
    explicit LC_AbstractPresetManager(const QString& initialKey, QObject* parent = nullptr);
    ~LC_AbstractPresetManager() override = default;

    // --- Identification & Key Management ---
    QString getActivePresetKey() const override;
    QString getAppliedPresetKey() const override;
    bool isReadOnlyDefault() const override;
    QString defaultPresetDisplayName() const override;

    // --- State & Rollback ---
    void rollbackState() override;
    bool supportsApply() const override { return true; }
    bool supportsImportExport() const override { return true; }
    void setScopeDirty(bool dirty) { setDirtyState(dirty); }

    // --- Callback Registration ---
    void setChangedCallback(std::function<void(bool isDirty)> callback) override;
    void setResetCallback(std::function<void()> callback) override;
    void setSaveCommitCallback(std::function<void()> callback) override;

    // --- Default Read-Only Gating (Can be overridden by subclasses) ---
    bool isGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;
    bool onDialogAccept(QWidget* parentDialog) override;
    bool onDialogReject(QWidget* parentDialog) override;

    virtual void notifyWorkingConfigChanged() {
        setDirtyState(true);
    }

protected:
    // Helper to update dirty state and notify listeners in a single call
    void setDirtyState(bool dirty);

    virtual void applyActiveConfigToSystem(const QString& activeKey) = 0;

    QString m_activeKey;
    QString m_originalActiveKey;
    bool m_isDirty = false;

    std::function<void(bool)> m_changedCallback;
    std::function<void()> m_resetCallback;
    std::function<void()> m_saveCommitCallback;
};

#endif // LC_ABSTRACT_PRESET_MANAGER_H
