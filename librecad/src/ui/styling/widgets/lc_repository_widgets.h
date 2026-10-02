
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

#ifndef LC_REPOSITORY_WIDGETS_H
#define LC_REPOSITORY_WIDGETS_H

#include "lc_preset_repository_base.h"
#include "lc_widgets_config.h"

inline const QString WIDGETS_EXTENSION = ".lcwc";
inline const QString WIDGETS_FILE_IDENTIFIER = "LibreCAD Config: Widgets";

class LC_RepositoryWidgets : public LC_PresetRepositoryBase<LC_WidgetsConfig> {
public:
    explicit LC_RepositoryWidgets(const QString& configDir)
        : LC_PresetRepositoryBase<LC_WidgetsConfig>(configDir, WIDGETS_EXTENSION, WIDGETS_FILE_IDENTIFIER, "widgets_index.lcix") {
    }

    QJsonObject configToJson(const LC_WidgetsConfig& config) const override;
    bool configFromJson(const QJsonObject& json, LC_WidgetsConfig& config) const override;
};

#endif
