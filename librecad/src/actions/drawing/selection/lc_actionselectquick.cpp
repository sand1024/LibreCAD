/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_actionselectquick.h"

#include "lc_dlgquickselection.h"
#include "qc_applicationwindow.h"
#include "rs_document.h"

void LC_ActionSelectQuick::init(int status) {
    RS_ActionInterface::init(status);
    QWidget* parent = QC_ApplicationWindow::getAppWindow().get();
    LC_DlgQuickSelection dlg(parent,m_document, m_viewport);
    if (dlg.exec() == QDialog::Accepted) {

    }
    finish();
}
