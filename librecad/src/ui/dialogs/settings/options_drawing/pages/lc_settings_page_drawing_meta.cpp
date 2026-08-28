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


#include "lc_settings_page_drawing_meta.h"
#include "ui_lc_settings_page_drawing_meta.h"
#include "lc_document_settings_backend.h"

LC_SettingsPageDrawingMeta::LC_SettingsPageDrawingMeta(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Metadata"), nullptr, parent),m_graphic{graphic} {
}

LC_SettingsPageDrawingMeta::~LC_SettingsPageDrawingMeta() = default;

void LC_SettingsPageDrawingMeta::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingMeta>();
    ui->setupUi(m_widget);
}

void LC_SettingsPageDrawingMeta::loadSettings() {
    if (!m_graphic) return;
    ui->leMetaTitle->setText(m_graphic->getVariableString("$TITLE", ""));
    ui->leMetaSubject->setText(m_graphic->getVariableString("$SUBJECT", ""));
    ui->leMetaAuthor->setText(m_graphic->getVariableString("$AUTHOR", ""));
    ui->leMetaKeywords->setText(m_graphic->getVariableString("$KEYWORDS", ""));
    ui->teMetaComments->setPlainText(m_graphic->getVariableString("$COMMENTS", ""));
}

bool LC_SettingsPageDrawingMeta::saveSettings() {
    if (!m_graphic) return false;

    // LC_ERR << "Meta::saveSettings() executed. Saving Title: '" << ui->leMetaTitle->text() << "'";
    m_graphic->addVariable("$TITLE", ui->leMetaTitle->text(), 1);
    m_graphic->addVariable("$SUBJECT", ui->leMetaSubject->text(), 1);
    m_graphic->addVariable("$AUTHOR", ui->leMetaAuthor->text(), 1);
    m_graphic->addVariable("$KEYWORDS", ui->leMetaKeywords->text(), 1);
    m_graphic->addVariable("$COMMENTS", ui->teMetaComments->toPlainText(), 1);
    return true;
}

bool LC_SettingsPageDrawingMeta::isModified() const {
    if (!m_graphic) {
        return false;
    }

    auto title = ui->leMetaTitle->text();
    auto titleDxf = m_graphic->getVariableString("$TITLE", "");
    const bool titleDifferent = title != titleDxf;
    const bool subjDifferent = ui->leMetaSubject->text() != m_graphic->getVariableString("$SUBJECT", "");
    const bool authorDifferent = ui->leMetaAuthor->text() != m_graphic->getVariableString("$AUTHOR", "");
    const bool keywordDifferent = ui->leMetaKeywords->text() != m_graphic->getVariableString("$KEYWORDS", "");
    const bool commentsDifferent = ui->teMetaComments->toPlainText() != m_graphic->getVariableString("$COMMENTS", "");
    const bool modified = titleDifferent || subjDifferent || authorDifferent || keywordDifferent || commentsDifferent;

    // LC_ERR << "Meta::isModified() called. "
    //        << "Widget Title: '" << ui->leMetaTitle->text() << "' "
    //        << " | Doc Title: '" << m_graphic->getVariableString("$TITLE", "") << "' "
    //        << " | Result: " << (modified ? "MODIFIED" : "CLEAN");
    return modified;
}
