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

#include "lc_propertiesprovider_dim_aligned.h"

#include "rs_dimaligned.h"

void LC_PropertiesProviderDimAligned::doCreateDimGeometrySection(LC_PropertyContainer* cont, const QList<RS_Entity*>& list) {
    addVector<RS_DimAligned>({"dimExtPoint1", tr("Ext point 1"), tr("Position of first extension point")},
                             [](const RS_DimAligned* e) -> RS_Vector {
                                 return e->getExtensionPoint1();
                             }, [](const RS_Vector& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_DimAligned* e) -> void {
                                 e->setExtensionPoint1(v);
                             }, list, cont);

    addVector<RS_DimAligned>({"dimExtPoint2", tr("Ext point 2"), tr("Position of second extension point")},
                             [](const RS_DimAligned* e) -> RS_Vector {
                                 return e->getExtensionPoint2();
                             }, [](const RS_Vector& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_DimAligned* e) -> void {
                                 e->setExtensionPoint2(v);
                             }, list, cont);

    addVector<RS_DimAligned>({"dimDefPoint", tr("Definition point "), tr("Position of definition point")},
                             [](const RS_DimAligned* e) -> RS_Vector {
                                 return e->getDefinitionPoint();
                             }, [](const RS_Vector& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_DimAligned* e) -> void {
                                 e->setDefinitionPoint(v);
                             }, list, cont);
}
