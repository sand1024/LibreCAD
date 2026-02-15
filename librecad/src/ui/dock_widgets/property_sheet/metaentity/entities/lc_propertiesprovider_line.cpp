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

#include "lc_propertiesprovider_line.h"

#include <QList>

#include "lc_convert.h"
#include "lc_property_action.h"
#include "lc_property_action_link_view.h"
#include "lc_property_multi.h"
#include "rs_line.h"
#include "rs_math.h"

void LC_PropertiesProviderLine::doFillEntitySpecificProperties(LC_PropertyContainer* cont, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(cont);

    addVector<RS_Line>({"start", tr("Start"), tr("Start point of line")}, [](const RS_Line* e) -> RS_Vector {
                           return e->getStartpoint();
                       }, [](const RS_Vector& v, RS_Line* l) -> void {
                           l->setStartpoint(v);
                       }, list, contGeometry);

    addVector<RS_Line>({"end", tr("End"), tr("End point of line")}, [](const RS_Line* line) -> RS_Vector {
                           return line->getEndpoint();
                       }, [](const RS_Vector& v, RS_Line* l) -> void {
                           l->setEndpoint(v);
                       }, list, contGeometry);

    add<RS_Line>({"reverse", tr("Reverse line direction"), tr("Swaps start and end point of line")},
                 [this](const LC_Property::Names& names, RS_Line* e, LC_PropertyContainer* container,
                        QList<LC_PropertyAtomic*>* props) -> void {
                     auto* property = new LC_PropertyAction(container, true);
                     property->setNames(names);
                     LC_PropertyViewDescriptor viewDescriptor("Link");
                     viewDescriptor[LC_PropertyActionLinkView::ATTR_TITLE] = names.displayName;
                     property->setEntity(e);
                     property->setClickHandler([this, container]([[maybe_unused]] const LC_PropertyAction* action, [[maybe_unused]] int linkIndex) {
                         auto multiPropertyList = container->findChildProperties("reverse");
                         for (const auto mp : multiPropertyList) {
                             // actually, it should be only one
                             const auto multiProperty = dynamic_cast<LC_PropertyMulti*>(mp);
                             if (multiProperty != nullptr) {
                                 for (const auto p : multiProperty->getProperties()) {
                                     const auto actionProperty = dynamic_cast<LC_PropertyAction*>(p);
                                     if (actionProperty != nullptr) {
                                         RS_Entity* ent = actionProperty->getEntity();
                                         if (ent != nullptr) {
                                             if (ent->rtti() == RS2::EntityLine) {
                                                 auto* entity = static_cast<RS_Line*>(ent);
                                                 const auto clone = static_cast<RS_Line*>(entity->clone());
                                                 clone->revertDirection();
                                                 m_widget->entityModified(entity, clone);
                                             }
                                         }
                                     }
                                 }
                             }
                             m_widget->onPropertyEdited(multiProperty);
                         }
                     });

                     property->setViewDescriptor(viewDescriptor);
                     props->push_back(property);
                 }, list, contGeometry);

    const auto contOther = createCalculatedInfoSection(cont);
    if (contOther == nullptr) {
        return;
    }

    addReadOnlyString<RS_Line>({"length", tr("Length"), tr("Length of line")}, [this](const RS_Line* e) -> QString {
        const double len = e->getLength();
        QString value = formatLinear(len);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Line>({"angle1", tr("Angle 1"), tr("Angle from 0.0 to first point of line")}, [this](const RS_Line* e) -> QString {
        const double wcsAngleRad = e->getAngle1();
        QString value = formatWCSAngleDegrees(wcsAngleRad);
        return value;
    }, list, contOther);

    addReadOnlyString<RS_Line>({"angle2", tr("Angle 2"), tr("Angle from 0.0 to second point of line")},
                               [this](const RS_Line* e) -> QString {
                                   const double wcsAngleRad = e->getAngle2();
                                   QString value = formatWCSAngleDegrees(wcsAngleRad);
                                   return value;
                               }, list, contOther);

    addReadOnlyString<RS_Line>({"inclination", tr("Inclination"), tr("Angle of the line inclination to x-axis")},
                               [this](const RS_Line* e) -> QString {
                                   const double wcsAngleRad = RS_Math::correctAngle0ToPi(e->getAngle1());
                                   QString value = formatWCSAngleDegrees(wcsAngleRad);
                                   return value;
                               }, list, contOther);

    if (!m_actionContext->getViewport()->hasUCS()) {
        addVector<RS_Line>({"delta", tr("Delta"), tr("Distance between start and end point")}, [](const RS_Line* line) -> RS_Vector {
            // fixme - this works only for WCS!?
            const auto deltaVectorWCS = line->getEndpoint() - line->getStartpoint();
            return deltaVectorWCS;
        }, nullptr, list, contOther);
    }

    addVector<RS_Line>({"middle", tr("Middle Point"), tr("Middle point of line")}, [](const RS_Line* e) -> RS_Vector {
        return e->getMiddlePoint();
    }, nullptr, list, contOther);
}
