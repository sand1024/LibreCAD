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

#ifndef LC_ACTIONOPTIONSPROPERTIESFILLER_H
#define LC_ACTIONOPTIONSPROPERTIESFILLER_H

#include <QTimer>

#include "lc_action_options_support.h"
#include "lc_enum_descriptor.h"
#include "lc_property_bool_checkbox_view.h"
#include "lc_property_container_builder.h"
#include "lc_propertysheetwidget.h"
#include "lc_tool_options_properties_container_provider.h"
#include "qc_applicationwindow.h"
#include "rs_actioninterface.h"

class LC_ActionOptionsPropertiesFiller : public LC_ToolOptionsPropertiesContainerProvider, public LC_ActionOptionsSupport,
                                         public LC_PropertyContainerBuilder {
public:
    // fixme - more interface high-level, move impl-specific methods into base impl class
    [[deprecated]]
    LC_ActionOptionsPropertiesFiller()
        : LC_PropertyContainerBuilder(nullptr, QC_ApplicationWindow::getAppWindow()->getPropertySheetWidget()) {
    }

    LC_ActionOptionsPropertiesFiller(LC_ActionContext* actionContext, LC_PropertySheetWidget* widget)
        : LC_PropertyContainerBuilder(actionContext, widget) {
    }

    ~LC_ActionOptionsPropertiesFiller() override = default;
    void hideOptions() override;
protected:

    template <class TValue>
    void createDelegatedStorage(LC_PropertySingle<TValue>* property, const typename LC_PropertyValueDelegated<TValue>::FunValueGet& funGet,
                                const typename LC_PropertyValueDelegated<TValue>::FunValueSetShort& funSet,
                                const typename LC_PropertyValueDelegated<TValue>::FunValueEqual& funEqual = nullptr) const;

    void addBoolean(const LC_Property::Names& names, typename LC_PropertyValueDelegated<bool>::FunValueGet funGet,
                    typename LC_PropertyValueDelegated<bool>::FunValueSetShort funSet, LC_PropertyContainer* cont,
                    const QString& viewName = LC_PropertyBoolCheckBoxView::VIEW_NAME,
                    std::function<bool(LC_PropertyViewDescriptor& descriptor)> funPrepareDescriptor = nullptr);

    void addDouble(const LC_Property::Names& names, LC_PropertyValueDelegated<double>::FunValueGet funGet,
                   LC_PropertyValueDelegated<double>::FunValueSetShort funSet, LC_PropertyContainer* cont,
                   std::function<bool(LC_PropertyViewDescriptor&)> funFillViewAttrs = nullptr);

    void addLinearDistance(const LC_Property::Names& names, LC_PropertyValueDelegated<double>::FunValueGet funGet,
                           LC_PropertyValueDelegated<double>::FunValueSetShort funSet, LC_PropertyContainer* cont,
                           std::function<bool(LC_PropertyViewDescriptor*)> funFillViewAttrs = nullptr);
    void addRawAngle(const LC_Property::Names& names, LC_PropertyValueDelegated<double>::FunValueGet funGet,
                     LC_PropertyValueDelegated<double>::FunValueSetShort funSet, LC_PropertyContainer* cont);
    void addRawAngleDegrees(const LC_Property::Names& names, LC_PropertyValueDelegated<double>::FunValueGet funGet,
                            LC_PropertyValueDelegated<double>::FunValueSetShort funSet, LC_PropertyContainer* cont);
    void addWCSAngle(const LC_Property::Names& names, LC_PropertyValueDelegated<double>::FunValueGet funGet,
                     LC_PropertyValueDelegated<double>::FunValueSetShort funSet, LC_PropertyContainer* cont);
    void addEnum(const LC_Property::Names& names, const LC_EnumDescriptor* enumDescriptor,
                 LC_PropertyValueDelegated<LC_PropertyEnumValueType>::FunValueGet funGetValue,
                 LC_PropertyValueDelegated<LC_PropertyEnumValueType>::FunValueSetShort funSetValue, LC_PropertyContainer* container,
                 std::function<bool(LC_PropertyViewDescriptor& descriptor)> funPrepareDescriptor = nullptr);
    void addVector(const LC_Property::Names& names, LC_PropertyValueDelegated<RS_Vector>::FunValueGet funGet,
                   LC_PropertyValueDelegated<RS_Vector>::FunValueSetShort funSet, LC_PropertyContainer* cont);
    void addIntSpinbox(const LC_Property::Names& names, LC_PropertyValueDelegated<int>::FunValueGet funGet,
                       LC_PropertyValueDelegated<int>::FunValueSetShort funSet, LC_PropertyContainer* container, int minVal = 1,
                       int maxVal = -1);
    void addString(const LC_Property::Names& names, LC_PropertyValueDelegated<QString>::FunValueGet funGet,
                   LC_PropertyValueDelegated<QString>::FunValueSetShort funSet, LC_PropertyContainer* container, bool multiLine = false,
                   std::function<bool(LC_PropertyViewDescriptor&)> funPrepareDescriptor = nullptr);
    void createCommandsLine(LC_PropertyContainer* container, const QString& propertyName, const QString& linkTitle,
                            const QString& linkTooltip, const QString& linkTitleRight, const QString& linkTooltipRight,
                            const std::function<void(int linkIndex)>& clickHandler, const QString& commonDescription,
                            bool leftEnabled = true, bool rightEnabled = true);

    RS_ActionInterface* m_action {nullptr};
    void doSetAction(RS_ActionInterface* a) override;
    void preSetupByAction(RS_ActionInterface* a) override;
    RS_ActionInterface* getAction() {return m_action;}

    void cleanup() override {
    };

    bool checkActionRttiValid(RS2::ActionType actionType) override {
        return true;
    } // fixme - temporary stub
};

template <typename ValueType>
class LC_PropertyValueDelegatedWithReload : public LC_PropertyValueDelegated<ValueType> {
public:
    explicit LC_PropertyValueDelegatedWithReload(RS_ActionInterface* action)
        : m_action(action) {
    }

protected:
    void doSetValue(ValueType newValue, LC_PropertyChangeReason reason) override {
        LC_PropertyValueDelegated<ValueType>::doSetValue(newValue, reason);
        QTimer::singleShot(10, [this] {
            m_action->updateOptions();
        });
    }

    RS_ActionInterface* m_action{nullptr};
};

template <typename TValue>
void LC_ActionOptionsPropertiesFiller::createDelegatedStorage(LC_PropertySingle<TValue>* property,
                                                              const typename LC_PropertyValueDelegated<TValue>::FunValueGet& funGet,
                                                              const typename LC_PropertyValueDelegated<TValue>::FunValueSetShort& funSet,
                                                              const typename LC_PropertyValueDelegated<TValue>::FunValueEqual& funEqual) const {
    auto valueStorage = new LC_PropertyValueDelegatedWithReload<TValue>(m_action);
    valueStorage->setup(funGet, funSet, funEqual);
    property->setValueStorage(valueStorage, true);
}

#endif
