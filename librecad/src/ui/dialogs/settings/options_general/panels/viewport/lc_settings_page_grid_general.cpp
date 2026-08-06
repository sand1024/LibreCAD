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

#include "lc_settings_page_grid_general.h"


#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_grid_general.h"
#include "lc_settings_backend.h"
#include "lc_settings_grid.h"
#include "lc_settings_list_widget.h"

LC_SettingsPageGridGeneral::LC_SettingsPageGridGeneral(QObject* parent)
    : LC_SettingsPageBase(tr("Grid"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Grid::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPageGridGeneral>()) {
    // setSortWeight(10);
}

LC_SettingsPageGridGeneral::~LC_SettingsPageGridGeneral() = default;

void LC_SettingsPageGridGeneral::setupUi() {
    ui->setupUi(m_widget);
}

void LC_SettingsPageGridGeneral::setChildPages(const QList<LC_SettingsPageInterface*>& children) {

    if (children.isEmpty()) {
        return;
    }

    QList<QPair<QString, QString>> links;
    for (auto* child : children) {
        // Symmetrical Update: Use child->displayName() to register category index targets [4.2]
        links.append({child->id(), child->displayName()});
    }

    auto m_linksWidget = new LC_SettingsLinksWidget(links, m_widget);
    auto m_mainLayout = ui->gbGridType->layout();
    m_mainLayout->addWidget(m_linksWidget);

    connect(m_linksWidget, &LC_SettingsLinksWidget::pageSelected,
            this, &LC_SettingsPageBase::navigateToPage);
}

void LC_SettingsPageGridGeneral::setupBindings() {
    using namespace CFG_Appearance;

    bindBoolean({
        { ui->cb_unitless_grid, o_UnitlessGrid },
        { ui->cbSimpleGridRendring, o_GridRenderSimple },
        { ui->cbDisableGridOnPanning, o_GridDisableWithinPan },
        { ui->cbScaleGrid, o_ScaleGrid },
        { ui->cbDrawGrid, o_GridDraw },
        { ui->cbDrawMetaGrid, o_metaGridDraw },
        { ui->cbDrawVerticalForIsoTop, o_GridDrawIsoVerticalForTop}
    });

    bindInt({
        { ui->sbGridMetaMajorEvery, o_MetaGridEvery },
        { ui->sbMinGridSpacing, o_MinGridSpacing }
    });

    bindComboIndex({
        { ui->cbGridType, CFG_Grid::o_GridType }
    });
}
