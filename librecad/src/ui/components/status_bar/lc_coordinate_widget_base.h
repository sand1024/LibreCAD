
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

#ifndef LC_COORDINATEWIDGETBASE_H
#define LC_COORDINATEWIDGETBASE_H

#include <functional>
#include <QIcon>
#include <QList>
#include <QWidget>

#include "lc_graphicviewaware.h"
#include "rs_vector.h"

class QAction;
class QLabel;
class QToolButton;
class LC_Formatter;
class LC_GraphicViewport;
class RS_Graphic;

struct LC_CoordinateField {
    QLabel* m_label = nullptr;
    QString m_description;
    std::function<RS_Vector()> m_getter;
};

class LC_CoordinateWidgetBase : public QWidget, public LC_GraphicViewAware {
    Q_OBJECT
public:
    explicit LC_CoordinateWidgetBase(QWidget* parent = nullptr, Qt::WindowFlags fl = {});
    ~LC_CoordinateWidgetBase() override = default;

    bool isLocked() const;
    void setLocked(bool locked);

    QAction* actionLock() const;
    QAction* actionUnlock() const;

    virtual void onInsertToCmd(const RS_Vector& coord, const QString& text);

    bool eventFilter(QObject* watched, QEvent* event) override;
    signals:
        void lockToggled(bool locked);
        void insertToCmdRequested(const RS_Vector& coord, const QString& text);
protected:
    void initLockControl(QToolButton* button);
    void registerCoordinateField(QLabel* label, const QString& description, std::function<RS_Vector()> getter);
    void updateLockVisuals();


    void showContextMenu(const QPoint& globalPos, QWidget* sourceWidget);

    void contextMenuEvent(QContextMenuEvent* event) override;
    void copyToClipboard(const QString& text) const;

    bool m_isLocked = false;
    QAction* m_actionLock = nullptr;
    QAction* m_actionUnlock = nullptr;
    QToolButton* m_lockButton = nullptr;
    QIcon m_iconLocked;
    QIcon m_iconUnlocked;

    QList<LC_CoordinateField> m_coordinateFields;

    RS_Graphic* m_graphic = nullptr;
    RS_GraphicView* m_graphicView = nullptr;
    LC_GraphicViewport* m_viewport = nullptr;
    LC_Formatter* m_formatter = nullptr;

private:
    void createActions();
};

#endif
