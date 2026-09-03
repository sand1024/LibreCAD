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

#ifndef LC_PALETTE_PREVIEW_WINDOW_H
#define LC_PALETTE_PREVIEW_WINDOW_H

#include <QMainWindow>

#include "lc_dockwidget.h"

class QListWidgetItem;
class QTreeWidgetItem;
class QTableWidgetItem;
class QGridLayout;
QT_BEGIN_NAMESPACE
namespace Ui { class LC_SkinPreviewWindow; }
QT_END_NAMESPACE

class LC_SkinPreviewWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LC_SkinPreviewWindow(QWidget *parent = nullptr);
    ~LC_SkinPreviewWindow() override;
    void applyToolbarsAndDocksConfig();
    void updateSemanticViews(const QColor& searchHighlightColor, const QColor& conflictingItemColor);
    void activateTab(const QString& tag);
signals:
        void windowClosed(); // Notifies the parent dialog to uncheck the toggle button (Note 3)

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::LC_SkinPreviewWindow *ui;

    QList<QToolBar*> m_mainToolBars;
    QList<QToolButton*> m_dockToolButtons;
    QList<QToolButton*> m_matrixToolButtons;
    QWidget* m_matrixGridContainer = nullptr;
    QGridLayout* m_matrixGridLayout = nullptr;
    QList<LC_DockWidget*> m_allDockWidgets;
    QTableWidgetItem* m_tableSearchItem = nullptr;
    QTableWidgetItem* m_tableConflictItem = nullptr;
    QTreeWidgetItem*  m_treeSearchItem = nullptr;
    QTreeWidgetItem*  m_treeConflictItem = nullptr;
    QListWidgetItem*  m_listSearchItem = nullptr;
    QListWidgetItem*  m_listConflictItem = nullptr;

    QWidget* setupDockContent(LC_DockWidget* propertiesDock, bool addTopToolbar);
    QWidget* setupCadToolsMatrixContent(LC_DockWidget* matrixDock);
    void setupScrollArea() const;
    void setupMenuBar();
    void setupToolbars();
    void setupDockWidget();
    void setupAdvancedViewsWidgets() const;
    void setupEditors() const;
    void setupToolButtonMenu() const;
    void setupTabPreviews();
    void setupStatusPillToolbars();
    void hideSliderAndDialControls() const;
};

#endif
