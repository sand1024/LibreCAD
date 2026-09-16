
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

#ifndef LC_SETTINGS_PAGE_COMMANDS_H
#define LC_SETTINGS_PAGE_COMMANDS_H

#include <memory>
#include "lc_settings_page_base.h"

struct KeywordDefinition;
struct CommandDefinition;
struct CommandsConfig;
class LC_ActionGroupManager;
class LC_CommandsTreeModel;
class LC_CommandsTreeItem;
class LC_PresetManagerCommands;

namespace Ui {
    class LC_SettingsPageCommands;
}

class LC_SettingsPageCommands : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageCommands(LC_ActionGroupManager* groupManager, QObject* parent = nullptr);
    ~LC_SettingsPageCommands() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    bool validate(QString& outErrorMessage) override;

    void autoIndexLabels() override;
    void highlightSearchPattern(const QString& pattern) override;
    void clearSearchHighlight() override;

private slots:
    void onTreeSelectionChanged();
    void onFilterChanged();
    void onFilterConflictsRequested();
    void onClearConflictsFilterRequested();
    void onOverrideEdited();
    void onResetMainClicked();
    void onSuppressAlias1Clicked();
    void onResetAlias1Clicked();
    void onSuppressAlias2Clicked();
    void onResetAlias2Clicked();
    void onTreeContextMenuRequested(const QPoint& pos);
    void onResetItemToDefaults();
    void onSuppressItemAliases();
    void onExportCheatsheetClicked();
protected:
    void setupUi() override;
    void setupBehavior() override;
    void setReadOnly(bool readOnly) override;
private:
    void selectItem(LC_CommandsTreeItem* item);
    void updateEffectiveSummary();
    bool validateCollisions();
    void rebuildTree(bool restoreSelection = true);
    void setKeycodeRowVisible(bool visible);

    std::unique_ptr<Ui::LC_SettingsPageCommands> ui;
    LC_ActionGroupManager* m_actionGroupManager{nullptr};
    LC_PresetManagerCommands* m_presetManager{nullptr};
    LC_CommandsTreeModel* m_commandsTreeModel{nullptr};

    LC_CommandsTreeItem* m_currentItem{nullptr};
    bool m_isReadOnly{false};
    bool m_blockSignals = false;
    QString m_selectedIdentifier;
    QStringList m_systemCommands;
    QStringList m_systemKeycodes;
    QStringList m_systemAliases;
};

#endif
