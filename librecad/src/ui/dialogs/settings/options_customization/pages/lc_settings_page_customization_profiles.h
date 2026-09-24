
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

#ifndef LC_SETTINGS_PAGE_CUSTOMIZATION_PROFILES_H
#define LC_SETTINGS_PAGE_CUSTOMIZATION_PROFILES_H

#include <memory>
#include "lc_customization_import_export_helper.h"
#include "lc_settings_page_profile_exchange_base.h"

class LC_ShortcutsManager;
class LC_ActionGroupManager;
class LC_CommandManager;
class LC_GraphicViewContextMenuProvider;
class LC_NavigationControlsCreator;

class LC_SettingsPageCustomizationProfiles : public LC_SettingsPageProfileExchangeBase {
    Q_OBJECT
public:
    LC_SettingsPageCustomizationProfiles(LC_ActionGroupManager* groupManager,
                                         LC_ShortcutsManager* shortcutsManager,
                                         LC_CommandManager* commandManager,
                                         LC_NavigationControlsCreator* navCreator,
                                         LC_GraphicViewContextMenuProvider* contextMenuProvider,
                                         QObject* parent = nullptr);
    ~LC_SettingsPageCustomizationProfiles() override = default;

    bool acceptsSharedPreview() override { return false; }

protected:
    void setupUi() override;
    QString profileFileFilter() const override;
    QString profileDefaultName() const override;
    QString profileDescriptionText() const override;
    QString profileBoxTitle() const override;

    QList<LC_ChecklistChoice> getExportChoices() const override;
    bool inspectProfileFile(const QString& filePath,
                            QString& outProfileName,
                            QList<LC_ChecklistChoice>& outChoices) const override;

    bool executeExport(const QString& filePath,
                       const QString& profileName,
                       const QList<LC_ChecklistChoice>& choices) override;

    bool executeImport(const QString& filePath,
                       const QList<LC_ChecklistChoice>& choices,
                       QString& outProfileName) override;

    void activateImportedProfile() override;

    QString profileExtension() const override { return ".lccp"; }

private:
    LC_ActionGroupManager* m_groupManager{nullptr};
    LC_ShortcutsManager* m_shortcutsManager{nullptr};
    LC_CommandManager* m_commandManager{nullptr};
    LC_NavigationControlsCreator* m_navCreator{nullptr};
    LC_GraphicViewContextMenuProvider* m_contextMenuProvider{nullptr};

    std::unique_ptr<LC_CustomizationImportExportHelper> m_helper;
    QString m_pendingShortcutsKey;
    QString m_pendingCommandsKey;
    QString m_pendingNavigationKey;
    QString m_pendingMenusKey;
};

#endif
