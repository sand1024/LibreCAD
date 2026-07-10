
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
#ifndef LC_STYLE_INDEX_HELPER_H
#define LC_STYLE_INDEX_HELPER_H

#include <QString>

#include "lc_palette_editor_shared.h"

class LC_StyleIndexHelper {
public:
    // Loads the index map from a given index filepath on disk
    static StyleIndexMap loadIndex(const QString& indexFilePath);
    // Writes the index map back to disk
    static bool saveIndex(const QString& indexFilePath, const StyleIndexMap& indexMap);
    // High-performance handshake: checks directory structure and returns files requiring a parse
    static QStringList getOutOfDateFiles(const QString& directoryPath,
                                         const QString& extension,
                                         const StyleIndexMap& indexMap,
                                         StyleIndexMap& outUpdatedIndex,
                                         bool& indexChanged);
};

#endif
