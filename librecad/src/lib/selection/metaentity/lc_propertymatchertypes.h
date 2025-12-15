/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * ********************************************************************************
 */

#ifndef LC_ENTITYPROPERTYMETATYPES_H
#define LC_ENTITYPROPERTYMETATYPES_H

#include "lc_propertymatchtypedescriptor.h"
#include "rs.h"

class RS_Layer;
class LC_DimStyle;
class RS_Color;


struct LC_PropertyMatcherTypes {
    static const LC_TypedPropertyMatchTypeDescriptor<RS_Color> COLOR;
    static const LC_TypedPropertyMatchTypeDescriptor<RS_Color> COLOR_RESOLVED;
    static const LC_TypedPropertyMatchTypeDescriptor<RS_Layer*> LAYER;
    static const LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> LINE_WIDTH;
    static const LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> LINE_WIDTH_RESOLVED;
    static const LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> LINE_TYPE;
    static const LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> LINE_TYPE_RESOLVED;
    static const LC_TypedPropertyMatchTypeDescriptor<double>  COORD_X;
    static const LC_ComparingPropertyMatchTypeDescriptor<QList<double>, double> COORD_X_IN_QLIST;
    static const LC_TypedPropertyMatchTypeDescriptor<double>  COORD_Y;
    static const LC_ComparingPropertyMatchTypeDescriptor<QList<double>, double>  COORD_Y_IN_QLIST;
    static const LC_TypedPropertyMatchTypeDescriptor<double> DOUBLE;
    static const LC_TypedPropertyMatchTypeDescriptor<double> LENGTH;
    static const LC_TypedPropertyMatchTypeDescriptor<double> ANGLE;
    static const LC_TypedPropertyMatchTypeDescriptor<double> INCLINATION;
    static const LC_TypedPropertyMatchTypeDescriptor<int> INT;
    static const LC_TypedPropertyMatchTypeDescriptor<int> INT_CHOICE;
    static const LC_TypedPropertyMatchTypeDescriptor<bool> BOOL;
    static const LC_TypedPropertyMatchTypeDescriptor<QString> STRING;
    static const LC_TypedPropertyMatchTypeDescriptor<QString> STRING_CHOICE;
    static const LC_TypedPropertyMatchTypeDescriptor<LC_DimStyle*> DIM_STYLE;
    static const LC_ComparingPropertyMatchTypeDescriptor<std::vector<double>, double> COORD_X_IN_VECTOR;
    static const LC_ComparingPropertyMatchTypeDescriptor<std::vector<double>, double> COORD_Y_IN_VECTOR;
};

#endif
