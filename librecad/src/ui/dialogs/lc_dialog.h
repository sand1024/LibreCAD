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

#ifndef LC_DIALOG_H
#define LC_DIALOG_H

#include <QDialog>

#include "lc_setting.h"

class LC_SettingsGroupDialog : public LC_SettingsGroupBase {
public:
    explicit LC_SettingsGroupDialog(const QString& dialogName)
        : LC_SettingsGroupBase("Dlg" + dialogName) {}

    ~LC_SettingsGroupDialog() override = default;

    // Fixed variable keys inside the dynamically named group [1]
    LC_Setting<bool> o_hasPosition{this, "hasPosition", false};
    LC_Setting<int> o_X{this, "X", 0};
    LC_Setting<int> o_Y{this, "Y", 0};
    LC_Setting<int> o_Height{this, "Height", 0};
    LC_Setting<int> o_Width{this, "Width", 0};
};

class LC_Dialog : public QDialog{
public:
    LC_Dialog(QWidget *parent, const QString &dlgName);
    ~LC_Dialog() override = default;
    void accept() override;
    void reject() override;
    int showModal();
protected:
    QString m_dialogName;
    bool m_positionLoaded = false;
    void setDialogName(const QString& dialogName) {m_dialogName = dialogName;} ;
    void saveDialogPosition() const;
    void loadDialogPosition();
    virtual void saveInnerDialogData([[maybe_unused]] LC_SettingsGroupDialog& group, [[maybe_unused]] bool savePositions) const {
    }
    virtual void loadInnerDialogData([[maybe_unused]] LC_SettingsGroupDialog& group, [[maybe_unused]] bool savePositions) {};

    void showEvent(QShowEvent *event) override;
};

#endif
