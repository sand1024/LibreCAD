
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

#ifndef LC_PRESET_MANAGER_SHORTCUTS_H
#define LC_PRESET_MANAGER_SHORTCUTS_H

#include "lc_abstract_preset_manager.h"
#include "lc_shortcuts_repository.h"

class LC_ActionGroupManager;
class LC_ShortcutsManager;
class LC_ShortcutsTreeModel;

class LC_PresetManagerShortcuts : public LC_AbstractPresetManager {
    Q_OBJECT
public:
    LC_PresetManagerShortcuts(LC_ActionGroupManager* groupMgr,
                              QObject* parent = nullptr);
    ~LC_PresetManagerShortcuts() override = default;

    // --- Preset Manager Interface ---
    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    bool deletePreset(const QString& key) override;
    void applyCurrentPreset() override;
    bool isPresetModified() override;

    QList<QPair<QString, QString>> getAvailablePresets() const override;
    LC_PresetManagerUIStrings presetStrings() const override;

    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;
    bool exportPresetToFile(const QString& key, const QString& filePath, QWidget* parent) override;

    void setTreeModel(LC_ShortcutsTreeModel* model);
protected:
    void applyActiveConfigToSystem(const QString& activeKey) override;
private:
    ShortcutsConfig collectCurrentConfig(const QString& name) const;

    LC_ActionGroupManager* m_groupManager = nullptr;
    LC_ShortcutsManager* m_shortcutsManager = nullptr;
    LC_ShortcutsRepository* m_repository = nullptr;
    LC_ShortcutsTreeModel* m_treeModel = nullptr;
};

#endif
