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

#ifndef LC_SETTINGS_PAGE_BASE_H
#define LC_SETTINGS_PAGE_BASE_H

#include <QColorDialog>
#include <QFileDialog>
#include <QLabel>
#include "lc_settings_binder.h"
#include "lc_settings_page_interface.h"

#include "lc_action_draw_line_angle_rel.h"
#include "lc_setting.h"
#include "qg_linetypebox.h"

class QG_WidthBox;
class LC_ColorButton;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QLineEdit;
class QAbstractButton;
class LC_SettingsLinksWidget;
class LC_SettingsBannerWidget;

class LC_SettingsPageBase : public QObject, public LC_SettingsPageInterface {
    Q_OBJECT public:


    struct BoolEntry {
        QCheckBox* widget = nullptr;
        const LC_Setting<bool>& setting;
        bool reqRestart = false;
    };

    struct IntEntry {
        QSpinBox* widget = nullptr;
        const LC_Setting<int>& setting;
        bool reqRestart = false;
    };

    struct DoubleEntry {
        QDoubleSpinBox* widget = nullptr;
        const LC_Setting<double>& setting;
        bool reqRestart = false;
    };

    struct StringEntry {
        QLineEdit* widget = nullptr;
        const LC_Setting<QString>& setting;
        bool reqRestart = false;
    };

    struct ComboIndexEntry {
        QComboBox* widget = nullptr;
        const LC_Setting<int>& setting;
        bool reqRestart = false;
    };

    struct ComboTextEntry {
        QComboBox* widget = nullptr;
        const LC_Setting<QString>& setting;
        bool reqRestart = false;
    };

    struct ColorEntry {
        QComboBox* comboBox;
        LC_ColorButton* button;
        const LC_Setting<QColor>& setting;
        QString dialogTitle = QString();
        bool reqRestart = false;
    };

    LC_SettingsPageBase(const QString& displayName, std::unique_ptr<LC_SettingsBackend> backend, QObject* parent = nullptr);
    ~LC_SettingsPageBase() override = default;

    QString id() const override {
        return m_id;
    }

    void setId(const QString& id) override {
        m_id = id;
    }

    QString parentId() const override {
        return m_parentId;
    }

    void setParentId(const QString& parentId) override {
        m_parentId = parentId;
    }

    QString displayName() const override {
        return m_displayName;
    }

    QWidget* getEditingWidget() override;

    bool isGated() const { return m_isGated; }

    void loadSettings() override {
        m_binder.loadAll();
    }

    bool saveSettings() override {
        m_binder.saveAll(true);
        return true;
    }

    void resetDefaults() override {
        m_binder.resetAllDefaults();
    }

    bool isModified() const override {
        return m_binder.isAnyModified();
    }

    bool requiresRestart() const override {
        return m_restartRequired;
    }

    void setRestartRequired(bool required) override {
        m_restartRequired = required;
    }

    bool validate(QString& outErrorMessage) override {
        Q_UNUSED(outErrorMessage);
        return true;
    }

    QList<LC_SearchTarget> searchTargets() const override {
        return m_searchTargets;
    }

    void registerSearchTarget(QWidget* target, const QString& text) override;
    void highlightSearchPattern(const QString& pattern) override;
    void clearSearchHighlight() override;
    void autoIndexLabels() override;

    void updateLivePreview() override;
signals:
    void navigateToPage(const QString& pageId);
    void livePreviewRequested();
protected:
    LC_SettingsBinder* binder() {
        return &m_binder;
    }

    LC_SettingsBackend* backend() const {
        return m_backend.get();
    }

    void bind(QCheckBox* cb, const LC_Setting<bool>& setting, bool reqRestart = false);
    void bind(QSpinBox* sb, const LC_Setting<int>& setting, bool reqRestart = false);
    void bind(QDoubleSpinBox* sb, const LC_Setting<double>& setting, bool reqRestart = false);
    void bind(QLineEdit* le, const LC_Setting<QString>& setting, bool reqRestart = false);
    void bindLineType(QG_LineTypeBox* lineTypeCombo, const LC_Setting<RS2::LineType>& setting, bool reqRestart = false);
    void bindLineWidth(QG_WidthBox* lineWidthCombo, const LC_Setting<RS2::LineWidth>& setting, bool reqRestart = false);

    void bindColor(QComboBox* comboBox, LC_ColorButton* button, const LC_Setting<QColor>& setting, const QString& dialogTitle = QString(),
                   bool reqRestart = false);

    void bindBoolean(const std::initializer_list<BoolEntry>& entries);
    void bindInt(const std::initializer_list<IntEntry>& entries);
    void bindDouble(const std::initializer_list<DoubleEntry>& entries);
    void bindString(const std::initializer_list<StringEntry>& entries);
    void bindComboIndex(const std::initializer_list<ComboIndexEntry>& entries);
    void bindComboText(const std::initializer_list<ComboTextEntry>& entries);
    void bindColor(const std::initializer_list<ColorEntry>& entries);

    template <typename WidgetType, typename ValueType>
    void bindCustom(WidgetType* widget, const LC_Setting<ValueType>& setting, bool reqRestart, std::function<ValueType(WidgetType*)> getter,
                    std::function<void(WidgetType*, ValueType)> setter) {
        m_binder.bindCustom<WidgetType, ValueType>(widget, setting.fullKey(), setting.defaultValue(), reqRestart, getter, setter);
    }

    template <typename WidgetType, typename ValueType, typename SignalSignature>
    void bindCustom(WidgetType* widget, const LC_Setting<ValueType>& setting, bool reqRestart, SignalSignature signal,
                    std::function<ValueType(WidgetType*)> getter, std::function<void(WidgetType*, ValueType)> setter) {
        m_binder.bindCustom<WidgetType, ValueType, SignalSignature>(widget, setting.fullKey(), setting.defaultValue(), reqRestart, signal,
                                                                    getter, setter);
    }

    void enableWhenChecked(QCheckBox* source, QWidget* target, bool invert = false);
    void enableWhenChecked(QRadioButton* source, QWidget* target, bool invert = false);
    void showWhenChecked(QCheckBox* source, QWidget* target, bool invert = false);

    void bindDirectoryChooser(QAbstractButton* button, QLineEdit* lineEdit, const QString& dialogTitle);
    void bindFileChooser(QAbstractButton* button, QLineEdit* lineEdit, const QString& dialogTitle, const QString& filter);

    LC_SettingsLinksWidget* createLinksWidget(const QList<QPair<QString, QString>>& links, QWidget* parent = nullptr);

    virtual void setupUi() = 0;

    virtual void setupBehavior() {}

    virtual void setupBindings() {}

    template <typename WidgetType, typename ValueType>
    void bindCustom(WidgetType* widget, const QString& key, const ValueType& defaultValue, bool reqRestart,
                    std::function<ValueType(WidgetType*)> getter, std::function<void(WidgetType*, ValueType)> setter) {
        m_binder.bindCustom<WidgetType, ValueType>(widget, key, defaultValue, reqRestart, getter, setter);
    }

    template <typename WidgetType, typename ValueType, typename SignalSignature>
    void bindCustom(WidgetType* widget, const QString& key, const ValueType& defaultValue, bool reqRestart, SignalSignature signal,
                    std::function<ValueType(WidgetType*)> getter, std::function<void(WidgetType*, ValueType)> setter) {
        m_binder.bindCustom<WidgetType, ValueType, SignalSignature>(widget, key, defaultValue, reqRestart, signal, getter, setter);
    }

    bool isSettingsDialogVisible() const;

    void setReadOnly(bool readOnly) override;

    QWidget* m_widget = nullptr;
private:
    QString m_id;
    QString m_parentId;
    QString m_displayName;
    std::unique_ptr<LC_SettingsBackend> m_backend;
    LC_SettingsBinder m_binder;
    bool m_restartRequired = false;
    QList<LC_SearchTarget> m_searchTargets;
    bool m_isGated = false;
};

#endif
