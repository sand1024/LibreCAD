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

#ifndef LC_ICONS_STYLE_SHARED_H
#define LC_ICONS_STYLE_SHARED_H

#include <QColor>
#include <QString>
#include <QList>
#include "lc_icon_engine_shared.h"

struct IconStateColors {
    QString  main;   // Replaces original black strokes (#000000)
    QString  accent; // Replaces original green details (#00FF7F)
    QString  back;   // Replaces original white backgrounds (#FFFFFF)
};

struct IconSchemeConfig {
    // Generic baseline colors (used as seed generators)
    QString  genericMain;
    QString  genericAccent;
    QString  genericAccentChecked;
    QString  genericBack;

    // Symmetrical state structures
    IconStateColors normalOn;
    IconStateColors normalOff;
    IconStateColors activeOn;
    IconStateColors activeOff;
    IconStateColors selectedOn;
    IconStateColors selectedOff;
    IconStateColors disabledOn;
    IconStateColors disabledOff;
};

// Represents a standalone Icon Style (.lcis) saved on disk
struct IconStyleConfig {
    QString name;
    IconSchemeConfig light;
    IconSchemeConfig dark;
    bool autoCalculateStates = true;
};

// Compact Icon-Table Row Mappings
struct IconStateRowMapping {
    QString name;
    LC_SVGIconEngineAPI::IconMode mode;
    LC_SVGIconEngineAPI::IconState state;
};

inline const QList<IconStateRowMapping> ICON_STATE_ROWS = {
    { "Normal ON",   LC_SVGIconEngineAPI::Normal,   LC_SVGIconEngineAPI::On },
    { "Normal OFF",  LC_SVGIconEngineAPI::Normal,   LC_SVGIconEngineAPI::Off },
    { "Active ON",   LC_SVGIconEngineAPI::Active,   LC_SVGIconEngineAPI::On },
    { "Active OFF",  LC_SVGIconEngineAPI::Active,   LC_SVGIconEngineAPI::Off },
    { "Selected ON",  LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::On },
    { "Selected OFF", LC_SVGIconEngineAPI::Selected, LC_SVGIconEngineAPI::Off },
    { "Disabled ON",  LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::On },
    { "Disabled OFF", LC_SVGIconEngineAPI::Disabled, LC_SVGIconEngineAPI::Off }
};

#endif
