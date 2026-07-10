
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

#ifndef LC_ICONS_STYLE_MANAGER_H
#define LC_ICONS_STYLE_MANAGER_H

#include <QString>

#include "lc_icon_colors_options.h"
#include "lc_icon_engine_shared.h"
#include "lc_palette_color_utils.h"

class LC_IconsStyleManager {
public:
    // Symmetrical on-the-fly theme-linked icons compiler [3]
    static void applyThemeLinkedIcons(const QString &linkedStyleName, bool useThemeDefaultIcons, bool isDarkMode);
    static void applyStyle(const LC_IconColorsOptions &options, bool isDarkMode,
                        LC_PaletteColorUtils::CVDType cvd = LC_PaletteColorUtils::CVDType::Normal);
    static void applyCurrentStyle();
    static QString resolveColorValue(const QString &value);
};

#endif
