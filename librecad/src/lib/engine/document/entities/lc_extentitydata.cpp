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

#include "lc_extentitydata.h"

#include <qalgorithms.h>

LC_ExtDataTag::LC_ExtDataTag() = default;

LC_ExtDataTag::LC_ExtDataTag(const int code, const RS_Vector &value) {
    const auto v = new RS_Variable(value, code);
    add(v);
}

LC_ExtDataTag::LC_ExtDataTag(const int code, const int value) {
    const auto v = new RS_Variable(value, code);
    add(v);
}

LC_ExtDataTag::LC_ExtDataTag(const int code, const double value) {
    const auto v = new RS_Variable(value, code);
    add(v);
}

LC_ExtDataTag::LC_ExtDataTag(const int code, const QString& value, const bool asReference) {
    const auto v = new RS_Variable(value, code);
    add(v);
    if (asReference){
      m_type = REF;
    }
}

LC_ExtDataTag::LC_ExtDataTag(RS_Variable* var) {
    add(var);
}

LC_ExtDataTag::~LC_ExtDataTag() {
    clear();
}

void LC_ExtDataTag::clear() const {
    if (m_type == VAR) {
        delete m_var;
    }
    else {
        qDeleteAll(m_list);
    }
}

void LC_ExtDataTag::add(RS_Variable* v) {
    m_var = v;
    m_type = VAR;
}

void LC_ExtDataTag::add(LC_ExtDataTag* tag) {
    m_list.push_back(tag);
    m_type = LIST;
}

bool LC_ExtDataTag::isAtomic() const {
    return m_type != LIST;
}

bool LC_ExtDataTag::isRef() const {
    return m_type == REF;
}

RS_Variable* LC_ExtDataTag::var() const {
    return m_var;
}

std::vector<LC_ExtDataTag*>* LC_ExtDataTag::list() {
    return &m_list;
}

LC_ExtDataGroup::LC_ExtDataGroup(const QString& groupName):m_name{groupName} {
}

void LC_ExtDataGroup::add(const int code, const int value) {
    const auto tagData = new LC_ExtDataTag(code, value);
    m_tagData.add(tagData);
}

void LC_ExtDataGroup::add(const int code, const double value) {
    const auto tagData = new LC_ExtDataTag(code, value);
    m_tagData.add(tagData);
}

void LC_ExtDataGroup::add(const int code, const QString& value) {
    const auto tagData = new LC_ExtDataTag(code, value);
    m_tagData.add(tagData);
}

void LC_ExtDataGroup::addRef(const int code, const QString& value) {
    const auto tagData = new LC_ExtDataTag(code, value, true);
    m_tagData.add(tagData);
}

void LC_ExtDataGroup::add(const int code, const RS_Vector& value) {
    const auto tagData = new LC_ExtDataTag(code, value);
    m_tagData.add(tagData);
}

void LC_ExtDataGroup::add([[maybe_unused]]int code, LC_ExtDataTag* tagData) {
    m_tagData.add(tagData);
}

QString LC_ExtDataGroup::getName() {
    return m_name;
}

LC_ExtDataTag* LC_ExtDataGroup::getData() {
    return &m_tagData;
}

std::vector<LC_ExtDataTag*>* LC_ExtDataGroup::getTagsList() {
    return m_tagData.list();
}

LC_ExtDataAppData::LC_ExtDataAppData(const QString& applicationName):m_applicationName{applicationName} {
}

LC_ExtDataAppData::~LC_ExtDataAppData() {
    m_groups.clear();
}

LC_ExtDataGroup* LC_ExtDataAppData::addGroup(const QString& groupName) {
    auto* group = new LC_ExtDataGroup(groupName);
    m_groups.push_back(group);
    return group;
}

LC_ExtDataGroup* LC_ExtDataAppData::getGroupByName(const QString& groupName) const {
    if (groupName.isEmpty()) {
        return nullptr;
    }
    const size_t count = m_groups.size();
    for (size_t i = 0; i < count; i++) {
        LC_ExtDataGroup* group = m_groups[i];
        if (group->getName() == groupName) {
            return group;
        }
    }
    return nullptr;
}

std::vector<LC_ExtDataGroup*>* LC_ExtDataAppData::getGroups() {
    return &m_groups;
}

QString LC_ExtDataAppData::getName() {
    return m_applicationName;
}

LC_ExtEntityData::LC_ExtEntityData() = default;

LC_ExtEntityData::~LC_ExtEntityData() {
    m_appData.clear();
}

LC_ExtDataAppData* LC_ExtEntityData::addAppData(const QString& appName) {
    auto* appData = getAppDataByName(appName);
    if (appData == nullptr) {
        appData = new LC_ExtDataAppData(appName);
        m_appData.push_back(appData);
    }
    return appData;

}

LC_ExtDataAppData* LC_ExtEntityData::getAppDataByName(const QString& groupName) const {
    if (groupName.isEmpty()) {
        return nullptr;
    }
    const size_t count = m_appData.size();
    for (size_t i = 0; i < count; i++) {
        LC_ExtDataAppData* group = m_appData[i];
        if (group->getName() == groupName) {
            return group;
        }
    }
    return nullptr;
}

LC_ExtDataGroup* LC_ExtEntityData::getGroupByName(const QString& appName, const QString& groupName) const {
    const auto appData = getAppDataByName(appName);
    if (appData != nullptr) {
        return appData->getGroupByName(groupName);
    }
    return nullptr;
}

std::vector<LC_ExtDataAppData*>* LC_ExtEntityData::getAppData() {
    return &m_appData;
}
