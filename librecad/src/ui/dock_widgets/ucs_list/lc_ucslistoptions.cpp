/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

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
 ******************************************************************************/

#include "lc_ucslistoptions.h"

#include "lc_settings_appearance.h"
#include "lc_settings_startup.h"
#include "lc_settings_ucs_list_widget.h"
#include "rs_settings.h"

LC_UCSListOptions::LC_UCSListOptions() = default;

void LC_UCSListOptions::load() {
    {
        using namespace CFG_Appearance;
        ucsApplyingPolicy = o_UCSApplyPolicy;
        highlightBlinksCount = o_UCSHighlightBlinkCount;
        highlightBlinksDelay =  o_UCSHighlightBlinkDelay;
    }

    {
        using namespace CFG_WidgetUCSList;
        showViewInfoToolTip =o_ShowTooltip;
        showColumnTypeIcon = o_ShowColumnTypeIcon;
        showColumnGridType = o_ShowColumnGridType;
        showColumnPositionAndAngle = o_ShowColumnPositionAndAngle;
        askForDeletionConfirmation = o_ConfirmDelete;
        restoreViewBySingleClick = o_SingleClickRestore;
        doubleClickPolicy = o_DoubleClickPolicy;
        showGrid = o_ShowGrid;
    }
}

void LC_UCSListOptions::save() const{
    {
        using namespace CFG_Appearance;
        o_UCSApplyPolicy = ucsApplyingPolicy;
        o_UCSHighlightBlinkCount = highlightBlinksCount;
        o_UCSHighlightBlinkDelay = highlightBlinksDelay;
    }
    LC_GROUP_END();    
    {
        using namespace CFG_WidgetUCSList;
        o_ShowTooltip= showViewInfoToolTip;
        o_ShowColumnTypeIcon= showColumnTypeIcon;
        o_ShowColumnGridType= showColumnGridType;
        o_ShowColumnPositionAndAngle= showColumnPositionAndAngle;
        o_ConfirmDelete= askForDeletionConfirmation;
        o_DoubleClickPolicy = doubleClickPolicy;
        o_SingleClickRestore= restoreViewBySingleClick;
        o_ShowGrid= showGrid;
    }
}
