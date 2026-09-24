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

#ifndef LC_PRESET_MANAGER_TYPOGRAPHY_H
#define LC_PRESET_MANAGER_TYPOGRAPHY_H

#include "lc_preset_manager_styling_base.h"
#include "lc_repository_typography.h"

class LC_PresetManagerTypography : public LC_PresetManagerStylingBase<FontConfig, LC_RepositoryTypography> {
    Q_OBJECT
public:
    explicit LC_PresetManagerTypography(LC_UIStyleManager* styleManager);
    ~LC_PresetManagerTypography() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    QString getAppliedPresetKey() const override;

    signals:
        void configLoaded(const FontConfig& config);

protected:
    void resetToDefaults(FontConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void updatePreview() override;
    void emitConfigLoaded() override;
    QString fusionGatingSubject() const override { return QString(); }
};

#endif
