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

#ifndef LC_SETTING_H
#define LC_SETTING_H

#include <QString>
#include <QVariant>
#include <QColor>
#include <type_traits>
#include "rs_settings.h"

class LC_SettingBase {
public:
    virtual ~LC_SettingBase() = default;
    virtual QString group() const = 0;
    virtual QString key() const = 0;
    virtual QString fullKey() const = 0;

    // Type-agnostic accessors for serialization engines [3.11]
    virtual QVariant getVariant() const = 0;
    virtual void setVariant(const QVariant& val) const = 0;
    virtual QVariant defaultVariant() const = 0;
};

class LC_SettingsGroupBase {
public:
    explicit LC_SettingsGroupBase(const QString& groupName)
        : m_groupName(groupName) {}
    virtual ~LC_SettingsGroupBase() = default;

    QString groupName() const { return m_groupName; }

private:
    QString m_groupName;
};

template <typename T>
class LC_Setting : public LC_SettingBase{
public:
    LC_Setting(const LC_SettingsGroupBase* group, const QString& key, const T& defaultValue)
        : m_group(group), m_key(key), m_defaultValue(defaultValue) {}

    ~LC_Setting() override = default;

    // Default copy-control definitions to ensure safe on-the-fly factory returns (RVO)
    LC_Setting(const LC_Setting&) = default;
    LC_Setting& operator=(const LC_Setting&) = default;
    LC_Setting(LC_Setting&&) noexcept = default;
    LC_Setting& operator=(LC_Setting&&) noexcept = default;

    // Implement base interface overrides
    QString group() const override { return m_group->groupName(); }
    QString key() const override { return m_key; }
    QString fullKey() const override { return m_group->groupName() + "/" + m_key; }

    QVariant getVariant() const override {
        if constexpr (std::is_same_v<T, QColor>) {
            return QVariant(get().name()); // Export cleanly as "#hex_name" string
        } else {
            return QVariant::fromValue(get());
        }
    }

    // Symmetrical Safe Importer
    void setVariant(const QVariant& val) const override {
        if constexpr (std::is_same_v<T, QColor>) {
            if (val.typeId() == QMetaType::QString) {
                // Safely translate JSON strings back to native QColors
                set(QColor::fromString(val.toString()));
            } else {
                set(val.value<QColor>());
            }
        } else {
            set(val.value<T>());
        }
    }

    QVariant defaultVariant() const override {
        if constexpr (std::is_same_v<T, QColor>) {
            return QVariant(m_defaultValue.name());
        } else {
            return QVariant::fromValue(m_defaultValue);
        }
    }

    T defaultValue() const { return m_defaultValue; }

    T get() const {
        return getFromBackend();
    }

    void set(const T& val) const {
        saveToBackend(val);
    }

    operator T() const {
        return get();
    }

    const LC_Setting<T>& operator=(const T& val) const {
        set(val);
        return *this;
    }

    // 2. Const Stream Operator: writes directly using '<<' safely [1.1, 3.11]
    const LC_Setting<T>& operator<<(const T& val) const {
        set(val);
        return *this;
    }

private:
    T getFromBackend() const {
        QString grp = m_group->groupName();

        if constexpr (std::is_enum_v<T>) {
            // Read as int and statically cast back to the targeted Enum type
            int val = RS_Settings::instance()->readIntSingle(grp, m_key, static_cast<int>(m_defaultValue));
            return static_cast<T>(val);
        } else if constexpr (std::is_same_v<T, QColor>) {
            // Read as QString and convert to QColor natively
            QString hex = RS_Settings::instance()->readStrSingle(grp, m_key, m_defaultValue.name());
            return QColor::fromString(hex);
        } else if constexpr (std::is_same_v<T, bool>) {
            return RS_Settings::instance()->readBoolSingle(grp, m_key, m_defaultValue);
        } else if constexpr (std::is_same_v<T, int>) {
            return RS_Settings::instance()->readIntSingle(grp, m_key, m_defaultValue);
        } else if constexpr (std::is_same_v<T, double>) {
            bool ok = false;
            QString str = RS_Settings::instance()->readStrSingle(grp, m_key, QString::number(m_defaultValue));
            double d = str.toDouble(&ok);
            return ok ? d : m_defaultValue;
        } else {
            return RS_Settings::instance()->readStrSingle(grp, m_key, m_defaultValue);
        }
    }

    void saveToBackend(const T& val) const {
        QString grp = m_group->groupName();

        if constexpr (std::is_enum_v<T>) {
            RS_Settings::instance()->writeEntrySingle(grp, m_key, QVariant(static_cast<int>(val)));
        } else if constexpr (std::is_same_v<T, QColor>) {
            RS_Settings::instance()->writeEntrySingle(grp, m_key, QVariant(val.name()));
        } else {
            RS_Settings::instance()->writeEntrySingle(grp, m_key, QVariant(val));
        }
    }

    const LC_SettingsGroupBase* m_group;
    QString m_key;
    T m_defaultValue;
};

#endif
