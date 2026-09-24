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

#ifndef LC_PRESET_MANAGER_ICONS_STYLE_H
#define LC_PRESET_MANAGER_ICONS_STYLE_H

#include "lc_icon_colors_options.h"
#include "lc_preset_manager_styling_base.h"
#include "lc_repository_icons_style.h"

class LC_PresetManagerIconsStyle : public LC_PresetManagerStylingBase<IconStyleConfig, LC_RepositoryIconsStyle> {
    Q_OBJECT
public:
    explicit LC_PresetManagerIconsStyle(LC_UIStyleManager* styleManager);
    ~LC_PresetManagerIconsStyle() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;

    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    QString getAppliedPresetKey() const override;

    void setPreviewController(LC_StylingPreviewController* controller) override;
    void setCurrentVariantDark(bool dark);
    bool isCurrentVariantDark() const { return m_currentVariantDark; }

    LC_IconColorsOptions& iconColorsOptions() { return m_iconColorsOptions; }
    const LC_IconColorsOptions& iconColorsOptions() const { return m_iconColorsOptions; }
    void applyTransientStyle();

    signals:
        void configLoaded();
    void variantChanged(bool isDark);

protected:
    void resetToDefaults(IconStyleConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void onPostApplyPreset() override;
    void onPostLoadPreset() override;
    void emitConfigLoaded() override;
    void updatePreview() override;
    QString fusionGatingSubject() const override { return QString(); }

private:
    LC_IconColorsOptions m_iconColorsOptions;
    bool m_currentVariantDark = false;
};

#endif
