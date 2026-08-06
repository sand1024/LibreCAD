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

#include "lc_settings_registry.h"

#include "lc_settings_registry.h"
#include "lc_settings_dialog.h"

#include "lc_settings_registry.h"
#include "lc_settings_dialog.h"

#include "lc_settings_registry.h"
#include "lc_settings_dialog.h"
#include "rs_settings.h"

LC_SettingsRegistry* LC_SettingsRegistry::instance() {
    static LC_SettingsRegistry s_instance;
    return &s_instance;
}

void LC_SettingsRegistry::registerPage(const QString& dialogId, const QString& id, const QString& idPath, const PageCreator& creator) {
    m_registrations[dialogId].pageCreators.push_back({id, idPath, creator});
}


void LC_SettingsRegistry::registerPages(const QString& dialogId, const std::initializer_list<PageRegistration>& pages) {
    auto& reg = m_registrations[dialogId];
    for (const auto& page : pages) {
        reg.pageCreators.push_back(page); // Standard vector insertion [1.1.2]
    }
}

void LC_SettingsRegistry::registerPresetManager(const QString& dialogId, const QString& groupPathId, const PresetManagerCreator& creator) {
    m_registrations[dialogId].presetCreators[groupPathId] = creator;
}

bool LC_SettingsRegistry::showDialog(const QString& dialogId, const QString& initialPageId, QWidget* parent) {
    if (!m_registrations.contains(dialogId)) {
        Q_ASSERT_X(true,QString("Unknown Dialog requested" + dialogId).toLatin1(), "LC_SettingsRegistry::showDialog");
        return false;
    }

    RS_Settings::instance()->startTransaction();
    auto& reg = m_registrations[dialogId];
    auto dialog = std::make_unique<LC_SettingsDialog>(parent, dialogId);

    // 1. Register preset managers
    for (auto it = reg.presetCreators.begin(); it != reg.presetCreators.end(); ++it) {
        dialog->registerPresetManager(it.key(), it.value()());
    }

    // 2. Instantiate and inject routing properties dynamically [4.1]
    for (const auto& pageReg : reg.pageCreators) {
        auto page = pageReg.creator();
        page->setId(pageReg.id);
        page->setParentId(pageReg.parentId);
        const int weight = pageReg.weight;
        if (weight != -1) {
            page->setSortWeight(pageReg.weight);
        }
        dialog->registerPage(std::move(page));
    }


    dialog->finalizeInitialization();

    if (!initialPageId.isEmpty()) {
        dialog->selectPage(initialPageId);
    }

    bool accepted = dialog->exec() == QDialog::Accepted;
    if (accepted) {
        RS_Settings::instance()->commitTransaction(); // Commit changes safely to disk
    } else {
        RS_Settings::instance()->rollbackTransaction(); // Discard and restore in-memory baseline 
    }
    return accepted;
}
