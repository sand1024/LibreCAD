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

#include "lc_shortcuts_repository.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "lc_shortcutsstorage.h"
#include "rs_debug.h"

LC_ShortcutsRepository::LC_ShortcutsRepository(const QString& configDir)
    : LC_StyleRepositoryBase<ShortcutsConfig>(
          configDir, SHORTCUTS_EXTENSION, SHORTCUTS_FILE_IDENTIFIER, "shortcuts_index.lcix") {
    QDir().mkpath(configDir);
    initializeIndex();
}

QJsonObject LC_ShortcutsRepository::configToJson(const ShortcutsConfig& config) const {
    QJsonObject root;
    QJsonObject shortcutsObj;

    for (auto it = config.shortcuts.constBegin(); it != config.shortcuts.constEnd(); ++it) {
        if (!it.value().isEmpty()) {
            shortcutsObj[it.key()] = it.value().toString(QKeySequence::PortableText);
        }
    }
    root["shortcuts"] = shortcutsObj;
    return root;
}

bool LC_ShortcutsRepository::configFromJson(const QJsonObject& json, ShortcutsConfig& config) const {
    config.shortcuts.clear();

    const QJsonObject shortcutsObj = json["shortcuts"].toObject();
    for (auto it = shortcutsObj.constBegin(); it != shortcutsObj.constEnd(); ++it) {
        const QString keyStr = it.value().toString();
        if (!keyStr.isEmpty()) {
            config.shortcuts.insert(it.key(), QKeySequence(keyStr, QKeySequence::PortableText));
        }
    }
    return true;
}

void LC_ShortcutsRepository::migrateLegacyShortcutsIfNeeded(const QString& legacyFolder) {
    if (!getAvailableNames().isEmpty()) {
        return; // Presets already exist; migration previously completed
    }

    QString legacyFilePath = QDir::toNativeSeparators(legacyFolder + "/shortcuts.lcsc");
    if (!QFile::exists(legacyFilePath)) {
        legacyFilePath = QDir::toNativeSeparators(legacyFolder + "/shortcuts.lcs");
        if (!QFile::exists(legacyFilePath)) {
            return;
        }
    }

    QMap<QString, QKeySequence> legacyShortcuts;
    const int loadResult = LC_ShortcutsStorage::loadShortcuts(legacyFilePath, &legacyShortcuts);

    if (loadResult != LC_ShortcutsStorage::OK || legacyShortcuts.isEmpty()) {
        return;
    }

    ShortcutsConfig migratedConfig;
    migratedConfig.name = QObject::tr("Imported User Shortcuts");
    migratedConfig.shortcuts = legacyShortcuts;

    QString outKey;
    if (save(migratedConfig.name, migratedConfig, outKey)) {
        LC_ERR << "LC_ShortcutsRepository: Successfully migrated legacy XML shortcuts to JSON preset:" << outKey;
        QFile::rename(legacyFilePath, legacyFilePath + ".migrated.bak");
        initializeIndex();
    }
}
