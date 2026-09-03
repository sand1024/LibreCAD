
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

#ifndef LC_PRESET_MANAGER_INTERFACE_H
#define LC_PRESET_MANAGER_INTERFACE_H

#include <QString>
#include <QList>
#include <functional>
#include <QObject>

#include "lc_palette_editor_shared.h"

class QWidget;

enum class LC_PresetAcceptPolicy {
    None,                 // Direct registry persistence; no preset prompt on OK
    PromptSaveIfModified, // Prompt user to save modified custom presets on OK
    AutoSaveIfModified    // Automatically commit working changes to preset file on OK
};

struct LC_PresetManagerUIStrings {
    QString labelText = QObject::tr("Select Preset:");
    QString selectToolTip = QObject::tr("Choose from available saved configurations.");
    QString saveToolTip = QObject::tr("Save current configurations directly to disk.");
    QString saveAsToolTip = QObject::tr("Save current configuration as a new preset file.");
    QString deleteToolTip = QObject::tr("Permanently delete the selected custom preset from disk.");
    QString applyToolTip = QObject::tr("Apply the selected custom preset.");
    QString importToolTip = QObject::tr("Import preset from external file.");
    QString exportToolTip = QObject::tr("Export preset to external file.");
    QString revertToolTip = QObject::tr("Discard active modifications and reload the preset as saved on disk.");
    QString saveAsDialogTitle = QObject::tr("Save Preset As");
    QString saveAsDialogLabel = QObject::tr("Select unique name of the preset");
    QString defaultPresetName = QObject::tr("Default Preset");
    QString defaultNewPresetName = QObject::tr("Custom Preset");
    QString deleteConfirmTitle = QObject::tr("Confirm Delete");
    QString deleteConfirmLabel = QObject::tr("Are you sure that you'd like to delete preset '%1'?");
    QString exportDialogTitle = QObject::tr("Export Preset");
    QString importDialogTitle = QObject::tr("Import Preset");
    QString presetFileFilter = QObject::tr("Preset file (*.json)");

    // Read-Only Default & Lifecycle Strings
    QString defaultReadOnlyMessage = QObject::tr(
        "The Default preset is a read-only template. To customize settings, duplicate it as a custom preset.");
    QString duplicateActionText = QObject::tr("Duplicate / Customize...");

    QString saveModifiedPromptTitle = QObject::tr("Unsaved Changes");
    QString saveModifiedPromptMessage = QObject::tr("You have unsaved changes to preset '%1'. Do you want to save them before applying?");
    QString discardConfirmTitle = QObject::tr("Unsaved Changes");
    QString discardConfirmMessage = QObject::tr("You have unsaved modifications to preset '%1'. Do you want to discard these changes?");
};

class LC_PresetManagerInterface {
public:
    virtual ~LC_PresetManagerInterface() = default;

    virtual bool loadPreset(const QString& key) = 0;
    virtual bool saveCurrentPreset() = 0;
    virtual bool savePresetAs(const QString& name, QString& outKey) = 0;
    virtual bool deletePreset(const QString& key) = 0;
    virtual QList<QPair<QString, QString>> getAvailablePresets() const = 0;
    virtual QString getActivePresetKey() const = 0;
    virtual QString getAppliedPresetKey() const { return getActivePresetKey(); }

    virtual void applyCurrentPreset() = 0;
    virtual void rollbackState() = 0;

    virtual void setChangedCallback(std::function<void(bool isDirty)> callback) = 0;

    // Dynamic Reset Callback: lets the manager trigger a clean visual page-defaults-reset
    virtual void setResetCallback(std::function<void()> callback) { m_resetCallback = callback; }
    virtual void setSaveCommitCallback(std::function<void()> callback) { m_saveCommitCallback = callback; }

    virtual bool importPresetFromFile([[maybe_unused]]const QString& filePath,[[maybe_unused]] QWidget* parent) { return false; }
    virtual bool exportPresetToFile([[maybe_unused]]const QString& key,[[maybe_unused]] const QString& filePath, [[maybe_unused]]QWidget* parent) { return false; }

    virtual LC_PresetManagerUIStrings presetStrings() const {
        return LC_PresetManagerUIStrings();
    }

    virtual bool supportsApply() const { return false; }
    virtual bool supportsImportExport() const { return true; }

    virtual QWidget* getSharedHeaderWidget() { return nullptr; }
    virtual QWidget* getSharedPreviewWidget() { return nullptr; }
    virtual QWidget* getSharedBottomWidget() { return nullptr; }
    virtual bool isPresetModified() {return false;}

    virtual LC_PresetAcceptPolicy acceptPolicy() const { return LC_PresetAcceptPolicy::None; }
    virtual bool onDialogAccept([[maybe_unused]] QWidget* parentDialog) { return true; }
    virtual bool onDialogReject([[maybe_unused]] QWidget* parentDialog) {return true;}

    // Scope-level Gating Queries
    virtual bool isReadOnlyDefault() const {
        const QString key = getActivePresetKey();
        return key.isEmpty() || key == DEFAULT_THEME_KEY;
    }

    virtual QString defaultPresetDisplayName() const {
        const QString name = presetStrings().defaultPresetName;
        return name.isEmpty() ? QObject::tr("Default") : name;
    }

    virtual bool isGated() const { return isReadOnlyDefault(); }
    virtual QString gatedMessage() const { return presetStrings().defaultReadOnlyMessage; }
    virtual QString gatedActionText() const { return presetStrings().duplicateActionText; }
    virtual std::function<void()> gatedActionCallback() const { return nullptr; }

    // --- Reusable Centralized Preset Helpers ---
    virtual QString currentPresetDisplayName() const;
    virtual bool promptSavePresetAs(QWidget* parentWidget, QString* outNewKey = nullptr);
    virtual bool handlePromptSaveOnAccept(QWidget* parentWidget);
    virtual bool handlePromptDiscardOnReject(QWidget* parentWidget);

protected:
    std::function<void()> m_resetCallback;
    std::function<void()> m_saveCommitCallback;
};

#endif
