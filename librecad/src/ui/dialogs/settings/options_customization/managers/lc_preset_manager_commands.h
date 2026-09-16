
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

#ifndef LC_PRESET_MANAGER_COMMANDS_H
#define LC_PRESET_MANAGER_COMMANDS_H

#include "lc_abstract_preset_manager.h"
#include "lc_action_type_mapper.h"
#include "lc_command_types.h"
#include "lc_repository_commands.h"

class LC_CommandManager;
class LC_ActionGroupManager;

class LC_PresetManagerCommands : public LC_AbstractPresetManager {
    Q_OBJECT
public:
    explicit LC_PresetManagerCommands(LC_CommandManager* commandManager,LC_ActionGroupManager* agm, QObject* parent = nullptr);
    ~LC_PresetManagerCommands() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    void updateActionForCommandsInMenu();

    void applyActiveConfigToSystem(const QString& activeKey) override;
    void applyCurrentPreset() override;
    void rollbackState() override;

    bool isPresetModified() override;

    QList<QPair<QString, QString>> getAvailablePresets() const override;

    bool importPresetFromFile(const QString& filePath, QWidget* parent = nullptr) override;
    bool exportPresetToFile(const QString& key, const QString& filePath, QWidget* parent = nullptr) override;

    CommandsConfig& workingConfig() { return m_workingConfig; }
    const CommandsConfig& workingConfig() const { return m_workingConfig; }

    LC_ActionTypeMapper* getActionTypeMapper() const { return m_actionTypeMapper.get(); }
    void notifyConfigChanged();

protected:
    bool doDeletePreset(const QString& key) override;

private:
    bool importLegacyAliasFile(const QString& filePath);

    CommandsConfig m_workingConfig;
    LC_RepositoryCommands* m_repository{nullptr};
    LC_CommandManager* m_commandManager{nullptr};
    std::unique_ptr<LC_ActionTypeMapper> m_actionTypeMapper;
    LC_ActionGroupManager* m_actionGroupManager{nullptr};
};

#endif
