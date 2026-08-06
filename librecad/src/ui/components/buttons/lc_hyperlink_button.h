
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

#ifndef LC_HYPERLINK_BUTTON_H
#define LC_HYPERLINK_BUTTON_H

#include <QLabel>
#include <QMouseEvent>

class LC_HyperlinkButton : public QLabel {
    Q_OBJECT
public:
    LC_HyperlinkButton(const QString& text, const QString& targetPageId, QWidget* parent = nullptr)
        : QLabel(text, parent)
        , m_targetPageId(targetPageId) {

        setCursor(Qt::PointingHandCursor);

        // Styling the link using standard QPalette values
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, pal.color(QPalette::Link));
        setPalette(pal);

        QFont f = font();
        f.setUnderline(true);
        setFont(f);
    }

    signals:
        void clicked(const QString& targetPageId);

protected:
    bool event(QEvent* ev) override {
        if (ev->type() == QEvent::Enter) {
            QFont f = font();
            f.setUnderline(true);
            setFont(f);
        } else if (ev->type() == QEvent::Leave) {
            QFont f = font();
            f.setUnderline(false);
            setFont(f);
        }
        return QLabel::event(ev);
    }
    
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
            emit clicked(m_targetPageId);
        }
        QLabel::mouseReleaseEvent(event);
    }

private:
    QString m_targetPageId;
};

#endif
