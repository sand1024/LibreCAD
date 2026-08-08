
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

#ifndef LC_SEARCH_LINE_EDIT_H
#define LC_SEARCH_LINE_EDIT_H

#include <QLineEdit>
#include <QStringList>

class QAction;

class LC_SearchLineEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY(int maxHistorySize READ maxHistorySize WRITE setMaxHistorySize)
    Q_PROPERTY(QStringList history READ history WRITE setHistory)

public:
    explicit LC_SearchLineEdit(QWidget* parent = nullptr);
    bool eventFilter(QObject* watched, QEvent* event) override;
    ~LC_SearchLineEdit() override = default;

    int maxHistorySize() const { return m_maxHistorySize; }
    void setMaxHistorySize(int size);

    QStringList history() const { return m_history; }
    void setHistory(const QStringList& history);

    void setErrorState(bool error);
    bool isErrorState() const { return m_isErrorState; }

public slots:
    void addCurrentTextToHistory();
    void clearSearch();

signals:
    // Emitted whenever a search is successfully committed to memory
    void historyChanged(const QStringList& history);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void showHistoryMenu();

private:
    QStringList m_history;
    int m_maxHistorySize = 10;
    bool m_isErrorState = false;
    QColor m_originalBaseColor;

    QAction* m_historyAction = nullptr;
    QAction* m_clearAction = nullptr;

    QToolButton* m_clearButton = nullptr;
    QIcon m_clearNormalIcon;
    QIcon m_clearHoverIcon;
};
#endif
