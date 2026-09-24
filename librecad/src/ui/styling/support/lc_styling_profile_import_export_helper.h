
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

#ifndef LC_WORKSPACE_IMPORT_EXPORT_HELPER_H
#define LC_WORKSPACE_IMPORT_EXPORT_HELPER_H

#include <QString>
#include <QJsonObject>

#include "lc_dlg_checklist_selection.h"
#include "lc_palette_editor_shared.h"
#include "lc_icons_style_shared.h"
#include "lc_profile_helper_base.h"

class LC_RepositoryPalette;
class LC_RepositoryFusionSkin;
class LC_RepositoryIconsStyle;
class LC_RepositoryTypography;
class LC_RepositoryMetrics;

class LC_StylingProfileImportExportHelper: public LC_ProfileExchangeHelperBase {
    public:
    LC_StylingProfileImportExportHelper(LC_RepositoryPalette* paletteRepo,
                                   LC_RepositoryFusionSkin* skinRepo,
                                   LC_RepositoryIconsStyle* iconRepo,
                                   LC_RepositoryTypography* typographyRepo,
                                   LC_RepositoryMetrics* metricsRepo);

    bool exportProfile(const QString& exportFilePath,
                       const QString& profileName,
                       const PaletteConfig* palette,
                       const ControlStyleConfig* controlStyle,
                       const IconStyleConfig* icon,
                       const FontConfig* font,
                       const StyleMetricsConfig* metrics) const;

    bool importProfile(const QString& importFilePath,
                       const QList<LC_ChecklistChoice>& selectedChoices,
                       QString& outProfileName,
                       QString& outPaletteName,
                       QString& outControlStyleName,
                       QString& outIconStyleName,
                       QString& outTypographyName,
                       QString& outMetricsName) const;


private:
    LC_RepositoryPalette*     m_paletteRepo;
    LC_RepositoryFusionSkin* m_skinRepo;
    LC_RepositoryIconsStyle*  m_iconRepo;
    LC_RepositoryTypography*  m_typographyRepo;
    LC_RepositoryMetrics*      m_metricsRepo;
};

#endif
