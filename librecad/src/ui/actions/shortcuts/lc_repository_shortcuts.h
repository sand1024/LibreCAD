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
 ******************************************************************************/

#ifndef LC_SHORTCUTS_REPOSITORY_H
#define LC_SHORTCUTS_REPOSITORY_H

#include <QKeySequence>
#include <QMap>
#include <QString>

#include "lc_preset_repository_base.h"

struct ShortcutsConfig {
    QString name;
    QMap<QString, QKeySequence> shortcuts;
};

class LC_RepositoryKeymaps : public LC_PresetRepositoryBase<ShortcutsConfig> {
public:
    explicit LC_RepositoryKeymaps(const QString& configDir);
    ~LC_RepositoryKeymaps() override = default;

    QJsonObject configToJson(const ShortcutsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, ShortcutsConfig& config) const override;
};

#endif
