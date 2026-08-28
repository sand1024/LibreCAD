/*******************************************************************************
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

#include "lc_settings_page_language.h"
#include "ui_lc_settings_page_language.h"
#include "lc_settings_backend.h"
#include "rs_system.h"
#include "lc_settings_appearance.h"

LC_SettingsPageLanguage::LC_SettingsPageLanguage(QObject* parent)
    : LC_SettingsPageBase(tr("Language"),
                           std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group),
                           parent) , ui(std::make_unique<Ui::LC_SettingsPageLanguage>()){
    // Priority 40 inside the Startup sub-parent index node
    setSortWeight(40);
}

LC_SettingsPageLanguage::~LC_SettingsPageLanguage() = default;

void LC_SettingsPageLanguage::setupUi() {
    ui->setupUi(m_widget);

    populateLanguages();

    m_initialLanguageGUIIdx = ui->cbLanguage->currentIndex();
}


void LC_SettingsPageLanguage::populateLanguages() const {
    QStringList languageList = RS_SYSTEM->getLanguageList();
    languageList.sort();
    languageList.prepend("en");

    for (const auto& lang : std::as_const(languageList)) {
        QString translatedName = RS_SYSTEM->symbolToLanguage(lang);
        if (!translatedName.isEmpty()) {
            if (ui->cbLanguage->findData(lang) == -1) {
                ui->cbLanguage->addItem(translatedName, lang);
                ui->cbLanguageCmd->addItem(translatedName, lang);
            }
        }
    }
}

void LC_SettingsPageLanguage::setupBindings() {
    using namespace CFG_Appearance;

    bindCustom<QComboBox, QString>(ui->cbLanguage, o_Language, true /*requiresRestart*/,
         &QComboBox::currentTextChanged,
        [](QComboBox* w) { return w->currentData().toString(); },
        [](QComboBox* w, const QString& v) {
            int idx = w->findData(v);
            if (idx >= 0) w->setCurrentIndex(idx);
        });

    bindCustom<QComboBox, QString>(ui->cbLanguageCmd, o_LanguageCmd, false,
    &QComboBox::currentTextChanged,
        [](QComboBox* w) { return w->currentData().toString(); },
        [](QComboBox* w, const QString& v) {
            int idx = w->findData(v);
            if (idx >= 0) w->setCurrentIndex(idx);
        });
}

bool LC_SettingsPageLanguage::saveSettings() {
    if (m_initialLanguageGUIIdx != ui->cbLanguage->currentIndex()) {
        setRestartRequired(true);
    }
    return LC_SettingsPageBase::saveSettings();
}
