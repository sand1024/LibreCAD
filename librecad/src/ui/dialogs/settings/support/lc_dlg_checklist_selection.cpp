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

#include "lc_dlg_checklist_selection.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

LC_DlgChecklistSelection::LC_DlgChecklistSelection(QWidget* parent,
                                                  const QString& title,
                                                  const QString& message,
                                                  QList<LC_ChecklistChoice>& choices)
    : QDialog(parent)
    , m_choices(choices) {
    setWindowTitle(title);
    setMinimumWidth(400);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    auto* lblMessage = new QLabel(message, this);
    lblMessage->setWordWrap(true);
    mainLayout->addWidget(lblMessage);

    // Optional Header Input Container (Hidden by default; revealed via setHeaderInput)
    m_inputContainer = new QWidget(this);
    auto* inputLayout = new QHBoxLayout(m_inputContainer);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);

    m_lblInput = new QLabel(m_inputContainer);
    m_leInput = new QLineEdit(m_inputContainer);
    connect(m_leInput, &QLineEdit::textChanged, this, &LC_DlgChecklistSelection::onInputTextChanged);

    inputLayout->addWidget(m_lblInput);
    inputLayout->addWidget(m_leInput);
    m_inputContainer->setVisible(false);
    mainLayout->addWidget(m_inputContainer);


    m_gbScope = new QGroupBox(this);
    auto* scopeLayout = new QVBoxLayout(m_gbScope);
    scopeLayout->setSpacing(4);

    m_rbFullScope = new QRadioButton(m_gbScope);
    m_rbActiveOnly = new QRadioButton(m_gbScope);

    scopeLayout->addWidget(m_rbFullScope);
    scopeLayout->addWidget(m_rbActiveOnly);
    m_gbScope->setVisible(false);
    mainLayout->addWidget(m_gbScope);

    auto* group = new QGroupBox(tr("Components"), this);
    auto* groupLayout = new QVBoxLayout(group);
    groupLayout->setSpacing(6);

    for (int i = 0; i < m_choices.size(); ++i) {
        auto* cb = new QCheckBox(m_choices.at(i).label, group);
        cb->setChecked(m_choices.at(i).checked);
        cb->setEnabled(m_choices.at(i).enabled);
        if (!m_choices.at(i).tooltip.isEmpty()) {
            cb->setToolTip(m_choices.at(i).tooltip);
        }
        connect(cb, &QCheckBox::toggled, this, &LC_DlgChecklistSelection::onChoiceToggled);
        m_checkBoxes.push_back(cb);
        groupLayout->addWidget(cb);
    }
    mainLayout->addWidget(group);

    auto* btnRowLayout = new QHBoxLayout();
    auto* btnSelectAll = new QPushButton(tr("Select All"), this);
    btnSelectAll->setToolTip(tr("Select all components in the list"));
    auto* btnDeselectAll = new QPushButton(tr("Deselect All"), this);
    btnDeselectAll->setToolTip(tr("Deselect all components in the list"));

    connect(btnSelectAll, &QPushButton::clicked, this, &LC_DlgChecklistSelection::onSelectAllClicked);
    connect(btnDeselectAll, &QPushButton::clicked, this, &LC_DlgChecklistSelection::onDeselectAllClicked);

    btnRowLayout->addWidget(btnSelectAll);
    btnRowLayout->addWidget(btnDeselectAll);
    btnRowLayout->addStretch();
    mainLayout->addLayout(btnRowLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);

    updateOkButtonState();
}

void LC_DlgChecklistSelection::setHeaderInput(const QString& label, const QString& defaultValue, const QString& tooltip) {
    if (m_lblInput != nullptr && m_leInput != nullptr && m_inputContainer != nullptr) {
        m_lblInput->setText(label);
        m_leInput->setText(defaultValue);
        if (!tooltip.isEmpty()) {
            m_leInput->setToolTip(tooltip);
        }
        m_inputContainer->setVisible(true);
        m_leInput->setFocus();
        m_leInput->selectAll();
        updateOkButtonState();
    }
}

QString LC_DlgChecklistSelection::headerInputText() const {
    return (m_leInput != nullptr) ? m_leInput->text().trimmed() : QString();
}

bool LC_DlgChecklistSelection::selectChoices(QWidget* parent,
                                            const QString& title,
                                            const QString& message,
                                            QList<LC_ChecklistChoice>& choices) {
    if (choices.isEmpty()) {
        return false;
    }

    LC_DlgChecklistSelection dlg(parent, title, message, choices);
    if (dlg.exec() == QDialog::Accepted) {
        for (size_t i = 0; i < dlg.m_checkBoxes.size(); ++i) {
            choices[i].checked = dlg.m_checkBoxes[i]->isChecked();
        }
        return true;
    }
    return false;
}

bool LC_DlgChecklistSelection::selectChoicesWithInput(QWidget* parent,
                                                     const QString& title,
                                                     const QString& message,
                                                     const QString& inputLabel,
                                                     const QString& defaultInputText,
                                                     QString& outInputText,
                                                     QList<LC_ChecklistChoice>& choices) {
    if (choices.isEmpty()) {
        return false;
    }

    LC_DlgChecklistSelection dlg(parent, title, message, choices);
    dlg.setHeaderInput(inputLabel, defaultInputText, tr("Enter profile identifier"));

    if (dlg.exec() == QDialog::Accepted) {
        outInputText = dlg.headerInputText();
        for (size_t i = 0; i < dlg.m_checkBoxes.size(); ++i) {
            choices[i].checked = dlg.m_checkBoxes[i]->isChecked();
        }
        return true;
    }
    return false;
}

void LC_DlgChecklistSelection::onSelectAllClicked() {
    for (auto* cb : m_checkBoxes) {
        if (cb != nullptr && cb->isEnabled()) {
            cb->setChecked(true);
        }
    }
    updateOkButtonState();
}

void LC_DlgChecklistSelection::onDeselectAllClicked() {
    for (auto* cb : m_checkBoxes) {
        if (cb != nullptr && cb->isEnabled()) {
            cb->setChecked(false);
        }
    }
    updateOkButtonState();
}

void LC_DlgChecklistSelection::onChoiceToggled() {
    updateOkButtonState();
}

void LC_DlgChecklistSelection::onInputTextChanged(const QString&) {
    updateOkButtonState();
}

void LC_DlgChecklistSelection::updateOkButtonState() {
    if (m_buttonBox == nullptr) {
        return;
    }

    bool hasAnyChecked = false;
    for (const auto* cb : m_checkBoxes) {
        if (cb != nullptr && cb->isChecked()) {
            hasAnyChecked = true;
            break;
        }
    }

    bool hasValidName = true;
    if (m_inputContainer != nullptr && m_inputContainer->isVisible() && m_leInput != nullptr) {
        hasValidName = !m_leInput->text().trimmed().isEmpty();
    }

    auto* okBtn = m_buttonBox->button(QDialogButtonBox::Ok);
    if (okBtn != nullptr) {
        okBtn->setEnabled(hasAnyChecked && hasValidName);
    }
}

void LC_DlgChecklistSelection::setScopeSelection(const QString& groupTitle,
                                                 const QString& fullScopeText,
                                                 const QString& activeOnlyText,
                                                 const bool defaultFullScope,
                                                 const QString& fullScopeTooltip,
                                                 const QString& activeOnlyTooltip) {
    if (m_gbScope != nullptr && m_rbFullScope != nullptr && m_rbActiveOnly != nullptr) {
        m_gbScope->setTitle(groupTitle);
        m_rbFullScope->setText(fullScopeText);
        m_rbActiveOnly->setText(activeOnlyText);

        if (!fullScopeTooltip.isEmpty()) {
            m_rbFullScope->setToolTip(fullScopeTooltip);
        }
        if (!activeOnlyTooltip.isEmpty()) {
            m_rbActiveOnly->setToolTip(activeOnlyTooltip);
        }

        m_rbFullScope->setChecked(defaultFullScope);
        m_rbActiveOnly->setChecked(!defaultFullScope);
        m_gbScope->setVisible(true);
    }
}

bool LC_DlgChecklistSelection::isFullScopeSelected() const {
    if (m_rbFullScope != nullptr && m_gbScope != nullptr && m_gbScope->isVisible()) {
        return m_rbFullScope->isChecked();
    }
    return true;
}

bool LC_DlgChecklistSelection::selectChoicesWithInputAndScope(QWidget* parent,
                                                             const QString& title,
                                                             const QString& message,
                                                             const QString& inputLabel,
                                                             const QString& defaultInputText,
                                                             const QString& scopeTitle,
                                                             const QString& fullScopeText,
                                                             const QString& activeOnlyText,
                                                             bool& outIsFullScope,
                                                             QString& outInputText,
                                                             QList<LC_ChecklistChoice>& choices) {
    if (choices.isEmpty()) {
        return false;
    }

    LC_DlgChecklistSelection dlg(parent, title, message, choices);
    dlg.setHeaderInput(inputLabel, defaultInputText, tr("Enter unique label"));
    dlg.setScopeSelection(scopeTitle, fullScopeText, activeOnlyText, outIsFullScope);

    if (dlg.exec() == QDialog::Accepted) {
        outInputText = dlg.headerInputText();
        outIsFullScope = dlg.isFullScopeSelected();
        for (size_t i = 0; i < dlg.m_checkBoxes.size(); ++i) {
            choices[i].checked = dlg.m_checkBoxes[i]->isChecked();
        }
        return true;
    }
    return false;
}
