
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

#include "lc_abstract_preset_manager.h"
#include "lc_repository_graphic_view_context_menus.h"

class LC_GraphicViewContextMenuProvider;
class LC_NavigationControlsCreator;

class LC_PresetManagerMenus : public LC_AbstractPresetManager {
    Q_OBJECT
public:
    explicit LC_PresetManagerMenus(LC_GraphicViewContextMenuProvider* provider, QObject* parent = nullptr);
    ~LC_PresetManagerMenus() override = default;

    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    void applyCurrentPreset() override;
    void rollbackState() override;

    QList<QPair<QString, QString>> getAvailablePresets() const override;
    LC_PresetManagerUIStrings presetStrings() const override;

    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;
    bool exportPresetToFile(const QString& key, const QString& filePath, QWidget* parent) override;

    ContextMenusConfig& workingConfig() { return m_workingConfig; }
    const ContextMenusConfig& workingConfig() const { return m_workingConfig; }

protected:
    void applyActiveConfigToSystem(const QString& activeKey) override;
    bool doDeletePreset(const QString& key) override;
private:
    LC_GraphicViewContextMenuProvider* m_contextMenuProvider{nullptr};
    LC_RepositoryGraphicViewContextMenus* m_repository;
    ContextMenusConfig m_workingConfig;
};

#endif
