/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

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
 ******************************************************************************/

#include "lc_namedviewslistoptions.h"

#include "lc_settings_named_views_list_widget.h"
#include "rs_settings.h"

LC_NamedViewsListOptions::LC_NamedViewsListOptions() = default;

void LC_NamedViewsListOptions::load() {
    using namespace CFG_WidgetNamedViewsList;
    showViewInfoToolTip = o_ShowTooltip;
    showColumnIconType = o_ShowIconType;
    showColumnGridType = o_ShowColumnIconGrid;
    showColumnUCSType = o_ShowColumnIconUCSType;
    showColumnViewDetails = o_ShowColumnViewDetails;
    showColumnUCSDetails = o_ShowColumnUCSDetails;
    askForDeletionConfirmation = o_ConfirmDelete;
    duplicatedNameReplacesSilently = o_ReplaceDuplicateSilently;
    doubleClickPolicy = o_DoubleClickPolicy;
    restoreViewBySingleClick = o_RestoreViewBySingleClick;
    showGrid = o_ShowGrid;
}

void LC_NamedViewsListOptions::save() const {
    using namespace CFG_WidgetNamedViewsList;
    o_ShowTooltip = showViewInfoToolTip;
    o_ConfirmDelete = askForDeletionConfirmation;
    o_ReplaceDuplicateSilently = duplicatedNameReplacesSilently;
    o_DoubleClickPolicy = doubleClickPolicy;
    o_RestoreViewBySingleClick = restoreViewBySingleClick;
    o_ShowIconType = showColumnIconType;
    o_ShowColumnIconGrid = showColumnGridType;
    o_ShowColumnIconUCSType = showColumnUCSType;
    o_ShowColumnViewDetails = showColumnViewDetails;
    o_ShowColumnUCSDetails = showColumnUCSDetails;
    o_ShowGrid = showGrid;
}
