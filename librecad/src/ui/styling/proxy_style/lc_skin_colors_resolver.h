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

#ifndef LC_STYLEDESCRIPTORCACHE_H
#define LC_STYLEDESCRIPTORCACHE_H

#include <QPalette>
#include <QMap>
#include <QIcon>
#include <QFont>
#include <QStyle>
#include <QStyleOption>
#include <QWidget>

#include "lc_proxy_style_shared.h"
#include "lc_palette_editor_shared.h"

class LC_SkinColorsResolver {
public:
    LC_SkinColorsResolver();
    ~LC_SkinColorsResolver();

    void setSkin(const SkinConfig& skin);
    void invalidate() const;

    SkinColors getDescriptor(const QPalette &palette, QPalette::ColorGroup group) const;
    QIcon getStandardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const;
    const QFont& getResolvedMonoFont(const QFont &baseFont) const;
    void precomputeSegmentedGroupColors(QWidget *widget, int totalGroups) const;

    DockTitleBarStyle activeDockTitleStyle() const { return m_activeDockTitleStyle; } // Added

    SkinColors resolveDockTitleBarDescriptor(const QStyleOptionDockWidget *option, const QWidget *widget) const; // Added
    SkinColors resolveStandardButtonDescriptor(const QStyleOptionButton *option, const QWidget *widget) const;
    SkinColors resolveToolButtonDescriptor(const QStyleOption *option, const QWidget *widget) const;
    SkinColors resolveTabBorders(const QStyleOptionTab *option, const QWidget *widget) const;
    SkinColors resolveHeaderSectionDescriptor(const QStyleOptionHeader *option) const;
    SkinColors resolveLineEditDescriptor(const QStyleOption *option, const QWidget *widget) const;
private:
    SkinColors resolveStyleDescriptor(StyleArchetype archetype, const QPalette &palette, QPalette::ColorGroup group) const;
    void resolveDynamicGroupBoxFallback(SkinColors& desc, const QPalette& palette, QPalette::ColorGroup group) const;
    static QColor mixColors(const QColor &c1, const QColor &c2, double factor);
    static QPalette::ColorGroup resolveColorGroup(QStyle::State state);

    SkinConfig m_skin;

    bool m_isFlat = false;
    bool m_isClassic = false;
    bool m_isFlatModern = false;
    bool m_isGlossy = false;
    bool m_isSoftSatin = false;
    bool m_isAccentOnline = false;
    DockTitleBarStyle m_activeDockTitleStyle = DockTitleBarStyle::Native;

    struct CacheSlot {
        QPalette palette;
        SkinColors descriptors[3]; // Active, Inactive, Disabled
    };

    mutable CacheSlot m_globalSlot;
    mutable CacheSlot m_customSlot;

    mutable QMap<int, QIcon> m_iconCache;
    mutable QFont m_cachedMonoFont;
    mutable int m_cachedMonoPointSize = -1;
};


#endif
