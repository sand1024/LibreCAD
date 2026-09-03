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

#ifndef LC_PRESET_MANAGER_PALETTE_H
#define LC_PRESET_MANAGER_PALETTE_H

#include "lc_palette_repository.h"
#include "lc_preset_manager_base.h"

class LC_PresetManagerPalette : public LC_PresetManagerBase<PaletteConfig, LC_PaletteRepository> {
    Q_OBJECT

public:
    explicit LC_PresetManagerPalette(QObject* parent = nullptr);
    ~LC_PresetManagerPalette() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    bool loadPreset(const QString& key) override;
    QString getAppliedPresetKey() const override;
    void applyCurrentPreset() override;
    void setPreviewController(LC_StylingPreviewController* controller) override;

    bool isGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;

    bool isCurrentVariantDark() const { return m_currentVariantDark; }
    void setCurrentVariantDark(bool dark);

    void onSubPageControlChanged() {
        notifyWorkingConfigChanged();
    }

    void calculateProceduralBevels(bool isDarkMode, StyleArchetype archetype);
    void generateHarmonizedTheme(const QColor& baseColor);
    void generateTwoColorTheme(const QColor& surface, const QColor& accent);
    void generateHighContrastTheme(const QColor& baseColor);
    void activatePreviewTab(const QString& tag);

    signals:
        void configLoaded(const PaletteConfig& config);
    void variantChanged(bool isDark);

protected:
    void updatePreview() override;
    void resetToDefaults(PaletteConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;

private:
    bool m_currentVariantDark = false;
};

#endif
