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


#include "lc_settings_manager_drawing.h"
#include "lc_settings_registry.h"
#include "rs_graphic.h"

// Pages
#include "lc_settings_page_drawing_paper.h"
#include "lc_settings_page_drawing_units.h"
#include "lc_settings_page_drawing_grid.h"
#include "lc_settings_page_drawing_dimensions.h"
#include "lc_settings_page_drawing_points.h"
#include "lc_settings_page_drawing_splines.h"
#include "lc_settings_page_drawing_meta.h"
#include "lc_settings_page_drawing_user_data.h"
#include "lc_settings_page_drawing_variables.h"

namespace {
    // Static pointer used ONLY during the dialog instantiation phase
    static RS_Graphic* g_contextGraphic = nullptr;
}

template<class T>
    static LC_SettingsRegistry::PageCreator pageWithContext() {
    return [] {return std::make_unique<T>(g_contextGraphic);};
}


void LC_SettingsManagerDrawing::initialize() {
    auto* reg = LC_SettingsRegistry::instance();

    using namespace LC_SettingsPagesDrawing;

    const QString dlgId = DLG_DOCUMENT_PREFERENCES;

    reg->configureDialog(dlgId, {QObject::tr("Drawing Preferences"), false, false,false, ":/icons/drawing_settings.lci"});

    auto addPage = [&](const QString& id, std::function<std::unique_ptr<LC_SettingsPageInterface>()> creator) {
        reg->registerPage(dlgId, id, "", creator);
    };


    addPage(PAGE_UNITS,      pageWithContext<LC_SettingsPageDrawingUnits>());
    addPage(PAGE_PAPER,      pageWithContext<LC_SettingsPageDrawingPaper>());
    addPage(PAGE_GRID,       pageWithContext<LC_SettingsPageDrawingGrid>());
    addPage(PAGE_DIMENSIONS, pageWithContext<LC_SettingsPageDrawingDimensions>());
    addPage(PAGE_POINTS,     pageWithContext<LC_SettingsPageDrawingPoints>());
    addPage(PAGE_SPLINES,    pageWithContext<LC_SettingsPageDrawingSplines>());
    addPage(PAGE_META,       pageWithContext<LC_SettingsPageDrawingMeta>());
    addPage(PAGE_USER_DATA,  pageWithContext<LC_SettingsPageDrawingUserData>());
    addPage(PAGE_VARIABLES,  pageWithContext<LC_SettingsPageDrawingVariables>());

}

bool LC_SettingsManagerDrawing::show(RS_Graphic* graphic, QWidget* parent, const QString& initialPageId) {
    if (graphic == nullptr) {
        return false;
    }

    g_contextGraphic = graphic;

    using namespace LC_SettingsPagesDrawing;
    bool accepted = LC_SettingsRegistry::instance()->showDialog(DLG_DOCUMENT_PREFERENCES,
                                                                initialPageId.isEmpty() ? PAGE_UNITS : initialPageId, parent);

    g_contextGraphic = nullptr;
    return accepted;
}
