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

#ifndef LC_PRESET_MANAGER_WIDGETS_H
#define LC_PRESET_MANAGER_WIDGETS_H

#include "lc_preset_manager_styling_base.h"
#include "lc_repository_widgets.h"

class LC_PresetManagerWidgets : public LC_PresetManagerStylingBase<LC_WidgetsConfig, LC_RepositoryWidgets> {
    Q_OBJECT

public:
    explicit LC_PresetManagerWidgets(LC_UIStyleManager* styleManager);
    ~LC_PresetManagerWidgets() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    QString getAppliedPresetKey() const override;

    bool isFusionGated() const override { return false; }

    signals:
        void configLoaded(const LC_WidgetsConfig& config);

protected:
    void resetToDefaults(LC_WidgetsConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
    void updatePreview() override;
    void emitConfigLoaded() override;
    QString fusionGatingSubject() const override { return QString(); }
};

#endif
