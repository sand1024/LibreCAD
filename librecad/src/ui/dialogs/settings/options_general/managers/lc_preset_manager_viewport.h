
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

#ifndef LC_VIEWPORTTHEMEEDITOR_H
#define LC_VIEWPORTTHEMEEDITOR_H

#include "lc_abstract_preset_manager.h"
#include "lc_preset_manager_interface.h"
#include "lc_viewport_theme_repository.h"

class LC_PresetManagerViewport : public LC_AbstractPresetManager {
    Q_OBJECT
public:
    explicit LC_PresetManagerViewport(QObject* parent = nullptr, QWidget* previewWidget = nullptr);
    ~LC_PresetManagerViewport() override = default;

    // --- Domain-Specific Preset Lifecycle ---
    bool isPresetModified() override;
    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    bool deletePreset(const QString& key) override;
    void applyCurrentPreset() override;
    bool resetToDefaults(const QString& key);

    QList<QPair<QString, QString>> getAvailablePresets() const override;
    LC_PresetManagerUIStrings presetStrings() const override;

    QWidget* getSharedPreviewWidget() override { return m_previewWidget; }
    bool importPresetFromFile(const QString& filePath, QWidget* parent) override;
    bool exportPresetToFile(const QString& key, const QString& filePath, QWidget* parent) override;
protected:
    void applyActiveConfigToSystem(const QString& activeKey) override;
private:
    QJsonObject serializeCurrentSettings() const;
    void applyThemeJson(const QJsonObject& obj);
    bool calculateDivergence() const;

    LC_ViewportThemeRepository m_repository;
    bool m_registryDiverged = false;
    QWidget* m_previewWidget = nullptr;
};

#endif
