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

#ifndef LC_COLOR_BUTTON_H
#define LC_COLOR_BUTTON_H

#include <QPushButton>
#include <QColor>

class LC_ColorButton : public QPushButton {
    Q_OBJECT public:
    explicit LC_ColorButton(const QColor& color = Qt::white, QWidget* parent = nullptr);
    explicit LC_ColorButton(QWidget* parent = nullptr);
    QColor color() const;
    void setColor(const QColor& color);
    void setLocked(bool locked);
    void setToolTip(const QString& text);

    bool isLocked() const {
        return m_locked;
    }

    void setLockedToolTip(const QString& toolTip);

    QString lockedToolTip() const {
        return m_lockedToolTip;
    }

    void setDialogTitle(const QString& title);

    QString dialogTitle() const {
        return m_dialogTitle;
    }

signals:
    void colorChanged(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void chooseColor();

private:
    void updateToolTip();
    bool m_locked = false;
    void updateSwatch();
    QColor m_color;
    QString m_normalToolTip;
    QString m_lockedToolTip;
    QString m_dialogTitle;
};

#endif
