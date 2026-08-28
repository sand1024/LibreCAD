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

#ifndef LC_SETTINGS_PAGE_METRICS_VIEWS_TABS_H
#define LC_SETTINGS_PAGE_METRICS_VIEWS_TABS_H

#include "lc_settings_page_base.h"

namespace Ui {
    class LC_SettingsPageMetricsViewsTabs;
}

class LC_PresetManagerMetrics;

class LC_SettingsPageMetricsViewsTabs : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageMetricsViewsTabs(QObject* parent = nullptr);
    ~LC_SettingsPageMetricsViewsTabs() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;

protected:
    void setupUi() override;
    void setupBehavior() override;

private slots:
    void onControlChanged();

private:
    void populateUiFromWorkingConfig();
    void syncUiToWorkingConfig();

    LC_PresetManagerMetrics* m_presetManager = nullptr;
    std::unique_ptr<Ui::LC_SettingsPageMetricsViewsTabs> ui;
    bool m_blockSignals = false;
};

#endif
