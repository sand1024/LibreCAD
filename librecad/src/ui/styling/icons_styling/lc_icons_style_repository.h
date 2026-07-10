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

#ifndef LC_ICONS_STYLE_REPOSITORY_H
#define LC_ICONS_STYLE_REPOSITORY_H

#include <QString>
#include <QStringList>
#include "lc_icons_style_shared.h"
#include "lc_style_repository_base.h"

class QFile;

class LC_IconsStyleRepository : public LC_StyleRepositoryBase<IconStyleConfig> {
public:
    explicit LC_IconsStyleRepository(const QString &configDir)
        : LC_StyleRepositoryBase<IconStyleConfig>(configDir, ICON_STYLE_EXTENSION, "LibreCAD Icons Style", "icons_index.lcix") {}

    QJsonObject configToJson(const IconStyleConfig& config) const override;
    bool configFromJson(const QJsonObject& json, IconStyleConfig& config) const override;
};
#endif
