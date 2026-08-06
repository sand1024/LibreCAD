/*******************************************************************************
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
 ******************************************************************************/


#ifndef LC_INTERACTIVE_INPUT_INFO_H
#define LC_INTERACTIVE_INPUT_INFO_H

#include <QString>
#include "rs_vector.h"
#include "lc_latecompletionrequestor.h"

struct InteractiveInputInfo {
    enum State {
        NONE,
        REQUESTED
    };

    enum InputType {
        POINT,
        POINT_X,
        POINT_Y,
        DISTANCE,
        ANGLE,
        NOTNEEDED,
    };

    State state {NONE};
    double distance {0};
    double angleRad{0};
    RS_Vector wcsPoint;
    InputType inputType {NOTNEEDED};
    QString requestorTag;
    LC_LateCompletionRequestor* requestor {nullptr};

    void copyTo(InteractiveInputInfo& copy) const {
        copy.state = state;
        copy.distance = distance;
        copy.angleRad  = angleRad;
        copy.wcsPoint = wcsPoint;
        copy.inputType = inputType;
        copy.requestorTag = requestorTag;
        // don't need requestor in copy!
    }
};

#endif
