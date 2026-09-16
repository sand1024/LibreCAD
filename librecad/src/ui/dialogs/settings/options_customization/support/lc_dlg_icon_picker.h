
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

#ifndef LC_DLG_ICON_PICKER_H
#define LC_DLG_ICON_PICKER_H

#include <QDialog>
#include <QStringList>
#include <memory>

namespace Ui {
    class LC_DlgIconPicker;
}

class QListWidgetItem;

class LC_DlgIconPicker : public QDialog {
    Q_OBJECT
public:
    explicit LC_DlgIconPicker(QWidget* parent = nullptr, const QString& currentIcon = QString());
    ~LC_DlgIconPicker() override;

    QString selectedIconPath() const;

    static QString getIcon(QWidget* parent, const QString& currentIcon = QString());
protected:
    void showEvent(QShowEvent* event) override;
private slots:
    void onFilterChanged(const QString& filter);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void populateIcons();
    static const QStringList& availableIcons();

    std::unique_ptr<Ui::LC_DlgIconPicker> ui;
    QString m_selectedIcon;
};

#endif
