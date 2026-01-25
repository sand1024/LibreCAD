/****************************************************************************
**
* Options widget for pen transform action

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
**********************************************************************/
#ifndef LC_PASTETRANSFORMOPTIONS_H
#define LC_PASTETRANSFORMOPTIONS_H

#include "lc_actionoptionswidgetbase.h"

namespace Ui {
    class LC_PasteTransformOptions;
}

class LC_ActionEditPasteTransform;

class LC_PasteTransformOptions : public LC_ActionOptionsWidgetBase{
    Q_OBJECT
public:
    explicit LC_PasteTransformOptions();
    ~LC_PasteTransformOptions() override;
public slots:
    void languageChange() override;
    void onAngleEditingFinished();
    void onFactorEditingFinished();
    void onArraySpacingXEditingFinished();
    void onArraySpacingYEditingFinished();
    void onArrayClicked(bool clicked);
    void onArrayAngleEditingFinished();
    void onArrayXCountChanged(int value);
    void onArrayYCountChanged(int value);
    void cbSameAnglesClicked(bool value);
protected:
    void doSaveSettings() override;
    void doSetAction(RS_ActionInterface *a, bool update) override;
private:
    Ui::LC_PasteTransformOptions *ui;
    LC_ActionEditPasteTransform* m_action = nullptr;
    void setAngleToActionAndView(const QString& val);
    void setFactorToActionAndView(const QString& val);
    void setIsArrayToActionAndView(bool val) const;
    void setSameAnglesToActionAndView(bool val) const;
    void setArrayXCountToActionAndView(int count) const;
    void setArrayYCountToActionAndView(int count) const;
    void setArrayXSpacingToActionAndView(const QString& val);
    void setArrayYSpacingToActionAndView(const QString& val);
    void setArrayAngleToActionAndView(const QString& val);
};

#endif
