
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

#ifndef LC_COMMAND_MANAGER_H
#define LC_COMMAND_MANAGER_H

#include <map>
#include <memory>
#include <QStringList>

#include "lc_command_types.h"
#include "rs.h"

class LC_ActionTypeMapper;
class LC_RepositoryCommands;

class LC_CommandManager {
public:
    explicit LC_CommandManager(LC_RepositoryCommands* repo);
    ~LC_CommandManager();

    LC_CommandManager(const LC_CommandManager&) = delete;
    LC_CommandManager& operator=(const LC_CommandManager&) = delete;

    // --- Runtime CAD Execution ---
    RS2::ActionType cmdToAction(const QString& cmd, bool verbose = true, QString* outAmbiguityDetails = nullptr) const;
    RS2::ActionType keycodeToAction(const QString& code) const;
    bool checkCommand(const QString& keyword, const QString& input, RS2::ActionType action = RS2::ActionNone) const;
    QString command(const QString& cmd) const;
    QStringList complete(const QString& prefix) const;
    QString getCommandForAction(RS2::ActionType action) const;
    QStringList getCommandsForAction(RS2::ActionType action) const;

    QString msgAvailableCommands() const;

    // --- Scheme Lifecycle & Repository ---
    void loadActiveScheme(const LC_ActionTypeMapper* mapper);
    void applyCommandsScheme(const CommandsConfig& config, const LC_ActionTypeMapper* mapper);
    LC_RepositoryCommands* getRepository() const;
    const CommandsConfig& activeConfig() const { return m_activeConfig; }

private:
    RS2::ActionType commandToAction(const QString& cmd) const;
    void populateFactoryDefaults();

    LC_RepositoryCommands* m_repository;
    std::map<QString, RS2::ActionType> m_mainCommands;
    std::map<QString, RS2::ActionType> m_shortCommands;
    std::map<RS2::ActionType, QString> m_actionToCommand;
    std::map<QString, QString> m_cmdTranslation;
    std::map<QString, QString> m_revTranslation;
    CommandsConfig m_activeConfig;
};

#endif
