/****************************************************************************
 *
* Options for QuickInfo widget related functions

Copyright (C) 2024 LibreCAD.org
Copyright (C) 2024 sand1024

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/

#include "lc_quickinfowidgetoptions.h"

#include "lc_settings_quick_info_widget.h"

void LC_QuickInfoOptions::load(){
    using namespace CFG_WidgetQuickInfo;

    displayDistanceAndAngle = o_ShowDistanceAndAngle;
    displayEntityBoundaries = o_ShowEntityBoundaries;
    displayPointsPath = o_ShowPointsPathOnPreview;
    displayPolylineDetailed = o_ShowPolylineDetails;
    selectEntitiesInDefaultActionByCTRL = o_SelectEntityInDefaultAction;
    autoSelectEntitiesInDefaultAction = o_AutoSelectEntityInDefaultAction;

    const auto color = RS_Color(o_penHighlightColor);
    const auto lineType = o_penHighlightLineType;
    const auto lineWidth = o_penHighlightLineWidth;

    pen = RS_Pen(color, lineWidth, lineType);
}

void LC_QuickInfoOptions::save() const {
    using namespace CFG_WidgetQuickInfo;
    o_ShowDistanceAndAngle = displayDistanceAndAngle;
    o_ShowEntityBoundaries = displayEntityBoundaries;
    o_ShowPointsPathOnPreview = displayPointsPath;
    o_ShowPolylineDetails = displayPolylineDetailed;
    o_SelectEntityInDefaultAction = selectEntitiesInDefaultActionByCTRL;
    o_AutoSelectEntityInDefaultAction = autoSelectEntitiesInDefaultAction;
    o_penHighlightColor = pen.getColor().name();
    o_penHighlightLineType = pen.getLineType();
    o_penHighlightLineWidth = pen.getWidth();
}
