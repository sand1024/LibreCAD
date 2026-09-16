
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

#ifndef LC_REPOSITORY_COMMANDS_H
#define LC_REPOSITORY_COMMANDS_H

#include <QJsonObject>
#include "lc_command_types.h"
#include "lc_style_repository_base.h"

inline constexpr const char* COMMANDS_EXTENSION = ".lccmd";
inline constexpr const char* COMMANDS_FILE_IDENTIFIER = "LibreCAD_Command_Aliases";
inline constexpr const char* COMMANDS_INDEX_FILE = "commands_index.lcix";

class LC_RepositoryCommands : public LC_PresetRepositoryBase<CommandsConfig> {
public:
    explicit LC_RepositoryCommands(const QString& configDir);
    ~LC_RepositoryCommands() override = default;

    QJsonObject configToJson(const CommandsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, CommandsConfig& config) const override;
};

#endif
