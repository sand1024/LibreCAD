
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

#ifndef LC_DLG_CHEATSHEET_OPTIONS_H
#define LC_DLG_CHEATSHEET_OPTIONS_H

#include <QDialog>
#include <memory>
#include "lc_cheatsheet_options.h"
#include "lc_dialog.h"

namespace Ui {
    class LC_DlgCheatsheetOptions;
}

class LC_DlgCheatsheetOptions : public LC_Dialog {
    Q_OBJECT
public:
    const QString getDefaultFileName();
    explicit LC_DlgCheatsheetOptions(QWidget* parent, LC_CheatsheetOptions& options);
    ~LC_DlgCheatsheetOptions() override;

private slots:
    void onDestinationChanged();
    void onBrowsePdfClicked();
    void validateAndAccept();

private:
    std::unique_ptr<Ui::LC_DlgCheatsheetOptions> ui;
    LC_CheatsheetOptions& m_options;
};

#endif
