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

#include "lc_propertiesprovider_arc.h"

#include "lc_convert.h"
#include "lc_entity_type_propertiesprovider.h"
#include "lc_propertysheetwidget.h"
#include "rs_arc.h"

void LC_PropertiesProviderArc::doFillEntitySpecificProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(container);

    addVector<RS_Arc>({"center", tr("Center"), tr("Center point of arc")}, [](const RS_Arc* e) -> RS_Vector {
                          return e->getCenter();
                      }, [](const RS_Vector& v, RS_Arc* e) -> void {
                          e->setCenter(v);
                      }, list, contGeometry);

    addLinearDistance<RS_Arc>({"radius", tr("Radius"), tr("Radius of arc")}, [](const RS_Arc* e) -> double {
                                  return e->getRadius();
                              }, [](const double& v, RS_Arc* l) -> void {
                                  l->setRadius(v);
                              }, list, contGeometry);

    addLinearDistance<RS_Arc>({"diameter", tr("Diameter"), tr("Diameter of arc")}, [](const RS_Arc* e) -> double {
                                  return e->getRadius() * 2.0;
                              }, [](const double& v, RS_Arc* l) -> void {
                                  l->setRadius(v / 2.0);
                              }, list, contGeometry);

    addBoolean<RS_Arc>({"reversed", tr("Is Reversed"), tr("Clockwise direction if reversed, counterclockwise otherwise")},
                       [](const RS_Arc* e) -> bool {
                           return e->isReversed();
                       }, [](const bool& v, RS_Arc* e) -> void {
                           e->setReversed(v);
                       }, list, contGeometry);

    addWCSAngle<RS_Arc>({"angle1", tr("Start Angle"), tr("Start angle of arc")}, [](const RS_Arc* e) -> double {
                            return e->getAngle1();
                        }, [](const double& v, RS_Arc* l) -> void {
                            l->setAngle1(v);
                        }, list, contGeometry);

    addWCSAngle<RS_Arc>({"angle2", tr("End Angle"), tr("End angle of arc")}, [](const RS_Arc* e) -> double {
                            return e->getAngle2();
                        }, [](const double& v, RS_Arc* l) -> void {
                            l->setAngle2(v);
                        }, list, contGeometry);

    const auto contOther = createCalculatedInfoSection(container);
    if (contOther == nullptr) {
        return;
    }

    addVector<RS_Arc>({"start", tr("Start Point"), tr("Start point of arc")}, [](const RS_Arc* e) -> RS_Vector {
        return e->getStartpoint();
    }, nullptr, list, contOther);

    addVector<RS_Arc>({"end", tr("End Point"), tr("End point of arc")}, [](const RS_Arc* e) -> RS_Vector {
        return e->getEndpoint();
    }, nullptr, list, contOther);

    addReadOnlyString<RS_Arc>({"arcLength", tr("Arc Length"), tr("Total angle of arc")}, [this](const RS_Arc* e)-> QString {
        const double angleRad = e->getAngleLength();
        QString value = formatRawAngle(angleRad);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Arc>({"circumference", tr("Circumference", "arc"), tr("Total linear length of arc")}, [this](const RS_Arc* e)-> QString {
        const double len = e->getLength();
        QString value = formatLinear(len);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Arc>({"chord", tr("Chord Length"), tr("Distance between end points of arc")}, [this](const RS_Arc* e)-> QString {
        const double len = e->getChord();
        QString value = formatLinear(len);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Arc>({"sagitta", tr("Sagitta"), tr("Height of arc")}, [this](const RS_Arc* e)-> QString {
        const double len = e->getSagitta();
        QString value = formatLinear(len);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Arc>({"bulge", tr("Bulge"), tr("Bulge of arc")}, [this](const RS_Arc* e)-> QString {
        const double len = e->getBulge();
        QString value = formatDouble(len);
        return value;
    }, list, contOther);
}
