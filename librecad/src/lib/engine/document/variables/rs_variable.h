/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/

#ifndef RS_VARIABLE_H
#define RS_VARIABLE_H

#include <QString>

#include "rs.h"
#include "rs_vector.h"

/**
 * A variable of type int, double, string or vector.
 * The variable also contains its type and an int code
 * which can identify its contents in any way.
 *
 * @author Andrew Mustun
 */
class RS_Variable {
    struct RS_VariableContents {
        QString s;
        int i = 0;
        double d = 0.;
        RS_Vector v{false};
    };

public:
    RS_Variable() = default;

    RS_Variable(const RS_Vector& v, const int c) : m_code{c} {
        setVector(v);
    }

    RS_Variable(const QString& v, const int c) : m_code{c} {
        setString(v);
    }

    RS_Variable(const int v, const int c) : m_code{c} {
        setInt(v);
    }

    RS_Variable(const double v, const int c) : m_code{c} {
        setDouble(v);
    }

    void setString(const QString& str) {
        m_contents.s = str;
        m_type = RS2::VariableString;
    }

    void setInt(const int i) {
        m_contents.i = i;
        m_type = RS2::VariableInt;
    }

    void setDouble(const double d) {
        m_contents.d = d;
        m_type = RS2::VariableDouble;
    }

    void setVector(const RS_Vector& v) {
        m_contents.v = v;
        m_type = RS2::VariableVector;
    }

    QString getString() const {
        return m_contents.s;
    }

    int getInt() const {
        return m_contents.i;
    }

    double getDouble() const {
        return m_contents.d;
    }

    RS_Vector getVector() const {
        return m_contents.v;
    }

    RS2::VariableType getType() const {
        return m_type;
    }

    int getCode() const {
        return m_code;
    }

    QString toString();

    //friend std::ostream& operator << (std::ostream& os, RS_Variable& v);

private:
    RS_VariableContents m_contents;
    RS2::VariableType m_type = RS2::VariableVoid;
    int m_code = 0;
};

inline QString RS_Variable::toString() {
    switch (m_type) {
        case RS2::VariableString:
            return m_contents.s;
        case RS2::VariableInt:
            return QString::number(m_contents.i);
        case RS2::VariableDouble:
            return QString::number(m_contents.d);
        case RS2::VariableVector:
            return QString::number(m_contents.v.x) + " " + QString::number(m_contents.v.y);
        default:
            return "";
    }
}

#endif
