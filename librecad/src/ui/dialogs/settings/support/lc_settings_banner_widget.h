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

#ifndef LC_SETTINGS_BANNER_WIDGET_H
#define LC_SETTINGS_BANNER_WIDGET_H

#include <QWidget>
#include <functional>

class QLabel;
class QPushButton;

class LC_SettingsBannerWidget : public QWidget {
    Q_OBJECT
public:
    explicit LC_SettingsBannerWidget(QWidget* parent = nullptr);
    ~LC_SettingsBannerWidget() override = default;

    void setBanner(const QString& message,
                   const QString& actionText = QString(),
                   std::function<void()> actionCallback = nullptr);

    void clearAction();
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    QLabel* m_lblIcon = nullptr;
    QLabel* m_lblMessage = nullptr;
    QPushButton* m_btnAction = nullptr;
    std::function<void()> m_actionCallback;
};

#endif
