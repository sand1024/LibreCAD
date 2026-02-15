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

#include "lc_propertiesprovider_dim_linear.h"

#include "lc_property_action.h"
#include "rs_dimlinear.h"

void LC_PropertiesProviderDimLinear::doCreateDimGeometrySection(LC_PropertyContainer* container, const QList<RS_Entity*>& list) {
    addVector<RS_DimLinear>({"dimExtPoint1", tr("Ext point 1"), tr("Position of first extension point")},
                            [](const RS_DimLinear* e) -> RS_Vector {
                                return e->getExtensionPoint1();
                            }, [](const RS_Vector& v, RS_DimLinear* e) -> void {
                                e->setExtensionPoint1(v);
                            }, list, container);

    addVector<RS_DimLinear>({"dimExtPoint2", tr("Ext point 2"), tr("Position of second extension point")},
                            [](const RS_DimLinear* e) -> RS_Vector {
                                return e->getExtensionPoint2();
                            }, [](const RS_Vector& v, RS_DimLinear* e) -> void {
                                e->setExtensionPoint2(v);
                            }, list, container);

    addVector<RS_DimLinear>({"dimDefPoint", tr("Definition point "), tr("Position of definition point")},
                            [](const RS_DimLinear* e) -> RS_Vector {
                                return e->getDefinitionPoint();
                            }, [](const RS_Vector& v, RS_DimLinear* e) -> void {
                                e->setDefinitionPoint(v);
                            }, list, container);

    addWCSAngle<RS_DimLinear>({"dimAngle", tr("Angle"), tr("Rotation angle")}, [](const RS_DimLinear* e) -> double {
                                  return e->getAngle();
                              }, [](const double& v, RS_DimLinear* l) -> void {
                                  l->setAngle(v);
                              }, list, container);

    addWCSAngle<RS_DimLinear>({"dimOblique", tr("Oblique angle"), tr("Oblique angle")}, [](const RS_DimLinear* e) -> double {
                                  return e->getOblique();
                              }, [](const double& v, RS_DimLinear* l) -> void {
                                  l->setOblique(v);
                              }, list, container);
    /*
    fixme - think how Continue and Baseline may be invoke for the dimension.... The issue - selection of the proper dimension side...
    if (list.size() == 1) {
        auto* propertyContinue = new LC_PropertyAction(cont, true);
        LC_Property::Names names = {
            "dimCreateContinue",
            tr("Continue"),
            tr("Create another dimension that continues this one")
        };
        propertyContinue->setNames(names);
        LC_PropertyViewDescriptor viewDescriptor("Link");
        viewDescriptor["title"] = names.displayName;
        propertyContinue->setClickHandler([this, list]([[maybe_unused]] const LC_PropertyAction* action) {
            for (auto e : list) {
                auto d = dynamic_cast<RS_Dimension*>(e);
                if (d != nullptr) {
                    m_actionContext->setCurrentAction(RS2::ActionDimContinue, e);
                    return;
                }
            }
        });
        propertyContinue->setViewDescriptor(viewDescriptor);
        cont->addChildProperty(propertyContinue);
    }
    */
}
