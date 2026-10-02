
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

#ifndef LC_SETTINGS_PAGE_TOOLBARS_AND_DOCKS_H
#define LC_SETTINGS_PAGE_TOOLBARS_AND_DOCKS_H

#include "lc_preset_manager_widgets.h"
#include "lc_settings_page_base.h"

namespace Ui {
    class LC_SettingsPageToolbarsAndDocks;
}

class LC_SettingsPageToolbarsAndDocks : public LC_SettingsPageBase, public LC_StylingPreviewAware {
    Q_OBJECT

public:
    explicit LC_SettingsPageToolbarsAndDocks(QObject* parent = nullptr);
    ~LC_SettingsPageToolbarsAndDocks() override;
    void bindToPresetManager(LC_PresetManagerInterface* manager);

    bool saveSettings() override;
    bool isModified() const;
    bool acceptsSharedPreview() override { return true; }
    void updateLivePreview() override;
    void setPreviewController(LC_StylingPreviewController* controller) override;
    QWidget* getBottomWidget() override;
protected:
    void onControlChanged();
    void setupUi() override;
    void setupBehavior() override;
    void loadSettings();

    void populateUiFromConfig();
    void syncUiToWorkingConfig();

    bool m_blockSignals = false;
    LC_UIStyleManager* m_styleManager = nullptr;
    LC_PresetManagerWidgets* m_presetManager = nullptr;

private:
    LC_StylingPreviewController* m_previewController = nullptr;
    std::unique_ptr<Ui::LC_SettingsPageToolbarsAndDocks> ui;
};
#endif
