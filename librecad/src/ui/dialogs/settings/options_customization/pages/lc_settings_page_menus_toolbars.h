
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

#ifndef LC_SETTINGS_PAGE_MENUS_TOOLBARS_H
#define LC_SETTINGS_PAGE_MENUS_TOOLBARS_H

#include <memory>
#include "lc_settings_page_base.h"

namespace Ui {
    class LC_SettingsPageMenusToolbars;
}

class LC_ActionGroupManager;
class LC_PresetManagerMenusToolbars;

class LC_SettingsPageMenusToolbars : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageMenusToolbars(LC_ActionGroupManager* groupManager, QObject* parent = nullptr);
    ~LC_SettingsPageMenusToolbars() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
protected:
    void setupUi() override;
    void setupBehavior() override;
    void setReadOnly(bool readOnly) override;
private slots:
    void onTargetSelected(int index);
    void onActionsModified();
    void onDockAreaChanged(int index);
    void onMakeActiveMenuClicked();
    void onNewToolbarClicked();
    void onRenameToolbarClicked();
    void onDeleteToolbarClicked();

private:
    enum ContainerKind {
      MenuCompact,
      MenuCompactTools,
      MenuExtended,
      ToolbarItem,
      CadDockWidgetItem,
      CadMatrixItem
    };

    struct ComboEntry {
        ContainerKind kind = ToolbarItem;
        int toolbarIndex = -1;
    };

    std::unique_ptr<Ui::LC_SettingsPageMenusToolbars> ui;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
    LC_PresetManagerMenusToolbars* m_presetManager = nullptr;
    int m_currentComboIndex = -1;
    int m_currentIndex = -1;
    bool m_blockSignals = false;

    void updateControlsState(ContainerKind kind, int tbIdx, bool isReadOnly);
    void loadContainerContent(ContainerKind kind, int tbIdx);
    
    void populateCombobox();
    void syncUiFromWorkingConfig();
    void syncCurrentContainerToConfig();
    void markModified();
    void updateActiveMenuBold();
    void updateEditorState(ContainerKind kind, int tbIdx, bool readOnly);
    bool promptForUniqueName(const QString& title, const QString& label, const QStringList& existingNames,
                             const QString& initialSuggestion, QString& outName);
};

#endif
