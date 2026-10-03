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

#ifndef LC_ACTION_H
#define LC_ACTION_H

#include <QAction>
#include <QElapsedTimer>

namespace LC_ActionKeys {
    inline const char* PROP_DESCRIPTION = "lc_description";
    inline constexpr const char* PROP_READ_ONLY_SHORTCUT   = "_lc_read_only_shortcut";
    inline constexpr const char* PROP_DATA_ONLY_ACTION     = "_lc_data_only_action";
    inline constexpr const char* PROP_CUSTOM_SHORTCUT_TEXT = "_lc_custom_shortcut_text";

    inline bool isReadOnly(const QAction* a) {
        return a->property(PROP_READ_ONLY_SHORTCUT).toBool();
    }

    inline bool isReadOnlyAction(const QAction* a) {
        return a->property(PROP_DATA_ONLY_ACTION).toBool();
    }




}

class LC_Action: public QAction {
    Q_OBJECT
public:
    LC_Action() = default;

    LC_Action(const QString& text, QObject* const parent)
        : QAction(text, parent) {
    }

    LC_Action(const QIcon& icon, const QString& text, QObject* const parent)
        : QAction(icon, text, parent) {
    }

    LC_Action(QActionPrivate& dd, QObject* const parent)
        : QAction(dd, parent) {
    }

    bool isInvokedViaShortcut();

    QString description() const { return m_description; }
    void setDescription(const QString& desc) { m_description = desc; }

    QString getClearedText() const {
        return text().remove('&').trimmed();
    }

protected:
    bool event(QEvent*) override;
    QElapsedTimer m_elapsedTimer;
    QString m_description;
};

#endif
