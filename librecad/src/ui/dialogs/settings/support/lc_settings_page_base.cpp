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

#include "lc_settings_page_base.h"

#include <QGroupBox>
#include <QRadioButton>

#include "lc_color_button.h"
#include "lc_highlight_overlay.h"
#include "lc_settings_banner_widget.h"
#include "lc_settings_list_widget.h"
#include "qg_linetypebox.h"
#include "qg_widthbox.h"
#include "rs.h"

LC_SettingsPageBase::LC_SettingsPageBase(const QString& displayName,
                                         std::unique_ptr<LC_SettingsBackend> backend,
                                         QObject* parent)
    : QObject(parent), m_displayName(displayName)
    , m_backend(std::move(backend)), m_binder(m_backend.get(), this) {}


void LC_SettingsPageBase::registerSearchTarget(QWidget* target, const QString& text) {
    if (target) {
        m_searchTargets.append({text, target});
    }
}

void LC_SettingsPageBase::bind(QCheckBox* cb, const LC_Setting<bool>& setting, bool reqRestart) {
    m_binder.bind(cb, setting.fullKey(), setting.defaultValue(), reqRestart);
}

void LC_SettingsPageBase::bind(QSpinBox* sb, const LC_Setting<int>& setting, bool reqRestart) {
    m_binder.bind(sb, setting.fullKey(), setting.defaultValue(), reqRestart);
}

void LC_SettingsPageBase::bind(QDoubleSpinBox* sb, const LC_Setting<double>& setting, bool reqRestart) {
    m_binder.bind(sb, setting.fullKey(), setting.defaultValue(), reqRestart);
}

void LC_SettingsPageBase::bind(QLineEdit* le, const LC_Setting<QString>& setting, bool reqRestart) {
    m_binder.bind(le, setting.fullKey(), setting.defaultValue(), reqRestart);
}

void LC_SettingsPageBase::bindLineType(QG_LineTypeBox* lineTypeCombo, const LC_Setting<RS2::LineType>& setting, bool reqRestart){
    bindCustom<QG_LineTypeBox, RS2::LineType>(lineTypeCombo, setting, reqRestart,
           &QG_LineTypeBox::lineTypeChanged,
           [](QG_LineTypeBox* w) { return w->getLineType(); },
           [](QG_LineTypeBox* w, RS2::LineType v) { w->setLineType(v); });
  }

void LC_SettingsPageBase::bindLineWidth(QG_WidthBox* lineWidthCombo, const LC_Setting<RS2::LineWidth>& setting, bool reqRestart){
    bindCustom<QG_WidthBox, RS2::LineWidth>(lineWidthCombo, setting, reqRestart,
           &QG_WidthBox::widthChanged,
           [](QG_WidthBox* w) { return w->getWidth(); },
           [](QG_WidthBox* w, RS2::LineWidth v) { w->setWidth(v); });
  }

void LC_SettingsPageBase::bindColor(QComboBox* comboBox, LC_ColorButton* button, const LC_Setting<QColor>& setting,
                                    const QString& dialogTitle, bool reqRestart) {
    if (comboBox == nullptr || button == nullptr) {
        return;
    }

    if (!dialogTitle.isEmpty()) {
        button->setDialogTitle(dialogTitle);
    }

    // Connect both widgets directly to our specialized double-widget color binding in the binder
    m_binder.bindColor(comboBox, button, setting.fullKey(), setting.defaultValue(), reqRestart);

    // Real-Time UI Editing Sync
    connect(comboBox, &QComboBox::currentTextChanged, this, [button](const QString& text) {
        button->setColor(QColor::fromString(text));
    });

    connect(button, &LC_ColorButton::colorChanged, this, [comboBox](const QColor& color) {
        const QString hexName = color.name();
        const int idx = comboBox->findText(hexName);
        if (idx >= 0) {
            comboBox->setCurrentIndex(idx);
        } else {
            comboBox->insertItem(0, hexName);
            comboBox->setCurrentIndex(0);
        }
    });
}

void LC_SettingsPageBase::enableWhenChecked(QCheckBox* source, QWidget* target, bool invert) {
    if (source == nullptr || target == nullptr) {
        return;
    }
    auto updateState = [target, invert](bool checked) {
        target->setEnabled(invert ? !checked : checked);
    };
    connect(source, &QCheckBox::toggled, this, updateState);
    updateState(source->isChecked());
}

void LC_SettingsPageBase::enableWhenChecked(QRadioButton* source, QWidget* target, bool invert) {
    if (source == nullptr || target == nullptr) {
        return;
    }
    auto updateState = [target, invert](bool checked) {
        target->setEnabled(invert ? !checked : checked);
    };
    connect(source, &QRadioButton::toggled, this, updateState);
    updateState(source->isChecked());
}

void LC_SettingsPageBase::showWhenChecked(QCheckBox* source, QWidget* target, bool invert) {
    if (source == nullptr || target == nullptr) {
        return;
    }
    auto updateState = [target, invert](bool checked) {
        target->setVisible(invert ? !checked : checked);
    };
    connect(source, &QCheckBox::toggled, this, updateState);
    updateState(source->isChecked());
}

void LC_SettingsPageBase::bindDirectoryChooser(QAbstractButton* button, QLineEdit* lineEdit, const QString& dialogTitle) {
    if (button == nullptr || lineEdit == nullptr) {
        return;
    }
    connect(button, &QAbstractButton::clicked, this, [this, lineEdit, dialogTitle]() {
        QFileDialog dlg(getEditingWidget());
        dlg.setWindowTitle(dialogTitle);
        dlg.setFileMode(QFileDialog::Directory);
        dlg.setOption(QFileDialog::ShowDirsOnly);
        if (dlg.exec() == QDialog::Accepted) {
            lineEdit->setText(QDir::toNativeSeparators(dlg.selectedFiles().at(0)));
        }
    });
}

void LC_SettingsPageBase::bindFileChooser(QAbstractButton* button, QLineEdit* lineEdit, const QString& dialogTitle, const QString& filter) {
    if (button == nullptr || lineEdit == nullptr) {
        return;
    }
    connect(button, &QAbstractButton::clicked, this, [this, lineEdit, dialogTitle, filter]() {
        const QString file = QFileDialog::getOpenFileName(getEditingWidget(), dialogTitle, QString(), filter);
        if (!file.isEmpty()) {
            lineEdit->setText(QDir::toNativeSeparators(file));
        }
    });
}

LC_SettingsLinksWidget* LC_SettingsPageBase::createLinksWidget(const QList<QPair<QString, QString>>& links, QWidget* parent) {
    auto* widget = new LC_SettingsLinksWidget(links, parent ? parent : this->getEditingWidget());
    connect(widget, &LC_SettingsLinksWidget::pageSelected, this, &LC_SettingsPageBase::navigateToPage);

    for (const auto& pair : links) {
        registerSearchTarget(widget, pair.second);
    }
    return widget;
}

bool LC_SettingsPageBase::isSettingsDialogVisible() const {
    const auto topWindow = m_widget->window();
    const auto* parentDialog = qobject_cast<QDialog*>(topWindow);

    if (parentDialog != nullptr && parentDialog->isVisible()) {
       return true;
    } else {
        return false;
    }
}

void LC_SettingsPageBase::autoIndexLabels() {
    if (getEditingWidget() == nullptr) {
        return;
    }
    m_searchTargets.clear();

    // Index Labels and map Highlight overlays to Buddy widgets
    auto labels = getEditingWidget()->findChildren<QLabel*>();
    for (auto* label : labels) {
        if (!label->text().isEmpty()) {
            QWidget* highlightTarget = label->buddy() ? label->buddy() : label;
            registerSearchTarget(highlightTarget, label->text());
        }
    }

    // Index Checkboxes
    auto checkBoxes = getEditingWidget()->findChildren<QCheckBox*>();
    for (auto* cb : checkBoxes) {
        if (!cb->text().isEmpty()) {
            registerSearchTarget(cb, cb->text());
        }
    }

    // Index Radio Buttons
    auto radioButtons = getEditingWidget()->findChildren<QRadioButton*>();
    for (auto* rb : radioButtons) {
        if (!rb->text().isEmpty()) {
            registerSearchTarget(rb, rb->text());
        }
    }

    // Index GroupBox Titles
    auto groupBoxes = getEditingWidget()->findChildren<QGroupBox*>();
    for (auto* gb : groupBoxes) {
        if (!gb->title().isEmpty()) {
            registerSearchTarget(gb, gb->title());
        }
    }
}


void LC_SettingsPageBase::highlightSearchPattern(const QString& pattern) {
    clearSearchHighlight();
    if (pattern.isEmpty()) {
        return;
    }

    for (const auto& target : m_searchTargets) {
        if (target.targetWidget == nullptr) {
            continue;
        }
        if (target.originalText.contains(pattern, Qt::CaseInsensitive)) {
            new LC_HighlightOverlay(target.targetWidget);
        }
    }
}

void LC_SettingsPageBase::clearSearchHighlight() {
    for (const auto& target : m_searchTargets) {
        if (target.targetWidget == nullptr) {
            continue;
        }

        // Symmetrical RTTI lookup: queries children list and filters using dynamic_cast
        const auto childrenList = target.targetWidget->children();
        for (QObject* child : childrenList) {
            if (auto* overlay = dynamic_cast<LC_HighlightOverlay*>(child)) {
                overlay->deleteLater();
            }
        }
    }
}

void LC_SettingsPageBase::bindBoolean(const std::initializer_list<BoolEntry>& entries) {
    for (const auto& entry : entries) {
        bind(entry.widget, entry.setting, entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindInt(const std::initializer_list<IntEntry>& entries) {
    for (const auto& entry : entries) {
        bind(entry.widget, entry.setting, entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindDouble(const std::initializer_list<DoubleEntry>& entries) {
    for (const auto& entry : entries) {
        bind(entry.widget, entry.setting, entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindString(const std::initializer_list<StringEntry>& entries) {
    for (const auto& entry : entries) {
        bind(entry.widget, entry.setting, entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindComboIndex(const std::initializer_list<ComboIndexEntry>& entries) {
    for (const auto& entry : entries) {
        m_binder.bindIndex(entry.widget, entry.setting.fullKey(), entry.setting.defaultValue(), entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindComboText(const std::initializer_list<ComboTextEntry>& entries) {
    for (const auto& entry : entries) {
        m_binder.bindText(entry.widget, entry.setting.fullKey(), entry.setting.defaultValue(), entry.reqRestart);
    }
}

void LC_SettingsPageBase::bindColor(const std::initializer_list<ColorEntry>& entries) {
    for (const auto& entry : entries) {
        bindColor(entry.comboBox, entry.button, entry.setting, entry.dialogTitle, entry.reqRestart);
    }
}

QWidget* LC_SettingsPageBase::getEditingWidget() {
    if (!m_widget) {
        m_widget = new QWidget();

        setupUi();         // Compile layout
        setupBehavior();   // Wire UI interactivity
        setupBindings();   // Declare settings binders
        autoIndexLabels(); // Automatically index all search strings
    }
    return m_widget;
}

void LC_SettingsPageBase::updateLivePreview() {
    if (m_backend != nullptr) {
        // Write the current widget states temporarily to the in-memory settings backend
        m_binder.saveAll(false);
    }
    // Emit the notification signal to let the dialog update the repaint slots
    emit livePreviewRequested();
}
