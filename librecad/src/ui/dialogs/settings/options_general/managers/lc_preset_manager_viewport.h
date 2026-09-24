
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

#ifndef LC_PRESET_MANAGER_VIEWPORT_H
#define LC_PRESET_MANAGER_VIEWPORT_H

#include "lc_preset_manager_config_base.h"
#include "lc_repository_viewport_theme.h"

class QWidget;

class LC_PresetManagerViewport : public LC_PresetManagerConfigBase<LC_ViewportThemeConfig, LC_RepositoryViewportTheme> {
    Q_OBJECT
public:
    explicit LC_PresetManagerViewport(LC_RepositoryViewportTheme* repository,
                                      QObject* parent = nullptr,
                                      QWidget* previewWidget = nullptr);
    ~LC_PresetManagerViewport() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool loadPreset(const QString& key) override;
    bool isPresetModified() override;
    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;

protected:
    void prepareWorkingConfigForSave() override;
    void onPostSavePreset(const QString& outKey) override;
    void onPostApplyPreset() override;
    void applyActiveConfigToSystem(const QString& activeKey) override;

    bool resetToDefaults(const QString& key);
    bool calculateDivergence() const;
    QJsonObject serializeCurrentSettings() const;
    void applyThemeJson(const QJsonObject& obj);

private:
    QWidget* m_previewWidget = nullptr;
    bool m_registryDiverged = false;
};

#endif
