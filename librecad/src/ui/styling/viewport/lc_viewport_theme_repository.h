
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

#ifndef LC_VIEWPORT_THEME_EDITOR_H
#define LC_VIEWPORT_THEME_EDITOR_H

#include <QObject>
#include "lc_preset_manager_interface.h"
#include "lc_setting.h"
#include "lc_settings_types.h"
#include "lc_style_repository_base.h"


struct LC_ViewportThemeConfig {
    QString name;
    QJsonObject rootObject;
};

class LC_SettingBase;
// Pure structure definition - no global variable instances declared here
struct LC_ViewportThemeMapEntry {
    QString jsonPath;
    const LC_SettingBase* setting;
    LC_ViewportThemeComponent component  = LC_ViewportThemeComponent::Style;

    LC_ViewportThemeMapEntry(const QString pathInJson, const LC_SettingBase& settingValue, LC_ViewportThemeComponent comp) {
          jsonPath = pathInJson;
          setting = &settingValue;
          component = comp;
    }

    LC_ViewportThemeMapEntry(const LC_SettingBase& settingValue, LC_ViewportThemeComponent comp) {
        jsonPath = settingValue.group() + "." + settingValue.key();
        setting = &settingValue;
        component = comp;
    }


    LC_ViewportThemeMapEntry(const QString pathInJson, const LC_SettingBase& settingValue) {
        jsonPath = pathInJson;
        setting = &settingValue;
    }

    LC_ViewportThemeMapEntry(const LC_SettingBase& settingValue) {
        jsonPath = settingValue.group() + "." + settingValue.key();
        setting = &settingValue;
    }
};

class LC_ViewportThemeRepository : public LC_StyleRepositoryBase<LC_ViewportThemeConfig> {
public:
    LC_ViewportThemeRepository();
    ~LC_ViewportThemeRepository() override = default;

    QJsonObject configToJson(const LC_ViewportThemeConfig& config) const override;
    bool configFromJson(const QJsonObject& json, LC_ViewportThemeConfig& config) const override;
};


#endif
