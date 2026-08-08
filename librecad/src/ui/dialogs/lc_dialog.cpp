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

#include "lc_dialog.h"

#include <QApplication>
#include <QTimer>

#include "lc_settings_appearance.h"

// fixme - sand - review all dialogs and actions and make all conversions (double/string, angle/string and vise versa consistent)!
LC_Dialog::LC_Dialog(QWidget* parent, const QString& dlgName)
    :QDialog(parent)
    ,m_dialogName(dlgName){

}

int LC_Dialog::showModal() {
#ifdef Q_OS_WIN
    // fixme - sand - potentially here we'll may start displaying Wait cursor .. check with other dialogs
    int result = exec();
    return result;
#    elif
    return exec();
#     endif
}


#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif


void LC_Dialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    if (!m_positionLoaded) {
        loadDialogPosition();
        m_positionLoaded = true;
    }
}

void LC_Dialog::loadDialogPosition() {
    LC_SettingsGroupDialog CFG_DlgSettings(m_dialogName);
    if (CFG_Appearance::o_PersistDialogPositions) {
        if (CFG_DlgSettings.o_hasPosition) {
            const int x = CFG_DlgSettings.o_X;
            const int y = CFG_DlgSettings.o_Y;
            const int h = CFG_DlgSettings.o_Height;
            const int w = CFG_DlgSettings.o_Width;
            if (x > 0 && y > 0 && h > 0 && w > 0) {
                if (!CFG_Appearance::o_PersistDialogRestoreSizeOnly) {
                    move(x, y);
                }
                resize(w, h);
            }
            loadInnerDialogData(CFG_DlgSettings, true);
        }
    }
    else {
        loadInnerDialogData(CFG_DlgSettings, false);
    }
}

void LC_Dialog::saveDialogPosition() const {
    LC_SettingsGroupDialog CFG_DlgSettings(m_dialogName);
    if (CFG_Appearance::o_PersistDialogPositions) {
        CFG_DlgSettings.o_hasPosition = true;

        const QPoint& point = pos();
        const QSize& size = QWidget::size();

        const int x = point.x();
        const int y = point.y();
        const int h = size.height();
        const int w = size.width();
        CFG_DlgSettings.o_X = x;
        CFG_DlgSettings.o_Y = y;
        CFG_DlgSettings.o_Height = h;
        CFG_DlgSettings.o_Width = w;
        saveInnerDialogData(CFG_DlgSettings, true);
    }
    else {
        saveInnerDialogData(CFG_DlgSettings, false);
    }
}

void LC_Dialog::accept() {
    saveDialogPosition();
    QDialog::accept();
}

void LC_Dialog::reject() {
    saveDialogPosition();
    QDialog::reject();
}
