
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

#ifndef LC_TYPOGRAPHYREPOSITORY_H
#define LC_TYPOGRAPHYREPOSITORY_H

#include "lc_style_repository_base.h"

class LC_TypographyRepository : public LC_StyleRepositoryBase<FontConfig> {
public:
    explicit LC_TypographyRepository(const QString& configDir)
        : LC_StyleRepositoryBase<FontConfig>(configDir, FONT_EXTENSION, FONT_FILE_IDENTIFIER, "fonts_index.lcix") {}

    QJsonObject configToJson(const FontConfig& config) const override;
    bool configFromJson(const QJsonObject& json, FontConfig& config) const override;
};

#endif
