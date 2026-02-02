/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_property_qstring_list_arrows_combobox_view_handler.h"

#include <QComboBox>

#include "lc_inputtextdialog.h"
#include "lc_property_qstring_list_arrows_combobox_view.h"

LC_PropertyQStringListArrowsComboboxViewHandler::LC_PropertyQStringListArrowsComboboxViewHandler(LC_PropertyViewEditable* view,
    QComboBox& editor, const LC_PropertyViewDescriptor& descriptor)
    : LC_PropertyQStringListComboBoxViewHandler(view, editor, descriptor) {
    descriptor.load(LC_PropertyQStringListArrowsComboboxView::ATTR_BLOCK_NAMES, m_blocksList);
}

void LC_PropertyQStringListArrowsComboboxViewHandler::connectCombobox(QComboBox& editor) {
    connect(&editor, &QComboBox::currentIndexChanged, [this](const int index)-> void {
        const auto itemData = getEditor()->itemData(index);
        const QString data = itemData.toString();
        bool hasValue = false;
        QString value;
        if (data == "_CUSTOM_SELECT") {
            const QString blockName = LC_InputTextDialog::getText(getEditor(), tr("Select Block for arrow"),
                                                                  "Enter the name of existing block that will be used as arrow",
                                                                  m_blocksList, false, "", &hasValue);
            value = blockName;
        }
        else {
            value = data;
            hasValue = true;
        }
        if (hasValue) {
            onValueChanged(value);
        }
    });
}

void LC_PropertyQStringListArrowsComboboxViewHandler::doUpdateEditor() {
    m_updating++;

    auto* typedView = static_cast<LC_PropertyQStringListArrowsComboboxView*>(view());

    const auto cb = getEditor();
    cb->setEnabled(isEditableByUser());

    const auto lineEdit = cb->lineEdit();
    if (isMultiValue()) {
        cb->clearEditText();
        if (lineEdit) {
            lineEdit->setPlaceholderText(LC_Property::getMultiValuePlaceholder());
        }
    }
    else {
        const int currentIndex = typedView->getCurrentIndex();
        cb->blockSignals(true);
        cb->setCurrentIndex(currentIndex);
        cb->blockSignals(false);
    }
    m_updating--;
}
