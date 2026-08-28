/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#ifndef LC_WIDGETOPTIONSDIALOG_H
#define LC_WIDGETOPTIONSDIALOG_H

#include "lc_dialog.h"
#include "lc_icon_colors_options.h"
#include "lc_icons_style_repository.h"
#include "ui_lc_widgetoptionsdialog.h"

class LC_UIStyleManager;
class LC_FusionSkinsRepository;

class LC_WidgetOptionsDialog : public LC_Dialog, public Ui::LC_WidgetOptionsDialog {
    Q_OBJECT
public:
    LC_WidgetOptionsDialog(QWidget* parent, LC_UIStyleManager* styleManager);
    ~LC_WidgetOptionsDialog() override = default;

    void reject() override;

public slots:
    void accept() override;
    void chooseStyleSheet();
    void updateSkinComboFonts() const;
    void updateIconStyleComboFonts() const;
    void updateTypographyComboFonts() const;
    void updateMetricsComboFonts() const;

private slots:
    void updateStyleDependencyStates();
    void setIconsOverrideFolder();

    // Pluggable preset launchers
    void onEditSkinsClicked();
    void onEditIconStylesClicked();
    void onEditTypographyClicked();
    void onEditMetricsClicked();

    // 1-Click consolidated workspace profiles and test generators
    void onImportProfileClicked();
    void onExportProfileClicked();
    void applyTransientStylePreview();
private:
    void setupGenericTabUI() const;
    void setupThemeModeOverrideCombobox();
    void populateSkinsCombobox();
    void populateIconsStyleCombobox();
    void populateTypographyCombobox();
    void setComboboxToActive(QString activeIconStyleKey, QComboBox* cb);
    void populateMetricsCombobox();
    void populateDropdowns();
    QString selectFolder(const QString& title);

    // Legacy CAD widget setups
    void setupMegaCADBarSettingUI() const;
    void setupCADBarSettingsUI() const;
    void setupDockWidgetSettingsUI() const;
    void setupToolbarsSettingsUI() const;
    void setupStatusBarSettingsUI() const;
    void setupDockingSettingsUI() const;

    LC_UIStyleManager* m_styleManager;
    LC_IconColorsOptions m_iconColorsOptions;

    // Symmetrical rollbacks: original active presets captured on startup
    QString m_origSkinKey;
    QString m_origIconStyleKey;
    QString m_origTypographyKey;
    QString m_origMetricsKey;

    bool    m_origAllowStyle = false;
    QString m_origStyle;
    ThemeModeOverride  m_origThemeMode = ThemeModeOverride::FollowSystem;
    QString m_origStyleSheet;
    bool    m_origIgnoreIconStyling = false;
};

#endif
