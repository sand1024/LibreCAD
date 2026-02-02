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

#include "lc_property_action_link_view.h"

#include <QKeyEvent>

#include "lc_properties_sheet.h"
#include "lc_property_view_part.h"

const QByteArray LC_PropertyActionLinkView::VIEW_NAME = QByteArrayLiteral("Link");
const QByteArray LC_PropertyActionLinkView::ATTR_TITLE = QByteArrayLiteral("title");

LC_PropertyActionLinkView::LC_PropertyActionLinkView(LC_PropertyAction& property)
    : LC_PropertyView(property) {
}

void LC_PropertyActionLinkView::doApplyAttributes(const LC_PropertyViewDescriptor& info) {
    info.load(ATTR_TITLE, m_title);
}

LC_PropertyAction& LC_PropertyActionLinkView::typedProperty() const {
    return *static_cast<LC_PropertyAction*>(this->getProperty());
}

void LC_PropertyActionLinkView::buildPartBackground(const LC_PropertyPaintContext& ctx, QList<LC_PropertyViewPart>& parts) {
    LC_PropertyViewPart part(ctx.rect);
    if (!part.rect.isValid()) {
        return;
    }
    part.funPaint = [](const LC_PropertyPaintContext& paintContext, const LC_PropertyViewPart& p) {
        auto& painter = *paintContext.painter;
        const auto& rect = p.rect;

        const QPen oldPen = painter.pen();
        const QPen linesPen(paintContext.getPalette().color(QPalette::Button));
        painter.setPen(linesPen);

        // draw part borders
        painter.drawLine(rect.bottomLeft(), rect.bottomRight());
        painter.setPen(oldPen);
    };

    parts.append(part);
}

void LC_PropertyActionLinkView::doBuildViewParts(LC_PropertyPaintContext& ctx, QList<LC_PropertyViewPart>& parts) {
    buildPartBackground(ctx, parts);

    LC_PropertyViewPart part(ctx.rect.marginsRemoved(ctx.margins));
    // part.m_rect.setWidth(ctx.painter->fontMetrics().boundingRect(m_title).width() + 5);
    part.setPropertyDescriptionAsTooltip(typedProperty());
    part.trackState();

    part.funPaint = [this](const LC_PropertyPaintContext& paintContext, const LC_PropertyViewPart& p) {
        const auto painter = paintContext.painter;
        painter->save();
        const QColor linkColor = paintContext.getPalette().color(paintContext.getCurrentColorGroup(),
                                                                 paintContext.isActive ? QPalette::HighlightedText : QPalette::Link);

        if (p.isUnderCursor()) {
            auto font = painter->font();
            font.setUnderline(true);
            painter->setFont(font);
        }
        else if (p.isPushed()) {
            auto font = painter->font();
            font.setUnderline(true);
            painter->setFont(font);
        }
        if (paintContext.isActive) {
            painter->fillRect(paintContext.rect, paintContext.getPalette().color(QPalette::Highlight));
        }

        painter->setPen(linkColor);
        painter->drawText(p.rect, Qt::AlignLeading | Qt::AlignVCenter, m_title);
        painter->restore();
    };

    part.funHandleEvent = [this](LC_PropertyEventContext& eventContext, const LC_PropertyViewPart&, LC_PropertyEditContext*) -> bool {
        bool doClick = false;
        switch (eventContext.eventType()) {
            case QEvent::KeyPress: {
                const int key = eventContext.typedEvent<QKeyEvent>()->key();
                doClick = (key == Qt::Key_Space) || (key == Qt::Key_Return);
                break;
            }
            case LC_PropertyViewPartEvent::Activated: {
                m_cursorSet = true;
                m_widgetCursor = eventContext.sheet->cursor();
                eventContext.sheet->setCursor(Qt::PointingHandCursor);
                break;
            }
            case LC_PropertyViewPartEvent::Deactivated: {
                if (m_cursorSet) {
                    const auto sheet = eventContext.sheet;
                    if (!sheet->isInTreeRebuild()) {
                        sheet->setCursor(m_widgetCursor);
                    }
                }
                break;
            }
            case LC_PropertyViewPartEvent::ReleaseMouse: {
                doClick = true;
                break;
            }
            default:
                break;
        }
        if (doClick) {
            // ctx.m_sheet->setSkipNextMouseReleaseEvent();
            typedProperty().invokeClick();
            return false;
        }
        return false;
    };
    parts.append(part);
}
