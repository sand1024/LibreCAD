
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

#ifndef LC_SETTINGS_PAGE_CUSTOM_MENUS_H
#define LC_SETTINGS_PAGE_CUSTOM_MENUS_H

#include <memory>
#include "lc_settings_page_base.h"

struct ContextMenuDef;

namespace Ui {
    class LC_SettingsPageCustomMenus;
}

class LC_ActionGroupManager;
class LC_PresetManagerMenus;
class LC_MenuActivator;

class LC_SettingsPageCustomMenus : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageCustomMenus(LC_ActionGroupManager* groupManager, QObject* parent = nullptr);
    ~LC_SettingsPageCustomMenus() override;

    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    bool acceptsSharedPreview() override { return false; }

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
protected:
    void setupUi() override;
    void setupBehavior() override;
    void setReadOnly(bool readOnly) override;
    void markModified();
    bool promptForUniqueName(const QString& title,
                            const QString& label,
                            const QStringList& existingNames,
                            const QString& initialSuggestion,
                            QString& outName);

    QString formatMenuDisplayName(const QString& menuName, const QString& activatorShortcut) const;
    bool isBuiltInContextMenu(int menuIndex) const;
private slots:
    void onMenuSelected(int index);
    void onNewMenuClicked();
    void onSaveAsMenuClicked();
    void onDeleteMenuClicked();
    void onContentModified();

private:
    enum class MenuCategory {
        DrawingArea,
        Geometry,
        Annotations,
        BlocksMedia,
        FallbackGestures
    };

    MenuCategory getMenuCategory(const ContextMenuDef& menuDef) const;
    QString formatMenuDisplayName(const ContextMenuDef& menuDef) const;
    int findRowForMenuIndex(int menuIdx) const;
    void selectMenuByIndex(int menuIdx);
    void updateCurrentMenuInCombobox();

    void syncCurrentMenuToConfig();
    void syncUiFromWorkingConfig();
    std::unique_ptr<Ui::LC_SettingsPageCustomMenus> ui;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
    LC_PresetManagerMenus* m_presetManager = nullptr;
    int m_currentMenuIndex = -1;
    bool m_blockSignals = false;
};

#endif
