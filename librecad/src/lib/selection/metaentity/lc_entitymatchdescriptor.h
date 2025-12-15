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

#include "lc_propertymatchtypedescriptor.h"
#include "lc_typedentitymatcher.h"
#include "lc_vectorentitymatcher.h"
#include "lc_vectorlistentitymatcher.h"
#include "rs.h"

class RS_Entity;

class LC_EntityMatchDescriptor {
public:
    LC_EntityMatchDescriptor(QString name, RS2::EntityType entityType)
       : m_name{name}, m_entityType{entityType} {
    }
    void collectPropertiesInfo(QList<QPair<QString, QString>> &propertyNames);
    LC_PropertyMatchDescriptor* findPropertyDescriptor(const QString& propertyName);
protected:
    QString m_name;
    RS2::EntityType m_entityType;
    QList<LC_PropertyMatchDescriptor*> m_entityPropertyDescriptors;
};

inline void LC_EntityMatchDescriptor::collectPropertiesInfo(QList<QPair<QString, QString>>& propertyNames) {
    for (const auto p: m_entityPropertyDescriptors) {
        QPair<QString, QString> pair(p->getName(), p->getDisplayName());
        propertyNames.append(pair);
    }
}

inline LC_PropertyMatchDescriptor* LC_EntityMatchDescriptor::findPropertyDescriptor(const QString& propertyName) {
    for (const auto p: m_entityPropertyDescriptors) {
        if (p->getName() == propertyName) {
            return p;
        }
    }
    return nullptr;
}

template <typename EntityType> class LC_TypedEntityMatchDescriptor: public LC_EntityMatchDescriptor {
public:
    LC_TypedEntityMatchDescriptor(QString name, RS2::EntityType entityType):LC_EntityMatchDescriptor(name, entityType) {};
    template<typename ValueType> void add(const QString &name, std::function<ValueType(EntityType*)> funAccess,  const QString &displayName, const QString& description,  const LC_TypedPropertyMatchTypeDescriptor<ValueType>& type);
    template<typename ValueType> void add(const QString &name,std::function<ValueType(EntityType*)> funAccess,
                                                              const QString &displayName, const QString& description, const LC_TypedPropertyMatchTypeDescriptor<ValueType>& type,
                                                              const QList<QPair<QString, QVariant>> & choicesList);


    void addVector(const QString &name, std::function<RS_Vector(EntityType*)> funAccess, const QString &displayName, const QString& description,  const LC_TypedPropertyMatchTypeDescriptor<double>& type);

    template <class ListType>
    void addContains(const QString& name, std::function<ListType(EntityType*)> funAccess, const QString& displayName, const QString& description,
                     const LC_ComparingPropertyMatchTypeDescriptor<QList<double>, double>& type);
};

template <typename EntityType>
template <typename ValueType>
void LC_TypedEntityMatchDescriptor<EntityType>::add(const QString&name,
    std::function<ValueType(EntityType*)> funAccess, const QString& displayName, const QString& description, const LC_TypedPropertyMatchTypeDescriptor<ValueType>& type) {
    auto propertyMatchDescriptor = new LC_TypedPropertyMatchDescriptor<ValueType, EntityType>(name, displayName, description, type, funAccess);
    m_entityPropertyDescriptors.push_back(propertyMatchDescriptor);
}

template <typename EntityType>
template <typename ValueType>
void LC_TypedEntityMatchDescriptor<EntityType>::add(const QString &name,
    std::function<ValueType(EntityType*)> funAccess, const QString &displayName, const QString& description, const LC_TypedPropertyMatchTypeDescriptor<ValueType>& type, const QList<QPair<QString, QVariant>> & choicesList) {
    auto propertyMatchDescriptor = new LC_TypedPropertyMatchDescriptor<ValueType, EntityType>(name, displayName, description, type, funAccess);
    propertyMatchDescriptor->setChoiceValues(choicesList);
    m_entityPropertyDescriptors.push_back(propertyMatchDescriptor);
}

template <typename EntityType>
void LC_TypedEntityMatchDescriptor<EntityType>::addVector(const QString &name,
    std::function<RS_Vector(EntityType*)> funAccess, const QString &displayName, const QString& description,  const LC_TypedPropertyMatchTypeDescriptor<double>& type) {
    auto propertyMatchDescriptor = new LC_RS_VectorPropertyMatchDescriptor<EntityType>(name, displayName, description, type, funAccess);
    m_entityPropertyDescriptors.push_back(propertyMatchDescriptor);
}

template <typename EntityType>
template <typename ListType>
void LC_TypedEntityMatchDescriptor<EntityType>::addContains(const QString& name, std::function<ListType(EntityType*)> funAccess,
    const QString& displayName, const QString& description, const LC_ComparingPropertyMatchTypeDescriptor<QList<double>,double>& type) {
    auto propertyMatchDescriptor = new LC_RS_VectorListPropertyMatchDescriptor<EntityType, ListType>(name, displayName, description, type, funAccess);
    m_entityPropertyDescriptors.push_back(propertyMatchDescriptor);
}

#endif
