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

#include "lc_search_line_edit.h"
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QStyle>
#include <QToolButton>

LC_SearchLineEdit::LC_SearchLineEdit(QWidget* parent)
    : QLineEdit(parent) {
    setPlaceholderText(tr("Search settings..."));
    setClearButtonEnabled(false);

    // Cache the platform/theme-native background color before we make any edits
    m_originalBaseColor = palette().color(QPalette::Base);

    QIcon searchIcon = QIcon(":/icons/setting_search.lci");
    m_historyAction = addAction(searchIcon, QLineEdit::LeadingPosition);
    m_historyAction->setToolTip(tr("Recent Searches (Alt+Down)"));
    connect(m_historyAction, &QAction::triggered, this, &LC_SearchLineEdit::showHistoryMenu);

    m_clearNormalIcon = QIcon(":/icons/lineedit_clear.lci");
    m_clearHoverIcon = QIcon(":/icons/lineedit_clear_hover.lci");

    m_clearAction = addAction(m_clearNormalIcon, QLineEdit::TrailingPosition);
    m_clearAction->setToolTip(tr("Clear Search (Esc)"));
    m_clearAction->setVisible(false);

    connect(m_clearAction, &QAction::triggered, this, &LC_SearchLineEdit::clearSearch);

    for (auto* btn : findChildren<QToolButton*>()) {
        if (btn->defaultAction() == m_clearAction) {
            m_clearButton = btn;
            m_clearButton->installEventFilter(this);
            break;
        }
    }

    // Toggle clear action visibility in real-time as text is typed
    connect(this, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_clearAction->setVisible(!text.isEmpty());
        if (text.isEmpty()) {
            setErrorState(false); // Reset error background if cleared
            m_clearAction->setIcon(m_clearNormalIcon);
        }
    });

    // Enter/Return commits the search immediately
    connect(this, &QLineEdit::returnPressed, this, &LC_SearchLineEdit::addCurrentTextToHistory);
}

bool LC_SearchLineEdit::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_clearButton && m_clearButton != nullptr) {
        if (event->type() == QEvent::Enter) {
            m_clearAction->setIcon(m_clearHoverIcon); // Swap to hover icon
        }
        else if (event->type() == QEvent::Leave) {
            m_clearAction->setIcon(m_clearNormalIcon); // Restore normal icon
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

void LC_SearchLineEdit::setMaxHistorySize(int size) {
    if (size > 0) {
        m_maxHistorySize = size;
        while (m_history.size() > m_maxHistorySize) {
            m_history.removeLast();
        }
    }
}

void LC_SearchLineEdit::setHistory(const QStringList& history) {
    m_history = history;

    // Sanitize the loaded history
    while (m_history.size() > m_maxHistorySize) {
        m_history.removeLast();
    }

    clear(); // Ensure search bar starts completely empty on show
}

void LC_SearchLineEdit::setErrorState(bool error) {
    m_isErrorState = error;
    QPalette pal = palette();

    if (error) {
        bool isDarkTheme = m_originalBaseColor.lightness() < 128;
        if (isDarkTheme) {
            pal.setColor(QPalette::Base, QColor(76, 20, 20)); // Soft, desaturated dark-red
        }
        else {
            pal.setColor(QPalette::Base, QColor(253, 232, 232)); // Soft, desaturated light-red
        }
    }
    else {
        pal.setColor(QPalette::Base, m_originalBaseColor); // Restore clean native background
    }

    setPalette(pal);
}

void LC_SearchLineEdit::addCurrentTextToHistory() {
    QString query = text().trimmed();

    constexpr int MIN_SEARCH_LENGTH = 3;
    // Prevent empty, short, or failed (error state) searches from entering the history
    if (query.length() < MIN_SEARCH_LENGTH || m_isErrorState)
        return;

    // Deduplicate: If query exists, remove old position to promote to top
    m_history.removeAll(query);
    m_history.prepend(query);

    // Cap history length
    while (m_history.size() > m_maxHistorySize) {
        m_history.removeLast();
    }

    emit historyChanged(m_history);
}

void LC_SearchLineEdit::clearSearch() {
    clear(); // Emits textChanged automatically, resetting parent dialog filters
}

void LC_SearchLineEdit::showHistoryMenu() {
    QMenu menu(this);

    if (m_history.isEmpty()) {
        auto* action = menu.addAction(tr("No recent searches"));
        action->setEnabled(false);
    }
    else {
        for (const QString& item : m_history) {
            menu.addAction(item);
        }
    }

    // Position the native menu cleanly below the QLineEdit frame
    QPoint pos = mapToGlobal(QPoint(0, height()));
    QAction* selected = menu.exec(pos);

    if (selected && selected->isEnabled()) {
        setText(selected->text());
    }
}

void LC_SearchLineEdit::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Down:
            if (event->modifiers() & Qt::AltModifier) {
                showHistoryMenu();
                event->accept();
                return;
            }
            break;

        case Qt::Key_Escape:
            if (!text().isEmpty()) {
                clearSearch();
                event->accept();
                return;
            }
            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit returnPressed();
            event->accept();
            return;

        default:
            break;
    }
    QLineEdit::keyPressEvent(event);
}

void LC_SearchLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    addCurrentTextToHistory();
}
