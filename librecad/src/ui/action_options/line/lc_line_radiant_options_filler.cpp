/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 * Copyright (C) 2026 sand1024
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

#include "lc_line_radiant_options_filler.h"
#include "lc_action_draw_line_radiant.h"

void LC_RadiantLineOptionsFiller::fillToolOptionsContainer(LC_PropertyContainer* container) {
    auto options = static_cast<LC_ActionDrawLineRadiantOptions*>(m_action->getOptions());

    addIntSpinbox({"activeRadiantIdx", "Radiant Index", "Selection of radiant (center) point draw to"}, [options]() {
                      return options->getActiveRadiantIndex()+1;
                  }, [options](LC_PropertyEnumValueType index) {
                      options->setActiveRadiantIndex(static_cast<LC_ActionDrawLineRadiantOptions::RadiantIdx>(index-1));
                  }, container, 1, 4);

    addVector({"activeRadiant", "Radiant Point", "Specifies position of active radiant point"}, [options]() -> RS_Vector {
                  return options->getActiveRadiant();
              }, [options](const RS_Vector& v)-> void {
                  options->setActiveRadiantPoint(v);
              }, container);

    static LC_EnumDescriptor descriptor = {
        "lenTypeDescriptor",
        {
            {LC_ActionDrawLineRadiantOptions::LenghtType::LINE, tr("Line")},
            {LC_ActionDrawLineRadiantOptions::LenghtType::BY_X, tr("By X")},
            {LC_ActionDrawLineRadiantOptions::LenghtType::BY_Y, tr("By Y")},
            {LC_ActionDrawLineRadiantOptions::LenghtType::TO_POINT, tr("To Point")},
            {LC_ActionDrawLineRadiantOptions::LenghtType::FREE, tr("Free")},
        }
    };

    addEnum({"lenType", "Length type:", "Defines how to handle length parameter"}, &descriptor, [options]() -> LC_PropertyEnumValueType {
                return options->getLenghType();
            }, [this](const LC_PropertyEnumValueType& v)-> void {
                auto action = static_cast<LC_ActionDrawLineRadiant*>(m_action);
                auto type = static_cast<LC_ActionDrawLineRadiantOptions::LenghtType>(v);
                action->setLengthType(type);
            }, container);

    addLinearDistance({"length", "Length:", "Length of line to draw"}, [options]() {
                          return options->getLength();
                      }, [options](double val) {
                          options->setLength(val);
                      }, container);
}
