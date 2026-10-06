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

#include "lc_app_window_tool_bar_manager.h"

#include <QAction>
#include <QLayout>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolButton>

#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "qc_applicationwindow.h"

namespace ToolBarConstants {
    constexpr int TOOLBAR_BUDGET_PADDING = 48;
}

LC_AppWindowToolBarManager::LC_AppWindowToolBarManager(QC_ApplicationWindow* appWin)
    : QObject(appWin), m_appWin(appWin) {
}

void LC_AppWindowToolBarManager::initializeToolBarAreas() {
    if (m_toolBarLayoutInitialized || m_appWin == nullptr) {
        return;
    }
    m_toolBarLayoutInitialized = true;

    // Top, Bottom, and Left (Categories) toolbars are enabled by default; Right is disabled
    m_requestedToolBarAreas.insert(int(Qt::TopToolBarArea), true);
    m_requestedToolBarAreas.insert(int(Qt::BottomToolBarArea), true);
    m_requestedToolBarAreas.insert(int(Qt::LeftToolBarArea), true);
    m_requestedToolBarAreas.insert(int(Qt::RightToolBarArea), false);

    m_toolbarAreasToggleActions.left = m_appWin->getAction(QStringLiteral("LeftTBAreaToggle"));
    m_toolbarAreasToggleActions.right = m_appWin->getAction(QStringLiteral("RightTBAreaToggle"));
    m_toolbarAreasToggleActions.top = m_appWin->getAction(QStringLiteral("TopTBAreaToggle"));
    m_toolbarAreasToggleActions.bottom = m_appWin->getAction(QStringLiteral("BottomTBAreaToggle"));

    updateToolBarAreaActions();
}

void LC_AppWindowToolBarManager::setToolBarAreaRequested(Qt::ToolBarArea area, bool enable) {
    if (m_appWin == nullptr) {
        return;
    }

    if (enable) {
        // 2. When enabling the area, restore only toolbars that were explicitly open in the snapshot
        for (QToolBar* toolbar : m_appWin->findChildren<QToolBar*>()) {
            if (toolbar != nullptr && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == area) {
                const QString name = toolbar->objectName();
                if (m_preCloseToolBarVisibility.contains(name)) {
                    toolbar->setVisible(m_preCloseToolBarVisibility.value(name));
                }
            }
        }
    }
    else {
        // 1. Snapshot visible toolbars in this area before disabling
        for (QToolBar* toolbar : m_appWin->findChildren<QToolBar*>()) {
            if (toolbar != nullptr && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == area) {
                m_preCloseToolBarVisibility[toolbar->objectName()] = !toolbar->isHidden();
            }
        }
        for (QToolBar* toolbar : m_appWin->findChildren<QToolBar*>()) {
            if (toolbar != nullptr && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == area) {
                toolbar->setVisible(false);
            }
        }
    }

    m_requestedToolBarAreas[int(area)] = enable;
    updateToolBarAreaActions();
}

bool LC_AppWindowToolBarManager::isToolBarAreaRequested(Qt::ToolBarArea area) const {
    return m_requestedToolBarAreas.value(int(area), true);
}

void LC_AppWindowToolBarManager::toggleLeftToolBarArea(bool state) {
    setToolBarAreaRequested(Qt::LeftToolBarArea, state);
}

void LC_AppWindowToolBarManager::toggleRightToolBarArea(bool state) {
    setToolBarAreaRequested(Qt::RightToolBarArea, state);
}

void LC_AppWindowToolBarManager::toggleTopToolBarArea(bool state) {
    setToolBarAreaRequested(Qt::TopToolBarArea, state);
}

void LC_AppWindowToolBarManager::toggleBottomToolBarArea(bool state) {
    setToolBarAreaRequested(Qt::BottomToolBarArea, state);
}

void LC_AppWindowToolBarManager::updateToolBarAreaActions() {
    if (m_appWin == nullptr) {
        return;
    }

    const auto setChecked = [](QAction* action, bool checked) {
        if (action != nullptr) {
            const QSignalBlocker blocker(action);
            action->setChecked(checked);
        }
    };

    const auto anyShown = [this](Qt::ToolBarArea area) -> bool {
        for (QToolBar* toolbar : m_appWin->findChildren<QToolBar*>()) {
            if (toolbar != nullptr && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == area && !toolbar->isHidden()) {
                return true;
            }
        }
        return false;
    };

    setChecked(m_toolbarAreasToggleActions.left, anyShown(Qt::LeftToolBarArea));
    setChecked(m_toolbarAreasToggleActions.right, anyShown(Qt::RightToolBarArea));
    setChecked(m_toolbarAreasToggleActions.top, anyShown(Qt::TopToolBarArea));
    setChecked(m_toolbarAreasToggleActions.bottom, anyShown(Qt::BottomToolBarArea));
}


void LC_AppWindowToolBarManager::reflowToolBarArea(Qt::ToolBarArea area, int budget) {
    if (m_appWin == nullptr || m_appWin->layout() == nullptr) {
        return;
    }

    // 1. Remove auto breaks previously inserted in this specific area
    for (auto it = m_autoToolbarBreaks.begin(); it != m_autoToolbarBreaks.end(); ) {
        QToolBar* toolbar = m_appWin->findChild<QToolBar*>(*it);
        if (toolbar != nullptr && m_appWin->toolBarArea(toolbar) == area) {
            m_appWin->removeToolBarBreak(toolbar);
            it = m_autoToolbarBreaks.erase(it);
        }
        else {
            ++it;
        }
    }

    // 2. Collect toolbars in layout order
    QList<QToolBar*> toolbars;
    const int itemCount = m_appWin->layout()->count();
    for (int i = 0; i < itemCount; ++i) {
        auto* item = m_appWin->layout()->itemAt(i);
        auto* toolbar = item != nullptr ? qobject_cast<QToolBar*>(item->widget()) : nullptr;
        if (toolbar != nullptr && !toolbar->isHidden() && !toolbar->isFloating() && m_appWin->toolBarArea(toolbar) == area) {
            toolbars.append(toolbar);
        }
    }

    // 3. Determine wrapping along area orientation
    const bool isHorizontal = (area == Qt::TopToolBarArea || area == Qt::BottomToolBarArea);
    const int safeBudget = qMax(1, budget);
    int currentLength = 0;

    for (QToolBar* toolbar : std::as_const(toolbars)) {
        if (m_appWin->toolBarBreak(toolbar)) {
            currentLength = 0;
        }
        const QSize sizeHint = toolbar->minimumSizeHint();
        const int length = isHorizontal ? sizeHint.width() : sizeHint.height();

        if (currentLength > 0 && currentLength + length > safeBudget) {
            m_appWin->insertToolBarBreak(toolbar);
            m_autoToolbarBreaks.insert(toolbar->objectName());
            currentLength = 0;
        }
        currentLength += length;
    }
}

void LC_AppWindowToolBarManager::reflowToolBars(int availableWidth, int availableHeight) {
    const int availWidth = availableWidth - ToolBarConstants::TOOLBAR_BUDGET_PADDING;
    const int availHeight = availableHeight - ToolBarConstants::TOOLBAR_BUDGET_PADDING;

    reflowToolBarArea(Qt::TopToolBarArea, availWidth);
    reflowToolBarArea(Qt::BottomToolBarArea, availWidth);
    reflowToolBarArea(Qt::LeftToolBarArea, availHeight);
    reflowToolBarArea(Qt::RightToolBarArea, availHeight);
}

void LC_AppWindowToolBarManager::updateToolbarsIconSize() {
    using namespace CFG_Widgets;
    updateToolbarsIconSize(o_ToolbarAllowIconSize, o_ToolbarIconSize);
}

void LC_AppWindowToolBarManager::updateToolbarsIconSize(bool allowCustom, int customSize) {
    if (m_appWin == nullptr) {
        return;
    }

    QSize targetSize;
    if (allowCustom && customSize > 0) {
        targetSize = QSize(customSize, customSize);
    }
    else {
        const int defSz = m_appWin->style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, m_appWin);
        targetSize = QSize(defSz, defSz);
    }

    m_appWin->setIconSize(targetSize);

    for (auto* tb : m_appWin->findChildren<QToolBar*>()) {
        if (tb != nullptr) {
            tb->setIconSize(targetSize);
            for (auto* btn : tb->findChildren<QToolButton*>()) {
                if (btn != nullptr) {
                    btn->setIconSize(targetSize);
                }
            }
        }
    }
}

void LC_AppWindowToolBarManager::updateToolbarsTooltips(bool show) {
    if (m_appWin == nullptr) {
        return;
    }

    for (auto* tb : m_appWin->findChildren<QToolBar*>()) {
        if (tb == nullptr) {
            continue;
        }
        if (show) {
            tb->setToolTip(QObject::tr("Toolbar: %1").arg(tb->windowTitle()));
        }
        else {
            tb->setToolTip(QString());
        }
    }
}

void LC_AppWindowToolBarManager::updateToolbarsTooltips() {
    const bool show = CFG_Startup::o_ShowToolbarsTooltip;
    updateToolbarsTooltips(show);
}

void LC_AppWindowToolBarManager::resetLayoutToDefault() {
    if (m_appWin == nullptr) {
        return;
    }

    // 1. Clear area snapshots and auto breaks
    m_preCloseToolBarVisibility.clear();
    m_autoToolbarBreaks.clear();

    // 2. Reset requested areas to default:
    // Top, Bottom, and Left = true; Right = false
    m_requestedToolBarAreas[int(Qt::TopToolBarArea)] = true;
    m_requestedToolBarAreas[int(Qt::BottomToolBarArea)] = true;
    m_requestedToolBarAreas[int(Qt::LeftToolBarArea)] = true;
    m_requestedToolBarAreas[int(Qt::RightToolBarArea)] = false;

    // 3. Reset toolbar icon sizes to settings default
    updateToolbarsIconSize();

    // 4. Update checkmarks on toggle actions
    updateToolBarAreaActions();
}
