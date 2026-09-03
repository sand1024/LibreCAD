
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

#ifndef LC_SETTINGSMANAGERBASE_H
#define LC_SETTINGSMANAGERBASE_H

#include "lc_settings_page_index.h"
#include "lc_settings_registry.h"

class LC_SettingsManagerBase {
protected:
    template<class T>
    static LC_SettingsRegistry::PageCreator page() {
        return [] {return std::make_unique<T>();};
    }

    static LC_SettingsRegistry::PageCreator index(const QString& displayName, const QString& description) {
        return [displayName, description] {
            return std::make_unique<LC_SettingsPageIndex>(displayName, description);
        };
    }

    static LC_SettingsRegistry::PageCreator indexNoPreview(const QString& displayName, const QString& description) {
        return [displayName, description] {
            auto settingsPageIndex = std::make_unique<LC_SettingsPageIndex>(displayName, description);
            settingsPageIndex->setAcceptsSharedPreview(false);
            return settingsPageIndex;
        };
    }


    static LC_SettingsRegistry::PageCreator gatedIndex(
       const QString& displayName,
       const QString& description,
       std::function<bool()> isGatedFunc,
       std::function<QString()> messageFunc,
       const QString& actionText,
       std::function<void()> actionCallback) {
        return [=]() {
            auto indexPage = std::make_unique<LC_SettingsPageIndex>(displayName, description);
            indexPage->setGating(isGatedFunc, messageFunc, actionText, actionCallback);
            return indexPage;
        };
    }
};

#endif
