
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

#ifndef LC_PROXYSTYLEEVENTFILTER_H
#define LC_PROXYSTYLEEVENTFILTER_H

#include <QObject>
class LC_ProxyStyle;

class LC_ProxyStyleEventFilter : public QObject {
    Q_OBJECT
public:
    explicit LC_ProxyStyleEventFilter(QObject *parent, const LC_ProxyStyle *style)
        : QObject(parent), m_style(style) {}
    ~LC_ProxyStyleEventFilter() override = default;

protected:
    const LC_ProxyStyle* style() const { return m_style; }

private:
    const LC_ProxyStyle *m_style;
};

#endif
