
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

#ifndef LC_DLG_ACTION_GROUP_CONFIG_H
#define LC_DLG_ACTION_GROUP_CONFIG_H

#include <QDialog>
#include <memory>
#include "lc_action_node.h"

class LC_ActionNamingServiceInterface;

namespace Ui {
    class LC_DlgActionGroupConfig;
}

class LC_DlgActionGroupConfig : public QDialog {
    Q_OBJECT
public:
    explicit LC_DlgActionGroupConfig(QWidget* parent = nullptr, bool isToolbarMode = true,
                                     const LC_ActionNamingServiceInterface* namingService = nullptr);
    ~LC_DlgActionGroupConfig() override;

    void setInitialValues(const QString& rawTitle, const QString& iconPath, ToolButtonPopupMode mode);

    QString title() const;
    QString iconPath() const;
    ToolButtonPopupMode popupMode() const;

private slots:
    void onGroupTypeToggled();
    void onSystemCategoryChanged(int index);
    void onChooseIconClicked();
    void onSplitToggled(bool checked);
    void validateAndAccept();

private:
    void populateSystemCategories();

    std::unique_ptr<Ui::LC_DlgActionGroupConfig> ui;
    bool m_isToolbarMode{true};
    QString m_iconPath;
    const LC_ActionNamingServiceInterface* m_namingService{nullptr};
};


#endif
