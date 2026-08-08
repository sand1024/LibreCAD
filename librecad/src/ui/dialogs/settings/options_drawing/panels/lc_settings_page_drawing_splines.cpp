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

#include "lc_settings_page_drawing_splines.h"
#include "ui_lc_settings_page_drawing_splines.h"
#include "lc_document_settings_backend.h"
#include "rs_graphic.h"

LC_SettingsPageDrawingSplines::LC_SettingsPageDrawingSplines(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Splines"), nullptr, parent)
    , m_graphic(graphic) {
}

LC_SettingsPageDrawingSplines::~LC_SettingsPageDrawingSplines() = default;

void LC_SettingsPageDrawingSplines::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingSplines>();
    ui->setupUi(m_widget);
}

void LC_SettingsPageDrawingSplines::loadSettings() {
    if (!m_graphic) return;

    ui->cbSplineSegs->setEditText(QString::number(m_graphic->getVariableInt("$SPLINESEGS", 8)));
    ui->cbLineCap->setCurrentIndex(m_graphic->getGraphicVariableInt("$ENDCAPS", 1));
    ui->cbLineJoin->setCurrentIndex(m_graphic->getGraphicVariableInt("$JOINSTYLE", 1));
}

bool LC_SettingsPageDrawingSplines::saveSettings() {
    if (!m_graphic) return false;

    m_graphic->addVariable("$SPLINESEGS", ui->cbSplineSegs->currentText().toInt(), 70);
    m_graphic->addVariable("$ENDCAPS", ui->cbLineCap->currentIndex(), 70);
    m_graphic->addVariable("$JOINSTYLE", ui->cbLineJoin->currentIndex(), 70);

    m_graphic->updateSplines(); // Force entities refresh on screen
    return true;
}

bool LC_SettingsPageDrawingSplines::isModified() const {
    if (!m_graphic) return false;

    // Direct value evaluation against the graphic's initial variables.
    // Note that we utilize 'getGraphicVariableInt' for endcaps/joinstyle
    // to match the document's internal API exactly.
    return ui->cbSplineSegs->currentText().toInt() != m_graphic->getVariableInt("$SPLINESEGS", 8) ||
           ui->cbLineCap->currentIndex() != m_graphic->getGraphicVariableInt("$ENDCAPS", 1) ||
           ui->cbLineJoin->currentIndex() != m_graphic->getGraphicVariableInt("$JOINSTYLE", 1);
}
