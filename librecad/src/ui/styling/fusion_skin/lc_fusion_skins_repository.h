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

#ifndef LC_FUSION_PALETTES_REPOSITORY_H
#define LC_FUSION_PALETTES_REPOSITORY_H

#include "lc_palette_editor_shared.h"
#include "lc_style_repository_base.h"

class LC_FusionSkinsRepository : public LC_StyleRepositoryBase<ControlStyleConfig> {
public:
    explicit LC_FusionSkinsRepository(const QString &configDir)
        : LC_StyleRepositoryBase<ControlStyleConfig>(configDir, SKIN_EXTENSION, SKIN_FILE_IDENTIFIER, "skins_index.lcix") {}

    QJsonObject configToJson(const ControlStyleConfig& config) const override;
    bool configFromJson(const QJsonObject& json, ControlStyleConfig& config) const override;
};

#endif
