
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
#include "lc_palette_editor_shared.h"
#include "lc_icons_style_shared.h"

class LC_PaletteRepository;
class LC_FusionSkinsRepository;
class LC_IconsStyleRepository;
class LC_TypographyRepository;
class LC_MetricsRepository;

class LC_WorkspaceImportExportHelper {
public:
    LC_WorkspaceImportExportHelper(LC_PaletteRepository* paletteRepo,
                                   LC_FusionSkinsRepository* skinRepo,
                                   LC_IconsStyleRepository* iconRepo,
                                   LC_TypographyRepository* typographyRepo,
                                   LC_MetricsRepository* metricsRepo);

    // Packages 5 active configurations into a single consolidated .lcws file
    bool exportProfile(const QString& exportFilePath,
                       const QString& profileName,
                       const PaletteConfig* palette,
                       const ControlStyleConfig* controlStyle,
                       const IconStyleConfig* icon,
                       const FontConfig* font,
                       const StyleMetricsConfig* metrics) const;

    // Unpacks configurations and delegates save/registration directly to the 5 repositories
    bool importProfile(const QString& importFilePath,
                       QString& outProfileName,
                       QString& outPaletteName,
                       QString& outControlStyleName,
                       QString& outIconStyleName,
                       QString& outTypographyName,
                       QString& outMetricsName) const;

private:
    LC_PaletteRepository*     m_paletteRepo;
    LC_FusionSkinsRepository* m_skinRepo;
    LC_IconsStyleRepository*  m_iconRepo;
    LC_TypographyRepository*  m_typographyRepo;
    LC_MetricsRepository*      m_metricsRepo;
};

#endif
