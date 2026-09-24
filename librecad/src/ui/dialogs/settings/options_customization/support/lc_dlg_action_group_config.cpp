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

#include "lc_dlg_action_group_config.h"
#include "ui_lc_dlg_action_group_config.h"

#include <QMessageBox>

#include "lc_action_group_manager.h"
#include "lc_dlg_icon_picker.h"

LC_DlgActionGroupConfig::LC_DlgActionGroupConfig(QWidget* parent, bool isToolbarMode, const LC_ActionGroupManager* groupManager)
    : QDialog(parent)
    , ui(std::make_unique<Ui::LC_DlgActionGroupConfig>())
    , m_isToolbarMode(isToolbarMode)
    , m_groupManager(groupManager) {
    ui->setupUi(this);

    ui->gbBehavior->setVisible(m_isToolbarMode);

    populateSystemCategories();

    connect(ui->rbPredefinedCategory, &QRadioButton::toggled, this, &LC_DlgActionGroupConfig::onGroupTypeToggled);
    connect(ui->rbCustomGroup, &QRadioButton::toggled, this, &LC_DlgActionGroupConfig::onGroupTypeToggled);
    connect(ui->cbSystemCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_DlgActionGroupConfig::onSystemCategoryChanged);

    connect(ui->pbChooseIcon, &QPushButton::clicked, this, &LC_DlgActionGroupConfig::onChooseIconClicked);
    connect(ui->rbSplit, &QRadioButton::toggled, this, &LC_DlgActionGroupConfig::onSplitToggled);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LC_DlgActionGroupConfig::validateAndAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    ui->rbInstant->setChecked(true);
    ui->cbSticky->setEnabled(false);

    onGroupTypeToggled();
}

LC_DlgActionGroupConfig::~LC_DlgActionGroupConfig() = default;

void LC_DlgActionGroupConfig::populateSystemCategories() {
    ui->cbSystemCategory->clear();
    if (m_groupManager == nullptr) {
        return;
    }

    const auto categories = m_groupManager->predefinedCategories();
    for (const auto& cat : categories) {
        const QString iconPath = m_groupManager->iconPath(cat.first);
        ui->cbSystemCategory->addItem(QIcon(iconPath), cat.second, cat.first);
    }
}

void LC_DlgActionGroupConfig::setInitialValues(const QString& rawTitle, const QString& iconPath, ToolButtonPopupMode mode) {
    m_iconPath = iconPath;

    // Block signals while setting up values programmatically to avoid false triggers
    const QSignalBlocker blocker1(ui->rbPredefinedCategory);
    const QSignalBlocker blocker2(ui->rbCustomGroup);
    const QSignalBlocker blocker3(ui->cbSystemCategory);

    const bool isPredefined = rawTitle.startsWith("Menu:");

    if (isPredefined) {
        ui->rbPredefinedCategory->setChecked(true);
        ui->rbCustomGroup->setChecked(false);

        int idx = ui->cbSystemCategory->findData(rawTitle);
        if (idx < 0) {
            // Token not in static list: insert dynamically so it is not lost
            const QString display = m_groupManager->displayName(rawTitle, /*stripAmpersand=*/true);
            const QString icon = m_groupManager->iconPath(rawTitle);
            ui->cbSystemCategory->addItem(QIcon(icon), display, rawTitle);
            idx = ui->cbSystemCategory->findData(rawTitle);
        }

        if (idx >= 0) {
            ui->cbSystemCategory->setCurrentIndex(idx);
        }

        if (m_iconPath.isEmpty()) {
            m_iconPath = m_groupManager->iconPath(rawTitle);
        }
        ui->leTitle->setText(ui->cbSystemCategory->currentText());
    } else {
        ui->rbPredefinedCategory->setChecked(false);
        ui->rbCustomGroup->setChecked(true);
        ui->leTitle->setText(rawTitle);
    }

    if (!m_iconPath.isEmpty()) {
        ui->tbIconPreview->setIcon(QIcon(m_iconPath));
    } else {
        ui->tbIconPreview->setIcon(QIcon());
    }

    if (m_isToolbarMode) {
        if (mode == ToolButtonPopupMode::InstantPopup) {
            ui->rbInstant->setChecked(true);
            ui->cbSticky->setChecked(false);
            ui->cbSticky->setEnabled(false);
        } else {
            ui->rbSplit->setChecked(true);
            ui->cbSticky->setEnabled(true);
            ui->cbSticky->setChecked(mode == ToolButtonPopupMode::SplitLastUsedAction);
        }
    }

    // Refresh enabled states of controls
    ui->cbSystemCategory->setEnabled(isPredefined);
    ui->lblSysCategory->setEnabled(isPredefined);
    ui->lblTitle->setEnabled(!isPredefined);
    ui->leTitle->setEnabled(!isPredefined);
    ui->pbChooseIcon->setEnabled(!isPredefined);
}

void LC_DlgActionGroupConfig::onGroupTypeToggled() {
    const bool isPredefined = ui->rbPredefinedCategory->isChecked();

    ui->cbSystemCategory->setEnabled(isPredefined);
    ui->lblSysCategory->setEnabled(isPredefined);

    ui->lblTitle->setEnabled(!isPredefined);
    ui->leTitle->setEnabled(!isPredefined);
    ui->pbChooseIcon->setEnabled(!isPredefined);

    if (isPredefined && m_groupManager != nullptr) {
        const QString token = ui->cbSystemCategory->currentData().toString();
        ui->leTitle->setText(ui->cbSystemCategory->currentText());
        m_iconPath = m_groupManager->iconPath(token);
        if (!m_iconPath.isEmpty()) {
            ui->tbIconPreview->setIcon(QIcon(m_iconPath));
        } else {
            ui->tbIconPreview->setIcon(QIcon());
        }
    }
}

void LC_DlgActionGroupConfig::onSystemCategoryChanged(int) {
    if (ui->rbPredefinedCategory->isChecked() && m_groupManager != nullptr) {
        const QString token = ui->cbSystemCategory->currentData().toString();
        ui->leTitle->setText(ui->cbSystemCategory->currentText());
        m_iconPath = m_groupManager->iconPath(token);
        ui->tbIconPreview->setIcon(QIcon(m_iconPath));
    }
}


void LC_DlgActionGroupConfig::onChooseIconClicked() {
    const QString picked = LC_DlgIconPicker::getIcon(this, m_iconPath);
    if (!picked.isEmpty()) {
        m_iconPath = picked;
        ui->tbIconPreview->setIcon(QIcon(m_iconPath));
    }
}

void LC_DlgActionGroupConfig::onSplitToggled(bool checked) {
    ui->cbSticky->setEnabled(checked);
    if (!checked) {
        ui->cbSticky->setChecked(false);
    }
}

void LC_DlgActionGroupConfig::validateAndAccept() {
    if (ui->leTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"), tr("Please enter a title for this group."));
        return;
    }
    accept();
}

QString LC_DlgActionGroupConfig::title() const {
    if (ui->rbPredefinedCategory->isChecked()) {
        return ui->cbSystemCategory->currentData().toString(); // Returns canonical "Menu:<Key>"
    }
    return ui->leTitle->text().trimmed(); // Returns custom literal text
}

QString LC_DlgActionGroupConfig::iconPath() const {
    if (ui->rbPredefinedCategory->isChecked() && m_groupManager != nullptr) {
        return m_groupManager->iconPath(title());
    }
    return m_iconPath;
}

ToolButtonPopupMode LC_DlgActionGroupConfig::popupMode() const {
    if (!m_isToolbarMode || ui->rbInstant->isChecked()) {
        return ToolButtonPopupMode::InstantPopup;
    }
    return ui->cbSticky->isChecked() ? ToolButtonPopupMode::SplitLastUsedAction
                                     : ToolButtonPopupMode::SplitFirstAction;
}
