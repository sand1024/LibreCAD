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

#ifndef LC_SETTINGS_PAGE_SHORTCUTS_H
#define LC_SETTINGS_PAGE_SHORTCUTS_H

#include <memory>
#include "lc_settings_page_base.h"

class LC_ShortcutInfo;

namespace Ui {
class LC_SettingsPageShortcuts;
}

class LC_ActionGroupManager;
class LC_ShortcutsTreeModel;
class LC_ShortcutTreeItem;
class LC_PresetManagerShortcuts;
class LC_ShortcutSearchPopup;

class LC_SettingsPageShortcuts : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageShortcuts(LC_ActionGroupManager* groupManager, QObject* parent = nullptr);
    ~LC_SettingsPageShortcuts() override;

    // --- Settings Framework Lifecycle ---
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    bool validate(QString& outErrorMessage) override;
    bool acceptsSharedPreview() override { return false; }
    // --- Search & Indexing Overrides ---
    void autoIndexLabels() override;
    void highlightSearchPattern(const QString& pattern) override;
    void clearSearchHighlight() override;
    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
protected:
    void setupUi() override;
    void setupBehavior() override;
private slots:
    void onFilteringMaskChanged();
    void onShortcutFilterChanged(const QKeySequence& sequence);
    void onTreeSelectionChanged();
    void onRecordToggled(bool recording);
    void onKeySequenceRecorded(const QKeySequence& key);
    void onResetItemClicked();
    void onClearItemClicked();
    void onShowConflictsRequested();
    void onControlChanged();
    void onTreeContextMenuRequested(const QPoint& pos);
    void onResetAllClicked();
private:
    void rebuildTree(bool restoreSelection);
    void selectItem(LC_ShortcutTreeItem* item, int row, int parentRow);
    bool checkHasCollisions(LC_ShortcutInfo* shortcutInfo) const;

    std::unique_ptr<Ui::LC_SettingsPageShortcuts> ui;
    LC_ActionGroupManager* m_actionGroupManager = nullptr;
    LC_ShortcutsTreeModel* m_mappingTreeModel = nullptr;
    LC_PresetManagerShortcuts* m_presetManager = nullptr;
    LC_ShortcutSearchPopup* m_searchPopup = nullptr;

    LC_ShortcutTreeItem* m_currentItem = nullptr;
    QKeySequence m_editingKeySequence;
    int m_selectedRow = -1;
    int m_selectedParentRow = -1;
};

#endif // LC_SETTINGS_PAGE_SHORTCUTS_H
