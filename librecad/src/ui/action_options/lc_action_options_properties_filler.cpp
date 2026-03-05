/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_action_options_properties_filler.h"

#include "lc_enum_value_descriptor.h"
#include "lc_property_action.h"
#include "lc_property_action_link_view.h"
#include "lc_property_container.h"
#include "lc_property_enum.h"
#include "lc_property_int.h"
#include "lc_property_int_spinbox_view.h"
#include "lc_property_qstring.h"
#include "lc_property_qstring_lineedit_view.h"
#include "lc_propertysheetwidget.h"
#include "rs_actioninterface.h"

class LC_EnumDescriptor;

void LC_ActionOptionsPropertiesFiller::preSetupByAction(RS_ActionInterface* a) {
    m_actionContext = a->getActionContext();
}

void LC_ActionOptionsPropertiesFiller::hideOptions() {
    // saveSettings();
}

void LC_ActionOptionsPropertiesFiller::doSetAction(RS_ActionInterface* a) {
    m_action = a;
}

void LC_ActionOptionsPropertiesFiller::addBoolean(const LC_Property::Names& names,
                                                  typename LC_PropertyValueDelegated<bool>::FunValueGet funGet,
                                                  typename LC_PropertyValueDelegated<bool>::FunValueSetShort funSet,
                                                  LC_PropertyContainer* cont, const QString& viewName,
                                                  std::function<bool(LC_PropertyViewDescriptor& descriptor)> funPrepareDescriptor) {
    auto property = new LC_PropertyBool(cont, false);
    property->setNames(names);
    LC_PropertyViewDescriptor descriptor(viewName.toLatin1());
    bool readOnly = false;
    if (funPrepareDescriptor != nullptr) {
        readOnly = funPrepareDescriptor(descriptor);
    }
    property->setViewDescriptor(descriptor);
    createDelegatedStorage<bool>(property, funGet, funSet);
    if (funSet == nullptr || readOnly) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addDouble(const LC_Property::Names& names,
                                                 typename LC_PropertyValueDelegated<double>::FunValueGet funGet,
                                                 typename LC_PropertyValueDelegated<double>::FunValueSetShort funSet,
                                                 LC_PropertyContainer* cont,
                                                 std::function<bool(LC_PropertyViewDescriptor&)> funFillViewAttrs) {
    auto property = createDoubleProperty(names, cont, LC_ActionContext::InteractiveInputInfo::InputType::NOTNEEDED, m_actionContext,
                                         m_widget);
    bool readonly = false;
    if (funFillViewAttrs != nullptr) {
        LC_PropertyViewDescriptor descriptor;
        readonly = funFillViewAttrs(descriptor);
        property->setViewDescriptor(descriptor);
    }

    createDelegatedStorage<double>(property, funGet, funSet);
    if (readonly || funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addLinearDistance(const LC_Property::Names& names,
                                                         typename LC_PropertyValueDelegated<double>::FunValueGet funGet,
                                                         typename LC_PropertyValueDelegated<double>::FunValueSetShort funSet,
                                                         LC_PropertyContainer* cont,
                                                         std::function<bool(LC_PropertyViewDescriptor*)> funFillViewAttrs) {
    auto property = createDoubleProperty(names, cont, LC_ActionContext::InteractiveInputInfo::InputType::DISTANCE, m_actionContext,
                                         m_widget);
    bool readonly = false;
    if (funFillViewAttrs != nullptr) {
        LC_PropertyViewDescriptor descriptor;
        readonly = funFillViewAttrs(&descriptor);
        property->setViewDescriptor(descriptor);
    }

    createDelegatedStorage<double>(property, funGet, funSet);
    if (readonly || funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addRawAngle(const LC_Property::Names& names,
                                                   typename LC_PropertyValueDelegated<double>::FunValueGet funGet,
                                                   typename LC_PropertyValueDelegated<double>::FunValueSetShort funSet,
                                                   LC_PropertyContainer* cont) {
    auto property = createDoubleProperty(names, cont, LC_ActionContext::InteractiveInputInfo::InputType::ANGLE, m_actionContext, m_widget);

    createDelegatedStorage<double>(property, funGet, funSet, [funGet](const double& v) -> bool {
        return LC_LineMath::isSameAngle(v, funGet());
    });
    if (funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addRawAngleDegrees(const LC_Property::Names& names,
                                                   typename LC_PropertyValueDelegated<double>::FunValueGet funGet,
                                                   typename LC_PropertyValueDelegated<double>::FunValueSetShort funSet,
                                                   LC_PropertyContainer* cont) {
    auto property = createDoubleProperty(names, cont, LC_ActionContext::InteractiveInputInfo::InputType::ANGLE, m_actionContext, m_widget);

    auto funGetValue = [funGet]()->double {
      return RS_Math::deg2rad(funGet());
    };

    auto funSetValue = funSet == nullptr ? funSet : [funSet](double v) ->void {
        funSet(RS_Math::rad2deg(v));
    };

    createDelegatedStorage<double>(property, funGetValue, funSetValue, [funGet](const double& v) -> bool {
        return LC_LineMath::isSameAngle(v, funGet());
    });
    if (funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addWCSAngle(const LC_Property::Names& names,
                                                   typename LC_PropertyValueDelegated<double>::FunValueGet funGet,
                                                   typename LC_PropertyValueDelegated<double>::FunValueSetShort funSet,
                                                   LC_PropertyContainer* cont) {
    auto property = createDoubleProperty(names, cont, LC_ActionContext::InteractiveInputInfo::InputType::ANGLE, m_actionContext, m_widget);

    auto funGetValue = [this, funGet]() -> double {
        const double wcsAngle = funGet();
        const double ucsAngle = toUCSBasisAngle(wcsAngle); // here we return in UCS for editing*/
        return ucsAngle;
    };

    auto funSetValue = (funSet != nullptr)
                           ? [this, funSet](const double& value) -> void {
                               // here we expect value in radians and in ucs
                               const double ucsBasisAngle = value;
                               double wcsAngle = toWCSAngle(ucsBasisAngle);
                               funSet(wcsAngle);
                           }
                           : funSet;

    auto funValueEqual = [funGet](const double& v) -> bool {
        return LC_LineMath::isSameAngle(v, funGet());
    };
    createDelegatedStorage<double>(property, funGetValue, funSetValue, funValueEqual);
    if (funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addEnum(const LC_Property::Names& names, const LC_EnumDescriptor* enumDescriptor,
                                               typename LC_PropertyValueDelegated<LC_PropertyEnumValueType>::FunValueGet funGetValue,
                                               typename LC_PropertyValueDelegated<LC_PropertyEnumValueType>::FunValueSetShort funSetValue,
                                               LC_PropertyContainer* container,
                                               std::function<bool(LC_PropertyViewDescriptor& descriptor)> funPrepareDescriptor) {
    auto property = new LC_PropertyEnum(container, false);
    property->setNames(names);
    property->setEnumInfo(enumDescriptor);

    createDelegatedStorage<LC_PropertyEnumValueType>(property, funGetValue, funSetValue);

    bool readonly = false;
    if (funPrepareDescriptor != nullptr) {
        LC_PropertyViewDescriptor descriptor;
        readonly = funPrepareDescriptor(descriptor);
        property->setViewDescriptor(descriptor);
    }
    if (readonly || funSetValue == nullptr) {
        property->setReadOnly();
    }
    container->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addVector(const LC_Property::Names& names,
                                                 typename LC_PropertyValueDelegated<RS_Vector>::FunValueGet funGet,
                                                 typename LC_PropertyValueDelegated<RS_Vector>::FunValueSetShort funSet,
                                                 LC_PropertyContainer* cont) {
    auto property = createVectorProperty(names, cont, m_actionContext, m_widget);


    auto funGetValue = [this, funGet]() -> RS_Vector {
        const RS_Vector wcsVector = funGet();
        const RS_Vector ucsVector = toUCS(wcsVector); // here we return in UCS for editing
        return ucsVector;
    };

    auto funSetValue = (funSet != nullptr)
                           ? [this, funSet](const RS_Vector& userUCS) -> void {
                               RS_Vector ucsVector = toWCS(userUCS);
                               funSet(ucsVector);
                           }
                           : funSet;

    auto funValueEqual = [this, funGet](const RS_Vector& userUCS) -> bool {
        auto originalWCS = funGet();
        auto originalUCS = toUCS(originalWCS);
        return userUCS == originalUCS;
    };

    createDelegatedStorage<RS_Vector>(property, funGetValue, funSetValue, funValueEqual);
    if (funSet == nullptr) {
        property->setReadOnly();
    }
    cont->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addIntSpinbox(const LC_Property::Names& names,
                                                     typename LC_PropertyValueDelegated<int>::FunValueGet funGet,
                                                     typename LC_PropertyValueDelegated<int>::FunValueSetShort funSet,
                                                     LC_PropertyContainer* container, int minVal, int maxVal) {
    auto* property = new LC_PropertyInt(container, false);
    property->setNames(names);

    LC_PropertyViewDescriptor descriptor(LC_PropertyIntSpinBoxView::VIEW_NAME);
    descriptor.attributes[LC_PropertyIntSpinBoxView::ATTR_MIN] = 1;
    descriptor.attributes[LC_PropertyIntSpinBoxView::ATTR_STEP] = minVal;
    if (maxVal > 0) {
        descriptor.attributes[LC_PropertyIntSpinBoxView::ATTR_MAX] = maxVal;
    }
    property->setViewDescriptor(descriptor);

    createDelegatedStorage<LC_PropertyEnumValueType>(property, funGet, funSet);

    if (funSet == nullptr) {
        property->setReadOnly();
    }
    container->addChildProperty(property);
}

void LC_ActionOptionsPropertiesFiller::addString(const LC_Property::Names& names,
                                                 typename LC_PropertyValueDelegated<QString>::FunValueGet funGet,
                                                 typename LC_PropertyValueDelegated<QString>::FunValueSetShort funSet,
                                                 LC_PropertyContainer* container, bool multiLine,
                                                 std::function<bool(LC_PropertyViewDescriptor&)> funPrepareDescriptor) {
    auto property = new LC_PropertyQString(container, false);
    property->setNames(names);
    LC_PropertyViewDescriptor viewDescriptor;
    viewDescriptor.viewName = LC_PropertyQStringLineEditView::VIEW_NAME;
    viewDescriptor.attributes[LC_PropertyQStringLineEditView::ATTR_MULTILINE_EDIT] = multiLine;
    property->setViewDescriptor(viewDescriptor);

    createDelegatedStorage<QString>(property, funGet, funSet);

    bool readonly = false;
    if (funPrepareDescriptor != nullptr) {
        readonly = funPrepareDescriptor(viewDescriptor);
    }
    if (readonly || funSet == nullptr) {
        property->setReadOnly();
    }
    container->addChildProperty(property);
}

   void LC_ActionOptionsPropertiesFiller::createCommandsLine(LC_PropertyContainer* container, const QString& propertyName, const QString& linkTitle,
                                                     const QString& linkTooltip, const QString& linkTitleRight,
                                                     const QString& linkTooltipRight,
                                                     const std::function<void(int linkIndex)> &clickHandler,
                                                     const QString &commonDescription,
                                                     bool leftEnabled, bool rightEnabled) {
    auto* property = new LC_PropertyAction(container, true);
    property->setName(propertyName);
    property->setDisplayName("");
    LC_PropertyViewDescriptor viewDescriptor("Link");
    viewDescriptor[LC_PropertyActionLinkView::ATTR_TITLE] = linkTitle;
    viewDescriptor[LC_PropertyActionLinkView::ATTR_TOOLTIP_LEFT] = linkTooltip;
    viewDescriptor[LC_PropertyActionLinkView::ATTR_ENABLED_LEFT] = leftEnabled;
    if (!linkTitleRight.isEmpty()) {
        viewDescriptor[LC_PropertyActionLinkView::ATTR_TITLE_RIGHT] = linkTitleRight;
        viewDescriptor[LC_PropertyActionLinkView::ATTR_TOOLTIP_RIGHT] = linkTooltipRight;
        viewDescriptor[LC_PropertyActionLinkView::ATTR_ENABLED_RIGHT] = rightEnabled;
    }

    property->setClickHandler([clickHandler](const LC_PropertyAction*, int linkIndex)  {
        QTimer::singleShot(10, [linkIndex, clickHandler] {
            clickHandler(linkIndex);
        });
    });
    property->setDescription(commonDescription);
    property->setViewDescriptor(viewDescriptor);
    container->addChildProperty(property);
}
