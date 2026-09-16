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

#include "lc_settings_page_custom_menus.h"
#include "ui_lc_settings_page_custom_menus.h"

#include <QInputDialog>
#include <QMessageBox>
#include  <QStandardItemModel>

#include "lc_default_context_menus_builder.h"
#include "lc_preset_manager_menus.h"

class QStandardItemModel;

namespace {
    const int ROLE_IS_HEADER = Qt::UserRole + 1;
    const int ROLE_MENU_INDEX = Qt::UserRole + 2;
    const int ROLE_CATEGORY   = Qt::UserRole + 3;
}

LC_SettingsPageCustomMenus::MenuCategory LC_SettingsPageCustomMenus::getMenuCategory(const ContextMenuDef& m) const {
    if (!m.activator.isEntityRequired()) {
        return MenuCategory::DrawingArea;
    }

    const auto type = m.activator.getEntityType();
    if (type == RS2::EntityLine || type == RS2::EntityCircle || type == RS2::EntityArc ||
        type == RS2::EntityPolyline || type == RS2::EntitySpline || type == RS2::EntitySplinePoints ||
        type == RS2::EntityEllipse || type == RS2::EntityPoint || type == RS2::EntityParabola) {
        return MenuCategory::Geometry;
    }

    if (type == RS2::EntityDimLinear || type == RS2::EntityDimAligned || type == RS2::EntityDimRadial ||
        type == RS2::EntityDimDiametric || type == RS2::EntityDimArc || type == RS2::EntityDimOrdinate ||
        type == RS2::EntityDimLeader || type == RS2::EntityText || type == RS2::EntityMText) {
        return MenuCategory::Annotations;
    }

    if (type == RS2::EntityInsert || type == RS2::EntityImage || type == RS2::EntityHatch) {
        return MenuCategory::BlocksMedia;
    }

    return MenuCategory::FallbackGestures;
}

QString LC_SettingsPageCustomMenus::formatMenuDisplayName(const ContextMenuDef& menuDef) const {
    const QString shortcutView = menuDef.activator.getShortcutView();
    return QString("%1 | %2").arg(menuDef.name, shortcutView);
}

int LC_SettingsPageCustomMenus::findRowForMenuIndex(int menuIdx) const {
    const int count = ui->cbMenuName->count();
    for (int i = 0; i < count; ++i) {
        if (!ui->cbMenuName->itemData(i, ROLE_IS_HEADER).toBool() &&
            ui->cbMenuName->itemData(i, ROLE_MENU_INDEX).toInt() == menuIdx) {
            return i;
        }
    }
    return -1;
}

void LC_SettingsPageCustomMenus::selectMenuByIndex(int menuIdx) {
    const int row = findRowForMenuIndex(menuIdx);
    if (row >= 0) {
        m_blockSignals = true;
        ui->cbMenuName->setCurrentIndex(row);
        m_blockSignals = false;
        onMenuSelected(row);
    }
}

void LC_SettingsPageCustomMenus::updateCurrentMenuInCombobox() {
    if (m_presetManager == nullptr || m_currentMenuIndex < 0) {
        return;
    }

    const auto& config = m_presetManager->workingConfig();
    if (m_currentMenuIndex >= config.menus.size()) {
        return;
    }

    const auto& currentMenu = config.menus.at(m_currentMenuIndex);
    const int currentRow = findRowForMenuIndex(m_currentMenuIndex);
    const auto currentCategory = getMenuCategory(currentMenu);

    bool categoryChanged = false;
    if (currentRow >= 0) {
        const int storedCategory = ui->cbMenuName->itemData(currentRow, ROLE_CATEGORY).toInt();
        if (storedCategory != static_cast<int>(currentCategory)) {
            categoryChanged = true;
        }
    } else {
        categoryChanged = true;
    }

    if (categoryChanged) {
        // Category changed: structural rebuild and safe re-selection
        const int activeIdx = m_currentMenuIndex;
        syncUiFromWorkingConfig();
        selectMenuByIndex(activeIdx);
    } else if (currentRow >= 0) {
        // Same category: transactional in-place title update
        m_blockSignals = true;
        ui->cbMenuName->setItemText(currentRow, formatMenuDisplayName(currentMenu));
        m_blockSignals = false;
    }
}

LC_SettingsPageCustomMenus::LC_SettingsPageCustomMenus(LC_ActionGroupManager* groupManager, QObject* parent)
    : LC_SettingsPageBase(tr("Context Menus"), nullptr, parent), ui(std::make_unique<Ui::LC_SettingsPageCustomMenus>()),
      m_actionGroupManager(groupManager) {
    setSortWeight(30);
}

LC_SettingsPageCustomMenus::~LC_SettingsPageCustomMenus() = default;

void LC_SettingsPageCustomMenus::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerMenus*>(manager);
    if (m_presetManager != nullptr) {
        syncUiFromWorkingConfig();
    }
}

void LC_SettingsPageCustomMenus::setupUi() {
    ui->setupUi(m_widget);
    ui->dualListWidget->setAllowNestedGroups(true); // Menus: arbitrary sub-menu nesting
    ui->dualListWidget->setActionGroupManager(m_actionGroupManager, ActionsFilterMode::ContextMenusOnly);

    ui->splMenu->setChildrenCollapsible(false);
    ui->splMenu->setStretchFactor(0, 3);
    ui->splMenu->setStretchFactor(1, 2);
    trackSplitter(ui->splMenu, 480);
}

void LC_SettingsPageCustomMenus::setupBehavior() {
    connect(ui->cbMenuName, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageCustomMenus::onMenuSelected);
    connect(ui->pbNewMenu, &QPushButton::clicked, this, &LC_SettingsPageCustomMenus::onNewMenuClicked);
    connect(ui->pbSaveAsMenu, &QPushButton::clicked, this, &LC_SettingsPageCustomMenus::onSaveAsMenuClicked);
    connect(ui->pbDeleteMenu, &QPushButton::clicked, this, &LC_SettingsPageCustomMenus::onDeleteMenuClicked);

    connect(ui->dualListWidget, &LC_ActionsDualListWidget::actionsChanged, this, &LC_SettingsPageCustomMenus::onContentModified);
    connect(ui->triggerInspector, &LC_MenuTriggerInspector::triggerChanged, this, &LC_SettingsPageCustomMenus::onContentModified);
}

void LC_SettingsPageCustomMenus::setReadOnly(bool readOnly) {
    const bool hasMenus = (m_presetManager != nullptr) && !m_presetManager->workingConfig().menus.isEmpty();
    const bool isBuiltIn = isBuiltInContextMenu(m_currentMenuIndex);

    ui->cbMenuName->setEnabled(hasMenus);
    ui->pbNewMenu->setEnabled(!readOnly);
    ui->pbSaveAsMenu->setEnabled(!readOnly && hasMenus);
    ui->pbDeleteMenu->setEnabled(!readOnly && hasMenus && !isBuiltIn);
    ui->dualListWidget->setReadOnly(readOnly);
    ui->triggerInspector->setReadOnly(readOnly || isBuiltIn);
}

void LC_SettingsPageCustomMenus::syncUiFromWorkingConfig() {
    if (m_presetManager == nullptr) {
        return;
    }

    m_blockSignals = true;
    ui->cbMenuName->clear();

    const auto& config = m_presetManager->workingConfig();
    auto* model = qobject_cast<QStandardItemModel*>(ui->cbMenuName->model());

    auto addCategoryHeader = [this, model](const QString& title) {
        const int row = ui->cbMenuName->count();
        ui->cbMenuName->addItem(title);
        ui->cbMenuName->setItemData(row, true, ROLE_IS_HEADER);
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

    auto addMenuEntry = [this](const ContextMenuDef& menuDef, int menuIdx) {
        const int row = ui->cbMenuName->count();
        ui->cbMenuName->addItem(formatMenuDisplayName(menuDef));
        ui->cbMenuName->setItemData(row, false, ROLE_IS_HEADER);
        ui->cbMenuName->setItemData(row, menuIdx, ROLE_MENU_INDEX);
        ui->cbMenuName->setItemData(row, static_cast<int>(getMenuCategory(menuDef)), ROLE_CATEGORY);
    };

    auto addCategoryIfNotEmpty = [&addCategoryHeader, &addMenuEntry](const QString& headerTitle,
                                                                     const QList<QPair<ContextMenuDef, int>>& entries) {
        if (!entries.isEmpty()) {
            addCategoryHeader(headerTitle);
            for (const auto& entry : entries) {
                addMenuEntry(entry.first, entry.second);
            }
        }
    };

    // 1. Drawing Area (No Entity)
    QList<QPair<ContextMenuDef, int>> drawingAreaMenus;
    for (int i = 0; i < config.menus.size(); ++i) {
        if (getMenuCategory(config.menus[i]) == MenuCategory::DrawingArea) {
            drawingAreaMenus.append({config.menus[i], i});
        }
    }
    addCategoryIfNotEmpty(tr("── Drawing Area (No Entity) ──"), drawingAreaMenus);

    // 2. Geometry Entities
    QList<QPair<ContextMenuDef, int>> geometryMenus;
    for (int i = 0; i < config.menus.size(); ++i) {
        if (getMenuCategory(config.menus[i]) == MenuCategory::Geometry) {
            geometryMenus.append({config.menus[i], i});
        }
    }
    addCategoryIfNotEmpty(tr("── Geometry Entities ──"), geometryMenus);

    // 3. Annotations & Dimensions
    QList<QPair<ContextMenuDef, int>> annotationMenus;
    for (int i = 0; i < config.menus.size(); ++i) {
        if (getMenuCategory(config.menus[i]) == MenuCategory::Annotations) {
            annotationMenus.append({config.menus[i], i});
        }
    }
    addCategoryIfNotEmpty(tr("── Annotations & Dimensions ──"), annotationMenus);

    // 4. Blocks & Media
    QList<QPair<ContextMenuDef, int>> blockMediaMenus;
    for (int i = 0; i < config.menus.size(); ++i) {
        if (getMenuCategory(config.menus[i]) == MenuCategory::BlocksMedia) {
            blockMediaMenus.append({config.menus[i], i});
        }
    }
    addCategoryIfNotEmpty(tr("── Blocks & Media ──"), blockMediaMenus);

    // 5. Fallback & Gestures
    QList<QPair<ContextMenuDef, int>> fallbackMenus;
    for (int i = 0; i < config.menus.size(); ++i) {
        if (getMenuCategory(config.menus[i]) == MenuCategory::FallbackGestures) {
            fallbackMenus.append({config.menus[i], i});
        }
    }
    addCategoryIfNotEmpty(tr("── Fallback & Gestures ──"), fallbackMenus);

    // Register activators for collision warnings
    QList<LC_MenuActivator*> existingActivators;
    for (const auto& m : config.menus) {
        auto* actCopy = m.activator.getCopy();
        if (actCopy != nullptr) {
            actCopy->setMenuName(m.name);
            existingActivators.append(actCopy);
        }
    }
    ui->triggerInspector->setExistingActivators(existingActivators);
    qDeleteAll(existingActivators);

    const bool isReadOnly = m_presetManager->isReadOnlyDefault();
    setReadOnly(isReadOnly);

    // Default select first selectable item
    int firstSelectableRow = -1;
    for (int i = 0; i < ui->cbMenuName->count(); ++i) {
        if (!ui->cbMenuName->itemData(i, ROLE_IS_HEADER).toBool()) {
            firstSelectableRow = i;
            break;
        }
    }

    if (firstSelectableRow >= 0) {
        ui->cbMenuName->setCurrentIndex(firstSelectableRow);
        const int mIdx = ui->cbMenuName->itemData(firstSelectableRow, ROLE_MENU_INDEX).toInt();
        m_currentMenuIndex = mIdx;
        const auto& m = config.menus[mIdx];
        ui->dualListWidget->setNodes(m.nodes);
        ui->triggerInspector->setActivator(m.name, m.activator.getShortcut());
    }
    else {
        ui->dualListWidget->clear();
        ui->triggerInspector->setActivator("", "");
    }
    m_blockSignals = false;
}

bool LC_SettingsPageCustomMenus::promptForUniqueName(const QString& title, const QString& label, const QStringList& existingNames,
                                                     const QString& initialSuggestion, QString& outName) {
    QString name;
    QString suggestedName = initialSuggestion;
    if (suggestedName.isEmpty()) {
        suggestedName = tr("Custom Menu");
    }

    const bool ok = LC_PresetManagerInterface::promptForUniqueName(getEditingWidget(), title, label, existingNames, suggestedName, name);
    if (ok) {
        outName = name;
    }
    return ok;
}

QString LC_SettingsPageCustomMenus::formatMenuDisplayName(const QString& menuName, const QString& activatorShortcut) const {
    if (activatorShortcut.isEmpty()) {
        return QString("%1 | %2").arg(menuName, tr("NOT ASSIGNED"));
    }

    std::unique_ptr<LC_MenuActivator> act(LC_MenuActivator::fromShortcut(activatorShortcut));
    if (act != nullptr) {
        return QString("%1 | %2").arg(menuName, act->getShortcutView());
    }

    return QString("%1 | %2").arg(menuName, tr("NOT ASSIGNED"));
}

bool LC_SettingsPageCustomMenus::isBuiltInContextMenu(int menuIndex) const {
    if (m_presetManager == nullptr || menuIndex < 0) {
        return false;
    }

    const auto& config = m_presetManager->workingConfig();
    if (menuIndex >= config.menus.size()) {
        return false;
    }

    return LC_DefaultContextMenusBuilder::isBuiltInContextMenu(config.menus.at(menuIndex));
}

void LC_SettingsPageCustomMenus::markModified() {
    if (m_blockSignals) {
        return;
    }
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
    emit livePreviewRequested();
}

bool LC_SettingsPageCustomMenus::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageCustomMenus::onContentModified() {
    if (m_blockSignals) {
        return;
    }
    syncCurrentMenuToConfig();
    updateCurrentMenuInCombobox();
    markModified();
}

void LC_SettingsPageCustomMenus::syncCurrentMenuToConfig() {
    if (m_presetManager == nullptr || m_currentMenuIndex < 0) {
        return;
    }

    auto& config = m_presetManager->workingConfig();
    if (m_currentMenuIndex < config.menus.size()) {
        config.menus[m_currentMenuIndex].nodes = ui->dualListWidget->getNodes();
        const QString shortcutCode = ui->triggerInspector->getActivatorCode();
        auto* parsed = LC_MenuActivator::fromShortcut(shortcutCode);
        if (parsed != nullptr) {
            config.menus[m_currentMenuIndex].activator = *parsed;
            delete parsed;
        } else {
            config.menus[m_currentMenuIndex].activator = LC_MenuActivator();
        }
    }
}

void LC_SettingsPageCustomMenus::onNewMenuClicked() {
    if (m_presetManager == nullptr) {
        return;
    }

    if (m_presetManager->isReadOnlyDefault()) {
        const auto res = QMessageBox::question(
            getEditingWidget(), tr("Default Scheme Read-Only"),
            tr("The Default scheme cannot be modified. Would you like to create a custom scheme to add your new menu?"),
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
    for (const auto& m : config.menus) {
        existingNames.append(m.name);
    }

    QString newName;
    if (!promptForUniqueName(tr("New Menu"), tr("Enter unique menu name:"), existingNames, tr("Custom Menu"), newName)) {
        return;
    }

    syncCurrentMenuToConfig();

    ContextMenuDef newM;
    newM.name = newName;
    newM.isBuiltIn = false;
    newM.activator = LC_MenuActivator(); // Unassigned (m_button = NONE)
    config.menus.append(newM);

    const int newMenuIdx = config.menus.size() - 1;
    syncUiFromWorkingConfig();

    // Locate the correct row in the categorized combobox
    int targetRow = -1;
    for (int i = 0; i < ui->cbMenuName->count(); ++i) {
        if (!ui->cbMenuName->itemData(i, ROLE_IS_HEADER).toBool() &&
            ui->cbMenuName->itemData(i, ROLE_MENU_INDEX).toInt() == newMenuIdx) {
            targetRow = i;
            break;
            }
    }

    if (targetRow >= 0) {
        ui->cbMenuName->setCurrentIndex(targetRow);
        onMenuSelected(targetRow);
    }

    markModified();
}

void LC_SettingsPageCustomMenus::onSaveAsMenuClicked() {
    if (m_presetManager == nullptr || m_currentMenuIndex < 0) {
        return;
    }

    auto& config = m_presetManager->workingConfig();
    if (m_currentMenuIndex >= config.menus.size()) {
        return;
    }

    // Flush current UI selections into the source menu before copying
    syncCurrentMenuToConfig();

    const auto& sourceMenu = config.menus[m_currentMenuIndex];
    const QString oldName = sourceMenu.name;

    QStringList existingNames;
    for (const auto& m : config.menus) {
        existingNames.append(m.name);
    }

    QString newName;
    const QString suggestedName = oldName + tr("_Copy");
    if (!promptForUniqueName(tr("Save Menu As"), tr("Enter unique name for menu copy:"), existingNames, suggestedName, newName)) {
        return;
    }

    ContextMenuDef copyMenu;
    copyMenu.name = newName;
    copyMenu.nodes = sourceMenu.nodes; // Copy hierarchical ActionNode tree
    copyMenu.activator = LC_MenuActivator(); // Clear trigger on the copy to avoid collisions

    config.menus.append(copyMenu);

    m_blockSignals = true;
    syncUiFromWorkingConfig();
    m_currentMenuIndex = config.menus.size() - 1;
    ui->cbMenuName->setCurrentIndex(m_currentMenuIndex);
    ui->dualListWidget->setNodes(copyMenu.nodes);
    ui->triggerInspector->setActivator(newName, "");
    m_blockSignals = false;

    markModified();
}

void LC_SettingsPageCustomMenus::onDeleteMenuClicked() {
    if (m_presetManager == nullptr || m_currentMenuIndex < 0) {
        return;
    }

    if (isBuiltInContextMenu(m_currentMenuIndex)) {
        return;
    }

    auto& config = m_presetManager->workingConfig();
    if (m_currentMenuIndex >= config.menus.size()) {
        return;
    }

    const QString name = config.menus[m_currentMenuIndex].name;
    if (QMessageBox::question(getEditingWidget(), tr("Delete Menu"), tr("Are you sure you want to delete menu '%1'?").arg(name),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        config.menus.removeAt(m_currentMenuIndex);
        m_currentMenuIndex = -1;
        syncUiFromWorkingConfig();
        markModified();
    }
}

void LC_SettingsPageCustomMenus::loadSettings() {
    if (m_presetManager != nullptr) {
        m_presetManager->loadPreset(m_presetManager->getActivePresetKey());
    }
    syncUiFromWorkingConfig();
}

bool LC_SettingsPageCustomMenus::saveSettings() {
    syncCurrentMenuToConfig();
    if (m_presetManager != nullptr) {
        return m_presetManager->saveCurrentPreset();
    }
    return true;
}


void LC_SettingsPageCustomMenus::onMenuSelected(int row) {
    if (m_blockSignals || row < 0 || row >= ui->cbMenuName->count()) {
        return;
    }

    if (ui->cbMenuName->itemData(row, ROLE_IS_HEADER).toBool()) {
        return;
    }

    syncCurrentMenuToConfig();
    const int menuIdx = ui->cbMenuName->itemData(row, ROLE_MENU_INDEX).toInt();
    m_currentMenuIndex = menuIdx;

    if (m_presetManager != nullptr && menuIdx >= 0 && menuIdx < m_presetManager->workingConfig().menus.size()) {
        const auto& m = m_presetManager->workingConfig().menus[menuIdx];
        const bool isReadOnly = m_presetManager->isReadOnlyDefault();
        const bool isBuiltIn = m.isBuiltIn;

        ui->pbDeleteMenu->setEnabled(!isReadOnly && !isBuiltIn);
        ui->pbSaveAsMenu->setEnabled(!isReadOnly);
        ui->triggerInspector->setReadOnly(isReadOnly || isBuiltIn);
        ui->dualListWidget->setReadOnly(isReadOnly);

        m_blockSignals = true;
        ui->dualListWidget->setNodes(m.nodes);
        ui->triggerInspector->setActivator(m.name, m.activator.getShortcut());
        m_blockSignals = false;
    }
}
