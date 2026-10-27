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

#include "lc_coordinates_widget.h"

#include "lc_convert.h"
#include "lc_formatter.h"
#include "lc_graphicviewport.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_settings.h"
#include "rs_units.h"
#include "ui_lc_coordinates_widget.h"

LC_CoordinatesWidget::LC_CoordinatesWidget(QWidget* parent,
                                           const char* name,
                                           const Qt::WindowFlags fl)
    : LC_CoordinateWidgetBase(parent, fl)
    , m_ui(new Ui::LC_CoordinatesWidget) {
    setObjectName(name);
    m_ui->setupUi(this);

    m_ui->lCoord1->setText(QString());
    m_ui->lCoord2->setText(QString());
    m_ui->lCoord1b->setText(QString());
    m_ui->lCoord2b->setText(QString());

    m_graphic = nullptr;

    initLockControl(m_ui->tbLock);
    registerCoordinateField(m_ui->lCoord1, tr("Absolute Cartesian"), [this]() {
        return m_absoluteCoordinates;
    });
    registerCoordinateField(m_ui->lCoord2, tr("Relative Cartesian"), [this]() {
        return m_relativeCoordinates;
    });
    registerCoordinateField(m_ui->lCoord1b, tr("Absolute Polar"), [this]() {
        return m_absoluteCoordinates;
    });
    registerCoordinateField(m_ui->lCoord2b, tr("Relative Polar"), [this]() {
        return m_relativeCoordinates;
    });
}

LC_CoordinatesWidget::~LC_CoordinatesWidget() {
    delete m_ui;
}

void LC_CoordinatesWidget::languageChange() {
    if (m_ui != nullptr) {
        m_ui->retranslateUi(this);
    }
}

void LC_CoordinatesWidget::setGraphicView(RS_GraphicView* gv) {
    m_graphicView = gv;
    if (gv != nullptr) {
        m_viewport = gv->getViewPort();
        m_graphic = gv->getGraphic(true);
        m_formatter = (m_viewport != nullptr) ? m_viewport->getFormatter() : nullptr;
        if (!m_isLocked) {
            setCoordinates(0.0, 0.0, 0.0, 0.0, true);
        }
    }
    else {
        m_viewport = nullptr;
        m_graphic = nullptr;
        m_formatter = nullptr;
    }
}

void LC_CoordinatesWidget::setCoordinates(const RS_Vector& wcsAbs, const RS_Vector& wcsDelta, const bool updateFormat) {
    if (m_isLocked) {
        return;
    }

    double ucsX = 0.0;
    double ucsY = 0.0;
    double ucsDeltaX = 0.0;
    double ucsDeltaY = 0.0;

    if (m_viewport != nullptr) {
        m_viewport->toUCS(wcsAbs, ucsX, ucsY);
        m_viewport->toUCSDelta(wcsDelta, ucsDeltaX, ucsDeltaY);
    }
    else {
        ucsX = wcsAbs.x;
        ucsY = wcsAbs.y;
        ucsDeltaX = wcsDelta.x;
        ucsDeltaY = wcsDelta.y;
    }
    setCoordinates(ucsX, ucsY, ucsDeltaX, ucsDeltaY, updateFormat);
}

void LC_CoordinatesWidget::clearContent() const {
    if (m_isLocked || m_ui == nullptr) {
        return;
    }

    m_ui->lCoord1->setText(QStringLiteral("0 , 0"));
    m_ui->lCoord2->setText(QStringLiteral("@  0 , 0"));
    m_ui->lCoord1b->setText(QStringLiteral("0 < 0"));
    m_ui->lCoord2b->setText(QStringLiteral("@  0 < 0"));
}

void LC_CoordinatesWidget::setCoordinates(double ucsX, double ucsY,
                                         double ucsDeltaX, double ucsDeltaY, const bool updateFormat) {
    if (m_isLocked || m_ui == nullptr) {
        return;
    }

    if (m_graphic != nullptr) {
        if (m_viewport != nullptr) {
            if (updateFormat) {
                m_formatter = m_viewport->getFormatter();
            }

            if (!CFG_Appearance::o_UnitlessGrid) {
                ucsX = RS_Units::convert(ucsX);
                ucsY = RS_Units::convert(ucsY);
                ucsDeltaX = RS_Units::convert(ucsDeltaX);
                ucsDeltaY = RS_Units::convert(ucsDeltaY);
            }

            if (m_formatter != nullptr) {
                // Cartesian coordinates
                const QString absX = m_formatter->formatLinear(ucsX);
                const QString absY = m_formatter->formatLinear(ucsY);
                const QString relX = m_formatter->formatLinear(ucsDeltaX);
                const QString relY = m_formatter->formatLinear(ucsDeltaY);

                m_ui->lCoord1->setText(absX + QStringLiteral(" , ") + absY);
                m_ui->lCoord2->setText(QStringLiteral("@  ") + relX + QStringLiteral(" , ") + relY);

                // Absolute polar coordinates
                const auto polarCoordinate = RS_Vector(ucsX, ucsY);
                const QString polarMagnitude = m_formatter->formatLinear(polarCoordinate.magnitude());
                const double ucsAngle = polarCoordinate.angle();
                const double basicUcsAngle = m_viewport->toBasisUCSAngle(ucsAngle);
                QString angleStr = m_formatter->formatRawAngle(basicUcsAngle);

                m_ui->lCoord1b->setText(polarMagnitude + QStringLiteral(" < ") + angleStr);

                // Relative polar coordinates
                const auto polarRelativeCoordinate = RS_Vector(ucsDeltaX, ucsDeltaY);
                const QString relPolarMagnitude = m_formatter->formatLinear(polarRelativeCoordinate.magnitude());
                const double relUcsAngle = polarRelativeCoordinate.angle();
                const double basicRelUcsAngle = m_viewport->toBasisUCSAngle(relUcsAngle);
                angleStr = m_formatter->formatRawAngle(basicRelUcsAngle);

                m_ui->lCoord2b->setText(QStringLiteral("@  ") + relPolarMagnitude + QStringLiteral(" < ") + angleStr);
            }

            m_absoluteCoordinates = RS_Vector(ucsX, ucsY, 0.0);
            m_relativeCoordinates = RS_Vector(ucsDeltaX, ucsDeltaY, 0.0);
        }
        else {
            clearContent();
        }
    }
}
