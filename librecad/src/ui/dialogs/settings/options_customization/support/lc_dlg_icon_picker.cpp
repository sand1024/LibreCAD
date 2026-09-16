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

#include "lc_dlg_icon_picker.h"
#include "ui_lc_dlg_icon_picker.h"

#include <QDirIterator>
#include <QFileInfo>

namespace {
    QStringList s_cachedIconPaths;
}

const QStringList& LC_DlgIconPicker::availableIcons() {
    if (s_cachedIconPaths.isEmpty()) {
        QDirIterator it(":/icons", QStringList() << "*.lci" << "*.svg" << "*.png",
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            s_cachedIconPaths.append(it.next());
        }
        s_cachedIconPaths.sort();
    }
    return s_cachedIconPaths;
}

LC_DlgIconPicker::LC_DlgIconPicker(QWidget* parent, const QString& currentIcon)
    : QDialog(parent)
    , ui(std::make_unique<Ui::LC_DlgIconPicker>())
    , m_selectedIcon(currentIcon) {
    ui->setupUi(this);

    ui->lwIcons->setResizeMode(QListView::Adjust);

    connect(ui->leFilter, &QLineEdit::textChanged, this, &LC_DlgIconPicker::onFilterChanged);
    connect(ui->lwIcons, &QListWidget::itemSelectionChanged, this, &LC_DlgIconPicker::onItemSelectionChanged);
    connect(ui->lwIcons, &QListWidget::itemDoubleClicked, this, &LC_DlgIconPicker::onItemDoubleClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    populateIcons();
}

LC_DlgIconPicker::~LC_DlgIconPicker() = default;

void LC_DlgIconPicker::populateIcons() {
    ui->lwIcons->clear();
    const auto& paths = availableIcons();

    QListWidgetItem* initialItem = nullptr;
    for (const auto& path : paths) {
        const QString baseName = QFileInfo(path).baseName();
        auto* item = new QListWidgetItem(QIcon(path), "", ui->lwIcons);
        item->setToolTip(baseName);
        item->setData(Qt::UserRole, path);

        if (path == m_selectedIcon) {
            initialItem = item;
        }
    }

    if (initialItem != nullptr) {
        ui->lwIcons->setCurrentItem(initialItem);
        m_selectedIcon = initialItem->data(Qt::UserRole).toString();
        ui->lblSelectedPath->setText(tr("Selected: %1").arg(m_selectedIcon));
    }
}

void LC_DlgIconPicker::onFilterChanged(const QString& filter) {
    const QString term = filter.trimmed();
    const int count = ui->lwIcons->count();

    for (int i = 0; i < count; ++i) {
        auto* item = ui->lwIcons->item(i);
        if (item != nullptr) {
            const QString path = item->data(Qt::UserRole).toString();
            const bool matches = term.isEmpty() || path.contains(term, Qt::CaseInsensitive);
            item->setHidden(!matches);
        }
    }
}

void LC_DlgIconPicker::onItemSelectionChanged() {
    const auto* item = ui->lwIcons->currentItem();
    if (item != nullptr) {
        m_selectedIcon = item->data(Qt::UserRole).toString();
        ui->lblSelectedPath->setText(tr("Selected: %1").arg(m_selectedIcon));
    }
}

void LC_DlgIconPicker::onItemDoubleClicked(QListWidgetItem* item) {
    if (item != nullptr) {
        m_selectedIcon = item->data(Qt::UserRole).toString();
        accept();
    }
}

QString LC_DlgIconPicker::selectedIconPath() const {
    return m_selectedIcon;
}

QString LC_DlgIconPicker::getIcon(QWidget* parent, const QString& currentIcon) {
    LC_DlgIconPicker dlg(parent, currentIcon);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.selectedIconPath();
    }
    return QString();
}

void LC_DlgIconPicker::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    auto* item = ui->lwIcons->currentItem();
    if (item != nullptr) {
        ui->lwIcons->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}
