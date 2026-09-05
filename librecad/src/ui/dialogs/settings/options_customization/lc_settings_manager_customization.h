
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

#ifndef LC_SETTINGS_MANAGER_CUSTOMIZATION_H
#define LC_SETTINGS_MANAGER_CUSTOMIZATION_H

#include <QString>

class QWidget;
class LC_ActionGroupManager;
class LC_ShortcutsManager;

namespace LC_SettingsPagesCustomization {
    inline const QString DLG_CUSTOMIZATION = "DlgCustomization";
    inline const QString PAGE_SHORTCUTS    = "Shortcuts";
}

class LC_SettingsManagerCustomization {
public:
    static void initialize(LC_ActionGroupManager* groupManager);

    static bool showCustomizationDialog(QWidget* parent = nullptr,
                                        const QString& initialPageId = "");
};

#endif
