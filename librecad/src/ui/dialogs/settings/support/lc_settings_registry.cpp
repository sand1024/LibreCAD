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

void LC_SettingsRegistry::configureDialog(const QString& dialogId, const QString& title, bool useGlobalTransaction) {
    auto& reg = m_registrations[dialogId];
    reg.properties.title = title;
    reg.properties.useGlobalTransaction = useGlobalTransaction;
}

void LC_SettingsRegistry::registerPage(const QString& dialogId, const QString& id, const QString& idPath, const PageCreator& creator) {
    m_registrations[dialogId].pageCreators.push_back({id, idPath, creator});
}

void LC_SettingsRegistry::registerPages(const QString& dialogId, const std::initializer_list<PageRegistration>& pages) {
    auto& reg = m_registrations[dialogId];
    for (const auto& page : pages) {
        reg.pageCreators.push_back(page); // Standard vector insertion
    }
}

void LC_SettingsRegistry::registerPresetManager(const QString& dialogId, const QString& groupPathId, const PresetManagerCreator& creator) {
    m_registrations[dialogId].presetCreators[groupPathId] = creator;
}


bool LC_SettingsRegistry::showDialog(const QString& dialogId,
                                     const QString& initialPageId,
                                     QWidget* parent,
                                     const std::function<void(LC_SettingsDialog*)>& preExecHook,
                                     const std::function<void(LC_SettingsDialog*, bool accepted)>& postExecHook) {
    if (!m_registrations.contains(dialogId)) {
        Q_ASSERT_X(false, "LC_SettingsRegistry::showDialog", QString("Unknown Dialog requested: " + dialogId).toLatin1());
        return false;
    }

    auto& reg = m_registrations[dialogId];

    //  Conditionally start transaction
    if (reg.properties.useGlobalTransaction) {
        RS_Settings::instance()->startTransaction();
    }

    const auto dialog = std::make_unique<LC_SettingsDialog>(parent, dialogId);

    // Set the configured title
    dialog->setWindowTitle(reg.properties.title);

    // Register preset managers
    for (auto it = reg.presetCreators.begin(); it != reg.presetCreators.end(); ++it) {
        dialog->registerPresetManager(it.key(), it.value()());
    }

    // Instantiate and inject routing properties dynamically
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

    // Run custom pre-execution hook (e.g. preview controller attachment)
    if (preExecHook != nullptr) {
        preExecHook(dialog.get());
    }

    dialog->finalizeInitialization();

    if (!initialPageId.isEmpty()) {
        dialog->selectPage(initialPageId);
    }

    const bool accepted = (dialog->exec() == QDialog::Accepted);

    // Run custom post-execution hook (e.g. preview controller detachment & teardown)
    if (postExecHook != nullptr) {
        postExecHook(dialog.get(), accepted);
    }

    // Conditionally commit or rollback transaction
    if (reg.properties.useGlobalTransaction) {
        if (accepted) {
            RS_Settings::instance()->commitTransaction();
        }
        else {
            RS_Settings::instance()->rollbackTransaction();
        }
    }

    return accepted;
}
