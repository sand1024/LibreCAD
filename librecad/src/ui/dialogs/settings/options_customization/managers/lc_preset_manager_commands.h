
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

#include <memory>
#include "lc_action_type_mapper.h"
#include "lc_commandItems.h"
#include "lc_command_manager.h"
#include "lc_preset_manager_config_base.h"
#include "lc_repository_commands.h"

class LC_ActionGroupManager;

class LC_PresetManagerCommands : public LC_PresetManagerConfigBase<CommandsConfig, LC_RepositoryCommands> {
    Q_OBJECT
public:
    explicit LC_PresetManagerCommands(LC_CommandManager* commandManager, LC_ActionGroupManager* agm, QObject* parent = nullptr);
    ~LC_PresetManagerCommands() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool loadPreset(const QString& key) override;
    void rollbackState() override;
    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;

    void notifyConfigChanged();
    void updateActionForCommandsInMenu();

    LC_ActionTypeMapper* getActionTypeMapper() const {
        return m_actionTypeMapper.get();
    }

protected:
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void onPostApplyPreset() override;

    bool importLegacyAliasFile(const QString& filePath);

private:
    LC_CommandManager* m_commandManager = nullptr;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
    std::unique_ptr<LC_ActionTypeMapper> m_actionTypeMapper;
};

#endif
