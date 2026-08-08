
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

class QWidget;

struct LC_PresetManagerUIStrings {
    QString labelText = "Select Preset:";
    QString selectToolTip = "Choose from available saved configurations.";
    QString saveToolTip = "Save current configurations directly to disk.";
    QString saveAsToolTip = "Save current configuration as a new preset file.";
    QString deleteToolTip = "Permanently delete the selected custom preset from disk.";
    QString applyToolTip = "Apply the selected custom preset.";
    QString importToolTip = "Import preset from external file.";
    QString exportToolTip = "Export preset to external file.";
    QString revertToolTip = "Discard active modifications and reload the preset as saved on disk.";
    QString saveAsDialogTitle = "Save Preset As";
    QString saveAsDialogLabel = "Select unique name of the preset";
    QString deleteConfirmTitle = "Confirm Delete";
    QString deleteConfirmLabel = "Are you sure that you'd like to delete preset?";
    QString exportDialogTitle = "Export Preset";
    QString importDialogTitle = "Import Preset";
    QString presetFileFilter = "Preset file(*.json)";
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

    virtual QWidget* getSharedPreviewWidget() { return nullptr; }
    virtual QWidget* getSharedBottomWidget() { return nullptr; }
    virtual bool isPresetModified() {return false;}

protected:
    std::function<void()> m_resetCallback;
    std::function<void()> m_saveCommitCallback;
};

#endif
