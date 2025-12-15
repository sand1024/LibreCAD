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

#ifndef LC_PROPERTYMATCHTYPEDESCRIPTOR_H
#define LC_PROPERTYMATCHTYPEDESCRIPTOR_H

#include <functional>
#include <QString>

class RS_Entity;

enum LC_PropertyMatchOperation {
    MATCH_OPERATION_EQUALS        = 1,
    MATCH_OPERATION_NOT_EQUALS    = 2,
    MATCH_OPERATION_GREATER       = 4,
    MATCH_OPERATION_LESS          = 8,
    MATCH_OPERATION_PATTERN_MATCH = 16,
    MATCH_OPERATION_ALL           = 32
};

Q_DECLARE_FLAGS(LC_PropertyComparingOperations, LC_PropertyMatchOperation);


enum LC_PropertyMatchTypeEnum {
    ENTITY_PROPERTY_INT,
    ENTITY_PROPERTY_INT_CHOICE,
    ENTITY_PROPERTY_BOOL,
    ENTITY_PROPERTY_COORD_X,
    ENTITY_PROPERTY_COORD_X_CONTAINS,
    ENTITY_PROPERTY_COORD_Y,
    ENTITY_PROPERTY_COORD_Y_CONTAINS,
    ENTITY_PROPERTY_DOUBLE,
    ENTITY_PROPERTY_LENGTH,
    ENTITY_PROPERTY_ANGLE,
    ENTITY_PROPERTY_LINETYPE,
    ENTITY_PROPERTY_LINETYPE_RESOLVED,
    ENTITY_PROPERTY_LINEWIDTH,
    ENTITY_PROPERTY_LINEWIDTH_RESOLVED,
    ENTITY_PROPERTY_COLOR,
    ENTITY_PROPERTY_COLOR_RESOLVED,
    ENTITY_PROPERTY_LAYER,
    ENTITY_PROPERTY_DIM_STYLE,
    ENTITY_PROPERTY_STRING,
    ENTITY_PROPERTY_STRING_CHOICE

    // todo - arrow type as type?
};

struct LC_PropertyMatchTypeDescriptor {
    LC_PropertyMatchTypeDescriptor(LC_PropertyMatchTypeEnum type)
        : m_type(type) {
    }
    LC_PropertyMatchTypeEnum getType() const {return m_type;}

    void hasAll() {
        m_supportedOperations.setFlag(MATCH_OPERATION_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_NOT_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_GREATER);
        m_supportedOperations.setFlag(MATCH_OPERATION_LESS);
        m_supportedOperations.setFlag(MATCH_OPERATION_PATTERN_MATCH);
        m_supportedOperations.setFlag(MATCH_OPERATION_ALL);
    }

    void hasAllExceptPattern() {
        m_supportedOperations.setFlag(MATCH_OPERATION_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_NOT_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_GREATER);
        m_supportedOperations.setFlag(MATCH_OPERATION_LESS);
        m_supportedOperations.setFlag(MATCH_OPERATION_PATTERN_MATCH, false);
        m_supportedOperations.setFlag(MATCH_OPERATION_ALL);
    }

    void hasBasic() {
        m_supportedOperations.setFlag(MATCH_OPERATION_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_NOT_EQUALS);
        m_supportedOperations.setFlag(MATCH_OPERATION_GREATER, false);
        m_supportedOperations.setFlag(MATCH_OPERATION_LESS, false);
        m_supportedOperations.setFlag(MATCH_OPERATION_PATTERN_MATCH, false);
        m_supportedOperations.setFlag(MATCH_OPERATION_ALL);
    }

    LC_PropertyComparingOperations m_supportedOperations;
    LC_PropertyMatchTypeEnum m_type;
};

template <typename PropertyType, typename MatchValueType>
class LC_ComparingPropertyMatchTypeDescriptor: public LC_PropertyMatchTypeDescriptor{
public:
    using FunValueComparisonOperation = std::function<bool(PropertyType&,  const MatchValueType&,  const MatchValueType&)>;

    explicit LC_ComparingPropertyMatchTypeDescriptor(LC_PropertyMatchTypeEnum type)
        : LC_PropertyMatchTypeDescriptor(type) {
    }

    LC_ComparingPropertyMatchTypeDescriptor() = default;

    FunValueComparisonOperation m_equals;
    FunValueComparisonOperation m_notEquals;
    FunValueComparisonOperation m_greater;
    FunValueComparisonOperation m_less;
    FunValueComparisonOperation m_any;
    FunValueComparisonOperation m_patternMatch;
};

template <typename PropertyType>
class LC_TypedPropertyMatchTypeDescriptor: public LC_ComparingPropertyMatchTypeDescriptor<PropertyType, PropertyType>{
public:
    explicit LC_TypedPropertyMatchTypeDescriptor(LC_PropertyMatchTypeEnum type)
        : LC_ComparingPropertyMatchTypeDescriptor<PropertyType, PropertyType>(type) {
    }
    LC_TypedPropertyMatchTypeDescriptor() = default;
};
#endif
