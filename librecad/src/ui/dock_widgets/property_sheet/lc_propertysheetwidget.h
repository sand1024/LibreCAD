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

#ifndef LC_PROPERTYSHEETWIDGET_H
#define LC_PROPERTYSHEETWIDGET_H

#include <QWidget>

#include "lc_graphicviewawarewidget.h"
#include "lc_graphicviewport.h"
#include "lc_property.h"
#include "lc_selectedsetlistener.h"

namespace Ui {
class LC_PropertySheetWidget;
}

class LC_PropertySheetWidget : public LC_GraphicViewAwareWidget, public LC_SelectedSetListener{
    Q_OBJECT
public:
    explicit LC_PropertySheetWidget(QWidget *parent = nullptr);
    ~LC_PropertySheetWidget() override;
    void setGraphicView(RS_GraphicView* view) override;
    void selectionChanged() override;
    void updateFormats();
public slots:
    void onUcsChanged(LC_UCS* ucs);
    void onRelativeZeroChanged(const RS_Vector &);
    void setupSelectionCombobox() const;
    void updateWidgetSettings() ;
    void onBeforePropertyEdited(LC_Property* property, LC_Property::PropertyValuePtr newValue, int typeId);
    void onPropertyEdited(LC_Property* property);
private:
    Ui::LC_PropertySheetWidget *ui;
    void initPropertySheet();
    RS_GraphicView* m_graphicView = nullptr; // fixme - sand - review dependency
    RS_Document* m_document = nullptr;
    LC_GraphicViewport* m_viewport = nullptr;
};

#endif // LC_PROPERTYSHEETWIDGET_H
