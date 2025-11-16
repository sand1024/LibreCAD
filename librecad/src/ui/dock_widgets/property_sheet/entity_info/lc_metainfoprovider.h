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

#ifndef LC_METAINFOPROVIDER_H
#define LC_METAINFOPROVIDER_H

#include <QString>
#include <QMap>
#include <QObject>
#include <boost/geometry/strategies/cartesian/box_in_box.hpp>

#include "rs.h"

enum PropertyOperatorFlag {
    OPERATION_EQUALS,
    OPERATION_NOT_EQUALS,
    OPERATION_GREATER,
    OPERATION_LESS
};

Q_DECLARE_FLAGS(LC_PropertyCompareOperators, PropertyOperatorFlag);

struct LC_PropertyDescriptor {
    int id;
    QString m_name;
    bool m_hasLessGreaterOperators = false;
    bool m_hasPatternMatch = false;
};


class LC_MetaInfoProvider: public QObject {
    Q_OBJECT
public:
    LC_MetaInfoProvider();
    void fillPropertyDescriptorsForSelection(RS2::EntityType entityType, QList<LC_PropertyDescriptor> &propertyDescriptors);
protected:
    struct ShortPropertyDescriptor {
        QString name;
        bool hasLessGreater = false;
        bool hasPattern = false;

        ShortPropertyDescriptor(const QString& name, bool lessGreater = false, bool pattern = false)
            : name(name), hasLessGreater(lessGreater), hasPattern(pattern) {
        }
    };
    void initGenericProperties();
    void initLineProperties();
    void initCircleProperties();
    void initArcProperties();
    void initPolylineProperties();
    void initPropertyDescriptors();
    void initPropertiesDescriptors(RS2::EntityType, const std::list<ShortPropertyDescriptor> &descriptorsList);
    QMap<RS2::EntityType, QList<LC_PropertyDescriptor>> m_entityPropertyDescriptors;
};

#endif
