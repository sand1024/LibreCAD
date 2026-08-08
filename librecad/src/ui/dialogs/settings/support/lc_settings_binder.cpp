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

#include "lc_settings_binder.h"
#include "lc_settings_page_interface.h"
#include "rs_debug.h"

void LC_ColorBinding::load(const LC_SettingsBackend* backend) {
    // LC_ERR << "LC_ColorBinding::load: Checking widget pointers. Key:" << m_key;

    // if (!m_comboBox) {
    //     LC_ERR << "LC_ColorBinding::load: ERROR - m_comboBox is NULL!";
    // }
    // if (!m_button) {
    //     LC_ERR << "LC_ColorBinding::load: ERROR - m_button is NULL!";
    // }
    if (!m_comboBox || !m_button) {
        return;
    }

    const QString defaultHex = m_defaultValue.name();
    // LC_ERR << "LC_ColorBinding::load: Querying backend. Key:" << m_key << "Default value:" << defaultHex;

    m_cleanValue = backend->value(m_key, defaultHex).toString();
    // LC_ERR << "LC_ColorBinding::load: Backend returned value:" << m_cleanValue;
    if (m_cleanValue.isEmpty()) {
        m_cleanValue = defaultHex;
    }
    // Force update the combobox text safely
    const int idx = m_comboBox->findText(m_cleanValue);
    // LC_ERR << "LC_ColorBinding::load: Search combobox for value index:" << idx;

    m_comboBox->blockSignals(true);
    if (idx >= 0) {
        m_comboBox->setCurrentIndex(idx);
    }
    else if (m_comboBox->isEditable()) {
        // LC_ERR << "LC_ColorBinding::load: Value not in index list, setting edit text directly.";
        m_comboBox->setEditText(m_cleanValue);
    }
    else {
        // LC_ERR << "LC_ColorBinding::load: Value not in list and combo is read-only, inserting at index 0.";
        m_comboBox->insertItem(0, m_cleanValue);
        m_comboBox->setCurrentIndex(0);
    }
    m_comboBox->blockSignals(false);
    // Force update the color button swatch
    // LC_ERR << "LC_ColorBinding::load: Updating m_button swatch color to:" << m_cleanValue;
    m_button->blockSignals(true);
    m_button->setColor(QColor::fromString(m_cleanValue));
    m_button->blockSignals(false);
    // LC_ERR << "LC_ColorBinding::load: Finished loading for key:" << m_key;
}

void LC_ColorBinding::save(LC_SettingsBackend* backend, bool& outRestartRequired, bool commitBaseline) {
    if (!m_comboBox) {
        return;
    }
    const QString currentVal = m_comboBox->currentText();
    if (m_requiresRestart && currentVal != m_cleanValue) {
        outRestartRequired = true;
    }
    backend->setValue(m_key, currentVal);

    // Symmetrical Baseline Guard: only commit baseline if explicitly requested on OK/Apply
    if (commitBaseline) {
        m_cleanValue = currentVal;
    }
}

void LC_ColorBinding::resetToDefault() {
    if (!m_comboBox || !m_button) {
        return;
    }
    const QString defHex = m_defaultValue.name();

    const int idx = m_comboBox->findText(defHex);
    if (idx >= 0) {
        m_comboBox->setCurrentIndex(idx);
    }
    else if (m_comboBox->isEditable()) {
        m_comboBox->setEditText(defHex);
    }

    m_button->setColor(m_defaultValue);
}

bool LC_ColorBinding::isModified() const {
    if (!m_comboBox) {
        return false;
    }
    const auto currentText = m_comboBox->currentText();
    const bool modified = currentText != m_cleanValue;
    return modified;
}

LC_SettingsBinder::LC_SettingsBinder(LC_SettingsBackend* backend, LC_SettingsPageInterface* page)
    : m_backend(backend), m_page(page) {
}

void LC_SettingsBinder::bind(QCheckBox* checkbox, const QString& key, bool defaultValue, bool reqRestart) {
    bindCustom<QCheckBox, bool>(checkbox, key, defaultValue, reqRestart, &QCheckBox::toggled, [](QCheckBox* w) {
                                    return w->isChecked();
                                }, [](QCheckBox* w, bool v) {
                                    w->setChecked(v);
                                });
}

void LC_SettingsBinder::bind(QSpinBox* spinBox, const QString& key, int defaultValue, bool reqRestart) {
    bindCustom<QSpinBox, int>(spinBox, key, defaultValue, reqRestart, QOverload<int>::of(&QSpinBox::valueChanged),
                       [](QSpinBox* w) {
                                  return w->value();
                              }, [](QSpinBox* w, int v) {
                                  w->setValue(v);
                              });
}

void LC_SettingsBinder::bind(QDoubleSpinBox* spinBox, const QString& key, double defaultValue, bool reqRestart) {
    bindCustom<QDoubleSpinBox, double>(spinBox, key, defaultValue, reqRestart, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                                       [](QDoubleSpinBox* w) {
                                           return w->value();
                                       }, [](QDoubleSpinBox* w, double v) {
                                           w->setValue(v);
                                       });
}

void LC_SettingsBinder::bind(QLineEdit* lineEdit, const QString& key, const QString& defaultValue, bool reqRestart) {
    bindCustom<QLineEdit, QString>(lineEdit, key, defaultValue, reqRestart, &QLineEdit::textChanged, [](QLineEdit* w) {
                                       return w->text();
                                   }, [](QLineEdit* w, const QString& v) {
                                       w->setText(v);
                                   });
}

void LC_SettingsBinder::bindIndex(QComboBox* comboBox, const QString& key, int defaultIndex, bool reqRestart) {
    bindCustom<QComboBox, int>(comboBox, key, defaultIndex, reqRestart, QOverload<int>::of(&QComboBox::currentIndexChanged),
                               [](QComboBox* w) {
                                   return w->currentIndex();
                               }, [](QComboBox* w, int v) {
                                   w->setCurrentIndex(v);
                               });
}

void LC_SettingsBinder::bindText(QComboBox* comboBox, const QString& key, const QString& defaultText, bool reqRestart) {
    bindCustom<QComboBox, QString>(comboBox, key, defaultText, reqRestart, &QComboBox::currentTextChanged, [](QComboBox* w) {
                                       return w->currentText();
                                   }, [](QComboBox* w, const QString& v) {
                                       const int idx = w->findText(v);
                                       if (idx >= 0) {
                                           w->setCurrentIndex(idx);
                                       }
                                       else if (w->isEditable()) {
                                           w->setEditText(v);
                                       }
                                       else {
                                           w->insertItem(0, v);
                                           w->setCurrentIndex(0);
                                       }
                                   });
}

void LC_SettingsBinder::bindColor(QComboBox* comboBox, LC_ColorButton* button, const QString& key, const QColor& defaultValue,
                                  bool reqRestart) {
    m_bindings.push_back(std::make_unique<LC_ColorBinding>(comboBox, button, key, defaultValue, reqRestart));
    if (comboBox) {
        connect(comboBox, &QComboBox::currentTextChanged, this, [this]() {
            notifyChanged();
        });
    }
}

void LC_SettingsBinder::loadAll() {
    m_blockSignals = true;
    // LC_ERR << "LC_SettingsBinder::loadAll: Initializing. Total bindings registered:" << m_bindings.size();

    int index = 0;
    for (const auto& binding : m_bindings) {
        // LC_ERR << "LC_SettingsBinder::loadAll: Loading binding index:" << index++;
        binding->load(m_backend);
    }
    // LC_ERR << "LC_SettingsBinder::loadAll: Completed.";
    m_blockSignals = false;
}

void LC_SettingsBinder::saveAll(bool commitBaseline) const {

    bool restartRequired = false;
    for (const auto& binding : m_bindings) {
        bool bindingRestart = false;
        binding->save(m_backend, bindingRestart, commitBaseline); // Forward parameter [1.1.2]
        if (bindingRestart) {
            restartRequired = true;
        }
    }
    m_backend->sync();
    if (restartRequired) {
        m_page->setRestartRequired(true);
    }
}

void LC_SettingsBinder::resetAllDefaults() {
    m_blockSignals = true; // Block signals during bulk defaults reset [2.3]
    for (const auto& binding : m_bindings) {
        binding->resetToDefault();
    }
    m_blockSignals = false; // Unblock

    // Explicitly notify the dialog once after a full visual reset
    notifyChanged();
}

bool LC_SettingsBinder::isAnyModified() const {
    for (const auto& binding : m_bindings) {
        if (binding->isModified()) {
            return true;
        }
    }
    return false;
}
