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

#include "lc_propertiesprovider_ellipse.h"

#include "rs_ellipse.h"

class RS_Ellipse;

void LC_PropertiesProviderEllipse::doFillEntitySpecificProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(container);

    addVector<RS_Ellipse>({"center", tr("Center"), tr("Center point of ellipse")}, [](const RS_Ellipse* e) -> RS_Vector {
                              return e->getCenter();
                          }, [](const RS_Vector& v, RS_Ellipse* e) -> void {
                              e->setCenter(v);
                          }, list, contGeometry);

    addWCSAngle<RS_Ellipse>({"angle", tr("Angle"), tr("Angle of the ellipse major axis")}, [](const RS_Ellipse* e) -> double {
                                return e->getAngle();
                            }, [](double& v, RS_Ellipse* l) -> void {
                                //fixme = update geometry
                                // l->setAngle(v);
                            }, list, contGeometry);

    addLinearDistance<RS_Ellipse>({"radiusMajor", tr("Radius Major"), tr("Major radius of ellipse")}, [](const RS_Ellipse* e) -> double {
                                      return e->getMajorRadius();
                                  }, [](double& v, RS_Ellipse* l) -> void {
                                      // fixme = update geometry
                                      // l->setRadius(v);
                                  }, list, contGeometry);

    addLinearDistance<RS_Ellipse>({"radiusMinor", tr("Radius Minor"), tr("Minor radius of ellipse")}, [](const RS_Ellipse* e) -> double {
                                      return e->getMinorRadius();
                                  }, [](double& v, RS_Ellipse* l) -> void {
                                      // fixme = update geometry
                                      // l->setRadius(v);
                                  }, list, contGeometry);

    addLinearDistance<RS_Ellipse>({"radiusRatio", tr("Radius Ratio"), tr("Radius ratio of ellipse")}, [](const RS_Ellipse* e) -> double {
                                      return e->getRatio();
                                  }, [](const double& v, RS_Ellipse* l) -> void {
                                      l->setRatio(v);
                                  }, list, contGeometry);

    addWCSAngle<RS_Ellipse>({"angle1", tr("Start Angle"), tr("Start angle of elliptic arc")}, [](const RS_Ellipse* e) -> double {
                                return e->getAngle1();
                            }, [](const double& v, RS_Ellipse* l) -> void {
                                l->setAngle1(v);
                            }, list, contGeometry);

    addWCSAngle<RS_Ellipse>({"angle2", tr("End Angle"), tr("End angle of elliptic arc")}, [](const RS_Ellipse* e) -> double {
                                return e->getAngle2();
                            }, [](const double& v, RS_Ellipse* l) -> void {
                                l->setAngle2(v);
                            }, list, contGeometry);

    const auto contOther = createCalculatedInfoSection(container);
    if (contOther == nullptr) {
        return;
    }

    addReadOnlyString<RS_Ellipse>({"circumference", tr("Circumference", "ellipse"), tr("Circumference of ellipse")},
                                  [this](const RS_Ellipse* e) -> QString {
                                      const double len = e->getLength();
                                      QString value = formatLinear(len);
                                      return value;
                                  }, list, contOther);

    addReadOnlyString<RS_Ellipse>({"area", tr("Area"), tr("Area of ellipse")}, [this](const RS_Ellipse* e) -> QString {
        const double area = e->areaLineIntegral();
        QString value = formatLinear(area);
        return value;
    }, list, contOther);

    addVector<RS_Ellipse>({"start", tr("Start"), tr("Start point of elliptic arc")}, [](const RS_Ellipse* e) -> RS_Vector {
        return e->getStartpoint();
    }, nullptr, list, contOther);

    addVector<RS_Ellipse>({"end", tr("End"), tr("End point of elliptic arc")}, [](const RS_Ellipse* e) -> RS_Vector {
        return e->getEndpoint();
    }, nullptr, list, contOther);
}
