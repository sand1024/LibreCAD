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

#include "lc_settings_page_menus_toolbars.h"
#include "ui_lc_settings_page_menus_toolbars.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItemModel>

#include "lc_action_group.h"
#include "lc_action_group_manager.h"
#include "lc_preset_manager_menus_toolbars.h"
#include "lc_settings_startup.h"

namespace {
    const int ROLE_IS_HEADER = Qt::UserRole + 1;
    const int ROLE_CONTAINER_KIND = Qt::UserRole + 2;
    const int ROLE_TOOLBAR_INDEX = Qt::UserRole + 3;

    QString resolveToolbarTitle(const ToolbarDef& tb, const LC_ActionGroupManager* agm) {
        if (tb.kind == ToolbarKind::CadMatrix) {
            return QObject::tr("CAD Tools Matrix");
        }

        if (tb.kind == ToolbarKind::Cad || tb.kind == ToolbarKind::CadDockWidget) {
            QString groupKey = tb.name;
            if (groupKey.startsWith("dock_cad_")) {
                groupKey = groupKey.mid(9);
            }
            else if (groupKey.startsWith("tb_cad_")) {
                groupKey = groupKey.mid(7);
            }
            else if (groupKey.startsWith("dock_")) {
                groupKey = groupKey.mid(5);
            }
            else if (groupKey.startsWith("cad_")) {
                groupKey = groupKey.mid(4);
            }

            if (groupKey == "mega") {
                return QObject::tr("CAD Tools Matrix");
            }

            if (agm != nullptr) {
                const auto* group = agm->getActionGroup(groupKey);
                if (group != nullptr) {
                    return group->cleanTitle();
                }
                return agm->displayName(groupKey, /*stripAmpersand=*/true);
            }
            return groupKey;
        }

        QString token = tb.name;
        if (token.startsWith("tb_s_") || token.startsWith("tb_c_")) {
            token = token.mid(5);
        }
        else if (token.startsWith("dock_")) {
            token = token.mid(5);
        }

        if (agm != nullptr) {
            return agm->displayName(token, /*stripAmpersand=*/true);
        }
        return token;
    }
}

LC_SettingsPageMenusToolbars::LC_SettingsPageMenusToolbars(LC_ActionGroupManager* groupManager, QObject* parent)
    : LC_SettingsPageBase(tr("Menus & Toolbars"), nullptr, parent), ui(std::make_unique<Ui::LC_SettingsPageMenusToolbars>()),
      m_actionGroupManager(groupManager) {
    setSortWeight(20);
}

LC_SettingsPageMenusToolbars::~LC_SettingsPageMenusToolbars() = default;

void LC_SettingsPageMenusToolbars::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMenusToolbars*>(manager);
    if (m_presetManager != nullptr) {
        syncUiFromWorkingConfig();
    }
}

void LC_SettingsPageMenusToolbars::setupUi() {
    ui->setupUi(m_widget);
    ui->dualListWidget->setActionGroupManager(m_actionGroupManager, ActionsFilterMode::ToolbarsAndMenus);

    ui->cbDockArea->clear();
    ui->cbDockArea->addItem(tr("Top"), static_cast<int>(Qt::TopToolBarArea));
    ui->cbDockArea->addItem(tr("Bottom"), static_cast<int>(Qt::BottomToolBarArea));
    ui->cbDockArea->addItem(tr("Left"), static_cast<int>(Qt::LeftToolBarArea));
    ui->cbDockArea->addItem(tr("Right"), static_cast<int>(Qt::RightToolBarArea));
}

void LC_SettingsPageMenusToolbars::setupBehavior() {
    connect(ui->cbTarget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageMenusToolbars::onTargetSelected);
    connect(ui->dualListWidget, &LC_ActionsDualListWidget::actionsChanged, this, &LC_SettingsPageMenusToolbars::onActionsModified);
    connect(ui->cbDockArea, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageMenusToolbars::onDockAreaChanged);
    connect(ui->pbNewToolbar, &QPushButton::clicked, this, &LC_SettingsPageMenusToolbars::onNewToolbarClicked);
    connect(ui->pbRenameToolbar, &QPushButton::clicked, this, &LC_SettingsPageMenusToolbars::onRenameToolbarClicked);
    connect(ui->pbDeleteToolbar, &QPushButton::clicked, this, &LC_SettingsPageMenusToolbars::onDeleteToolbarClicked);
}

void LC_SettingsPageMenusToolbars::populateCombobox() {
    if (m_presetManager == nullptr) {
        return;
    }

    m_blockSignals = true;
    ui->cbTarget->clear();
    m_currentComboIndex = -1;

    auto* model = qobject_cast<QStandardItemModel*>(ui->cbTarget->model());

    auto addCategoryHeader = [this, model](const QString& title) {
        const int row = ui->cbTarget->count();
        ui->cbTarget->addItem(title);
        ui->cbTarget->setItemData(row, true, ROLE_IS_HEADER);
        if (model != nullptr) {
            auto* item = model->item(row);
            if (item != nullptr) {
                item->setEnabled(false);
                QFont f = item->font();
                f.setBold(true);
                item->setFont(f);
            }
        }
    };

    auto addEntry = [this](const QString& title, ContainerKind kind, int tbIdx) {
        const int row = ui->cbTarget->count();
        ui->cbTarget->addItem(title);
        ui->cbTarget->setItemData(row, false, ROLE_IS_HEADER);
        ui->cbTarget->setItemData(row, static_cast<int>(kind), ROLE_CONTAINER_KIND);
        ui->cbTarget->setItemData(row, tbIdx, ROLE_TOOLBAR_INDEX);
    };

    const auto& config = m_presetManager->workingConfig();

    // 1. Main Menus Category
    addCategoryHeader(tr("── Main Menus ──"));
    addEntry(tr("Main Menu - Minimal"), ContainerKind::MenuMinimal, -1);
    addEntry(tr("Main Menu - Compact"), ContainerKind::MenuCompact, -1);
    addEntry(tr("Main Menu - Extended"), ContainerKind::MenuExtended, -1);

    // 2. Standard Toolbars Category
    addCategoryHeader(tr("── Standard Toolbars ──"));
    for (int i = 0; i < config.toolbars.size(); ++i) {
        const auto& tb = config.toolbars[i];
        if (tb.kind == ToolbarKind::Standard) {
            // // Omit special persistent toolbars that are not user-managed action containers
            // if (tb.name == tr("Snap Selection") || tb.name == tr("Tool Options")) {
            //     continue;
            // }
            const QString title = resolveToolbarTitle(tb, m_actionGroupManager);
            addEntry(title, ContainerKind::ToolbarItem, i);
        }
    }

    // 3. CAD Toolbars Category
    addCategoryHeader(tr("── CAD Toolbars ──"));
    for (int i = 0; i < config.toolbars.size(); ++i) {
        if (config.toolbars[i].kind == ToolbarKind::Cad) {
            const QString title = resolveToolbarTitle(config.toolbars[i], m_actionGroupManager);
            addEntry(title, ContainerKind::ToolbarItem, i);
        }
    }

    // 4. CAD Dock Widgets Category
    addCategoryHeader(tr("── CAD Dock Widgets ──"));
    for (int i = 0; i < config.toolbars.size(); ++i) {
        if (config.toolbars[i].kind == ToolbarKind::CadMatrix) {
            addEntry(config.toolbars[i].name, ContainerKind::CadMatrixItem, i);
        }
    }
    for (int i = 0; i < config.toolbars.size(); ++i) {
        if (config.toolbars[i].kind == ToolbarKind::CadDockWidget) {
            const QString title = resolveToolbarTitle(config.toolbars[i], m_actionGroupManager);
            const QString docTitle = tr("%1 (Dock)").arg(title);
            addEntry(docTitle, ContainerKind::CadDockWidgetItem, i);
        }
    }

    // 5. Custom Toolbars Category
    addCategoryHeader(tr("── Custom Toolbars ──"));
    for (int i = 0; i < config.toolbars.size(); ++i) {
        if (config.toolbars[i].kind == ToolbarKind::Custom) {
            addEntry(config.toolbars[i].name, ContainerKind::ToolbarItem, i);
        }
    }

    updateActiveMenuBold();
    m_blockSignals = false;
}

void LC_SettingsPageMenusToolbars::updateActiveMenuBold() {
    if (m_presetManager == nullptr) {
        return;
    }
    const int activeVar = CFG_Appearance::o_MainMenuType;
    auto* model = qobject_cast<QStandardItemModel*>(ui->cbTarget->model());
    if (model == nullptr) {
        return;
    }

    QFont normalFont = ui->cbTarget->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    const int total = ui->cbTarget->count();
    for (int i = 0; i < total; ++i) {
        if (ui->cbTarget->itemData(i, ROLE_IS_HEADER).toBool()) {
            continue;
        }

        auto* item = model->item(i);
        if (item == nullptr) {
            continue;
        }

        const int kindInt = ui->cbTarget->itemData(i, ROLE_CONTAINER_KIND).toInt();
        if (kindInt == ContainerKind::MenuMinimal || kindInt == ContainerKind::MenuCompact || kindInt == ContainerKind::MenuExtended) {
            const bool isActive = (kindInt == activeVar);
            item->setFont(isActive ? boldFont : normalFont);
        }
    }
}

void LC_SettingsPageMenusToolbars::updateEditorState(ContainerKind kind, int tbIdx, bool isReadOnly) {
    const auto& config = m_presetManager->workingConfig();

    const bool isMenu = (kind == ContainerKind::MenuMinimal ||
                         kind == ContainerKind::MenuCompact ||
                         kind == ContainerKind::MenuExtended);

    const bool isMatrix = (kind == ContainerKind::CadMatrixItem);
    const bool isToolbar = (kind == ContainerKind::ToolbarItem);
    const bool isDockWidget = (kind == ContainerKind::CadDockWidgetItem || kind == ContainerKind::CadMatrixItem);


    const ToolbarDef* tb = (tbIdx >= 0 && tbIdx < config.toolbars.size()) ? &config.toolbars[tbIdx] : nullptr;
    const bool isCustom = (tb != nullptr && tb->kind == ToolbarKind::Custom);

    // 1. Dock Area Controls
    ui->lblArea->setVisible(isToolbar);
    ui->cbDockArea->setVisible(isToolbar);
    ui->cbDockArea->setEnabled(!isReadOnly);
    if (isToolbar && tb != nullptr) {
        const int areaIdx = ui->cbDockArea->findData(static_cast<int>(tb->area));
        if (areaIdx >= 0) {
            ui->cbDockArea->setCurrentIndex(areaIdx);
        }
    }

    // fixme - sand - Should we mark somehow in UI that we're editing active menu ?
    // ui->pbMakeActiveMenu->setVisible(isMenu);
    // ui->pbMakeActiveMenu->setEnabled(!isReadOnly && isMenu && (config.activeMenuVariant != static_cast<int>(kind)));

    // 3. Toolbar Creation / Modification buttons
    ui->pbNewToolbar->setEnabled(!isReadOnly);
    ui->pbRenameToolbar->setEnabled(!isReadOnly && isCustom);
    ui->pbDeleteToolbar->setEnabled(!isReadOnly && isCustom);

    // 4. Dual List Widget: enforce depth policies and palette filter modes
    using Policy = LC_ActionsDualListWidget::GroupsPolicy;
    if (isMenu || isMatrix) {
        ui->dualListWidget->setGroupsPolicy(Policy::NestedGroups);
    } else if (isDockWidget) {
        ui->dualListWidget->setGroupsPolicy(Policy::NoGroups); // Strict Depth 1
    } else {
        ui->dualListWidget->setGroupsPolicy(Policy::SingleLevelAtRoot); // Depth 2
    }

    // Filter available actions palette to CAD tools only when editing CAD containers
    const bool isCadContainer = (isMatrix || isDockWidget || (tb != nullptr && tb->kind == ToolbarKind::Cad));
    const auto filterMode = isCadContainer
        ? ActionsFilterMode::CadToolsOnly
        : ActionsFilterMode::ToolbarsAndMenus;

    ui->dualListWidget->setActionGroupManager(m_actionGroupManager, filterMode);
    ui->dualListWidget->setReadOnly(isReadOnly);

    // 5. Populate nodes
    if (kind == ContainerKind::MenuMinimal) {
        ui->dualListWidget->setNodes(config.menuMinimal);
    } else if (kind == ContainerKind::MenuCompact) {
        ui->dualListWidget->setNodes(config.menuCompact);
    } else if (kind == ContainerKind::MenuExtended) {
        ui->dualListWidget->setNodes(config.menuExtended);
    } else if (tb != nullptr) {
        ui->dualListWidget->setNodes(tb->nodes);
    } else {
        ui->dualListWidget->clear();
    }
}

void LC_SettingsPageMenusToolbars::syncUiFromWorkingConfig() {
    if (m_presetManager == nullptr) {
        return;
    }

    populateCombobox();

    // Select the first non-header item (row 1: "Main Menu - Compact")
    if (ui->cbTarget->count() > 1) {
        ui->cbTarget->setCurrentIndex(1);
    }
    else {
        ui->dualListWidget->clear();
    }
}

void LC_SettingsPageMenusToolbars::syncCurrentContainerToConfig() {
    if (m_presetManager == nullptr || m_currentComboIndex < 0 || m_currentComboIndex >= ui->cbTarget->count()) {
        return;
    }

    if (ui->cbTarget->itemData(m_currentComboIndex, ROLE_IS_HEADER).toBool()) {
        return;
    }

    auto& config = m_presetManager->workingConfig();
    const auto kind = static_cast<ContainerKind>(ui->cbTarget->itemData(m_currentComboIndex, ROLE_CONTAINER_KIND).toInt());
    const int tbIdx = ui->cbTarget->itemData(m_currentComboIndex, ROLE_TOOLBAR_INDEX).toInt();

    if (kind == ContainerKind::MenuMinimal) {
        config.menuMinimal = ui->dualListWidget->getNodes();
    }
    else if (kind == ContainerKind::MenuCompact) {
        config.menuCompact = ui->dualListWidget->getNodes();
    }
    else if (kind == ContainerKind::MenuExtended) {
        config.menuExtended = ui->dualListWidget->getNodes();
    }
    else if ((kind == ContainerKind::ToolbarItem || kind == ContainerKind::CadDockWidgetItem || kind == ContainerKind::CadMatrixItem) &&
        tbIdx >= 0 && tbIdx < config.toolbars.size()) {
        config.toolbars[tbIdx].nodes = ui->dualListWidget->getNodes();
        if (kind == ContainerKind::ToolbarItem) {
            config.toolbars[tbIdx].area = static_cast<Qt::ToolBarArea>(ui->cbDockArea->currentData().toInt());
        }
    }
}

void LC_SettingsPageMenusToolbars::onTargetSelected(int index) {
    if (m_blockSignals || index < 0 || index >= ui->cbTarget->count()) {
        return;
    }

    if (ui->cbTarget->itemData(index, ROLE_IS_HEADER).toBool()) {
        return;
    }

    syncCurrentContainerToConfig();
    m_currentComboIndex = index;

    if (m_presetManager == nullptr) {
        return;
    }

    const auto kind = static_cast<ContainerKind>(ui->cbTarget->itemData(index, ROLE_CONTAINER_KIND).toInt());
    const int tbIdx = ui->cbTarget->itemData(index, ROLE_TOOLBAR_INDEX).toInt();
    const bool isReadOnly = m_presetManager->isReadOnlyDefault();

    m_blockSignals = true;
    updateControlsState(kind, tbIdx, isReadOnly);
    loadContainerContent(kind, tbIdx);
    m_blockSignals = false;
}

void LC_SettingsPageMenusToolbars::onActionsModified() {
    if (m_blockSignals) {
        return;
    }
    syncCurrentContainerToConfig();
    markModified();
}

void LC_SettingsPageMenusToolbars::onDockAreaChanged(int) {
    if (m_blockSignals) {
        return;
    }
    syncCurrentContainerToConfig();
    markModified();
}

void LC_SettingsPageMenusToolbars::onNewToolbarClicked() {
    if (m_presetManager == nullptr) {
        return;
    }

    if (m_presetManager->isReadOnlyDefault()) {
        const auto res = QMessageBox::question(getEditingWidget(), tr("Default Scheme Read-Only"),
                                               tr(
                                                   "The Default scheme cannot be modified. Would you like to create a custom scheme to add your new toolbar?"),
                                               QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes) {
            return;
        }
        if (!m_presetManager->promptSavePresetAs(getEditingWidget())) {
            return;
        }
    }

    auto& config = m_presetManager->workingConfig();
    QStringList existingNames;
    for (const auto& tb : config.toolbars) {
        existingNames.append(tb.name);
    }

    QString newName;
    if (!promptForUniqueName(tr("New Toolbar"), tr("Enter unique toolbar name:"), existingNames, tr("Custom Toolbar"), newName)) {
        return;
    }

    syncCurrentContainerToConfig();

    ToolbarDef newTb;
    newTb.name = newName;
    newTb.area = Qt::BottomToolBarArea;
    newTb.kind = ToolbarKind::Custom;
    config.toolbars.append(newTb);

    populateCombobox();

    // Locate and select the newly created toolbar
    const int newTbIdx = config.toolbars.size() - 1;
    for (int i = 0; i < ui->cbTarget->count(); ++i) {
        if (!ui->cbTarget->itemData(i, ROLE_IS_HEADER).toBool() && ui->cbTarget->itemData(i, ROLE_CONTAINER_KIND).toInt() ==
            ContainerKind::ToolbarItem && ui->cbTarget->itemData(i, ROLE_TOOLBAR_INDEX).toInt() == newTbIdx) {
            ui->cbTarget->setCurrentIndex(i);
            break;
        }
    }

    markModified();
}

void LC_SettingsPageMenusToolbars::onRenameToolbarClicked() {
    if (m_presetManager == nullptr || m_currentComboIndex < 0 || m_currentComboIndex >= ui->cbTarget->count()) {
        return;
    }

    if (ui->cbTarget->itemData(m_currentComboIndex, ROLE_CONTAINER_KIND).toInt() != ContainerKind::ToolbarItem) {
        return;
    }

    const int tbIdx = ui->cbTarget->itemData(m_currentComboIndex, ROLE_TOOLBAR_INDEX).toInt();
    auto& config = m_presetManager->workingConfig();
    if (tbIdx < 0 || tbIdx >= config.toolbars.size() || config.toolbars[tbIdx].kind != ToolbarKind::Custom) {
        return;
    }

    const QString oldName = config.toolbars[tbIdx].name;
    QStringList existingNames;
    for (int i = 0; i < config.toolbars.size(); ++i) {
        if (i != tbIdx) {
            existingNames.append(config.toolbars[i].name);
        }
    }

    QString newName;
    if (!promptForUniqueName(tr("Rename Toolbar"), tr("Enter new toolbar name:"), existingNames, oldName, newName)) {
        return;
    }

    if (newName == oldName) {
        return;
    }

    config.toolbars[tbIdx].name = newName;
    ui->cbTarget->setItemText(m_currentComboIndex, newName);
    markModified();
}

void LC_SettingsPageMenusToolbars::onDeleteToolbarClicked() {
    if (m_presetManager == nullptr || m_currentComboIndex < 0 || m_currentComboIndex >= ui->cbTarget->count()) {
        return;
    }

    if (ui->cbTarget->itemData(m_currentComboIndex, ROLE_CONTAINER_KIND).toInt() != ContainerKind::ToolbarItem) {
        return;
    }

    const int tbIdx = ui->cbTarget->itemData(m_currentComboIndex, ROLE_TOOLBAR_INDEX).toInt();
    auto& config = m_presetManager->workingConfig();
    if (tbIdx < 0 || tbIdx >= config.toolbars.size() || config.toolbars[tbIdx].kind != ToolbarKind::Custom) {
        return;
    }

    const QString name = config.toolbars[tbIdx].name;
    if (QMessageBox::question(getEditingWidget(), tr("Delete Toolbar"), tr("Are you sure you want to delete toolbar '%1'?").arg(name),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        config.toolbars.removeAt(tbIdx);
        syncUiFromWorkingConfig();
        markModified();
    }
}

void LC_SettingsPageMenusToolbars::loadSettings() {
    if (m_presetManager != nullptr) {
        m_presetManager->loadPreset(m_presetManager->getActivePresetKey());
    }
    syncUiFromWorkingConfig();
}

bool LC_SettingsPageMenusToolbars::saveSettings() {
    syncCurrentContainerToConfig();
    if (m_presetManager != nullptr) {
        return m_presetManager->saveCurrentPreset();
    }
    return true;
}

bool LC_SettingsPageMenusToolbars::isModified() const {
    return (m_presetManager != nullptr) && m_presetManager->isPresetModified();
}

void LC_SettingsPageMenusToolbars::setReadOnly(bool readOnly) {
    if (m_currentComboIndex >= 0 && m_currentComboIndex < ui->cbTarget->count()) {
        const auto kind = static_cast<ContainerKind>(ui->cbTarget->itemData(m_currentComboIndex, ROLE_CONTAINER_KIND).toInt());
        const int tbIdx = ui->cbTarget->itemData(m_currentComboIndex, ROLE_TOOLBAR_INDEX).toInt();
        updateControlsState(kind, tbIdx, readOnly);
    }
}

void LC_SettingsPageMenusToolbars::markModified() {
    if (m_blockSignals) {
        return;
    }
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
    emit livePreviewRequested();
}

bool LC_SettingsPageMenusToolbars::promptForUniqueName(const QString& title, const QString& label, const QStringList& existingNames,
                                                       const QString& initialSuggestion, QString& outName) {
    QString name;
    QString suggestedName = initialSuggestion;
    if (suggestedName.isEmpty()) {
        suggestedName = tr("Custom Toolbar");
    }

    const bool ok = LC_PresetManagerInterface::promptForUniqueName(getEditingWidget(), title, label, existingNames, suggestedName, name);
    if (ok) {
        outName = name;
    }
    return ok;
}

void LC_SettingsPageMenusToolbars::updateControlsState(ContainerKind kind, int tbIdx, bool isReadOnly) {
    const auto& config = m_presetManager->workingConfig();

    const bool isMenu = (kind == ContainerKind::MenuMinimal ||
                         kind == ContainerKind::MenuCompact ||
                         kind == ContainerKind::MenuExtended);

    const bool isMatrix = (kind == ContainerKind::CadMatrixItem);
    const bool isDockWidget = (kind == ContainerKind::CadDockWidgetItem);
    const bool isToolbar = (kind == ContainerKind::ToolbarItem);

    const ToolbarDef* tb = (tbIdx >= 0 && tbIdx < config.toolbars.size()) ? &config.toolbars[tbIdx] : nullptr;
    const bool isCustom = (tb != nullptr && tb->kind == ToolbarKind::Custom);

    // 1. Dock Area Controls
    ui->lblArea->setVisible(isToolbar);
    ui->cbDockArea->setVisible(isToolbar);
    ui->cbDockArea->setEnabled(!isReadOnly);
    if (isToolbar && tb != nullptr) {
        const int areaIdx = ui->cbDockArea->findData(static_cast<int>(tb->area));
        if (areaIdx >= 0) {
            ui->cbDockArea->setCurrentIndex(areaIdx);
        }
    }

    // 3. Toolbar Creation / Modification buttons
    ui->pbNewToolbar->setEnabled(!isReadOnly);
    ui->pbRenameToolbar->setEnabled(!isReadOnly && isCustom);
    ui->pbDeleteToolbar->setEnabled(!isReadOnly && isCustom);

    // 4. Dual List Widget: policies and read-only state (NO content reloading!)
    using Policy = LC_ActionsDualListWidget::GroupsPolicy;
    if (isMenu || isMatrix) {
        ui->dualListWidget->setGroupsPolicy(Policy::NestedGroups);
    } else if (isDockWidget) {
        ui->dualListWidget->setGroupsPolicy(Policy::NoGroups);
    } else {
        ui->dualListWidget->setGroupsPolicy(Policy::SingleLevelAtRoot);
    }

    const bool isCadContainer = (isMatrix || isDockWidget || (tb != nullptr && tb->kind == ToolbarKind::Cad));
    const auto filterMode = isCadContainer
        ? ActionsFilterMode::CadToolsOnly
        : ActionsFilterMode::ToolbarsAndMenus;

    ui->dualListWidget->setActionGroupManager(m_actionGroupManager, filterMode);
    ui->dualListWidget->setReadOnly(isReadOnly);
}

void LC_SettingsPageMenusToolbars::loadContainerContent(ContainerKind kind, int tbIdx) {
    const auto& config = m_presetManager->workingConfig();
    const ToolbarDef* tb = (tbIdx >= 0 && tbIdx < config.toolbars.size()) ? &config.toolbars[tbIdx] : nullptr;

    if (kind == ContainerKind::MenuMinimal) {
        ui->dualListWidget->setNodes(config.menuMinimal);
    } else if (kind == ContainerKind::MenuCompact) {
        ui->dualListWidget->setNodes(config.menuCompact);
    } else if (kind == ContainerKind::MenuExtended) {
        ui->dualListWidget->setNodes(config.menuExtended);
    } else if (tb != nullptr) {
        ui->dualListWidget->setNodes(tb->nodes);
    } else {
        ui->dualListWidget->clear();
    }
}
