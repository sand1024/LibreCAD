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

#ifndef LC_SETTINGS_COLORS_SEMANTICS_H
#define LC_SETTINGS_COLORS_SEMANTICS_H

enum class LC_SemanticColors {
    FilteredItem = 0, // Default: Blue (#2a82da)
    ConflictingItem,     // Default: Red (#d9534f / #e06c77)
    SearchResultItem     // Default: Amber (#d97706 / #bd6313)
};


struct LC_SemanticColorDef {
    LC_SemanticColors role;
    const char* key;
    const char* defaultLightHex;
    const char* defaultDarkHex;
};

inline constexpr const char* SEMANTIC_COLOR_KEY_FILTERED_ITEM = "SearchHighlight";
inline constexpr const char* SEMANTIC_COLOR_KEY_CONFLICTING_ITEM  = "ConflictingItem";
inline constexpr const char* SEMANTIC_COLOR_KEY_SEARCH_RESULT    = "SearchResultItem";

inline constexpr LC_SemanticColorDef SEMANTIC_COLOR_DEFS[] = {
    { LC_SemanticColors::FilteredItem,  SEMANTIC_COLOR_KEY_FILTERED_ITEM, "#2a82da", "#2a82da" },
    { LC_SemanticColors::ConflictingItem,   SEMANTIC_COLOR_KEY_CONFLICTING_ITEM,  "#d9534f", "#e06c77" },
    { LC_SemanticColors::SearchResultItem,  SEMANTIC_COLOR_KEY_SEARCH_RESULT,     "#d97706", "#bd6313" }
};

inline QString semanticColorToKey(LC_SemanticColors role) {
    for (const auto& def : SEMANTIC_COLOR_DEFS) {
        if (def.role == role) {
            return def.key;
        }
    }
    return QString();
}

inline bool semanticColorFromKey(const QString& key, LC_SemanticColors& outRole) {
    for (const auto& def : SEMANTIC_COLOR_DEFS) {
        if (key == def.key) {
            outRole = def.role;
            return true;
        }
    }
    return false;
}
#endif
