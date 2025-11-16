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

#include "lc_selectedset.h"

LC_SelectedSet::LC_SelectedSet() {}

LC_SelectedSet::~LC_SelectedSet() {
    m_entitiesList.clear();
    m_listeners.clear();
}

void LC_SelectedSet::clear() {
    m_entitiesList.clear();
    fireSelectionChanged();
}

void LC_SelectedSet::add(RS_Entity* entity) {
    for (auto const e : m_entitiesList) {
        if (e == entity) {
            return;
        }
    }
    m_entitiesList.push_back(entity);
    fireSelectionChanged();
}

void LC_SelectedSet::remove(RS_Entity* entity) {
    m_entitiesList.removeOne(entity);
    fireSelectionChanged();
}

void LC_SelectedSet::replaceBy(QList<RS_Entity*>& entities) {
    m_entitiesList.clear();
    m_entitiesList.append(entities);
    fireSelectionChanged();
}

void LC_SelectedSet::addListener(LC_SelectedSetListener* listener) {
    if (listener == nullptr) {
        return;
    }
    for (auto const l : m_listeners) {
        if (l == listener) {
            return;
        }
    }
    m_listeners.append(listener);
}

void LC_SelectedSet::removeListener(LC_SelectedSetListener* listener) {
    m_listeners.removeOne(listener);
}

void LC_SelectedSet::fireSelectionChanged() {
    if (m_silentMode == 0) {
        for (auto l : m_listeners) {
            l->selectionChanged();
        }
    }
    else{
        m_changedInSilent = true;
    }
}

void LC_SelectedSet::disableListeners() {
    m_silentMode++;
}

bool LC_SelectedSet::enableListeners() {
    m_silentMode --;
    if (m_silentMode == 0) {
        if (m_changedInSilent) {
            m_changedInSilent = false; // fixme - review what to return!
            fireSelectionChanged();
            return true;
        }
        return false;
    }
    return false;
}
