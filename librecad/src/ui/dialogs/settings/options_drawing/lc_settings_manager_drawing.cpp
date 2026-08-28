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
#include "pages/lc_settings_page_drawing_units.h"
#include "pages/lc_settings_page_drawing_grid.h"
#include "lc_settings_page_drawing_dimensions.h"
#include "pages/lc_settings_page_drawing_points.h"
#include "pages/lc_settings_page_drawing_splines.h"
#include "pages/lc_settings_page_drawing_meta.h"
#include "pages/lc_settings_page_drawing_user_data.h"
#include "pages/lc_settings_page_drawing_variables.h"

namespace {
    // Static pointer used ONLY during the dialog instantiation phase
    static RS_Graphic* g_contextGraphic = nullptr;
}

void LC_SettingsManagerDrawing::initialize() {
    auto* reg = LC_SettingsRegistry::instance();
    static bool isInitialized = false;
    if (isInitialized) return;

    const QString dlgId = "drawing_preferences"; // fixme - to namespace

    reg->configureDialog(dlgId, QObject::tr("Drawing Preferences"), false);

    auto addPage = [&](const QString& id, std::function<std::unique_ptr<LC_SettingsPageInterface>()> creator) {
        reg->registerPage(dlgId, id, "", creator);
    };

    using namespace LC_SettingsPagesDrawing;
    addPage(Units,      [] { return std::make_unique<LC_SettingsPageDrawingUnits>(g_contextGraphic); });
    addPage(Paper,      [] { return std::make_unique<LC_SettingsPageDrawingPaper>(g_contextGraphic); });
    addPage(Grid,       [] { return std::make_unique<LC_SettingsPageDrawingGrid>(g_contextGraphic); });
    addPage(Dimensions, [] { return std::make_unique<LC_SettingsPageDrawingDimensions>(g_contextGraphic); });
    addPage(Points,     [] { return std::make_unique<LC_SettingsPageDrawingPoints>(g_contextGraphic); });
    addPage(Splines,    [] { return std::make_unique<LC_SettingsPageDrawingSplines>(g_contextGraphic); });
    addPage(Meta,       [] { return std::make_unique<LC_SettingsPageDrawingMeta>(g_contextGraphic); });
    addPage(UserData,   [] { return std::make_unique<LC_SettingsPageDrawingUserData>(g_contextGraphic); });
    addPage(Variables,  [] { return std::make_unique<LC_SettingsPageDrawingVariables>(g_contextGraphic); });

    isInitialized = true;
}

bool LC_SettingsManagerDrawing::show(RS_Graphic* graphic, QWidget* parent, const QString& initialPageId) {
    if (!graphic) return false;

    initialize();

    g_contextGraphic = graphic;

    bool accepted = LC_SettingsRegistry::instance()->showDialog(
        "drawing_preferences",
        initialPageId.isEmpty() ? "draw_doc.paper" : initialPageId,
        parent
    );

    g_contextGraphic = nullptr;
    return accepted;
}
