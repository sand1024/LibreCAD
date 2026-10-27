/*******************************************************************************
 *
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

#ifndef LC_COORDINATESWIDGET_H
#define LC_COORDINATESWIDGET_H

#include "lc_coordinate_widget_base.h"
#include "rs_vector.h"

class LC_GraphicViewport;
class RS_Graphic;
class RS_GraphicView;

namespace Ui {
    class LC_CoordinatesWidget;
}

class LC_CoordinatesWidget : public LC_CoordinateWidgetBase {
    Q_OBJECT
public:
    explicit LC_CoordinatesWidget(QWidget* parent = nullptr,
                                  const char* name = nullptr,
                                  Qt::WindowFlags fl = {});
    ~LC_CoordinatesWidget() override;

    void clearContent() const;
    void setGraphicView(RS_GraphicView* gv) override;

public slots:
    void setCoordinates(const RS_Vector& wcsAbs, const RS_Vector& wcsDelta, bool updateFormat);

protected slots:
    void languageChange();
    void setCoordinates(double ucsX, double ucsY, double ucsDeltaX, double ucsDeltaY, bool updateFormat);

private:
    Ui::LC_CoordinatesWidget* m_ui = nullptr;
    RS_Vector m_absoluteCoordinates;
    RS_Vector m_relativeCoordinates;
};

#endif
