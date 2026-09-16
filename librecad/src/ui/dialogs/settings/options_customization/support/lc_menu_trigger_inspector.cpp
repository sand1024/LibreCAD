/*******************************************************************************
 *
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

#include "lc_menu_trigger_inspector.h"
#include "ui_lc_menu_trigger_inspector.h"

#include <QMouseEvent>

LC_MenuTriggerInspector::LC_MenuTriggerInspector(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_MenuTriggerInspector>())
    , m_currentActivator(std::make_unique<LC_MenuActivator>()) {
    ui->setupUi(this);

    initEntityCombobox();

    ui->lblClickPad->installEventFilter(this);

    connect(ui->cbCtrl, &QCheckBox::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->cbShift, &QCheckBox::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->cbAlt, &QCheckBox::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);

    connect(ui->rbLeft, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbMiddle, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbRight, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbBack, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbForward, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbTask, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);

    connect(ui->rbClick, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);
    connect(ui->rbDblClick, &QRadioButton::toggled, this, &LC_MenuTriggerInspector::onManualControlToggled);

    connect(ui->cbEntityContext, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_MenuTriggerInspector::onEntityContextChanged);
    connect(ui->pbUnassign, &QPushButton::clicked, this, &LC_MenuTriggerInspector::onUnassignClicked);
}

LC_MenuTriggerInspector::~LC_MenuTriggerInspector() {
    qDeleteAll(m_existingActivators);
    m_existingActivators.clear();
}

void LC_MenuTriggerInspector::initEntityCombobox() {
    ui->cbEntityContext->clear();
    ui->cbEntityContext->addItem(tr("Either Absent or Any Entity"), "EE");
    ui->cbEntityContext->addItem(tr("Absent Entity (Empty Space)"), "NE");
    ui->cbEntityContext->addItem(tr("Any Entity"), "AE");
    ui->cbEntityContext->addItem(tr("Line"), "LI");
    ui->cbEntityContext->addItem(tr("Circle"), "CI");
    ui->cbEntityContext->addItem(tr("Arc"), "AR");
    ui->cbEntityContext->addItem(tr("Polyline"), "PL");
    ui->cbEntityContext->addItem(tr("Spline"), "SL");
    ui->cbEntityContext->addItem(tr("Spline By Points"), "SP");
    ui->cbEntityContext->addItem(tr("Ellipse"), "EL");
    ui->cbEntityContext->addItem(tr("Point"), "PO");
    ui->cbEntityContext->addItem(tr("Parabola"), "PA");
    ui->cbEntityContext->addItem(tr("Image"), "IM");
    ui->cbEntityContext->addItem(tr("Hatch"), "HA");
    ui->cbEntityContext->addItem(tr("Insert"), "IN");
    ui->cbEntityContext->addItem(tr("Text"), "TE");
    ui->cbEntityContext->addItem(tr("MText"), "MT");
    ui->cbEntityContext->addItem(tr("Dimension Linear"), "DL");
    ui->cbEntityContext->addItem(tr("Dimension Aligned"), "DA");
    ui->cbEntityContext->addItem(tr("Dimension Diametric"), "DD");
    ui->cbEntityContext->addItem(tr("Dimension Radial"), "DR");
    ui->cbEntityContext->addItem(tr("Dimension Ordinate"), "DO");
    ui->cbEntityContext->addItem(tr("Dimension Arc"), "DC");
    ui->cbEntityContext->addItem(tr("Leader"), "LD");
}

void LC_MenuTriggerInspector::setExistingActivators(const QList<LC_MenuActivator*>& list) {
    qDeleteAll(m_existingActivators);
    m_existingActivators.clear();

    for (const auto* a : list) {
        if (a != nullptr) {
            m_existingActivators.append(a->getCopy());
        }
    }

    validateTrigger();
}

void LC_MenuTriggerInspector::setReadOnly(bool readOnly) {
    m_isReadOnly = readOnly;
    ui->pbUnassign->setEnabled(!readOnly);
    ui->cbCtrl->setEnabled(!readOnly);
    ui->cbShift->setEnabled(!readOnly);
    ui->cbAlt->setEnabled(!readOnly);
    ui->rbLeft->setEnabled(!readOnly);
    ui->rbMiddle->setEnabled(!readOnly);
    ui->rbRight->setEnabled(!readOnly);
    ui->rbBack->setEnabled(!readOnly);
    ui->rbForward->setEnabled(!readOnly);
    ui->rbTask->setEnabled(!readOnly);
    ui->rbClick->setEnabled(!readOnly);
    ui->rbDblClick->setEnabled(!readOnly);
    ui->cbEntityContext->setEnabled(!readOnly);
}

void LC_MenuTriggerInspector::setActivator(const QString& menuName, const QString& shortcutCode) {
    m_blockSignals = true;
    m_currentActivator->setMenuName(menuName);

    if (shortcutCode.isEmpty()) {
        m_currentActivator = std::make_unique<LC_MenuActivator>();
        m_currentActivator->setMenuName(menuName);
    } else {
        auto* parsed = LC_MenuActivator::fromShortcut(shortcutCode);
        if (parsed != nullptr) {
            m_currentActivator.reset(parsed);
            m_currentActivator->setMenuName(menuName);
        }
    }

    syncUiFromActivator();
    m_blockSignals = false;
    validateTrigger();
}

QString LC_MenuTriggerInspector::getActivatorCode() const {
    if (ui->lblTriggerBadge->text() == tr("NOT ASSIGNED")) {
        return "";
    }
    m_currentActivator->update();
    return m_currentActivator->getShortcut();
}

void LC_MenuTriggerInspector::syncUiFromActivator() {
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    m_currentActivator->getKeysState(ctrl, alt, shift);

    ui->cbCtrl->setChecked(ctrl);
    ui->cbAlt->setChecked(alt);
    ui->cbShift->setChecked(shift);

    switch (m_currentActivator->getButtonType()) {
        case LC_MenuActivator::LEFT:    ui->rbLeft->setChecked(true); break;
        case LC_MenuActivator::MIDDLE:  ui->rbMiddle->setChecked(true); break;
        case LC_MenuActivator::RIGHT:   ui->rbRight->setChecked(true); break;
        case LC_MenuActivator::BACK:    ui->rbBack->setChecked(true); break;
        case LC_MenuActivator::FORWARD: ui->rbForward->setChecked(true); break;
        case LC_MenuActivator::TASK:    ui->rbTask->setChecked(true); break;
        default: ui->rbRight->setChecked(true); break;
    }

    if (m_currentActivator->getEventType() == LC_MenuActivator::DBL_CLICK) {
        ui->rbDblClick->setChecked(true);
    } else {
        ui->rbClick->setChecked(true);
    }

    const int idx = ui->cbEntityContext->findData(m_currentActivator->getEntityTypeStr());
    if (idx >= 0) {
        ui->cbEntityContext->setCurrentIndex(idx);
    }

    m_currentActivator->update();
    if (m_currentActivator->getShortcut().isEmpty()) {
        ui->lblTriggerBadge->setText(tr("NOT ASSIGNED"));
    } else {
        ui->lblTriggerBadge->setText(m_currentActivator->getShortcutView());
    }
}

void LC_MenuTriggerInspector::updateActivatorFromUi() {
    m_currentActivator->setKeys(ui->cbCtrl->isChecked(), ui->cbAlt->isChecked(), ui->cbShift->isChecked());

    LC_MenuActivator::Button btn = LC_MenuActivator::RIGHT;
    if (ui->rbLeft->isChecked()) {
        btn = LC_MenuActivator::LEFT;
    } else if (ui->rbMiddle->isChecked()) {
        btn = LC_MenuActivator::MIDDLE;
    } else if (ui->rbRight->isChecked()) {
        btn = LC_MenuActivator::RIGHT;
    } else if (ui->rbBack->isChecked()) {
        btn = LC_MenuActivator::BACK;
    } else if (ui->rbForward->isChecked()) {
        btn = LC_MenuActivator::FORWARD;
    } else if (ui->rbTask->isChecked()) {
        btn = LC_MenuActivator::TASK;
    }
    m_currentActivator->setButtonType(btn);

    m_currentActivator->setEventType(ui->rbDblClick->isChecked() ? LC_MenuActivator::DBL_CLICK : LC_MenuActivator::CLICK_RELEASE);

    const QString entityStr = ui->cbEntityContext->currentData().toString();
    bool req = true;
    RS2::EntityType type = RS2::EntityUnknown;
    LC_MenuActivator::parseEntityType(entityStr, req, type);
    m_currentActivator->setEntityRequired(req);
    m_currentActivator->setEntityType(type);

    m_currentActivator->update();
    ui->lblTriggerBadge->setText(m_currentActivator->getShortcutView());
    validateTrigger();
}

bool LC_MenuTriggerInspector::eventFilter(QObject* obj, QEvent* event) {
    if (m_isReadOnly) {
        return QWidget::eventFilter(obj, event);
    }
    if (obj == ui->lblClickPad) {
        const QMouseEvent* mouseEvent = nullptr;

        if (event->type() == QEvent::MouseButtonRelease) {
            const bool isDbl = m_clickTimer.isValid() && m_clickTimer.elapsed() < 400;
            if (!isDbl) {
                mouseEvent = static_cast<QMouseEvent*>(event);
                ui->rbClick->setChecked(true);
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            mouseEvent = static_cast<QMouseEvent*>(event);
            ui->rbDblClick->setChecked(true);
            m_clickTimer.start();
        }

        if (mouseEvent != nullptr) {
            switch (mouseEvent->button()) {
                case Qt::LeftButton:    ui->rbLeft->setChecked(true); break;
                case Qt::MiddleButton:  ui->rbMiddle->setChecked(true); break;
                case Qt::RightButton:   ui->rbRight->setChecked(true); break;
                case Qt::BackButton:    ui->rbBack->setChecked(true); break;
                case Qt::ForwardButton: ui->rbForward->setChecked(true); break;
                case Qt::TaskButton:    ui->rbTask->setChecked(true); break;
                default: break;
            }

            const auto mods = mouseEvent->modifiers();
            ui->cbCtrl->setChecked(mods & Qt::ControlModifier);
            ui->cbShift->setChecked(mods & Qt::ShiftModifier);
            ui->cbAlt->setChecked(mods & Qt::AltModifier);

            updateActivatorFromUi();
            emit triggerChanged();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void LC_MenuTriggerInspector::onManualControlToggled() {
    if (m_blockSignals) return;
    updateActivatorFromUi();
    emit triggerChanged();
}

void LC_MenuTriggerInspector::onEntityContextChanged(int) {
    if (m_blockSignals) return;
    updateActivatorFromUi();
    emit triggerChanged();
}

void LC_MenuTriggerInspector::onUnassignClicked() {
    ui->lblTriggerBadge->setText(tr("NOT ASSIGNED"));
    ui->bannerStatus->setMessage(tr("No trigger assigned. Menu will not appear on mouse clicks."), LC_SettingsBannerWidget::BannerType::Info);
    emit triggerChanged();
}

void LC_MenuTriggerInspector::validateTrigger() {
    if (ui->lblTriggerBadge->text() == tr("NOT ASSIGNED")) {
        ui->bannerStatus->setMessage(tr("No trigger assigned. Menu will not appear on mouse clicks."), LC_SettingsBannerWidget::BannerType::Info);
        return;
    }

    bool ctrl = false, alt = false, shift = false;
    m_currentActivator->getKeysState(ctrl, alt, shift);
    const bool noKeys = !m_currentActivator->hasKeys();
    const bool dbl = (m_currentActivator->getEventType() == LC_MenuActivator::DBL_CLICK);
    const bool click = (m_currentActivator->getEventType() == LC_MenuActivator::CLICK_RELEASE);
    const auto btn = m_currentActivator->getButtonType();

    QString warn;

    if (btn == LC_MenuActivator::LEFT && noKeys && dbl) {
        warn = tr("Warning: Reserved for 'Entity Properties'.");
    } else if (btn == LC_MenuActivator::LEFT && noKeys && click) {
        warn = tr("Warning: Reserved for 'Entity Selection'.");
    } else if (btn == LC_MenuActivator::LEFT && ctrl && click) {
        warn = tr("Warning: Reserved for 'Viewport Pan'.");
    } else if (btn == LC_MenuActivator::MIDDLE && noKeys && click) {
        warn = tr("Warning: Reserved for 'Viewport Pan'.");
    } else if (btn == LC_MenuActivator::RIGHT && noKeys && click) {
        warn = tr("Notice: Overrides default context menu.");
    }

    if (warn.isEmpty()) {
        const QString activeMenu = m_currentActivator->getMenuName();
        for (const auto* other : m_existingActivators) {
            if (other != nullptr && other->getMenuName() != activeMenu) {
                if (other->isSameAs(m_currentActivator.get())) {
                    warn = tr("Conflict: Already assigned to menu '%1'.").arg(other->getMenuName());
                    break;
                }
            }
        }
    }

    if (!warn.isEmpty()) {
        ui->bannerStatus->setMessage(warn, LC_SettingsBannerWidget::BannerType::Warning);
    } else {
        ui->bannerStatus->setMessage(tr("Shortcut is valid to use."), LC_SettingsBannerWidget::BannerType::Info);
    }
}
