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

#include "lc_propertiesprovider_polyline.h"

#include "lc_property_rsvector_view.h"
#include "rs_polyline.h"

void LC_PropertiesProviderPolyline::doFillEntitySpecificProperties(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(container);

    addBoolean<RS_Polyline>({"closed", tr("Is Closed"), tr("Determines whether polyline is closed")}, [](const RS_Polyline* e) -> bool {
                                return e->isClosed();
                            }, [](const bool& v, [[maybe_unused]] LC_PropertyChangeReason reason, RS_Polyline* e) -> void {
                                e->setClosed(v);
                            }, list, contGeometry);

    addReadOnlyString<RS_Polyline>({"segmentsCount", tr("Segments Count"), tr("Amount of polyline segments")},
                                   [this](const RS_Polyline* e) -> QString {
                                       const int len = e->count();
                                       QString value = formatInt(len);
                                       return value;
                                   }, list, contGeometry);

    const bool singleEntity = list.size() == 1;
    if (singleEntity) {
        // single entity
        auto* polyline = static_cast<RS_Polyline*>(list.first());
        const auto vertexes = polyline->getVertexes();
        const qsizetype vertexesCount = vertexes.count();

        static int polylineVertexIndex = 1;

        if (polylineVertexIndex > vertexesCount) {
            polylineVertexIndex = 1;
        }

        const int initialIndex = polylineVertexIndex;

        createIndexAndPointProperties(contGeometry, polyline, {"vertexIdx", tr("Vertex Index"), tr("Index of current vertex of polyline")},
                                      {"vertexValue", tr("Vertex"), tr("Index of current vertex of polyline")}, vertexesCount, initialIndex,
                                      [polyline](const int newValue) -> RS_Vector {
                                          const RS_Vector wcsVector = polyline->getVertex(newValue - 1);
                                          return wcsVector;
                                      }, [](const int newValue) -> void {
                                          polylineVertexIndex = newValue;
                                      });
    }

    const auto contOther = createCalculatedInfoSection(container);
    if (contOther == nullptr) {
        return;
    }

    addReadOnlyString<RS_Polyline>({"length", tr("Length"), tr("Length of polyline")}, [this](const RS_Polyline* e) -> QString {
        const double len = e->getLength();
        QString value = formatLinear(len);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Polyline>({"hasArcs", tr("Has Arcs"), tr("Determines whether polyline includes arc segments")},
                                   [](const RS_Polyline* e) -> QString {
                                       const bool hasArcs = e->containsArc();
                                       QString value = hasArcs ? tr("Yes") : tr("No");
                                       return value;
                                   }, list, contOther);

    if (singleEntity) {
        addVector<RS_Polyline>({"start", tr("Start"), tr("Start point of polyline")}, [](const RS_Polyline* e) -> RS_Vector {
            return e->getStartpoint();
        }, nullptr, list, contOther);

        addVector<RS_Polyline>({"end", tr("End"), tr("End point of polyline")}, [](const RS_Polyline* line) -> RS_Vector {
            return line->getEndpoint();
        }, nullptr, list, contOther);
    }
}
