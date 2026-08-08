
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
#ifndef LC_SETTINGS_REGISTRY_H
#define LC_SETTINGS_REGISTRY_H

#include <QString>
#include <QMap>
#include <vector>
#include <functional>
#include <memory>

#include "lc_preset_manager_interface.h"
#include "lc_settings_page_interface.h"
#include "lc_style_editor_interface.h"

class LC_SettingsRegistry {
public:
    using PageCreator = std::function<std::unique_ptr<LC_SettingsPageInterface>()>;
    using PresetManagerCreator = std::function<std::unique_ptr<LC_PresetManagerInterface>()>;

    static LC_SettingsRegistry* instance();

    struct PageRegistration {
        QString id;
        QString parentId;
        PageCreator creator;
        int weight = -1;
    };

    void configureDialog(const QString& dialogId, const QString& title, bool useGlobalTransaction = true);
    void registerPage(const QString& dialogId, const QString& id, const QString& parentId, const PageCreator& creator);
    void registerPages(const QString& dialogId, const std::initializer_list<PageRegistration>& pages);
    void registerPresetManager(const QString& dialogId, const QString& groupPathId, const PresetManagerCreator& creator);
    bool showDialog(const QString& dialogId, const QString& initialPageId = QString(), QWidget* parent = nullptr);
private:
    LC_SettingsRegistry() = default;
    ~LC_SettingsRegistry() = default;

    struct DialogProperties {
        QString title = "Settings";
        bool useGlobalTransaction = true;
    };

    struct DialogRegistration {
        DialogProperties properties;
        std::vector<PageRegistration> pageCreators;
        QMap<QString, PresetManagerCreator> presetCreators;
    };

    QMap<QString, DialogRegistration> m_registrations;
};
#endif
