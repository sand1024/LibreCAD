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

#include "lc_style_repository_base.h"

inline const QString SHORTCUTS_EXTENSION = ".lcsc";
inline const QString SHORTCUTS_FILE_IDENTIFIER = "LibreCAD Shortcuts Config v_1";

struct ShortcutsConfig {
    QString name;
    QMap<QString, QKeySequence> shortcuts; // ActionName -> QKeySequence
};

class LC_ShortcutsRepository : public LC_StyleRepositoryBase<ShortcutsConfig> {
public:
    explicit LC_ShortcutsRepository(const QString& configDir);
    ~LC_ShortcutsRepository() override = default;

    QJsonObject configToJson(const ShortcutsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, ShortcutsConfig& config) const override;

    // Automatic one-time migration of legacy shortcuts.lcsc / shortcuts.lcs XML files
    void migrateLegacyShortcutsIfNeeded(const QString& legacyFolder);
};

#endif
