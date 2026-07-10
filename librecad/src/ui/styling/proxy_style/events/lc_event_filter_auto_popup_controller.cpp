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

#include "lc_event_filter_auto_popup_controller.h"

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>

#include "lc_proxy_style.h"

class QMenuBar;

LC_EventFilterAutoPopupController::LC_EventFilterAutoPopupController(const LC_ProxyStyle* style)
    : LC_ProxyStyleEventFilter(nullptr, style) {
    m_toolbarPopupTimer = new QTimer(this);
    m_toolbarPopupTimer->setSingleShot(true);
    connect(m_toolbarPopupTimer, &QTimer::timeout, this, &LC_EventFilterAutoPopupController::handlePopupTimeout);

    m_leaveCloseTimer = new QTimer(this);
    m_leaveCloseTimer->setSingleShot(true);
    m_leaveCloseTimer->setInterval(300); // 300ms grace period
    connect(m_leaveCloseTimer, &QTimer::timeout, this, &LC_EventFilterAutoPopupController::handleLeaveCloseTimeout);
}

LC_EventFilterAutoPopupController::~LC_EventFilterAutoPopupController() = default;

void LC_EventFilterAutoPopupController::handlePopupTimeout() {
    if (m_hoveredPopupTrigger) {
        QWidget* trigger = m_hoveredPopupTrigger.data();

        if (const auto* toolBar = qobject_cast<QToolBar*>(trigger)) {
            auto* extButton = findToolbarExtensionButton(toolBar);
            if (extButton && extButton->isVisible() && !extButton->isChecked()) {
                const QMenu* menu = extButton->menu();
                if (menu) {
                    connect(menu, &QMenu::aboutToHide, this, &LC_EventFilterAutoPopupController::onMenuAboutToHide, Qt::UniqueConnection);
                }
                extButton->click();
            }
        }
        else if (auto* toolButton = qobject_cast<QToolButton*>(trigger)) {
            const bool isPopup = (toolButton->popupMode() == QToolButton::InstantPopup || toolButton->popupMode() ==
                QToolButton::MenuButtonPopup);
            if (isPopup) {
                const int threshold = toolButton->defaultAction() ? 1 : 0;
                const bool hasMenu = toolButton->menu() || (toolButton->defaultAction() && toolButton->defaultAction()->menu()) || (
                    toolButton->actions().size() > threshold);

                if (hasMenu) {
                    const QMenu* menu = toolButton->menu();
                    if (!menu && toolButton->defaultAction()) {
                        menu = toolButton->defaultAction()->menu();
                    }
                    if (menu) {
                        connect(menu, &QMenu::aboutToHide, this, &LC_EventFilterAutoPopupController::onMenuAboutToHide, Qt::UniqueConnection);
                    }
                    toolButton->showMenu();
                }
            }
        }
        else if (auto* menuBar = qobject_cast<QMenuBar*>(trigger)) {
            if (m_hoveredMenuBarAction && m_hoveredMenuBarAction->menu()) {
                const QMenu* menu = m_hoveredMenuBarAction->menu();
                connect(menu, &QMenu::aboutToHide, this, &LC_EventFilterAutoPopupController::onMenuAboutToHide, Qt::UniqueConnection);
                const QRect geom = menuBar->actionGeometry(m_hoveredMenuBarAction);
                const QPoint clickPoint = geom.center();

                QMouseEvent press(QEvent::MouseButtonPress, clickPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(menuBar, &press);

                QMouseEvent release(QEvent::MouseButtonRelease, clickPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(menuBar, &release);
            }
            // Retain m_hoveredMenuBarAction so subsequent hovers over the active dropdown's trigger are recognized as identical.
        }
    }
}

void LC_EventFilterAutoPopupController::handleLeaveCloseTimeout() {
    if (QWidget* activePopup = QApplication::activePopupWidget()) {
        if (m_hoveredPopupTrigger && qobject_cast<QToolButton*>(m_hoveredPopupTrigger.data())) {
            activePopup->close();
        }
    }
}

void LC_EventFilterAutoPopupController::onMenuAboutToHide() {
    m_hoveredPopupTrigger = nullptr;
    m_hoveredMenuBarAction = nullptr;
}

bool LC_EventFilterAutoPopupController::eventFilter(QObject* watched, QEvent* event) {
    const QEvent::Type type = event->type();

    switch (type) {
        case QEvent::Enter: {
            handleSegmentedButtonEnter(watched);
            handleToolbarOverflowEnter(watched);
            handleInstantButtonEnter(watched);
            handleReEntryCancellation(watched);
            break;
        }

        case QEvent::Leave: {
            handleSegmentedButtonLeave(watched);
            handleLabelLinkLeave(watched);
            handleHoverLeaveCleanup(watched);
            break;
        }

        case QEvent::MouseMove:
        case QEvent::HoverMove: {
            if (handleActiveGrabNavigation(watched)) {
                return true;
            }
            handleMenuBarHover(watched);
            break;
        }

        default:
            break;
    }

    return false;
}


void LC_EventFilterAutoPopupController::handleSegmentedButtonEnter(QObject* watched) {
    if (style()->useSegmentedToolButtons()) {
        if (const auto* btn = qobject_cast<QToolButton*>(watched)) {
            const QVariant groupVal = btn->property(PROP_BUTTON_GROUP);
            if (groupVal.isValid()) {
                if (QWidget* parent = btn->parentWidget()) {
                    parent->setProperty(PROP_HOVERED_GROUP_ID, groupVal);
                    parent->update();
                }
            }
        }
    }
}

void LC_EventFilterAutoPopupController::handleToolbarOverflowEnter(QObject* watched) {
    if (auto* toolBar = qobject_cast<QToolBar*>(watched)) {
        if (toolBar->window() && toolBar->window() == QApplication::activeWindow()) {
            QToolButton* extButton = toolBar->property(PROP_EXT_BUTTON).value<QToolButton*>();
            if (!extButton) {
                extButton = findToolbarExtensionButton(toolBar);
                const_cast<QToolBar*>(toolBar)->setProperty(PROP_EXT_BUTTON, QVariant::fromValue(extButton));
            }

            if (extButton && extButton->isVisible()) {
                if (m_hoveredPopupTrigger != toolBar) {
                    m_hoveredPopupTrigger = toolBar;
                    m_toolbarPopupTimer->stop();
                    m_toolbarPopupTimer->setInterval(style()->metrics().toolbarPopupDelay);
                    m_toolbarPopupTimer->start();
                }
                else if (!m_toolbarPopupTimer->isActive() && !extButton->isChecked()) {
                    m_toolbarPopupTimer->start();
                }
            }
        }
    }
}

void LC_EventFilterAutoPopupController::handleInstantButtonEnter(QObject* watched) {
    if (auto* toolButton = qobject_cast<QToolButton*>(watched)) {
        if (style()->autoPopupInstantButtonsEnabled() && toolButton->window() && toolButton->window() ==
            QApplication::activeWindow()) {
            const bool isPopup = (toolButton->popupMode() == QToolButton::InstantPopup || toolButton->popupMode() ==
                QToolButton::MenuButtonPopup);
            if (isPopup) {
                const int threshold = toolButton->defaultAction() ? 1 : 0;
                const bool hasMenu = toolButton->menu() || (toolButton->defaultAction() && toolButton->defaultAction()->menu()) || (
                    toolButton->actions().size() > threshold);

                if (hasMenu) {
                    if (toolButton->property(PROP_DISABLE_AUTO_EXPANSION).toBool() != true) {
                        if (m_hoveredPopupTrigger != toolButton) {
                            m_hoveredPopupTrigger = toolButton;
                            m_toolbarPopupTimer->stop();

                            int delay = style()->metrics().toolbarPopupDelay;
                            if (toolButton->popupMode() == QToolButton::MenuButtonPopup) {
                                delay = static_cast<int>(delay * 1.5);
                            }
                            m_toolbarPopupTimer->setInterval(delay);
                            m_toolbarPopupTimer->start();
                        }
                        else if (!m_toolbarPopupTimer->isActive()) {
                            QMenu* btnMenu = toolButton->menu();
                            if (!btnMenu && toolButton->defaultAction()) {
                                btnMenu = toolButton->defaultAction()->menu();
                            }
                            if (!btnMenu || !btnMenu->isVisible()) {
                                m_toolbarPopupTimer->start();
                            }
                        }
                    }
                }
            }
        }
    }
}

void LC_EventFilterAutoPopupController::handleReEntryCancellation(QObject* watched) {
    if (auto* widget = qobject_cast<QWidget*>(watched)) {
        bool cursorInPopup = false;
        if (const QWidget* activePopup = QApplication::activePopupWidget()) {
            if (widget == activePopup || activePopup->isAncestorOf(widget)) {
                cursorInPopup = widget->rect().contains(widget->mapFromGlobal(QCursor::pos()));
            }
        }
        const bool cursorInTrigger = m_hoveredPopupTrigger && widget == m_hoveredPopupTrigger.data() && widget->rect().contains(
            widget->mapFromGlobal(QCursor::pos()));

        if (cursorInPopup || cursorInTrigger) {
            m_leaveCloseTimer->stop();
        }
    }
}

void LC_EventFilterAutoPopupController::handleSegmentedButtonLeave(QObject* watched) {
    if (style()->useSegmentedToolButtons()) {
        if (const auto* btn = qobject_cast<QToolButton*>(watched)) {
            if (btn->property(PROP_BUTTON_GROUP).isValid()) {
                if (QWidget* parent = btn->parentWidget()) {
                    parent->setProperty(PROP_HOVERED_GROUP_ID, QVariant());
                    parent->update();
                }
            }
        }
    }
}

void LC_EventFilterAutoPopupController::handleLabelLinkLeave(QObject* watched) {
    if (auto* label = qobject_cast<QLabel*>(watched)) {
        const bool alreadyHovered = label->property(PROP_LINK_ACTIVE_HOVER).toBool();
        if (alreadyHovered) {
            QTimer::singleShot(0, label, [label]() {
                label->blockSignals(true);
                label->setProperty(PROP_LINK_ACTIVE_HOVER, false);
                const QString originalHtml = label->property(PROP_ORIGINAL_HTML).toString();
                if (!originalHtml.isEmpty()) {
                    label->clear();
                    label->setText(originalHtml);
                }
                label->blockSignals(false);
                label->update();
            });
        }
    }
}

void LC_EventFilterAutoPopupController::handleHoverLeaveCleanup(QObject* watched) {
    if (auto* widget = qobject_cast<QWidget*>(watched)) {
        if (auto* menuBar = qobject_cast<QMenuBar*>(widget)) {
            // Only clear tracking variables on leave if no active menu bar dropdown is open
            if (!QApplication::activePopupWidget() && m_hoveredMenuBarAction) {
                QAction* oldAction = m_hoveredMenuBarAction;
                m_hoveredMenuBarAction = nullptr;
                menuBar->update(menuBar->actionGeometry(oldAction));
            }
        }

        if (m_hoveredPopupTrigger == widget) {
            m_toolbarPopupTimer->stop();

            if (qobject_cast<QToolButton*>(widget) || qobject_cast<QToolBar*>(widget) || qobject_cast<QMenuBar*>(widget)) {
                const QPoint localCursorPos = widget->mapFromGlobal(QCursor::pos());
                if (!widget->rect().contains(localCursorPos)) {
                    m_leaveCloseTimer->start();
                }
            }
            else {
                m_hoveredPopupTrigger = nullptr;
                m_hoveredMenuBarAction = nullptr;
            }
        }
        else if (const QWidget* activePopup = QApplication::activePopupWidget()) {
            if (widget == activePopup || activePopup->isAncestorOf(widget)) {
                if (m_hoveredPopupTrigger) {
                    const QPoint localCursorPos = activePopup->mapFromGlobal(QCursor::pos());
                    if (!activePopup->rect().contains(localCursorPos)) {
                        m_leaveCloseTimer->start();
                    }
                }
            }
        }
    }
}

bool LC_EventFilterAutoPopupController::handleActiveGrabNavigation(QObject* watched) {
    QWidget* activePopup = QApplication::activePopupWidget();
    if (!activePopup) {
        return false;
    }

    bool cursorInActiveMenu = activePopup->rect().contains(activePopup->mapFromGlobal(QCursor::pos()));
    if (!cursorInActiveMenu) {
        for (QObject* child : activePopup->children()) {
            if (auto* subMenu = qobject_cast<QMenu*>(child)) {
                if (subMenu->isVisible() && subMenu->rect().contains(subMenu->mapFromGlobal(QCursor::pos()))) {
                    cursorInActiveMenu = true;
                    break;
                }
            }
        }
    }

    bool cursorInTrigger = false;
    bool isToolButtonTrigger = false;
    if (m_hoveredPopupTrigger) {
        isToolButtonTrigger = qobject_cast<QToolButton*>(m_hoveredPopupTrigger.data()) != nullptr;
        cursorInTrigger = m_hoveredPopupTrigger->rect().contains(m_hoveredPopupTrigger->mapFromGlobal(QCursor::pos()));
    }

    if (cursorInActiveMenu || cursorInTrigger) {
        m_leaveCloseTimer->stop();
    }
    else {
        if (isToolButtonTrigger) {
            if (!m_leaveCloseTimer->isActive()) {
                m_leaveCloseTimer->start();
            }
        }
    }

    if (auto* watchedWidget = qobject_cast<QWidget*>(watched)) {
        QToolTip::hideText();
        if (watchedWidget->rect().contains(watchedWidget->mapFromGlobal(QCursor::pos()))) {
            m_leaveCloseTimer->stop();
        }

        if (qobject_cast<QMenu*>(activePopup)) {
            // Guard: If the cursor is physically inside the active popup menu (or its visible submenus),
            // do not perform hit-testing of underlying main window widgets. This prevents overlapped
            // toolbars and toolbuttons from hijacking the mouse focus and closing the menu bar popup.
            if (cursorInActiveMenu) {
                return false;
            }

            auto* mainWindow = qobject_cast<QMainWindow*>(QApplication::activeWindow());
            QMenuBar* menuBar = nullptr;
            QToolBar* toolBar = nullptr;
            QToolButton* toolButton = nullptr;

            if (mainWindow) {
                QPoint globalPos = QCursor::pos();
                if (auto* mb = mainWindow->menuBar()) {
                    if (mb->isVisible() && mb->rect().contains(mb->mapFromGlobal(globalPos))) {
                        menuBar = mb;
                    }
                }

                if (!menuBar) {
                    for (QToolBar* tb : mainWindow->findChildren<QToolBar*>()) {
                        if (tb->isVisible() && tb->rect().contains(tb->mapFromGlobal(globalPos))) {
                            toolBar = tb;
                            for (QToolButton* btn : tb->findChildren<QToolButton*>()) {
                                if (btn->isVisible() && btn->rect().contains(btn->mapFromGlobal(globalPos))) {
                                    toolButton = btn;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            if (menuBar) {
                // Determine robustly if the active popup menu belongs to the QMenuBar actions
                bool isMenuBarMenu = false;
                if (auto* menu = qobject_cast<QMenu*>(activePopup)) {
                    isMenuBarMenu = (menu->parentWidget() == menuBar ||
                                     menu->parent() == menuBar ||
                                     menuBar->actions().contains(menu->menuAction()));
                }

                if (!isMenuBarMenu) {
                    m_toolbarPopupTimer->stop();
                    m_leaveCloseTimer->stop();

                    QPoint localPos = menuBar->mapFromGlobal(QCursor::pos());
                    QAction* action = menuBar->actionAt(localPos);

                    activePopup->close();

                    if (action) {
                        m_hoveredPopupTrigger = menuBar;
                        m_hoveredMenuBarAction = action;

                        if (style()->autoPopupMenuBarEnabled() && action->menu()) {
                            QMenu* newMenu = action->menu();
                            connect(newMenu, &QMenu::aboutToHide, this, &LC_EventFilterAutoPopupController::onMenuAboutToHide,
                                    Qt::UniqueConnection);

                            QRect geom = menuBar->actionGeometry(action);
                            QPoint clickPoint = geom.center();
                            QMouseEvent press(QEvent::MouseButtonPress, clickPoint, Qt::LeftButton, Qt::LeftButton,
                                              Qt::NoModifier);
                            QCoreApplication::sendEvent(menuBar, &press);
                            QMouseEvent release(QEvent::MouseButtonRelease, clickPoint, Qt::LeftButton, Qt::LeftButton,
                                                Qt::NoModifier);
                            QCoreApplication::sendEvent(menuBar, &release);
                        }
                        else {
                            menuBar->update(menuBar->actionGeometry(action));
                        }
                        return true;
                    }
                }
            }
            else if (toolButton && style()->autoPopupInstantButtonsEnabled()) {
                bool isPopupMode = (toolButton->popupMode() == QToolButton::InstantPopup || toolButton->popupMode() ==
                    QToolButton::MenuButtonPopup);
                if (isPopupMode) {
                    int threshold = toolButton->defaultAction() ? 1 : 0;
                    bool hasMenu = toolButton->menu() || (toolButton->defaultAction() && toolButton->defaultAction()->menu()) ||
                        (toolButton->actions().size() > threshold);

                    if (hasMenu) {
                        QMenu* btnMenu = toolButton->menu();
                        if (!btnMenu && toolButton->defaultAction()) {
                            btnMenu = toolButton->defaultAction()->menu();
                        }

                        if (activePopup != btnMenu && m_hoveredPopupTrigger != toolButton) {
                            if (toolButton->property(PROP_DISABLE_AUTO_EXPANSION).toBool() != true) {
                                m_toolbarPopupTimer->stop();
                                m_leaveCloseTimer->stop();

                                activePopup->close();

                                m_hoveredPopupTrigger = toolButton;
                                QMenu* newMenu = toolButton->menu();
                                if (!newMenu && toolButton->defaultAction()) {
                                    newMenu = toolButton->defaultAction()->menu();
                                }
                                if (newMenu) {
                                    connect(newMenu, &QMenu::aboutToHide, this, &LC_EventFilterAutoPopupController::onMenuAboutToHide,
                                            Qt::UniqueConnection);
                                }
                                toolButton->showMenu();
                                return true;
                            }
                        }
                    }
                }
            }
            else if (toolBar && style()->autoPopupToolbarOverflowEnabled() && toolBar->isMovable()) {
                auto* extButton = findToolbarExtensionButton(toolBar);
                bool isWrapped = extButton && extButton->isVisible();
                if (isWrapped && !extButton->isChecked()) {
                    if (m_hoveredPopupTrigger != toolBar) {
                        m_toolbarPopupTimer->stop();
                        m_leaveCloseTimer->stop();

                        activePopup->close();

                        m_hoveredPopupTrigger = toolBar;
                        auto* targetExtButton = findToolbarExtensionButton(toolBar);
                        if (targetExtButton) {
                            QMenu* menu = targetExtButton->menu();
                            if (menu) {
                                connect(menu, &QMenu::aboutToHide, this, [this]() {
                                    m_hoveredPopupTrigger = nullptr;
                                }, Qt::UniqueConnection);
                            }
                            targetExtButton->click();
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void LC_EventFilterAutoPopupController::handleMenuBarHover(QObject* watched) {
    if (auto* menuBar = qobject_cast<QMenuBar*>(watched)) {
        if (menuBar->window() && menuBar->window() == QApplication::activeWindow()) {
            QPoint localPos = menuBar->mapFromGlobal(QCursor::pos());
            QAction* action = menuBar->actionAt(localPos);

            if (action != m_hoveredMenuBarAction) {
                QAction* oldAction = m_hoveredMenuBarAction;
                m_hoveredMenuBarAction = action;

                if (oldAction) {
                    menuBar->update(menuBar->actionGeometry(oldAction));
                }
                if (action) {
                    menuBar->update(menuBar->actionGeometry(action));
                }

                if (style()->autoPopupMenuBarEnabled() && action && action->menu()) {
                    m_hoveredPopupTrigger = menuBar;
                    m_toolbarPopupTimer->stop();
                    m_toolbarPopupTimer->setInterval(style()->metrics().toolbarPopupDelay);
                    m_toolbarPopupTimer->start();
                }
            }
        }
    }
}
