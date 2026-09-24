
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

#ifndef LC_PRESET_MANAGER_MENUS_H
#define LC_PRESET_MANAGER_MENUS_H


#include "lc_action_node.h"
#include "lc_preset_manager_config_base.h"
#include "lc_repository_graphic_view_context_menus.h"

class LC_ActionFactory;
class LC_ActionGroupManager;
class LC_GraphicViewContextMenuProvider;

class LC_PresetManagerMenus : public LC_PresetManagerConfigBase<ContextMenusConfig, LC_RepositoryGraphicViewContextMenus> {
    Q_OBJECT
public:
    static constexpr const char* THEME_EXTENDED_KEY = "extended";

    LC_PresetManagerMenus(LC_GraphicViewContextMenuProvider* provider,
                          LC_ActionFactory* actionFactory,
                          LC_ActionGroupManager* groupManager,
                          QObject* parent = nullptr);
    ~LC_PresetManagerMenus() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool isDefaultPreset(const QString& key) const override;
    bool loadPreset(const QString& key) override;
    void rollbackState() override;

protected:
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void onPostApplyPreset() override;

    void prependDefaultPresets(QList<QPair<QString, QString>>& choices) const override;

private:
    LC_GraphicViewContextMenuProvider* m_contextMenuProvider = nullptr;
    LC_ActionFactory* m_actionFactory = nullptr;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
};

#endif
