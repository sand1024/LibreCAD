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

#ifndef LC_PRESET_MANAGER_FUSION_SKIN_H
#define LC_PRESET_MANAGER_FUSION_SKIN_H

#include <QObject>
#include <memory>
#include "lc_palette_editor_shared.h"
#include "lc_preset_manager_base.h"
#include "lc_preset_manager_interface.h"
#include "lc_repository_fusion_skin.h"

class LC_UIStyleManager;
class LC_RepositoryFusionSkin;
class LC_SkinArchetypeHeaderBar;

class LC_PresetManagerFusionSkin : public LC_PresetManagerBase<ControlStyleConfig, LC_RepositoryFusionSkin>  {
    Q_OBJECT

public:
    explicit LC_PresetManagerFusionSkin(QObject* parent = nullptr);
    ~LC_PresetManagerFusionSkin() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    bool loadPreset(const QString& key) override;
    QString getAppliedPresetKey() const override;
    void applyCurrentPreset() override;
    QWidget* getSharedHeaderWidget() override;

    bool isGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;

    bool isClassicFusion() const {
        return m_workingConfig.styleArchetype == StyleArchetype::ClassicFusion;
    }

    void onSubPageControlChanged() {
        notifyWorkingConfigChanged();
    }

    signals:
        void configLoaded(const ControlStyleConfig& config);

private slots:
    void onArchetypeChanged(StyleArchetype archetype);
    void onDecorationChanged(BoxDecoration decoration);

protected:
    void updatePreview() override;
    void resetToDefaults(ControlStyleConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;

private:
    std::unique_ptr<LC_SkinArchetypeHeaderBar> m_headerBar;
};

#endif
