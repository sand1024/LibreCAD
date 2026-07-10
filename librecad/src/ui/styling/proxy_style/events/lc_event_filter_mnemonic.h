
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

#ifndef LC_MNEMONICEVENTFILTER_H
#define LC_MNEMONICEVENTFILTER_H

#include "lc_proxy_style_event_filter.h"

class QObject;
class LC_ProxyStyle;
class QEvent;

// Mnemonics Underline Filter
class LC_EventFilterMnemonic : public LC_ProxyStyleEventFilter {
    Q_OBJECT
public:
    explicit LC_EventFilterMnemonic(const LC_ProxyStyle *style)
        : LC_ProxyStyleEventFilter(nullptr, style) {}

    bool eventFilter(QObject *watched, QEvent *event) override;

    bool showUnderlines() const { return m_showUnderlines; }

private:
    bool m_altPressed = false;
    bool m_otherKeyPressed = false;
    bool m_showUnderlines = false;
};

#endif
