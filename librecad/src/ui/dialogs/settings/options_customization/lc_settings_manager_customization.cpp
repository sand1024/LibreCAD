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

#include "lc_settings_manager_customization.h"

#include "lc_actiongroupmanager.h"
#include "lc_preset_manager_shortcuts.h"
#include "lc_settings_dialog.h"
#include "lc_settings_page_shortcuts.h"
#include "lc_settings_registry.h"
#include "lc_shortcuts_manager.h"

void LC_SettingsManagerCustomization::initialize(
    LC_ActionGroupManager* groupManager){
    using namespace LC_SettingsPagesCustomization;

        auto* reg = LC_SettingsRegistry::instance();
    reg->configureDialog(DLG_CUSTOMIZATION, {QObject::tr("Application Customization"), true, false});

    // 1. Register Shortcuts Preset Manager
    reg->registerPresetManager(DLG_CUSTOMIZATION, PAGE_SHORTCUTS, [groupManager]() {
        return std::make_unique<LC_PresetManagerShortcuts>(groupManager);
    });

    // 2. Register Shortcuts Settings Page

    const std::initializer_list<LC_SettingsRegistry::PageRegistration> pages = {
        {PAGE_SHORTCUTS, "", [groupManager]() {
            return std::make_unique<LC_SettingsPageShortcuts>(groupManager);
        }, 10}
    };

    reg->registerPages(DLG_CUSTOMIZATION, pages);
}

bool LC_SettingsManagerCustomization::showCustomizationDialog(QWidget* parent, const QString& initialPageId) {
    using namespace LC_SettingsPagesCustomization;
    return LC_SettingsRegistry::instance()->showDialog(
        DLG_CUSTOMIZATION,
        initialPageId.isEmpty() ? PAGE_SHORTCUTS : initialPageId,
        parent
    );
}
