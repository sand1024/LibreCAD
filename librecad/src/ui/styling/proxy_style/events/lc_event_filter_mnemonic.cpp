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

#include "lc_event_filter_mnemonic.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>

#include "lc_palette_editor_shared.h"
#include "lc_proxy_style.h"

bool LC_EventFilterMnemonic::eventFilter(QObject* watched, QEvent* event) {
    Q_UNUSED(watched);
    const QEvent::Type type = event->type();
    const StyleMetricsConfig& metrics = style()->metrics();

    if (metrics.hideShortcutUnderlines) {
        switch (type) {
            case QEvent::KeyPress: {
                const auto* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Alt) {
                    if ((keyEvent->modifiers() & ~Qt::AltModifier) == Qt::NoModifier) {
                        m_altPressed = true;
                        m_otherKeyPressed = false;

                        if (metrics.mnemonicUnderlineMode == MnemonicUnderlineMode::PressAndHold) {
                            m_showUnderlines = true;
                            if (QWidget* activeWin = QApplication::activeWindow()) {
                                activeWin->update();
                            }
                        }
                    }
                }
                else if (m_altPressed) {
                    m_otherKeyPressed = true;
                }
                break;
            }

            case QEvent::KeyRelease: {
                const auto* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Alt) {
                    m_altPressed = false;

                    if (metrics.mnemonicUnderlineMode == MnemonicUnderlineMode::PressAndHold) {
                        m_showUnderlines = false;
                    }
                    else if (metrics.mnemonicUnderlineMode == MnemonicUnderlineMode::StickyToggle) {
                        if (!m_otherKeyPressed) {
                            m_showUnderlines = !m_showUnderlines;
                        }
                    }

                    m_otherKeyPressed = false;
                    if (QWidget* activeWin = QApplication::activeWindow()) {
                        activeWin->update();
                    }
                }
                break;
            }

            case QEvent::WindowDeactivate:
            case QEvent::FocusOut: {
                if (QApplication::activePopupWidget()) {
                    return false;
                }

                if (m_showUnderlines || m_altPressed || m_otherKeyPressed) {
                    m_altPressed = false;
                    m_otherKeyPressed = false;
                    m_showUnderlines = false;
                    if (QWidget* activeWin = QApplication::activeWindow()) {
                        activeWin->update();
                    }
                }
                break;
            }

            default:
                break;
        }
    }
    return false;
}
