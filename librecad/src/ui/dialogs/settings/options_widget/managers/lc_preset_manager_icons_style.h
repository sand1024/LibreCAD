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
#include "lc_icons_style_repository.h"
#include "lc_preset_manager_base.h"

class LC_PresetManagerIconsStyle : public LC_PresetManagerBase<IconStyleConfig, LC_IconsStyleRepository> {
    Q_OBJECT

public:
    explicit LC_PresetManagerIconsStyle(QObject* parent = nullptr);
    ~LC_PresetManagerIconsStyle() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    bool loadPreset(const QString& key) override;
    bool saveCurrentPreset() override;
    bool savePresetAs(const QString& name, QString& outKey) override;
    QString getAppliedPresetKey() const override;
    void applyCurrentPreset() override;
    void setPreviewController(LC_StylingPreviewController* controller) override;

    bool isGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;

    bool isCurrentVariantDark() const { return m_currentVariantDark; }
    void setCurrentVariantDark(bool dark);

    const LC_IconColorsOptions& iconColorsOptions() const { return m_iconColorsOptions; }
    LC_IconColorsOptions& iconColorsOptions() { return m_iconColorsOptions; }

    void applyTransientStyle();

    bool supportsPreviewWindow() const override { return false; }
    bool supportsAccessibilityCheck() const override { return true; }

signals:
    void configLoaded();
    void variantChanged(bool isDark);

protected:
    void updatePreview() override;
    void resetToDefaults(IconStyleConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;

private:
    LC_IconColorsOptions m_iconColorsOptions;
    bool m_currentVariantDark = false;
};

#endif
