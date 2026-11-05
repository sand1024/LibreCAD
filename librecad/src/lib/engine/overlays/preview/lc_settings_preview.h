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

#ifndef LC_SETTINGS_PREVIEW_H
#define LC_SETTINGS_PREVIEW_H
#include "dxf_format.h"
#include "lc_setting.h"

namespace CFG_DraftingPreview {
    inline const LC_SettingsGroupBase Group("Preview");

    inline const LC_Setting<QColor> o_ColorPreviewEntities(&Group, "ColorPreviewEntities", QColor("739373"));
    inline const LC_Setting<QColor> o_ColorPreviewReferences(&Group, "ColorPreviewReferences", QColor("Yellow"));
    inline const LC_Setting<QColor> o_ColorPreviewReferencesHighlight(&Group, "ColorPreviewReferencesHighlight", QColor("Green"));

    // --- Preview Option  ---
    inline const LC_Setting<int> o_MaxEntitiesNumberInPreview(&Group, "MaxEntitiesNumberInPreview", 100);
    inline const LC_Setting<double> o_PreviewScreenLineWidth(&Group, "PreviewScreenLineWidth", 1);

    inline const LC_Setting<int> o_PreviewRefPointType(&Group, "PreviewRefPointType", DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
    inline const LC_Setting<int> o_PreviewRefPointSize(&Group, "PreviewRefPointSize", 10);

    inline const LC_Setting<double> o_PreviewRefEntityScreenLineWidth(&Group, "PreviewRefEntityScreenLineWidth", 0);

    inline const LC_Setting<bool> o_ShowRefEntitiesOnPreview(&Group, "ShowRefEntitiesOnPreview", true);

    inline const LC_Setting<bool> o_HighlightEntitiesOnHover(&Group, "HighlightEntitiesOnHover", true);
    inline const LC_Setting<bool> o_HighlightEntitiesRefPointsOnHover(&Group, "HighlightEntitiesRefPointsOnHover", true);
}

#endif
