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

#include "lc_propertymatchertypes.h"

#include "rs_color.h"
#include "rs_math.h"

namespace {
    LC_TypedPropertyMatchTypeDescriptor<RS_Color> initColorType(LC_PropertyMatchTypeEnum type)  {
        LC_TypedPropertyMatchTypeDescriptor<RS_Color> result(type);
        result.hasAllExceptPattern();

        result.m_equals = [](const RS_Color &one, const RS_Color &second,[[maybe_unused]]  const RS_Color &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const RS_Color &one, const RS_Color &second,[[maybe_unused]]  const RS_Color &tolerance)-> bool {
            return one != second;
        };
        result.m_greater = [](const RS_Color &one, const RS_Color &second,[[maybe_unused]]  const RS_Color &tolerance)-> bool {
            return one.toIntColor() > second.toIntColor();
        };
        result.m_less = [](const RS_Color &one, const RS_Color &second,[[maybe_unused]]  const RS_Color &tolerance)-> bool {
            return one.toIntColor() < second.toIntColor();
        };
        result.m_any = []([[maybe_unused]]RS_Color &one,[[maybe_unused]]  const RS_Color &second, [[maybe_unused]]  const RS_Color &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<RS_Layer*> initLayerType() {
        LC_TypedPropertyMatchTypeDescriptor<RS_Layer*> result(ENTITY_PROPERTY_LAYER);
        result.hasBasic();

        result.m_equals = [](RS_Layer* &one,  RS_Layer* const &second,[[maybe_unused]] RS_Layer* const &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](RS_Layer* &one,   RS_Layer* const &second,[[maybe_unused]] RS_Layer* const  &tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]RS_Layer* &one,[[maybe_unused]]  RS_Layer* const &second,[[maybe_unused]]  RS_Layer* const  &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> initLineWidthType(LC_PropertyMatchTypeEnum type) {
        LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> result(type);
        result.hasBasic();

        result.m_equals = [](const RS2::LineWidth &one, const RS2::LineWidth &second,[[maybe_unused]]  const RS2::LineWidth &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const RS2::LineWidth &one, const RS2::LineWidth &second,[[maybe_unused]]  const RS2::LineWidth &tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]RS2::LineWidth &one, [[maybe_unused]] const RS2::LineWidth &second,[[maybe_unused]]  const RS2::LineWidth &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> initLineType(LC_PropertyMatchTypeEnum type) {
        LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> result(type);
        result.hasBasic();

        result.m_equals = [](const RS2::LineType &one, const RS2::LineType &second,[[maybe_unused]]  const RS2::LineType &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const RS2::LineType &one, const RS2::LineType &second,[[maybe_unused]]  const RS2::LineType &tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]RS2::LineType &one, [[maybe_unused]] const RS2::LineType &second,[[maybe_unused]]  const RS2::LineType &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<double> initAngleType() {
        LC_TypedPropertyMatchTypeDescriptor<double> result(ENTITY_PROPERTY_ANGLE);

        result.hasAllExceptPattern();

        result.m_equals = [](const double &one, const double &second, const double &tolerance)-> bool {
            double a1 = RS_Math::correctAngle(one);
            double a2 = RS_Math::correctAngle(second);
            bool result = RS_Math::equal(a1, a2, tolerance);
            return result;
        };
        result.m_notEquals = [](const double &one, const double &second, const double &tolerance)-> bool {
            double a1 = RS_Math::correctAngle(one);
            double a2 = RS_Math::correctAngle(second);
            bool result = !RS_Math::equal(a1, a2, tolerance);
            return result;
        };
        result.m_greater = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return RS_Math::correctAngle(one) > RS_Math::correctAngle(second); // fixme - avoid normalization of second angle as it will be for value to match?
        };
        result.m_less = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return RS_Math::correctAngle(one) < RS_Math::correctAngle(second); // fixme - avoid normalization of second angle as it will be for value to match?
        };
        result.m_any = []([[maybe_unused]]double &one, [[maybe_unused]] const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<double> initInclinationType() {
        LC_TypedPropertyMatchTypeDescriptor<double> result(ENTITY_PROPERTY_ANGLE);

        result.hasAllExceptPattern();

        result.m_equals = [](const double &one, const double &second,const double &tolerance)-> bool {
            bool result = RS_Math::isSameInclineAngles(one, second, tolerance);
            return result;
        };
        result.m_notEquals = [](const double &one, const double &second, const double &tolerance)-> bool {
            return !RS_Math::isSameInclineAngles(one, second, tolerance);
        };
        result.m_greater = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return RS_Math::correctAngle(one) > RS_Math::correctAngle(second); // fixme - avoid normalization of second angle as it will be for value to match?
        };
        result.m_less = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return RS_Math::correctAngle(one) < RS_Math::correctAngle(second); // fixme - avoid normalization of second angle as it will be for value to match?
        };
        result.m_any = []([[maybe_unused]]double &one, [[maybe_unused]] const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<double> initCoordType(LC_PropertyMatchTypeEnum type) {
        LC_TypedPropertyMatchTypeDescriptor<double> result(type);
        result.hasAllExceptPattern();

        result.m_equals = [](const double &one, const double &second, const double &tolerance)-> bool {
            return RS_Math::equal(one, second, tolerance);
        };
        result.m_notEquals = [](const double &one, const double &second, const double &tolerance)-> bool {
            return !RS_Math::equal(one, second, tolerance);
        };
        result.m_greater = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return one > second;
        };
        result.m_less = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return one < second;
        };
        result.m_any = []([[maybe_unused]]double &one, [[maybe_unused]] const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return true;
        };

        return result;
    }

    template <typename ListContainerType>
    LC_ComparingPropertyMatchTypeDescriptor<ListContainerType, double> initCoordContainsType(LC_PropertyMatchTypeEnum type) {
        LC_ComparingPropertyMatchTypeDescriptor<ListContainerType, double> result(type);
        result.hasAllExceptPattern();

        result.m_equals = [](const ListContainerType &one, const double &second, const double &tolerance)-> bool {
            for (const auto d: one) {
                if (RS_Math::equal(d, second, tolerance)) {
                    return true;
                }
            }
            return false;
        };
        result.m_notEquals = [](const ListContainerType &one, const double &second, const double &tolerance)-> bool {
            for (const auto d: one) {
                if (RS_Math::equal(d, second, tolerance)) {
                    return false;
                }
            }
            return true;
        };
        result.m_greater = [](const ListContainerType &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            for (const auto d: one) {
                if (d > second) {
                    return true;
                }
            }
            return false;
        };
        result.m_less = [](const ListContainerType &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            for (const auto d: one) {
                if (d < second) {
                    return true;
                }
            }
            return false;
        };
        result.m_any = []([[maybe_unused]]ListContainerType &one, [[maybe_unused]] const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return true;
        };

        return result;
    }


    LC_TypedPropertyMatchTypeDescriptor<double> initDoubleType(LC_PropertyMatchTypeEnum type) {
        LC_TypedPropertyMatchTypeDescriptor<double> result(type);
        result.hasAllExceptPattern();

        result.m_equals = [](const double &one, const double &second,const double &tolerance)-> bool {
            return RS_Math::equal(one, second, tolerance);
        };
        result.m_notEquals = [](const double &one, const double &second, const double &tolerance)-> bool {
            return !RS_Math::equal(one, second, tolerance);
        };
        result.m_greater = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return one > second;
        };
        result.m_less = [](const double &one, const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return one < second;
        };
        result.m_any = []([[maybe_unused]]double &one, [[maybe_unused]] const double &second,[[maybe_unused]]  const double &tolerance)-> bool {
            return true;
        };

        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<int> initIntType() {
        LC_TypedPropertyMatchTypeDescriptor<int> result(ENTITY_PROPERTY_INT);
        result.hasAllExceptPattern();

        result.m_equals = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one != second;
        };
        result.m_greater = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one > second;
        };
        result.m_less = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one < second;
        };
        result.m_any = []([[maybe_unused]]int &one, [[maybe_unused]] const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return true;
        };
        return result;
    }


    LC_TypedPropertyMatchTypeDescriptor<int> initIntChoiceType() {
        LC_TypedPropertyMatchTypeDescriptor<int> result(ENTITY_PROPERTY_INT_CHOICE);
        result.hasBasic();
        result.m_equals = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const int &one, const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]int &one, [[maybe_unused]] const int &second,[[maybe_unused]]  const int &tolerance)-> bool {
            return true;
        };
        return result;
    }

   LC_TypedPropertyMatchTypeDescriptor<bool> initBoolType() {
        LC_TypedPropertyMatchTypeDescriptor<bool> result(ENTITY_PROPERTY_BOOL);
        result.hasBasic();

        result.m_equals = [](const bool &one, const bool &second,[[maybe_unused]]  const bool &tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](const bool &one, const bool &second,[[maybe_unused]]  const bool &tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]bool &one, [[maybe_unused]] const bool &second,[[maybe_unused]]  const bool &tolerance)-> bool {
            return true;
        };
        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<QString> initStringType() {
        LC_TypedPropertyMatchTypeDescriptor<QString> result(ENTITY_PROPERTY_STRING);
        result.hasBasic();

        //fixme - add support wildcards

        result.m_equals = [](const QString &one, const QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return one.compare(second, Qt::CaseInsensitive) == 0;
        };
        result.m_notEquals = [](const QString &one, const QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return one.compare(second, Qt::CaseInsensitive) != 0;
        };
        result.m_any = []([[maybe_unused]]QString &one,[[maybe_unused]] const  QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return true;
        };

        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<QString> initStringChoiceType() {
        LC_TypedPropertyMatchTypeDescriptor<QString> result(ENTITY_PROPERTY_STRING_CHOICE);
        result.hasBasic();

        result.m_equals = [](const QString &one, const QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return one.compare(second, Qt::CaseInsensitive) == 0;
        };
        result.m_notEquals = [](const QString &one, const QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return one.compare(second, Qt::CaseInsensitive) != 0;
        };
        result.m_any = []([[maybe_unused]]QString &one,[[maybe_unused]] const QString &second,[[maybe_unused]]  const QString &tolerance)-> bool {
            return true;
        };

        return result;
    }

    LC_TypedPropertyMatchTypeDescriptor<LC_DimStyle*> initDimStyleType() {
        LC_TypedPropertyMatchTypeDescriptor<LC_DimStyle*> result(ENTITY_PROPERTY_DIM_STYLE);
        result.hasBasic();

        result.m_equals = [](LC_DimStyle* one, LC_DimStyle* second,[[maybe_unused]] LC_DimStyle* tolerance)-> bool {
            return one == second;
        };
        result.m_notEquals = [](LC_DimStyle* one, LC_DimStyle* second,[[maybe_unused]]  LC_DimStyle* tolerance)-> bool {
            return one != second;
        };
        result.m_any = []([[maybe_unused]]LC_DimStyle* one,[[maybe_unused]]  const LC_DimStyle* second,[[maybe_unused]]  LC_DimStyle* tolerance)-> bool {
            return true;
        };

        return result;
    }
}



const LC_TypedPropertyMatchTypeDescriptor<RS_Color> LC_PropertyMatcherTypes::COLOR = initColorType(ENTITY_PROPERTY_COLOR);
const LC_TypedPropertyMatchTypeDescriptor<RS_Color> LC_PropertyMatcherTypes::COLOR_RESOLVED = initColorType(ENTITY_PROPERTY_COLOR_RESOLVED);
const LC_TypedPropertyMatchTypeDescriptor<RS_Layer*> LC_PropertyMatcherTypes::LAYER = initLayerType();
const LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> LC_PropertyMatcherTypes::LINE_WIDTH = initLineWidthType(ENTITY_PROPERTY_LINEWIDTH);
const LC_TypedPropertyMatchTypeDescriptor<RS2::LineWidth> LC_PropertyMatcherTypes::LINE_WIDTH_RESOLVED = initLineWidthType(ENTITY_PROPERTY_LINEWIDTH_RESOLVED);
const LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> LC_PropertyMatcherTypes::LINE_TYPE = initLineType(ENTITY_PROPERTY_LINETYPE);
const LC_TypedPropertyMatchTypeDescriptor<RS2::LineType> LC_PropertyMatcherTypes::LINE_TYPE_RESOLVED = initLineType(ENTITY_PROPERTY_LINETYPE_RESOLVED);
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::COORD_X = initCoordType(ENTITY_PROPERTY_COORD_X);
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::COORD_Y = initCoordType(ENTITY_PROPERTY_COORD_Y);
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::ANGLE = initAngleType();
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::INCLINATION = initInclinationType();
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::DOUBLE = initDoubleType(ENTITY_PROPERTY_DOUBLE);
const LC_TypedPropertyMatchTypeDescriptor<double> LC_PropertyMatcherTypes::LENGTH = initDoubleType(ENTITY_PROPERTY_LENGTH);
const LC_TypedPropertyMatchTypeDescriptor<int> LC_PropertyMatcherTypes::INT = initIntType();
const LC_TypedPropertyMatchTypeDescriptor<int> LC_PropertyMatcherTypes::INT_CHOICE = initIntChoiceType();
const LC_TypedPropertyMatchTypeDescriptor<bool> LC_PropertyMatcherTypes::BOOL = initBoolType();
const LC_TypedPropertyMatchTypeDescriptor<QString> LC_PropertyMatcherTypes::STRING = initStringType();
const LC_TypedPropertyMatchTypeDescriptor<QString> LC_PropertyMatcherTypes::STRING_CHOICE = initStringChoiceType();
const LC_TypedPropertyMatchTypeDescriptor<LC_DimStyle*> LC_PropertyMatcherTypes::DIM_STYLE = initDimStyleType();
const LC_ComparingPropertyMatchTypeDescriptor<QList<double>, double> LC_PropertyMatcherTypes::COORD_X_IN_QLIST = initCoordContainsType<QList<double>>(ENTITY_PROPERTY_COORD_X);
const LC_ComparingPropertyMatchTypeDescriptor<QList<double>, double> LC_PropertyMatcherTypes::COORD_Y_IN_QLIST = initCoordContainsType<QList<double>>(ENTITY_PROPERTY_COORD_Y);
