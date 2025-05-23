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

#ifndef QG_DLGOPTIONSMAKERCAM_H
#define QG_DLGOPTIONSMAKERCAM_H

#include "ui_qg_dlgoptionsmakercam.h"
// fixme - sand - MakerCam is dead now - is it still relevant?
class QG_DlgOptionsMakerCam : public QDialog, public Ui::QG_DlgOptionsMakerCam{
    Q_OBJECT

public:
    QG_DlgOptionsMakerCam(QWidget* parent = nullptr, bool modal = false, Qt::WindowFlags fl = {});
	~QG_DlgOptionsMakerCam() = default;

public slots:
    void validate();
    void cancel();
protected slots:
    void languageChange();
private:
    void loadSettings();
    void updateCheckbox(QCheckBox* checkbox, QString name, int defaultValue);
    void updateDoubleSpinBox(QDoubleSpinBox* dSpinBox, QString name, double defaultValue);

    void saveSettings();
    void saveBoolean(QString name, QCheckBox* checkbox);
    void saveDouble(QString name, QDoubleSpinBox* dSpinBox);
};

#endif
