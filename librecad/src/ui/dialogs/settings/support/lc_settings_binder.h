
/*******************************************************************************
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

#ifndef LC_SETTINGS_BINDER_H
#define LC_SETTINGS_BINDER_H

#include <functional>
#include <memory>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QList>
#include <QPointer>
#include <QSpinBox>

#include "lc_color_button.h"
#include "lc_settings_backend.h"
#include "lc_settings_page_interface.h"


class LC_SettingBinding {
public:
    virtual ~LC_SettingBinding() = default;
    virtual void load(const LC_SettingsBackend* backend) = 0;
    virtual void save(LC_SettingsBackend* backend, bool& outRestartRequired, bool commitBaseline) = 0;
    virtual void resetToDefault() = 0;
    virtual bool isModified() const = 0;
};

template <typename WidgetType, typename ValueType>
class LC_SettingBindingImpl : public LC_SettingBinding {
public:
    using Getter = std::function<ValueType(WidgetType*)>;
    using Setter = std::function<void(WidgetType*, ValueType)>;

    LC_SettingBindingImpl(WidgetType* widget, const QString& key, const ValueType& defaultValue,
                          bool reqRestart, Getter g, Setter s)
        : m_widget(widget), m_key(key), m_defaultValue(defaultValue)
        , m_requiresRestart(reqRestart), m_getter(g), m_setter(s) {}

    void load(const LC_SettingsBackend* backend) override {
        if (!m_widget || backend == nullptr) {
            return;
        }
        m_cleanValue = backend->value(m_key, QVariant::fromValue(m_defaultValue)).template value<ValueType>();
        m_setter(m_widget, m_cleanValue);
    }

    void save(LC_SettingsBackend* backend, bool& outRestartRequired, bool commitBaseline) override {
        if (!m_widget || backend == nullptr) {
            return;
        }
        ValueType currentVal = m_getter(m_widget);
        if (m_requiresRestart && currentVal != m_cleanValue) {
            outRestartRequired = true;
        }
        backend->setValue(m_key, QVariant::fromValue(currentVal));

        // only commit baseline if explicitly requested on OK/Apply
        if (commitBaseline) {
            m_cleanValue = currentVal;
        }
    }

    void resetToDefault() override {
        if (!m_widget) {
            return;
        }
        m_setter(m_widget, m_defaultValue);
    }

    bool isModified() const override {
        if (!m_widget) {
            return false;
        }
        return m_getter(m_widget) != m_cleanValue;
    }

private:
    QPointer<WidgetType> m_widget;
    QString m_key;
    ValueType m_defaultValue;
    ValueType m_cleanValue;
    bool m_requiresRestart;
    Getter m_getter;
    Setter m_setter;
};

class LC_ColorBinding : public LC_SettingBinding {
public:
    LC_ColorBinding(QComboBox* comboBox, LC_ColorButton* button, const QString& key,
                    const QColor& defaultValue, bool reqRestart)
        : m_comboBox(comboBox), m_button(button), m_key(key)
        , m_defaultValue(defaultValue), m_requiresRestart(reqRestart) {}

    void load(const LC_SettingsBackend* backend) override;
    void save(LC_SettingsBackend* backend, bool& outRestartRequired, bool commitBaseline) override;
    void resetToDefault() override;
    bool isModified() const override;
private:
    QPointer<QComboBox> m_comboBox;
    QPointer<LC_ColorButton> m_button;
    QString m_key;
    QColor m_defaultValue;
    QString m_cleanValue;
    bool m_requiresRestart;
};

class LC_SettingsBinder: public QObject {
    Q_OBJECT
public:
    LC_SettingsBinder(LC_SettingsBackend* backend, LC_SettingsPageInterface* page);

    // Disabled copying to support unique_ptrs safely
    LC_SettingsBinder(const LC_SettingsBinder&) = delete;
    LC_SettingsBinder& operator=(const LC_SettingsBinder&) = delete;

    void bind(QCheckBox* checkbox, const QString& key, bool defaultValue, bool reqRestart = false);
    void bind(QSpinBox* spinBox, const QString& key, int defaultValue, bool reqRestart = false);
    void bind(QDoubleSpinBox* spinBox, const QString& key, double defaultValue, bool reqRestart = false);
    void bind(QLineEdit* lineEdit, const QString& key, const QString& defaultValue, bool reqRestart = false);

    void bindIndex(QComboBox* comboBox, const QString& key, int defaultIndex, bool reqRestart = false);
    void bindText(QComboBox* comboBox, const QString& key, const QString& defaultText, bool reqRestart = false);
    void bindColor(QComboBox* comboBox, LC_ColorButton* button, const QString& key, const QColor& defaultValue, bool reqRestart = false);


   template <typename WidgetType, typename ValueType, typename SignalSignature>
   void bindCustom(WidgetType* widget, const QString& key, const ValueType& defaultValue, bool reqRestart,
                   SignalSignature signal,
                   std::function<ValueType(WidgetType*)> getter,
                   std::function<void(WidgetType*, ValueType)> setter) {

        bindCustom<WidgetType, ValueType>(widget, key, defaultValue, reqRestart, getter, setter);
        if (widget) {
            connect(widget, signal, this, [this]() {
               notifyChanged();
            });
        }
    }

    template <typename WidgetType, typename ValueType>
    void bindCustom(WidgetType* widget, const QString& key, const ValueType& defaultValue, bool reqRestart,
                    std::function<ValueType(WidgetType*)> getter, std::function<void(WidgetType*, ValueType)> setter) {
        m_bindings.push_back(std::make_unique<LC_SettingBindingImpl<WidgetType, ValueType>>(
            widget, key, defaultValue, reqRestart, getter, setter));
    }

    void loadAll();
    void saveAll(bool commitBaseline) const;
    void resetAllDefaults();
    bool isAnyModified() const;
private:
    LC_SettingsBackend* m_backend;
    LC_SettingsPageInterface* m_page;
    std::vector<std::unique_ptr<LC_SettingBinding>> m_bindings;
    bool m_blockSignals = false;

    void notifyChanged() const {
        if (!m_blockSignals) {
            m_page->updateLivePreview();
        }
    }
};

#endif
