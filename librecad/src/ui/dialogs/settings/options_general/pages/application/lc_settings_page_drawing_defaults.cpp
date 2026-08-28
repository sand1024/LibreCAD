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

#include "lc_settings_page_drawing_defaults.h"
#include "lc_settings_backend.h"
#include "lc_settings_defaults.h"
#include "rs_units.h"
#include "ui_lc_settings_page_drawing_defaults.h"

LC_SettingsPageDrawingDefaults::LC_SettingsPageDrawingDefaults(QObject* parent)
    : LC_SettingsPageBase(tr("New Drawing Defaults"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Defaults::Group), parent),
      ui(std::make_unique<Ui::LC_SettingsPageDrawingDefaults>()) {
    setSortWeight(20);
}

LC_SettingsPageDrawingDefaults::~LC_SettingsPageDrawingDefaults() = default;

void LC_SettingsPageDrawingDefaults::setupUi() {
    ui->setupUi(m_widget);

    for (int i = RS2::None; i < RS2::LastUnit; ++i) {
        if (i != static_cast<int>(RS2::None)) {
            ui->cbUnit->addItem(RS_Units::unitToString(static_cast<RS2::Unit>(i)));
        }
    }
    ui->cbUnit->insertItem(0, RS_Units::unitToString(RS2::None));
}

void LC_SettingsPageDrawingDefaults::setupBindings() {
    using namespace CFG_Defaults;

    bindString({{ui->leDefAngleBaseZero, o_AnglesBaseAngle}});

    bindBoolean({{ui->cbNewDrawingGridOff, o_GridOffForNewDrawing}});

    bindCustom<QComboBox, QString>(ui->cbUnit, o_Unit, false,
        &QComboBox::currentTextChanged,
        [](QComboBox* w) {
                                       return RS_Units::unitToString(RS_Units::stringToUnit(w->currentText()), false);
                                   }, [](QComboBox* w, const QString& v) {
                                       const int idx = w->findText(QObject::tr(v.toUtf8().data()));
                                       if (idx >= 0) {
                                           w->setCurrentIndex(idx);
                                       }
                                   });

    // Isometric Grid Radios
    bindCustom<QWidget, bool>(m_widget, o_IsometricGrid, false,
        [this](QWidget*) {
                                  return !ui->rbGridOrtho->isChecked();
                              }, [this](QWidget*, bool isIso) {
                                  ui->rbGridOrtho->setChecked(!isIso);
                              });

    bindCustom<QWidget, RS2::IsoGridViewType>(m_widget, o_IsoGridView, false, [this](QWidget*) {
                                 if (ui->rbGridIsoLeft->isChecked())
                                     return RS2::IsoGridViewType::IsoLeft;
                                 if (ui->rbGridIsoRight->isChecked())
                                     return RS2::IsoGridViewType::IsoRight;
                                 return RS2::IsoGridViewType::IsoTop;
                             }, [this](QWidget*, int viewType) {
                                 ui->rbGridIsoLeft->setChecked(viewType == RS2::IsoGridViewType::IsoLeft);
                                 ui->rbGridIsoRight->setChecked(viewType == RS2::IsoGridViewType::IsoRight);
                                 ui->rbGridIsoTop->setChecked(
                                     viewType != RS2::IsoGridViewType::IsoLeft && viewType !=
                                         RS2::IsoGridViewType::IsoRight);
                             });

    // Angles Positive/Negative direction radios
    bindCustom<QRadioButton, bool>(ui->rbDefAngleBasePositive, o_AnglesCounterClockwise, false,
            &QRadioButton::toggled,  [](QRadioButton* w) {
                                       return w->isChecked();
                                   }, [this](QRadioButton* w, bool ccw) {
                                       w->setChecked(ccw);
                                       ui->rbDefAngleBaseNegative->setChecked(!ccw);
                                   });
}
