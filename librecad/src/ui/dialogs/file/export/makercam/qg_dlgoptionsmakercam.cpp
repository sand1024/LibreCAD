/*
 * **************************************************************************
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
 * *********************************************************************
 */

#include "qg_dlgoptionsmakercam.h"

#include "lc_settings_export_makercam.h"

QG_DlgOptionsMakerCam::QG_DlgOptionsMakerCam(QWidget* parent, const bool modal, const Qt::WindowFlags fl) : QDialog(parent, fl) {
    setModal(modal);
    setupUi(this);
    this->gbLayers->setToolTip(tr(
        "MakerCAM as of November 2014 does not hide SVG content \nthat has been set invisibe (\"display: none\" or \"visibility: hidden\")."));
    this->gbBlocks->setToolTip(tr(
        "MakerCAM as of November 2014 cannot correctly deal with blocks,\nbecause it does not take into account the reference point in the <use>."));
    this->gbEllipses->setToolTip(tr(
        "MakerCAM as of March 2015 cannot display ellipses and ellipse arcs correctly, \nwhen they are created using the <ellipse> tag  with a rotation in \n"
        "the <transform> attribute or as <path> using elliptic arc segments."));
    this->gbImages->setToolTip(tr("Exported images can be useful in SVG editors (Inkscape, etc), \nbut avoided in some CAM's."));
    this->gbDashLines->setToolTip(tr(
        "Many CAM's(MakerCAM, EleskCAM, LaserWeb) ignore dashed/doted line style, \nwhich can be useful in lasercut of plywood or for papercraft. "));
    this->dSpinBoxDefaultElementWidth->
          setToolTip(tr("Default width of elements can affect some CAM's/SVG Editors, \nbut ignored by other"));
    this->dSpinBoxDashLinePatternLength->setToolTip(
        tr("Length of line pattern related to zoom, \nso default step value required for baking"));
    gbImages->setToolTip(tr("Whether to export points"));

    loadSettings();
}

void QG_DlgOptionsMakerCam::languageChange() {
    retranslateUi(this);
}

void QG_DlgOptionsMakerCam::validate() {
    saveSettings();

    accept();
}

void QG_DlgOptionsMakerCam::cancel() {
    reject();
}

void QG_DlgOptionsMakerCam::loadSettings() const {
    using namespace CFG_ExportMakerCam;
    checkInvisibleLayers->setChecked(o_ExportInvisibleLayers);
    checkConstructionLayers->setChecked(o_ExportConstructionLayers);
    checkBlocksInline->setChecked(o_WriteBlocksInline);
    checkEllipsesToBeziers->setChecked(o_ConvertEllipsesToBeziers);
    checkImages->setChecked(o_ExportImages);
    checkDashDotLines->setChecked(o_BakeDashDotLines);
    checkPoint->setChecked(o_ExportPoints);
    dSpinBoxDefaultElementWidth->setValue(o_DefaultElementWidth);
    dSpinBoxDashLinePatternLength->setValue(o_DefaultDashLinePatternLength);
}

void QG_DlgOptionsMakerCam::saveSettings() {
    using namespace CFG_ExportMakerCam;
    o_ExportInvisibleLayers = checkInvisibleLayers->isChecked();
    o_ExportConstructionLayers = checkConstructionLayers->isChecked();
    o_WriteBlocksInline = checkBlocksInline->isChecked();
    o_ConvertEllipsesToBeziers = checkEllipsesToBeziers->isChecked();
    o_ExportImages = checkImages->isChecked();
    o_BakeDashDotLines = checkDashDotLines->isChecked();
    o_ExportPoints = checkPoint->isChecked();
    o_DefaultElementWidth = dSpinBoxDefaultElementWidth->value();
    o_DefaultDashLinePatternLength = dSpinBoxDashLinePatternLength->value();
}
