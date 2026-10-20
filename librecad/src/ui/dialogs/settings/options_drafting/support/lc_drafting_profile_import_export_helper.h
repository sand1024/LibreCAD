
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

#ifndef LC_DRAFTING_PROFILE_IMPORT_EXPORT_HELPER_H
#define LC_DRAFTING_PROFILE_IMPORT_EXPORT_HELPER_H

#include <QList>
#include <QString>

#include "lc_dlg_checklist_selection.h"
#include "lc_profile_helper_base.h"
#include "lc_repository_drafting_metrics.h"
#include "lc_repository_drafting_theme.h"

inline const QString DRAFTING_PROFILE_FILE_IDENTIFIER = "LibreCAD Drafting Profile v_1";

class LC_DraftingProfileImportExportHelper : public LC_ProfileExchangeHelperBase {
public:
    LC_DraftingProfileImportExportHelper(LC_RepositoryDraftingTheme* themeRepo,
                                         LC_RepositoryDraftingMetrics* metricsRepo);
    ~LC_DraftingProfileImportExportHelper() override = default;

    bool exportProfile(const QString& exportFilePath,
                       const QString& profileName,
                       const LC_ViewportThemeConfig* theme,
                       const LC_DraftingMetricsConfig* metrics) const;

    bool inspectProfile(const QString& importFilePath,
                        QString& outProfileName,
                        QList<LC_ChecklistChoice>& outChoices) const;

    bool importProfile(const QString& importFilePath,
                       const QList<LC_ChecklistChoice>& selectedChoices,
                       QString& outProfileName,
                       QString& outThemeKey,
                       QString& outMetricsKey) const;

private:
    LC_RepositoryDraftingTheme* m_themeRepo = nullptr;
    LC_RepositoryDraftingMetrics* m_metricsRepo = nullptr;
};

#endif
