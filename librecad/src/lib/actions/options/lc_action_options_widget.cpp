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

#include "lc_action_options_widget.h"

#include <QLineEdit>
#include <QToolButton>

#include "lc_actioncontext.h"
#include "lc_settings_defaults.h"
#include "lc_settings_widget.h"
#include "rs_actioninterface.h"

class LC_LateCompletionRequestor;

LC_ActionOptionsWidget::LC_ActionOptionsWidget(QWidget *parent, const Qt::WindowFlags fl) :
    QWidget(parent, fl) {
    m_interactiveInputControlsAutoRaise = CFG_Widgets::o_PickValueButtonsFlatIcons;
    m_interactiveInputControlsVisible = CFG_Defaults::o_InteractiveInputEnabled;
}

LC_ActionOptionsWidget::~LC_ActionOptionsWidget() = default;

void LC_ActionOptionsWidget::hideOptions(){
    hide();
    LC_ActionOptionsSupport::hideOptions();
}

void LC_ActionOptionsWidget::preSetupByAction(RS_ActionInterface* a) {
    m_actionContext = a->getActionContext();
    m_laterCompletionRequestor = dynamic_cast<LC_LateCompletionRequestor*>(a);
}

void LC_ActionOptionsWidget::cleanup() {
    m_laterCompletionRequestor = nullptr;
    m_actionContext = nullptr;
}

void LC_ActionOptionsWidget::connectInteractiveInputButton(QToolButton* button,
                                                           const InteractiveInputInfo::InputType inputType,
                                                           const QString& tag) const {
    if (m_interactiveInputControlsVisible) {
        button->setVisible(true);
        button->setProperty("_interactiveInputButton", inputType);
        button->setProperty("_interactiveInputTag", tag);
        button->connect(button, &QToolButton::clicked, this, &LC_ActionOptionsWidget::onInteractiveInputButtonClicked);
        button->setAutoRaise(m_interactiveInputControlsAutoRaise);
    }
    else {
        button->setVisible(false);
    }
}

void LC_ActionOptionsWidget::requestFocusForTag(const QString& tag) const {
    auto widgets = findChildren<QWidget*>();
    for (const auto le:std::as_const(widgets)) {
        auto tagProperty = le->property("_tagHolder");
        if (tagProperty.isValid() && !tagProperty.isNull()) {
            QString editTag = tagProperty.toString();
            if (tag == editTag) {
                le->setFocus();
                break;
            }
        }
    }
}

void LC_ActionOptionsWidget::pickDistanceSetup(const QString& tag, QToolButton* button, QLineEdit* lineedit) const {
    connectInteractiveInputButton(button, InteractiveInputInfo::DISTANCE, tag);
    lineedit->setProperty("_tagHolder", tag);
}

void LC_ActionOptionsWidget::pickAngleSetup(const QString& tag, QToolButton* button, QLineEdit* editor) const {
    connectInteractiveInputButton(button, InteractiveInputInfo::ANGLE, tag);
    editor->setProperty("_tagHolder", tag);
}

void LC_ActionOptionsWidget::onInteractiveInputButtonClicked([[maybe_unused]]bool checked) const {
    const auto senderButton = dynamic_cast<QToolButton*>(sender());
    if (senderButton != nullptr) {
        const auto property = senderButton->property ("_interactiveInputButton");
        if (property.isValid()) {
            const auto inputType = static_cast<InteractiveInputInfo::InputType>(property.toInt());
            const auto tagProperty = senderButton->property ("_interactiveInputTag");
            const QString tag = tagProperty.toString();
            switch (inputType) {
                case InteractiveInputInfo::DISTANCE:
                case InteractiveInputInfo::ANGLE: {
                    m_actionContext->interactiveInputStart(inputType, m_laterCompletionRequestor, tag);
                    break;
                }
                case InteractiveInputInfo::POINT:
                case InteractiveInputInfo::POINT_X:
                case InteractiveInputInfo::POINT_Y: {
                    m_actionContext->interactiveInputStart(inputType, m_laterCompletionRequestor, tag);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
