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

#include "lc_preset_manager_interface.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>
#include "lc_inputtextdialog.h"
#include "lc_palette_editor_shared.h"

QString LC_PresetManagerInterface::currentPresetDisplayName() const {
    const QString activeKey = getActivePresetKey();
    if (isReadOnlyDefault()) {
        return defaultPresetDisplayName();
    }

    const auto presets = getAvailablePresets();
    for (const auto& p : presets) {
        if (p.second == activeKey) {
            return p.first;
        }
    }
    return QFileInfo(activeKey).completeBaseName();
}

bool LC_PresetManagerInterface::promptSavePresetAs(QWidget* parentWidget, QString* outNewKey) {
    const LC_PresetManagerUIStrings strings = presetStrings();
    const auto presets = getAvailablePresets();
    QStringList existingNames;
    for (const auto& p : presets) {
        existingNames << p.first;
    }

    bool ok = false;
    QString name;
    QString suggestedName = currentPresetDisplayName();
    if (suggestedName == QObject::tr("Default") || getActivePresetKey() == DEFAULT_THEME_KEY) {
        suggestedName = strings.defaultNewPresetName;
    }
    bool isNotUnique = false;
    int i = 1;

    do {
        name = LC_InputTextDialog::getText(parentWidget, strings.saveAsDialogTitle, strings.saveAsDialogLabel,
                                           existingNames, true, suggestedName, &ok);
        name = name.trimmed();

        if (ok) {
            if (name.isEmpty()) {
                isNotUnique = true;
                continue;
            }
            isNotUnique = existingNames.contains(name, Qt::CaseInsensitive);
            if (isNotUnique) {
                suggestedName = name + "_" + QString::number(i++);
            }
        }
        else {
            return false; // User canceled
        }
    } while (isNotUnique);

    QString newKey;
    if (savePresetAs(name, newKey)) {
        if (outNewKey != nullptr) {
            *outNewKey = newKey;
        }
        return true;
    }
    return false;
}

bool LC_PresetManagerInterface::handlePromptSaveOnAccept(QWidget* parentWidget) {
    if (!isPresetModified() || isReadOnlyDefault()) {
        return true;
    }

    const LC_PresetManagerUIStrings strings = presetStrings();
    const QString currentName = currentPresetDisplayName();

    const auto reply = QMessageBox::question(
        parentWidget, strings.saveModifiedPromptTitle,
        strings.saveModifiedPromptMessage.arg(currentName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Save) {
        return saveCurrentPreset();
    }
    else if (reply == QMessageBox::Cancel) {
        return false; // Abort accept, keep dialog open
    }
    return true; // Discard -> proceed with accept
}

bool LC_PresetManagerInterface::handlePromptDiscardOnReject(QWidget* parentWidget) {
    if (!isPresetModified() || isReadOnlyDefault()) {
        return true;
    }

    const LC_PresetManagerUIStrings strings = presetStrings();
    const QString currentName = currentPresetDisplayName();
    const auto reply = QMessageBox::question(
        parentWidget, strings.discardConfirmTitle,
        strings.discardConfirmMessage.arg(currentName),
        QMessageBox::Yes | QMessageBox::No);

    return (reply == QMessageBox::Yes);
}
