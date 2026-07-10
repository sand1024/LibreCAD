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

#ifndef LC_STYLEGEOMETRYCACHE_H
#define LC_STYLEGEOMETRYCACHE_H
#include <QStyleOption>

#include "lc_palette_editor_shared.h"
#include "lc_proxy_style_shared.h"

class LC_SkinScaledGeometryProvider {
public:
    void setBaseMetrics(const StyleMetricsConfig &metrics);
    void invalidate() const;
    const SkinScaledGeometries& getGeometries(const QWidget *widget) const;
    const SkinScaledGeometries& getGeometries(const QPainter* painter) const;
private:
    const SkinScaledGeometries& getGeometriesForDpr(qreal dpr) const;
    void scaleInts(qreal dpr, SkinScaledGeometries& geoms) const;
    SkinScaledGeometries resolveGeometries(qreal dpr) const;

    inline int scaleMetric(int val, qreal dpr) const;
    StyleMetricsConfig scaleMetricsConfig(const StyleMetricsConfig &base, qreal dpr) const;

    void scaleTabs(SkinScaledGeometries::Tab &tab, qreal dpr) const;
    void scaleCheckboxes(SkinScaledGeometries::Checkbox &cb, qreal dpr) const;
    void scaleRadioButtons(SkinScaledGeometries::RadioButton &rb, qreal dpr) const;
    void scaleScrollbars(SkinScaledGeometries::Scrollbar &sb, qreal dpr) const;
    void scaleInputGlows(SkinScaledGeometries::FocusGlow &fg, qreal dpr) const;
    void scaleSplitterGrips(SkinScaledGeometries::SplitterGrip &sg, qreal dpr) const;
    void scaleProgressBars(SkinScaledGeometries::ProgressBar &pb, qreal dpr) const;
    void scaleGroupBoxes(SkinScaledGeometries::GroupBox &gb, qreal dpr) const;
    void scaleSegmentedButtons(SkinScaledGeometries::SegmentedButton &sb, qreal dpr) const;
    void scaleTooltips(SkinScaledGeometries::Tooltip &tt, qreal dpr) const;
    void scaleMenuColumns(SkinScaledGeometries::MenuColumn &mc, qreal dpr) const;

    StyleMetricsConfig m_baseMetrics;
    mutable qreal m_lastDpr = 0.0;
    mutable SkinScaledGeometries m_lastGeometries;
    mutable QHash<qreal, SkinScaledGeometries> m_dpiCache;
};

#endif
