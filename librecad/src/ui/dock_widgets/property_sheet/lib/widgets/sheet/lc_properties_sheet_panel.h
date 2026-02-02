/*******************************************************************************
Copyright (c) 2012-2016 Alex Zhondin <lexxmark.dev@gmail.com>
Copyright (c) 2015-2019 Alexandra Cherdantseva <neluhus.vagus@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#ifndef LC_PROPERTIESSHEETWIDGET_H
#define LC_PROPERTIESSHEETWIDGET_H

#include <QSplitter>
#include <QWidget>

#include "lc_inplace_property_editing_stopper.h"

enum LC_PropertyWidgetArea {
    PROPERTY_WIDGET_AREA_NONE    = 0x0000,
    PROPERTY_WIDGET_AREA_INFO    = 0x0001,
    PROPERTY_WIDGET_AREA_OTHER = 0x0002
};

class LC_PropertiesSheet;
class LC_PropertyContainer;
class LC_Property;
class QVBoxLayout;
class QLabel;

Q_DECLARE_FLAGS(LC_PropertyWidgetAreas, LC_PropertyWidgetArea)
Q_DECLARE_OPERATORS_FOR_FLAGS (LC_PropertyWidgetAreas)

class LC_PropertiesSheetPanel : public QWidget, public LC_InplacePropertyEditingStopper {
    Q_OBJECT Q_DISABLE_COPY(LC_PropertiesSheetPanel)

public:
    explicit LC_PropertiesSheetPanel(QWidget* parent = nullptr);
    ~LC_PropertiesSheetPanel() override;
    inline LC_PropertyWidgetAreas parts() const;
    void setParts(LC_PropertyWidgetAreas newAreas);
    const LC_PropertyContainer* propertyContainer() const;
    LC_PropertyContainer* propertyContainer();
    void setPropertyContainer(LC_PropertyContainer* newPropertiesContainer);
    inline LC_PropertiesSheet* propertiesSheet() const;
    bool stopInplaceEdit(bool deleteLater = true, bool restoreParentFocus = true) override;
    void createDescriptionLabel(QSplitter* splitter);
    signals  :
    void propertySetChanged();

private:
    void updateParts();
    void setActiveProperty(const LC_Property* activeProperty) const;

    LC_PropertyWidgetAreas m_areas;

    QVBoxLayout* m_layout;
    QLabel* m_toolbar;
    LC_PropertiesSheet* m_propertiesSheet;
    QWidget* m_descriptionSplitter;
    QLabel* m_propertyInfoLabel;
};

LC_PropertyWidgetAreas LC_PropertiesSheetPanel::parts() const {
    return m_areas;
}

LC_PropertiesSheet* LC_PropertiesSheetPanel::propertiesSheet() const {
    return m_propertiesSheet;
}

#endif
