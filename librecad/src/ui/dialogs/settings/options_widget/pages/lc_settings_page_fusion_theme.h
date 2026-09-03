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

#ifndef LC_SETTINGS_PAGE_FUSION_THEME_H
#define LC_SETTINGS_PAGE_FUSION_THEME_H

#include <memory>
#include "lc_settings_page_base.h"

class LC_UIStyleManager;

namespace Ui {
    class LC_SettingsPageFusionTheme;
}

class LC_SettingsPageFusionTheme : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageFusionTheme(QObject* parent = nullptr);
    ~LC_SettingsPageFusionTheme() override;


    void loadSettings() override;
    bool saveSettings() override;

    void setChildPages(const QList<LC_SettingsPageInterface*>& children) override;
    void onAboutToShow() override;

    bool isPageGated() const override;
    bool disablesWidgetOnGating() const override { return false; }
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;
    bool acceptsSharedPreview() override {return false;}
protected:
    void setupUi() override;
    void setupBehavior() override;
    void setupBindings() override;
private slots:
    void onControlChanged();

private:
    void setupThemeModeCombobox();
    void applyTransientTheme();
    void updateGatedControlsState() const;

    std::unique_ptr<Ui::LC_SettingsPageFusionTheme> ui;
    LC_UIStyleManager* m_styleManager = nullptr;
    bool m_blockSignals = false;
};

#endif
