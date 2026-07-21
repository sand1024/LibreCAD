
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

#ifndef LC_ICON_LABEL_H
#define LC_ICON_LABEL_H

#include <QWidget>
#include <QIcon>
#include <QSize>

class LC_IconLabel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QSize fallbackIconSize READ fallbackIconSize WRITE setFallbackIconSize)

signals:
    void clicked();

public:
    explicit LC_IconLabel(QWidget *parent = nullptr);
    LC_IconLabel(const QIcon &icon, QWidget *parent = nullptr);
    ~LC_IconLabel() override = default;

    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon &icon);

    QSize fallbackIconSize() const { return m_fallbackIconSize; }
    void setFallbackIconSize(const QSize &size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool event(QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QSize getBaseIconSize() const;

private:
    QIcon m_icon;
    QSize m_fallbackIconSize;

    bool m_isHovered = false;
    bool m_isPressed = false;
};

#endif
