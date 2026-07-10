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


#include "lc_event_filter_tool_tip.h"

#include <QApplication>
#include <QMenuBar>

#include "lc_action_draw_text.h"
#include "lc_event_filter_auto_popup_controller.h"
#include "lc_proxy_style.h"

bool LC_EventFilterToolTip::eventFilter(QObject* watched, QEvent* event) {
    const QEvent::Type type = event->type();

    switch (type) {
        case QEvent::ToolTip: {
            QWidget* activePopup = QApplication::activePopupWidget();
            if (!activePopup) {
                return false;
            }

            if (const auto* watchedWidget = qobject_cast<QWidget*>(watched)) {
                if (watchedWidget != activePopup && !activePopup->isAncestorOf(watchedWidget)) {
                    const auto* controller = style()->autoPopupController();
                    if (controller) {
                        const QWidget* trigger = controller->hoveredPopupTrigger().data();
                        if (trigger && qobject_cast<const QMenuBar*>(trigger)) {
                            // Do not close the main menu bar dropdown on adjacent tooltip sweeps
                            return false;
                        }
                        if (watchedWidget == trigger) {
                        event->accept();
                        return true;
                    }
                    }
                        activePopup->close();
                        return false;
                    }
                }
            break;
        }
        case QEvent::Show:
        case QEvent::Move: {
            if (style()->customToolTipCardEnabled()) {
                if (auto* label = qobject_cast<QLabel*>(watched)) {
                    if (label->window() && label->window()->windowFlags().testFlag(Qt::ToolTip)) {
                        const QString text = label->text();
                        if (text.contains("\n\n") && !Qt::mightBeRichText(text)) {
                            const int splitIdx = text.indexOf("\n\n");
                            if (splitIdx != -1) {
                                QString title = text.left(splitIdx).toHtmlEscaped();
                                QString body = text.mid(splitIdx + 2).toHtmlEscaped();
                                body.replace("\n", "<br>");
                                const QString htmlText = QString("<b>%1</b><hr>%2").arg(title, body);

                                label->blockSignals(true);
                                label->setText(htmlText);
                                label->blockSignals(false);
                            }
                        }
                    }
                }
            }
            break;
        }
        default: {
            break;
        }
    }
    return false;
}
