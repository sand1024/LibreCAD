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

#include "lc_propertiesprovider_circle.h"

#include "rs_circle.h"

void LC_PropertiesProviderCircle::doFillEntitySpecificProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(container);

    addVector<RS_Circle>({"сenter", tr("Center"), tr("Center point of circle")}, [](const RS_Circle* e) -> RS_Vector {
                             return e->getCenter();
                         }, [](const RS_Vector& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Circle* l) -> void {
                             l->setCenter(v);
                         }, list, contGeometry);

    addLinearDistance<RS_Circle>({"radius", tr("Radius"), tr("Radius of circle")}, [](const RS_Circle* e) -> double {
                                     return e->getRadius();
                                 }, [](const double& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Circle* l) -> void {
                                     l->setRadius(v);
                                 }, list, contGeometry);

    addLinearDistance<RS_Circle>({"diameter", tr("Diameter"), tr("Diameter of circle")}, [](const RS_Circle* e) -> double {
                                     return e->getRadius() * 2.0;
                                 }, [](const double& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Circle* l) -> void {
                                     l->setRadius(v / 2.0);
                                 }, list, contGeometry);

    addLinearDistance<RS_Circle>({"circumference", tr("Circumference"), tr("Circumference of circle")}, [](const RS_Circle* e) -> double {
                                     return e->getRadius() * 2.0 * M_PI;
                                 }, [](const double& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Circle* l) -> void {
                                     l->setRadius(v / 2.0 / M_PI);
                                 }, list, contGeometry);

    addLinearDistance<RS_Circle>({"area", tr("Area"), tr("Area of circle")}, [](const RS_Circle* e) -> double {
                                     const double radius = e->getRadius();
                                     return M_PI * radius * radius;
                                 }, [](const double& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Circle* l) -> void {
                                     const double radius = std::sqrt(v / M_PI);
                                     l->setRadius(radius);
                                 }, list, contGeometry);
}
