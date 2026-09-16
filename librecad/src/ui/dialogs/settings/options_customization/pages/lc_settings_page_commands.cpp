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

#include "lc_settings_page_commands.h"

#include <QClipboard>

#include "ui_lc_settings_page_commands.h"

#include <QHeaderView>
#include <QMenu>

#include "lc_action.h"
#include "lc_action_group_manager.h"
#include "lc_action_type_mapper.h"
#include "lc_cheatsheet_generator.h"
#include "lc_cheatsheet_options.h"
#include "lc_commandItems.h"
#include "lc_commands_tree_item.h"
#include "lc_commands_tree_model.h"
#include "lc_dlg_cheatsheet_options.h"
#include "lc_pages_utils.h"
#include "lc_palette_color_utils.h"
#include "lc_preset_manager_commands.h"
#include "lc_settings_colors_semantics.h"
#include "lc_shortcuts_manager.h"

namespace {
    QString resolveEffectiveTrigger(const QString& overrideText, const QString& activeDefault) {
        if (!overrideText.isEmpty()) {
            return (overrideText == "-") ? QString() : overrideText;
        }
        return (activeDefault == "-") ? QString() : activeDefault;
    }

    CommandDefinition* findOrAddCommandDef(CommandsConfig& config, const QString& actionName) {
        for (auto& def : config.commands) {
            if (def.actionName == actionName) {
                return &def;
            }
        }
        CommandDefinition def;
        def.actionName = actionName;
        config.commands.append(def);
        return &config.commands.last();
    }

    KeywordDefinition* findOrAddKeywordDef(CommandsConfig& config, const QString& key) {
        for (auto& kw : config.keywords) {
            if (kw.key == key) {
                return &kw;
            }
        }
        KeywordDefinition kw;
        kw.key = key;
        config.keywords.append(kw);
        return &config.keywords.last();
    }
}

LC_SettingsPageCommands::LC_SettingsPageCommands(LC_ActionGroupManager* groupManager, QObject* parent)
    : LC_SettingsPageBase(tr("Command Aliases"), nullptr, parent), ui(std::make_unique<Ui::LC_SettingsPageCommands>()),
      m_actionGroupManager(groupManager) {
    setSortWeight(15);
}

LC_SettingsPageCommands::~LC_SettingsPageCommands() {
    delete m_commandsTreeModel;
}

void LC_SettingsPageCommands::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerCommands*>(manager);
    if (m_presetManager != nullptr) {
        if (m_commandsTreeModel != nullptr) {
            m_commandsTreeModel->setFilterForConflicts(false);
        }
        ui->tvCommandsTree->clearSelection();
        ui->tvCommandsTree->setCurrentIndex(QModelIndex());
        m_selectedIdentifier.clear();
        selectItem(nullptr);

        rebuildTree(false);
        validateCollisions();
    }
}

void LC_SettingsPageCommands::setupUi() {
    ui->setupUi(m_widget);

    const QColor filteredCol = LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors::SearchResultItem, m_widget->palette());
    const QColor conflictCol = LC_PaletteColorUtils::getSemanticColor(LC_SemanticColors::ConflictingItem, m_widget->palette());

    m_commandsTreeModel = new LC_CommandsTreeModel(this, filteredCol, conflictCol);
    ui->tvCommandsTree->setModel(m_commandsTreeModel);
    ui->tvCommandsTree->setContextMenuPolicy(Qt::CustomContextMenu);

    auto* header = ui->tvCommandsTree->header();
    header->setVisible(true);
    header->setSectionResizeMode(LC_CommandsTreeModel::NAME, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(LC_CommandsTreeModel::KEYCODE, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(LC_CommandsTreeModel::COMMAND, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(LC_CommandsTreeModel::ALIAS, QHeaderView::Stretch);

    ui->splCommands->setChildrenCollapsible(false);
    ui->splCommands->setStretchFactor(0, 3);
    ui->splCommands->setStretchFactor(1, 2);
    trackSplitter(ui->splCommands, 480);

    LC_PagesUtils::updateLabelFont(ui->lblActionName, 1);

    connect(ui->bannerCollision, &LC_SettingsBannerWidget::linkActivated, this, &LC_SettingsPageCommands::onFilterConflictsRequested);

    ui->swDetails->setCurrentWidget(ui->pageEmpty);
}

void LC_SettingsPageCommands::setupBehavior() {
    connect(ui->tbExpandAll, &QToolButton::clicked, ui->tvCommandsTree, &QTreeView::expandAll);
    connect(ui->tbCollapseAll, &QToolButton::clicked, ui->tvCommandsTree, &QTreeView::collapseAll);

    connect(ui->leFilter, &QLineEdit::textChanged, this, &LC_SettingsPageCommands::onFilterChanged);
    connect(ui->cbMatchHighlight, &QCheckBox::toggled, this, &LC_SettingsPageCommands::onFilterChanged);

    connect(ui->tvCommandsTree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &LC_SettingsPageCommands::onTreeSelectionChanged);
    connect(ui->tvCommandsTree, &QTreeView::customContextMenuRequested, this, &LC_SettingsPageCommands::onTreeContextMenuRequested);

    connect(ui->leMainOverride, &QLineEdit::textChanged, this, &LC_SettingsPageCommands::onOverrideEdited);
    connect(ui->leAlias1Override, &QLineEdit::textChanged, this, &LC_SettingsPageCommands::onOverrideEdited);
    connect(ui->leAlias2Override, &QLineEdit::textChanged, this, &LC_SettingsPageCommands::onOverrideEdited);

    connect(ui->tbResetMain, &QToolButton::clicked, this, &LC_SettingsPageCommands::onResetMainClicked);
    connect(ui->tbSuppressAlias1, &QToolButton::clicked, this, &LC_SettingsPageCommands::onSuppressAlias1Clicked);
    connect(ui->tbResetAlias1, &QToolButton::clicked, this, &LC_SettingsPageCommands::onResetAlias1Clicked);
    connect(ui->tbSuppressAlias2, &QToolButton::clicked, this, &LC_SettingsPageCommands::onSuppressAlias2Clicked);
    connect(ui->tbResetAlias2, &QToolButton::clicked, this, &LC_SettingsPageCommands::onResetAlias2Clicked);

    connect(ui->bannerCollision, &LC_SettingsBannerWidget::linkActivated, this, &LC_SettingsPageCommands::onFilterConflictsRequested);

    connect(ui->tbExportCheatsheet, &QToolButton::clicked, this, &LC_SettingsPageCommands::onExportCheatsheetClicked);
}

void LC_SettingsPageCommands::setReadOnly(bool readOnly) {
    m_isReadOnly = readOnly;

    ui->paneTree->setEnabled(true);
    ui->tvCommandsTree->setEnabled(true);
    ui->leFilter->setEnabled(true);
    ui->cbMatchHighlight->setEnabled(true);
    ui->tbExpandAll->setEnabled(true);
    ui->tbCollapseAll->setEnabled(true);

    ui->gbCommandsEditor->setEnabled(!readOnly);
    ui->leMainOverride->setEnabled(!readOnly);
    ui->tbResetMain->setEnabled(!readOnly);
    ui->leAlias1Override->setEnabled(!readOnly);
    ui->tbSuppressAlias1->setEnabled(!readOnly);
    ui->tbResetAlias1->setEnabled(!readOnly);
    ui->leAlias2Override->setEnabled(!readOnly);
    ui->tbSuppressAlias2->setEnabled(!readOnly);
    ui->tbResetAlias2->setEnabled(!readOnly);
}

void LC_SettingsPageCommands::loadSettings() {
    if (m_commandsTreeModel != nullptr) {
        m_commandsTreeModel->setFilterForConflicts(false);
    }
    if (m_presetManager != nullptr) {
        m_presetManager->loadPreset(m_presetManager->getActivePresetKey());
    }

    ui->tvCommandsTree->clearSelection();
    ui->tvCommandsTree->setCurrentIndex(QModelIndex());
    m_selectedIdentifier.clear();
    selectItem(nullptr);

    rebuildTree(false);
    validateCollisions();
}

bool LC_SettingsPageCommands::saveSettings() {
    if (m_presetManager != nullptr) {
        return m_presetManager->saveCurrentPreset();
    }
    return true;
}

bool LC_SettingsPageCommands::isModified() const {
    return (m_presetManager != nullptr) && m_presetManager->isPresetModified();
}

bool LC_SettingsPageCommands::validate(QString& outErrorMessage) {
    if (!validateCollisions()) {
        outErrorMessage = tr("There are unresolved command alias conflicts. Please resolve all collisions before saving.");
        onFilterConflictsRequested();
        return false;
    }
    return true;
}

void LC_SettingsPageCommands::autoIndexLabels() {
    LC_SettingsPageBase::autoIndexLabels();
    autoIndexActionGroupManager(m_actionGroupManager, ui->tvCommandsTree);
}

void LC_SettingsPageCommands::highlightSearchPattern(const QString& pattern) {
    ui->leFilter->setText(pattern);
}

void LC_SettingsPageCommands::clearSearchHighlight() {
    ui->leFilter->clear();
}

void LC_SettingsPageCommands::onFilterChanged() {
    if (m_commandsTreeModel != nullptr) {
        m_commandsTreeModel->setFilteringRegexp(ui->leFilter->text(), ui->cbMatchHighlight->isChecked());
        rebuildTree(true);
    }
}

void LC_SettingsPageCommands::onFilterConflictsRequested() {
    if (m_commandsTreeModel != nullptr) {
        m_commandsTreeModel->setFilterForConflicts(true);
        rebuildTree(true);
    }
}

void LC_SettingsPageCommands::onClearConflictsFilterRequested() {
    if (m_commandsTreeModel != nullptr) {
        m_commandsTreeModel->setFilterForConflicts(false);
        rebuildTree(true);
    }
}

void LC_SettingsPageCommands::onTreeSelectionChanged() {
    const QModelIndexList selected = ui->tvCommandsTree->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        selectItem(nullptr);
        return;
    }

    auto* item = m_commandsTreeModel->getItemForIndex(selected.first());
    selectItem(item);
}

void LC_SettingsPageCommands::selectItem(LC_CommandsTreeItem* item) {
    m_currentItem = item;

    if (item == nullptr || item->isGroup()) {
        ui->swDetails->setCurrentWidget(ui->pageEmpty);
        m_systemCommands.clear();
        m_systemKeycodes.clear();
        m_systemAliases.clear();
        validateCollisions();
        return;
    }

    m_selectedIdentifier = item->identifier();
    ui->swDetails->setCurrentWidget(ui->pageDetails);
    ui->lblActionIcon->setPixmap(item->icon().pixmap(24, 24));
    ui->lblActionName->setText(item->name());

    // Display Category Name and Icon
    auto* parentItem = item->parent();
    if (parentItem != nullptr) {
        ui->lblGroupName->setText(parentItem->name());
        if (!parentItem->icon().isNull()) {
            ui->lblGroupIcon->setPixmap(parentItem->icon().pixmap(16, 16));
            ui->lblGroupIcon->setVisible(true);
        }
        else {
            ui->lblGroupIcon->setVisible(false);
        }
    }
    else {
        ui->lblGroupName->setText(tr("General"));
        ui->lblGroupIcon->setVisible(false);
    }

    m_systemCommands.clear();
    m_systemKeycodes.clear();
    m_systemAliases.clear();

    m_blockSignals = true;

    if (!item->isKeyword()) {
        // --- CAD Action Command Item ---
        ui->lblMain->setText(tr("Command:"));
        ui->lblAlias2->setText(tr("Alias:"));
        setKeycodeRowVisible(true);
        ui->leAlias1Override->setMaxLength(2);

        const QString actionName = item->identifier();
        const auto* mapper = (m_presetManager != nullptr) ? m_presetManager->getActionTypeMapper() : nullptr;
        const auto actionType = (mapper != nullptr) ? mapper->actionTypeFromName(actionName) : RS2::ActionNone;

        for (const auto& gItem : g_commandList) {
            if (gItem.actionType == actionType) {
                // Collect all system full commands (localized and English)
                for (const auto& [fullCmd, cmdTrans] : gItem.fullCmdList) {
                    if (!cmdTrans.isEmpty() && !m_systemCommands.contains(cmdTrans, Qt::CaseInsensitive)) {
                        m_systemCommands.append(cmdTrans);
                    }
                    if (!fullCmd.isEmpty() && !m_systemCommands.contains(fullCmd, Qt::CaseInsensitive)) {
                        m_systemCommands.append(fullCmd);
                    }
                }
                // Collect primary 2-letter keycodes
                if (!gItem.shortCmdList.empty()) {
                    const auto& [firstKey, firstTrans] = gItem.shortCmdList[0];
                    if (!firstTrans.isEmpty() && !m_systemKeycodes.contains(firstTrans, Qt::CaseInsensitive)) {
                        m_systemKeycodes.append(firstTrans);
                    }
                    if (!firstKey.isEmpty() && !m_systemKeycodes.contains(firstKey, Qt::CaseInsensitive)) {
                        m_systemKeycodes.append(firstKey);
                    }
                }
                // Collect all other aliases
                for (size_t i = 1; i < gItem.shortCmdList.size(); ++i) {
                    const auto& [aliasKey, aliasTrans] = gItem.shortCmdList[i];
                    if (!aliasTrans.isEmpty() && !m_systemAliases.contains(aliasTrans, Qt::CaseInsensitive)) {
                        m_systemAliases.append(aliasTrans);
                    }
                    if (!aliasKey.isEmpty() && !m_systemAliases.contains(aliasKey, Qt::CaseInsensitive)) {
                        m_systemAliases.append(aliasKey);
                    }
                }
                break;
            }
        }

        const QString actCmd = !m_systemCommands.isEmpty() ? m_systemCommands.first() : QString();
        const QString actKey = !m_systemKeycodes.isEmpty() ? m_systemKeycodes.first() : QString();
        const QString actAls = !m_systemAliases.isEmpty() ? m_systemAliases.first() : QString();

        ui->leMainOverride->setPlaceholderText(!actCmd.isEmpty() ? actCmd : tr("<None>"));
        ui->leAlias1Override->setPlaceholderText(!actKey.isEmpty() ? actKey : tr("<None>"));
        ui->leAlias2Override->setPlaceholderText(!actAls.isEmpty() ? actAls : tr("<None>"));

        // Tooltips with full system/localized defaults
        QString tipCmd = tr("Command\n\nOverride command name (leave empty to use default, '-' to suppress).");
        tipCmd += "\n" + tr("Default: %1").arg(!m_systemCommands.isEmpty() ? m_systemCommands.join(", ") : tr("<None>"));
        ui->leMainOverride->setToolTip(tipCmd);

        QString tipKey = tr(
            "Keycode\n\nOverride two-letter keycode (leave empty to use default, '-' to suppress).\nExecutes immediately without Enter in Keycode Mode.");
        tipKey += "\n" + tr("Default Keycode: %1").arg(!m_systemKeycodes.isEmpty() ? m_systemKeycodes.join(", ") : tr("<None>"));

        ui->leAlias1Override->setToolTip(tipKey);

        QString tipAls = tr("Alias\n\nOverride alternative alias (leave empty to use default, '-' to suppress).");
        tipAls += "\n" + tr("Default Alias: %1").arg(!m_systemAliases.isEmpty() ? m_systemAliases.join(", ") : tr("<None>"));
        ui->leAlias2Override->setToolTip(tipAls);

        QString ovCmd, ovKey, ovAls;
        if (m_presetManager != nullptr) {
            for (const auto& def : m_presetManager->workingConfig().commands) {
                if (def.actionName == actionName) {
                    ovCmd = def.customCommand;
                    ovKey = def.customKeycode;
                    ovAls = def.customAlias;
                    break;
                }
            }
        }

        ui->leMainOverride->setText(ovCmd);
        ui->leAlias1Override->setText(ovKey);
        ui->leAlias2Override->setText(ovAls);

        const QString desc = m_currentItem->description();
        ui->lblDescription->setText(!desc.isEmpty() ? desc : tr("No description available."));
    }
    else {
        // --- In-Prompt Keyword Item ---
        ui->lblMain->setText(tr("Keyword:"));
        ui->lblAlias2->setText(tr("Alias:"));
        setKeycodeRowVisible(false);

        // Keywords: Populate default keyword and default aliases from g_transList
        const QString key = item->identifier();
        QString transKw = key;
        for (const auto& [cmd, trans] : g_transList) {
            if (cmd == key) {
                transKw = trans;
                break;
            }
        }

        if (!transKw.isEmpty()) {
            m_systemCommands.append(transKw);
        }
        if (!key.isEmpty() && !m_systemCommands.contains(key, Qt::CaseInsensitive)) {
            m_systemCommands.append(key);
        }

        for (const auto& [cmd, trans] : g_transList) {
            if (trans == key) {
                const QString lower = cmd.trimmed().toLower();
                if (!lower.isEmpty() && lower != key.toLower() && lower != transKw.toLower()) {
                    if (!m_systemAliases.contains(lower, Qt::CaseInsensitive)) {
                        m_systemAliases.append(lower);
                    }
                }
            }
        }

        ui->leMainOverride->setPlaceholderText(transKw);
        ui->leAlias1Override->clear();
        ui->leAlias2Override->setPlaceholderText(!m_systemAliases.isEmpty() ? m_systemAliases.first() : tr("<None>"));

        QString tipKw = tr("Keyword\n\nOverride prompt keyword (leave empty to use default, '-' to suppress).");
        tipKw += "\n" + tr("Default: %1").arg(m_systemCommands.join(", "));
        ui->leMainOverride->setToolTip(tipKw);

        QString tipKwAls = tr("Alias\n\nOverride prompt keyword alias (leave empty to use default, '-' to suppress).");
        if (!m_systemAliases.isEmpty()) {
            tipKwAls += "\n" + tr("Default: %1").arg(m_systemAliases.join(", "));
        }
        ui->leAlias2Override->setToolTip(tipKwAls);

        QString ovKw, ovAls;
        if (m_presetManager != nullptr) {
            for (const auto& kwDef : m_presetManager->workingConfig().keywords) {
                if (kwDef.key == key) {
                    ovKw = kwDef.customKeyword;
                    ovAls = kwDef.customAlias;
                    break;
                }
            }
        }

        ui->leMainOverride->setText(ovKw);
        ui->leAlias1Override->clear();
        ui->leAlias2Override->setText(ovAls);

        ui->lblDescription->setText(tr("In-prompt option keyword used during active command execution."));
    }

    m_blockSignals = false;
    updateEffectiveSummary();
    validateCollisions();

    setReadOnly(m_isReadOnly);
}

void LC_SettingsPageCommands::updateEffectiveSummary() {
    if (m_currentItem == nullptr || m_currentItem->isGroup()) {
        ui->lblAllTriggers->setText(tr("<None>"));
        ui->lblKeycodes->setText(tr("<None>"));
        return;
    }

    // Row 2: Comprehensive list directly from the item's stored effectiveTriggers
    const QStringList triggers = m_currentItem->effectiveTriggers();
    ui->lblAllTriggers->setText(!triggers.isEmpty() ? triggers.join(", ") : tr("<None>"));

    // Row 3: Keycodes (Actions only)
    if (!m_currentItem->isKeyword()) {
        const QString ovKey = ui->leAlias1Override->text().trimmed();
        QStringList keycodes;
        auto appendUniqueKeycode = [&keycodes](const QString& str) {
            if (!str.isEmpty() && str != "-" && !keycodes.contains(str, Qt::CaseInsensitive)) {
                keycodes.append(str);
            }
        };

        if (ovKey != "-") {
            if (!ovKey.isEmpty()) {
                appendUniqueKeycode(ovKey);
            }
            for (const auto& key : m_systemKeycodes) {
                appendUniqueKeycode(key);
            }
        }

        ui->lblKeycodes->setText(!keycodes.isEmpty() ? keycodes.join(", ") : tr("<None>"));
    }
}

void LC_SettingsPageCommands::onOverrideEdited() {
    if (m_blockSignals || m_currentItem == nullptr || m_presetManager == nullptr) {
        return;
    }

    const QString id = m_currentItem->identifier();
    const QString ovCmd = ui->leMainOverride->text().trimmed();
    const QString ovKey = ui->leAlias1Override->text().trimmed();
    const QString ovAls = ui->leAlias2Override->text().trimmed();

    auto& config = m_presetManager->workingConfig();
    QStringList effectiveTriggers;

    if (!m_currentItem->isKeyword()) {
        auto* def = findOrAddCommandDef(config, id);
        if (def != nullptr) {
            def->customCommand = ovCmd;
            def->customKeycode = ovKey;
            def->customAlias = ovAls;

            const auto* mapper = m_presetManager->getActionTypeMapper();
            const auto actionType = (mapper != nullptr) ? mapper->actionTypeFromName(id) : RS2::ActionNone;
            effectiveTriggers = LC_CommandsTreeModel::computeActionEffectiveTriggers(actionType, *def);
        }
    }
    else {
        auto* kw = findOrAddKeywordDef(config, id);
        if (kw != nullptr) {
            kw->customKeyword = ovCmd;
            kw->customAlias = ovAls;

            effectiveTriggers = LC_CommandsTreeModel::computeKeywordEffectiveTriggers(id, *kw);
        }
    }

    // Format the working trigger lists for the 3 table columns
    auto formatCol = [](const QString& ov, const QStringList& defs) -> QString {
        if (ov == "-") {
            return tr("— (suppressed)");
        }
        QStringList list;
        if (!ov.isEmpty()) {
            list.append(ov);
        }
        for (const QString& d : defs) {
            if (!list.contains(d, Qt::CaseInsensitive)) {
                list.append(d);
            }
        }
        return list.join(", ");
    };

    const QString colCmd = formatCol(ovCmd, m_systemCommands);
    const QString colKey = !m_currentItem->isKeyword() ? formatCol(ovKey, m_systemKeycodes) : QString();
    const QString colAls = formatCol(ovAls, m_systemAliases);

    const bool isModified = !m_currentItem->isKeyword()
                                ? (!ovCmd.isEmpty() || !ovKey.isEmpty() || !ovAls.isEmpty())
                                : (!ovCmd.isEmpty() || !ovAls.isEmpty());

    m_commandsTreeModel->updateItemTriggers(m_currentItem, colCmd, colKey, colAls, effectiveTriggers, isModified);

    updateEffectiveSummary();

    const auto* mapper = m_presetManager->getActionTypeMapper();
    m_commandsTreeModel->recalculateCollisions(config, m_actionGroupManager, mapper);

    // If filtering for conflicts, rebuild to dynamically add newly conflicted or remove resolved items
    if (m_commandsTreeModel->isFilterForConflicts()) {
        rebuildTree(true);
    }
    else {
        // Force immediate viewport repaint for all visible sibling rows
        ui->tvCommandsTree->viewport()->update();
    }

    validateCollisions();

    m_presetManager->notifyConfigChanged();
    emit livePreviewRequested();
}

void LC_SettingsPageCommands::onResetMainClicked() {
    ui->leMainOverride->clear();
}

void LC_SettingsPageCommands::onSuppressAlias1Clicked() {
    ui->leAlias1Override->setText("-");
}

void LC_SettingsPageCommands::onResetAlias1Clicked() {
    ui->leAlias1Override->clear();
}

void LC_SettingsPageCommands::onSuppressAlias2Clicked() {
    ui->leAlias2Override->setText("-");
}

void LC_SettingsPageCommands::onResetAlias2Clicked() {
    ui->leAlias2Override->clear();
}

bool LC_SettingsPageCommands::validateCollisions() {
    if (m_commandsTreeModel == nullptr || m_presetManager == nullptr) {
        return true;
    }

    const auto* mapper = m_presetManager->getActionTypeMapper();
    QString conflictMsg;
    const bool hasCollisions = m_commandsTreeModel->checkForCollisions(m_presetManager->workingConfig(), m_actionGroupManager, mapper,
                                                                       &conflictMsg);

    const bool isFilteringConflicts = m_commandsTreeModel->isFilterForConflicts();

    const QString message = hasCollisions
                                ? tr("<b>Conflict Detected:</b> %1").arg(conflictMsg)
                                : tr("All command triggers and aliases are unique and valid.");

    const auto bannerType = hasCollisions ? LC_SettingsBannerWidget::BannerType::Error : LC_SettingsBannerWidget::BannerType::Info;

    QString actionText;
    std::function<void()> actionCallback;

    if (isFilteringConflicts) {
        actionText = tr("Show All Commands");
        actionCallback = [this]() {
            onClearConflictsFilterRequested();
        };
    }
    else if (hasCollisions) {
        actionText = tr("Filter Conflicts");
        actionCallback = [this]() {
            onFilterConflictsRequested();
        };
    }

    ui->bannerCollision->setMessage(message, bannerType, actionText, actionCallback);
    return !hasCollisions;
}

void LC_SettingsPageCommands::rebuildTree(bool restoreSelection) {
    if (m_commandsTreeModel == nullptr || m_presetManager == nullptr) {
        return;
    }

    // Capture the stable identifier by value before the tree is destroyed
    const QString targetId = m_selectedIdentifier;

    // Invalidate the raw pointer upfront to prevent any dangling pointer dereference
    m_currentItem = nullptr;

    m_commandsTreeModel->rebuildModel(m_presetManager->workingConfig(), m_actionGroupManager, m_presetManager->getActionTypeMapper());

    ui->tvCommandsTree->expandAll();

    bool restored = false;
    if (restoreSelection && !targetId.isEmpty()) {
        for (int r = 0; r < m_commandsTreeModel->rowCount(); ++r) {
            const QModelIndex groupIdx = m_commandsTreeModel->index(r, 0);
            for (int c = 0; c < m_commandsTreeModel->rowCount(groupIdx); ++c) {
                const QModelIndex childIdx = m_commandsTreeModel->index(c, 0, groupIdx);
                auto* item = m_commandsTreeModel->getItemForIndex(childIdx);
                if (item != nullptr && item->identifier() == targetId) {
                    ui->tvCommandsTree->setCurrentIndex(childIdx);
                    ui->tvCommandsTree->scrollTo(childIdx, QAbstractItemView::EnsureVisible);
                    selectItem(item);
                    restored = true;
                    break;
                }
            }
            if (restored) {
                break;
            }
        }
    }

    // If not restored (or filtered out by the search term), cleanly reset the detail pane
    if (!restored) {
        selectItem(nullptr);
    }
}

void LC_SettingsPageCommands::setKeycodeRowVisible(bool visible) {
    ui->lblAlias1->setVisible(visible);
    ui->leAlias1Override->setVisible(visible);
    ui->tbResetAlias1->setVisible(visible);
    ui->tbSuppressAlias1->setVisible(visible);

    ui->lblHdrKeycodes->setVisible(visible);
    ui->lblKeycodes->setVisible(visible);
}

void LC_SettingsPageCommands::onResetItemToDefaults() {
    if (m_currentItem != nullptr && !m_isReadOnly) {
        ui->leMainOverride->clear();
        ui->leAlias1Override->clear();
        ui->leAlias2Override->clear();
    }
}

void LC_SettingsPageCommands::onSuppressItemAliases() {
    if (m_currentItem != nullptr && !m_isReadOnly) {
        if (!m_currentItem->isKeyword()) {
            ui->leAlias1Override->setText("-");
        }
        ui->leAlias2Override->setText("-");
    }
}

void LC_SettingsPageCommands::onExportCheatsheetClicked() {
    LC_CheatsheetOptions options;
    options.type = CheatsheetType::CommandAliases;
    options.schemeName = (m_presetManager != nullptr) ? m_presetManager->currentPresetDisplayName() : tr("Default");

    options.isModelFiltered = (m_commandsTreeModel != nullptr) && m_commandsTreeModel->isFiltered();
    options.onlyFilteredItems = options.isModelFiltered;

    auto parent = getEditingWidget();
    LC_DlgCheatsheetOptions dlg(parent, options);
    if (dlg.exec() == QDialog::Accepted) {
        if (options.isModelFiltered && !options.onlyFilteredItems) {
            // User requested full catalog despite active UI filter: generate from unfiltered model
            LC_CommandsTreeModel fullModel;
            fullModel.rebuildModel(m_presetManager->workingConfig(), m_actionGroupManager, m_presetManager->getActionTypeMapper());
            LC_CheatsheetGenerator::generate(parent, options, &fullModel);
        }
        else {
            LC_CheatsheetGenerator::generate(parent, options, m_commandsTreeModel);
        }
    }
}

void LC_SettingsPageCommands::onTreeContextMenuRequested(const QPoint& pos) {
    const QModelIndex idx = ui->tvCommandsTree->indexAt(pos);
    auto* item = m_commandsTreeModel->getItemForIndex(idx);
    QMenu menu(ui->tvCommandsTree);

    // 1. Click on Leaf Action / Keyword Item
    if (item != nullptr && !item->isGroup()) {
        ui->tvCommandsTree->setCurrentIndex(idx);
        selectItem(item);

        if (!m_isReadOnly) {
            menu.addAction(QIcon(":/icons/rename_active_block.lci"), tr("Edit Triggers..."), this, [this]() {
                ui->leMainOverride->setFocus();
                ui->leMainOverride->selectAll();
            });

            menu.addSeparator();

            auto* actReset = menu.addAction(QIcon(":/icons/undo.lci"), tr("Reset to Defaults"), this,
                                            &LC_SettingsPageCommands::onResetItemToDefaults);
            actReset->setEnabled(item->isModified());

            menu.addAction(QIcon(":/icons/remove.lci"), tr("Suppress Aliases"), this, &LC_SettingsPageCommands::onSuppressItemAliases);

            menu.addSeparator();
        }

        // Column-aware copy resolution
        const int col = idx.column();
        QString textToCopy;
        QString copyLabel;

        if (col == LC_CommandsTreeModel::KEYCODE) {
            textToCopy = item->getKeyCode();
            copyLabel = tr("Copy Keycode");
        }
        else if (col == LC_CommandsTreeModel::ALIAS) {
            textToCopy = item->getAlias();
            copyLabel = tr("Copy Alias");
        }
        else {
            // Column NAME or COMMAND
            textToCopy = item->getMainCommand();
            copyLabel = tr("Copy Command");
        }

        if (!textToCopy.isEmpty()) {
            copyLabel += QString(" ('%1')").arg(textToCopy);
        }

        auto* actCopy = menu.addAction(QIcon(":/icons/copy.lci"), copyLabel, this, [textToCopy]() {
            if (!textToCopy.isEmpty()) {
                QGuiApplication::clipboard()->setText(textToCopy);
            }
        });
        actCopy->setEnabled(!textToCopy.isEmpty());

        menu.addSeparator();
        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvCommandsTree, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvCommandsTree, &QTreeView::collapseAll);
    }
    // 2. Click on Category Group Header
    else if (item != nullptr && item->isGroup()) {
        ui->tvCommandsTree->setCurrentIndex(idx);
        selectItem(item);

        if (ui->tvCommandsTree->isExpanded(idx)) {
            menu.addAction(tr("Collapse Group"), this, [this, idx]() {
                ui->tvCommandsTree->collapse(idx);
            });
        }
        else {
            menu.addAction(tr("Expand Group"), this, [this, idx]() {
                ui->tvCommandsTree->expand(idx);
            });
        }

        menu.addSeparator();
        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvCommandsTree, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvCommandsTree, &QTreeView::collapseAll);
    }
    // 3. Click on Empty Viewport Space
    else {
        const bool hasConflicts = !m_commandsTreeModel->checkForCollisions(m_presetManager->workingConfig(), m_actionGroupManager,
                                                                           m_presetManager->getActionTypeMapper());

        if (m_commandsTreeModel->isFilterForConflicts()) {
            menu.addAction(tr("Show All Commands"), this, &LC_SettingsPageCommands::onClearConflictsFilterRequested);
            menu.addSeparator();
        }
        else if (hasConflicts) {
            menu.addAction(tr("Filter Conflicts"), this, &LC_SettingsPageCommands::onFilterConflictsRequested);
            menu.addSeparator();
        }

        menu.addAction(QIcon(":/icons/expand_all.lci"), tr("Expand All"), ui->tvCommandsTree, &QTreeView::expandAll);
        menu.addAction(QIcon(":/icons/collapse_all.lci"), tr("Collapse All"), ui->tvCommandsTree, &QTreeView::collapseAll);
    }

    menu.exec(ui->tvCommandsTree->viewport()->mapToGlobal(pos));
}
