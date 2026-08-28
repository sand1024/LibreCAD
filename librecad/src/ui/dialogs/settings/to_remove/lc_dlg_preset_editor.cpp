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

#include "lc_dlg_preset_editor.h"
#include "lc_preset_container_widget.h"
#include "lc_style_editor_interface.h"
#include "lc_ui_style_manager.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCloseEvent>

LC_DlgPresetEditor::LC_DlgPresetEditor(QWidget* parent,
                                       LC_UIStyleManager* styleManager,
                                       LC_StyleEditorInterface* editor,
                                       const QString& dialogKey,
                                       const QString& initialPresetKey)
    : LC_Dialog(parent, dialogKey)
    , m_editor(editor) {

    m_editorCategoryName = m_editor->getPresetCategoryName();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(9, 9, 9, 9);
    mainLayout->setSpacing(8);

    // 1. Embed the generic preset container
    m_container = new LC_PresetContainerWidget(this, styleManager, editor);
    mainLayout->addWidget(m_container, 1);

    // 2. Add standard bottom close button box
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &LC_DlgPresetEditor::reject);
    connect(m_container, &LC_PresetContainerWidget::dirtinessChanged, this, &LC_DlgPresetEditor::onDirtinessChanged);
    connect(m_container, &LC_PresetContainerWidget::activePresetChanged, this, &LC_DlgPresetEditor::onActivePresetChanged);

    // Initialize the pluggable layout
    m_container->initialize(initialPresetKey);
    updateWindowTitle();
}

LC_DlgPresetEditor::~LC_DlgPresetEditor() = default;

void LC_DlgPresetEditor::onDirtinessChanged(bool isDirty) {
    m_isDirty = isDirty;
    updateWindowTitle();
}

void LC_DlgPresetEditor::onActivePresetChanged(const QString& displayName) {
    m_currentPresetDisplayName = displayName;
    updateWindowTitle();
}

void LC_DlgPresetEditor::updateWindowTitle() {
    // Symmetrical Title Formatter: e.g. "Skins Style Editor - Industrial Slate *"
    QString title = QString(tr("%1 Style Editor - %2"))
                    .arg(m_editorCategoryName, m_currentPresetDisplayName);
    if (m_isDirty) {
        title += " *";
    }
    setWindowTitle(title);
}

bool LC_DlgPresetEditor::canClose() {
    if (!m_isDirty) return true;

    auto reply = QMessageBox::question(this, tr("Unsaved Changes"),
        tr("You have unsaved changes. Do you want to discard them?"),
        QMessageBox::Yes | QMessageBox::No);

    return (reply == QMessageBox::Yes);
}

void LC_DlgPresetEditor::reject() {
    if (canClose()) {
        m_isDirty = false;
        m_container->rollback(); // Discards active draft edits and reverts style overrides
        LC_Dialog::accept();     // Returns Accepted so the parent options dialog reloads its dropdowns!
    }
}

void LC_DlgPresetEditor::closeEvent(QCloseEvent* event) {
    if (canClose()) {
        m_isDirty = false;
        m_container->rollback();
        event->accept();
        setResult(QDialog::Accepted); // Force Accepted result on window top-right "X" closure
    } else {
        event->ignore();
    }
}
