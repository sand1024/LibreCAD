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

#include "lc_proxy_style.h"
#include "lc_proxy_style.h"
#include "lc_proxy_style.h"
#include <algorithm> // for qBound

#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAbstractScrollArea>
#include <QAbstractSpinBox>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QApplication>
#include <QCheckBox>
#include <QCheckBox>
#include <QComboBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDockWidget>
#include <QEvent>
#include <QFontComboBox>
#include <QIconEngine>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionDockWidget>
#include <QStyleOptionHeader>
#include <QStyleOptionProgressBar>
#include <QStyleOptionSlider>
#include <QStyleOptionTab>
#include <QStyleOptionToolBar>
#include <QTabBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QTreeView>
#include <QVariant>

#include "lc_custom_title_bar_widget.h"
#include "lc_dock_title_bar.h"
#include "lc_event_filter_auto_popup_controller.h"
#include "lc_event_filter_floating_hud.h"
#include "lc_event_filter_mnemonic.h"
#include "lc_event_filter_tool_tip.h"
#include "lc_event_filter_win_32_window_cloaking.h"
#include "lc_mouse_tracking_table_view.h"
#include "lc_skin_widgets_layout_resolver.h"
#include "rs_debug.h"

#ifdef Q_OS_WIN
#include <windows.h>
typedef HRESULT (WINAPI *DwmSetWindowAttributePtr)(HWND, DWORD, LPCVOID, DWORD);
#endif

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <windowsx.h>

class LC_WinResizeNativeEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        if (eventType == "windows_generic_MSG") {
            const MSG *msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST) {
                const HWND hwnd = msg->hwnd;
                QWidget *w = QWidget::find(reinterpret_cast<WId>(hwnd));

                // Safely verifies whether target is the QDockWidget or its platform container wrapper
                if (w && (qobject_cast<QDockWidget*>(w) || w->findChild<QDockWidget*>())) {
                    RECT winRect;
                    ::GetWindowRect(hwnd, &winRect);
                    const int x = GET_X_LPARAM(msg->lParam);
                    const int y = GET_Y_LPARAM(msg->lParam);
                    const int border = 8; // Expanded comfortable hit zone

                    const bool left   = (x < winRect.left   + border);
                    const bool right  = (x > winRect.right  - border);
                    const bool top    = (y < winRect.top    + border);
                    const bool bottom = (y > winRect.bottom - border);

                    if (left  && top)    { *result = HTTOPLEFT;     return true; }
                    if (right && top)    { *result = HTTOPRIGHT;    return true; }
                    if (left  && bottom) { *result = HTBOTTOMLEFT;  return true; }
                    if (right && bottom) { *result = HTBOTTOMRIGHT; return true; }
                    if (left)            { *result = HTLEFT;        return true; }
                    if (right)           { *result = HTRIGHT;       return true; }
                    if (bottom)          { *result = HTBOTTOM;      return true; }
                }
            }
        }
        return false;
    }
};
#endif

class QTreeView;

namespace DEBUG_PROXY {
    QElapsedTimer g_globalPerfTimer;
    bool g_perfTimerStarted = false;
}

namespace {
    thread_local bool g_inSegmentedDraw = false;

   // Style Engine Internal Properties
    constexpr auto PROP_CACHED_HIGHLIGHT_ROW     = "lcfs_cachedHighlightRow";

    constexpr auto PROP_STYLE_FILTER_INSTALLED   = "lcfs_styleEventFilterInstalled";

    constexpr auto PROP_IS_WRAPPED               = "lcfs_isWrapped";
    constexpr auto PROP_HAS_CMD_ALIASES          = "lcfs_hasCmdAliases";
    constexpr auto PROP_MAX_CMD_WIDTH            = "lcfs_maxCmdWidth";
    constexpr auto PROP_MAX_SHORTCUT_WIDTH       = "lcfs_maxShortcutWidth";


    // LibreCAD Core / Standard Qt Properties
    constexpr auto PROP_DO_HOVER_ROW             = "_doHoverRow";

    constexpr auto PROP_CMD_LINE                 = "cmdLine";

    constexpr auto PROP_SPINBOX_PTR              = "lcfs_spinbox_ptr";
    constexpr auto PROP_SPINBOX_VALUE            = "value";
    constexpr auto PROP_SPINBOX_MIN              = "minimum";
    constexpr auto PROP_SPINBOX_MAX              = "maximum";

#ifdef Q_OS_WIN
    // Thread-safe static resolver to cache dwmapi.dll exactly once globally
    static DwmSetWindowAttributePtr getDwmSetWindowAttribute() {
        static DwmSetWindowAttributePtr ptr = nullptr;
        static bool resolved = false;
        if (!resolved) {
            const HMODULE dwmDll = ::LoadLibraryW(L"dwmapi.dll");
            if (dwmDll) {
                ptr = reinterpret_cast<DwmSetWindowAttributePtr>(
                    ::GetProcAddress(dwmDll, "DwmSetWindowAttribute"));
            }
            resolved = true;
        }
        return ptr;
    }


#endif

    class LCPainterGuard {
    public:
        explicit LCPainterGuard(QPainter *p, const bool antiAlias = true) : m_painter(p) {
            m_painter->save();
            m_painter->setRenderHint(QPainter::Antialiasing, antiAlias);
        }
        ~LCPainterGuard() {
            m_painter->restore();
        }
    private:
        QPainter *m_painter;
    };



    inline QRectF crispRect(const QRect &rect) {
        return QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    }


    inline QRectF crispRect(const QRect &rect, const qreal crispOffset) {
        return QRectF(rect).adjusted(crispOffset, crispOffset, -crispOffset, -crispOffset);
    }


    QString cleanMnemonic(const QString &text) {
        QString cleaned = text;
        cleaned.remove('&');
        return cleaned.trimmed();
    }


    QAction* findActionForOption(const QStyleOptionMenuItem *option, const QWidget *widget) {
        if (const auto *menu = qobject_cast<const QMenu*>(widget)) {
            for (QAction *action : menu->actions()) {
                if (menu->actionGeometry(action) == option->rect) {
                    return action;
                }
            }
        }
        return nullptr;
    }

}

// ================= CONSTRUCTORS & CACHE ENGINE COHESION =================

LC_ProxyStyle::LC_ProxyStyle(QStyle *baseStyle, const StyleMetricsConfig &metrics)
    : QProxyStyle(baseStyle), m_metrics(metrics) {
    m_scaledGeometryProvider.setBaseMetrics(metrics);
    setMetrics(metrics);
}

void LC_ProxyStyle::doSetStyleArtefact(const StyleArchetype archetype) {
    m_styleArchetype = archetype;

    // Symmetrical calculation of internal fast-drawing flags
    m_isFlat = (archetype == StyleArchetype::FlatModern || archetype == StyleArchetype::AccentOutline);
    m_isClassic = (archetype == StyleArchetype::ClassicFusion);
    m_isFlatModern = (archetype == StyleArchetype::FlatModern);
    m_isGlossy = (archetype == StyleArchetype::GlassyGloss);
    m_isSoftSatin = (archetype == StyleArchetype::SoftSatin);
    m_isAccentOnline = (archetype == StyleArchetype::AccentOutline);
}

void LC_ProxyStyle::setSkin(const SkinConfig& skin) {
    doSetStyleArtefact(skin.styleArchetype);
    m_boxDecoration         = skin.boxDecoration;
    m_customDockTitleBar    = skin.customDockTitleBar;
    m_dockTitleBarStyle     = skin.dockTitleBarStyle;
    m_accentedScrollbars    = skin.accentedScrollbars;
    m_transparentScrollbars = skin.transparentScrollbars;
    m_customGroupBoxBar     = skin.customGroupBoxBar;
    m_groupBoxHeaderStyle   = skin.groupBoxHeaderStyle;
    m_groupBoxBoundaryStyle = skin.groupBoxBoundaryStyle;
    m_groupBoxUseAccent      = skin.groupBoxUseAccent;
    m_showActiveRowSpotlight = skin.showActiveRowSpotlight;
    m_showItemViewHover      = skin.showItemViewHover;
    m_showTreeConnectingLines = skin.showTreeConnectingLines;
    m_branchIndicatorStyle = skin.branchIndicatorStyle;
    m_useFocusedInputGlow = skin.useFocusedInputGlow;

    m_customSplitterAnToolbarGrip = skin.customSplitterGrip;
    m_splitterGripStyle = skin.splitterGripStyle;
    m_highlightSplitterOnDrag = skin.highlightSplitterOnDrag;
    m_showGripBackgroundWell = skin.showGripBackgroundWell;
    m_accentGrips = skin.accentGrips;
    m_persistentDockSplitter = skin.persistentDockSplitter;
    m_useToolButtonUnderline = skin.useToolButtonUnderline;
    m_toolButtonIndicatorStyle = skin.toolButtonIndicatorStyle;
    m_customToolTipCard = skin.customToolTipCard;
    m_customVectorIconsInButtons = skin.customVectorIconsInButtons;
    m_tabStripeAtBottom = skin.tabStripeAtBottom;

    m_customToolbarOverflowGrip = skin.customToolbarOverflowGrip;
    m_autoPopupToolbarOverflow = skin.autoPopupToolbarOverflow;
    m_autoPopupInstantButtons = skin.autoPopupInstantButtons;
    m_autoPopupMenuBar = skin.autoPopupMenuBar;
    m_useMenuBarHoverCard = skin.useMenuBarHoverCard;
    m_showMenuCommandAliases = skin.showMenuCommandAliases;

    m_useSegmentedToolButtons = skin.useSegmentedToolButtons;
    m_segmentedSeparationStyle = skin.segmentedSeparationStyle;
    m_segmentedColorPolicy = skin.segmentedColorPolicy;

    m_useSpinBoxProgressBar = skin.useSpinBoxProgressBar;
    m_useStatusPillChips = skin.useStatusPillChips;

    // Forward skin variables directly to the descriptor cache
    m_skinColorsResolver.setSkin(skin);
    m_useFloatingHUD          = skin.useFloatingHUD;
    m_closeButtonColorPolicy  = skin.closeButtonColorPolicy;

    for (QWidget *widget : QApplication::allWidgets()) {
        if (auto *tb = qobject_cast<QToolBar*>(widget)) {
            // Recalculate toolbar geometry dynamically via sizes returned by sizeFromContents()
            if (m_customToolbarOverflowGrip && m_autoPopupToolbarOverflow && tb->isMovable()) {
                if (auto *extButton = findToolbarExtensionButton(tb)) {
                    extButton->hide();
                }
            } else {
                if (auto *extButton = findToolbarExtensionButton(tb)) {
                    extButton->show();
                }
            }
            tb->updateGeometry();
        }
        if (auto *dock = qobject_cast<QDockWidget*>(widget)) {
            setupPermanentTitleBar(dock);
        }
        if (widget && widget->inherits("LC_DetachedMenu") && !m_useFloatingHUD) {
            // Cleanly reclaim active custom menu palettes when floating HUD features are disabled
            widget->close();
        }
    }

    invalidateCache();
}

void LC_ProxyStyle::setMetrics(const StyleMetricsConfig &metrics) {
    m_metrics = metrics;
    m_scaledGeometryProvider.setBaseMetrics(metrics);
    invalidateCache();
}

void LC_ProxyStyle::invalidateCache() const {
    m_scaledGeometryProvider.invalidate();
    m_skinColorsResolver.invalidate();
}

void LC_ProxyStyle::polish(QApplication *app) {
    QProxyStyle::polish(app);
    if (app) {
        m_mnemonicFilter = std::make_unique<LC_EventFilterMnemonic>(this);
        app->installEventFilter(m_mnemonicFilter.get());

        m_toolTipFilter = std::make_unique<LC_EventFilterToolTip>(this);
        app->installEventFilter(m_toolTipFilter.get());

        m_autoPopupController = std::make_unique<LC_EventFilterAutoPopupController>(this);
        app->installEventFilter(m_autoPopupController.get());

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
        m_winResizeFilter = std::make_unique<LC_WinResizeNativeEventFilter>();
        app->installNativeEventFilter(m_winResizeFilter.get());
#endif

#ifdef Q_OS_WIN
        m_win32CloakingFilter = std::make_unique<LC_EventFilterWin32WindowCloaking>(this);
        app->installEventFilter(m_win32CloakingFilter.get());
#endif
    }
    invalidateCache();
}

void LC_ProxyStyle::unpolish(QApplication *app) {
    QProxyStyle::unpolish(app);

    if (app) {
        if (m_mnemonicFilter) {
            app->removeEventFilter(m_mnemonicFilter.get());
            m_mnemonicFilter.reset();
        }
        if (m_toolTipFilter) {
            app->removeEventFilter(m_toolTipFilter.get());
            m_toolTipFilter.reset();
        }
        if (m_autoPopupController) {
            app->removeEventFilter(m_autoPopupController.get());
            m_autoPopupController.reset();
        }
#ifdef USE_WIN_NATIVE_RESIZE_FILTER
        if (m_winResizeFilter) {
            app->removeNativeEventFilter(m_winResizeFilter.get());
            m_winResizeFilter.reset();
        }
#endif

#ifdef Q_OS_WIN
        if (m_win32CloakingFilter) {
            app->removeEventFilter(m_win32CloakingFilter.get());
            m_win32CloakingFilter.reset();
        }
#endif
    }
}

void LC_ProxyStyle::unpolish(QWidget *widget) {
    // if (auto *dock = qobject_cast<QDockWidget*>(widget)) {
    //     if (dock->titleBarWidget() && dock->titleBarWidget()->inherits("LC_DockTitleBar")) {
    //         QWidget *old = dock->titleBarWidget();
    //         dock->setTitleBarWidget(nullptr);
    //         old->deleteLater(); // Safe deferred execution prevents unpolish iterator crash
    //     }
    // }

    if (auto *dock = qobject_cast<QDockWidget*>(widget)) {
        if (dock->titleBarWidget() && (dock->titleBarWidget()->inherits("LC_CustomTitleBarWidget") || dock->titleBarWidget()->inherits("LC_DockTitleBar"))) {
            QWidget *old = dock->titleBarWidget();
            dock->setTitleBarWidget(nullptr);
            old->deleteLater(); // Safe deferred execution prevents unpolish iterator crash
        }
    }


    if (auto *menu = qobject_cast<QMenu*>(widget)) {
        // Safe, stateless cleanup of the substitution event filter from standard menus
        if (auto *filter = menu->findChild<LC_EventFilterFloatingHUD*>(QString(), Qt::FindDirectChildrenOnly)) {
            menu->removeEventFilter(filter);
            filter->deleteLater();
        }
    }

    if (widget && widget->inherits("LC_DetachedMenu")) {
        // Force-close active custom floating menus when style is uninstalled
        widget->close();
    }
    QProxyStyle::unpolish(widget);
}

void LC_ProxyStyle::polish(QPalette &palette) {
    QProxyStyle::polish(palette);
    invalidateCache();
}


void LC_ProxyStyle::polish(QWidget *widget) {
    QProxyStyle::polish(widget);

    if (widget) {

        // QTipLabel is the private native label class instantiated inside QToolTip
        if (widget->inherits("QTipLabel")) {
            if (m_customToolTipCard) {
                // Apply padding margins here so the widget's native sizeHint() incorporates them
                const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(widget);
                widget->setContentsMargins(geoms.tooltip.marginLeft, geoms.tooltip.marginTop,
                                           geoms.tooltip.marginRight, geoms.tooltip.marginBottom);
            }

            // Symmetrically enable mouse transparency so the cursor passes right through the tooltip.
            // This prevents the underlying active popup menus from closing during hover
            widget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        }

        // cache Input Component status (using fast qobject_cast check)
        const bool isInput = qobject_cast<QLineEdit*>(widget) ||
                       qobject_cast<QAbstractSpinBox*>(widget) ||
                       qobject_cast<QComboBox*>(widget) ||
                       qobject_cast<QTextEdit*>(widget) ||
                       qobject_cast<QPlainTextEdit*>(widget) ||
                       qobject_cast<QCheckBox*>(widget) ||
                       qobject_cast<QRadioButton*>(widget) ||
                       qobject_cast<QTabBar*>(widget) ||
                       qobject_cast<QAbstractItemView*>(widget);

        if (isInput) {
            widget->setAttribute(Qt::WA_Hover, true);
            widget->setProperty(PROP_IS_INPUT_COMPONENT, true);

            if (const auto* scrollArea = qobject_cast<QAbstractScrollArea*>(widget)) {
                if (scrollArea->viewport()) {
                    scrollArea->viewport()->setAttribute(Qt::WA_Hover, true);
                    scrollArea->viewport()->setProperty(PROP_IS_INPUT_COMPONENT, true);
                }
            }
        }
        else if (auto* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            if (widget->parentWidget() && (qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()) || qobject_cast<const QComboBox*>(
                widget->parentWidget()))) {
                lineEdit->setFrame(false); // Clean layout-level child frame disable
                if (qobject_cast<const QAbstractSpinBox*>(widget->parentWidget())) {
                    widget->setProperty(PROP_SPINBOX_PTR, reinterpret_cast<qlonglong>(widget->parentWidget()));
                }
            }
        }


        if (auto *toolBar = qobject_cast<QToolBar*>(widget)) {
            toolBar->setAttribute(Qt::WA_Hover, true);
        }

        if (auto *toolButton = qobject_cast<QToolButton*>(widget)) {
            const bool isPopup = (toolButton->popupMode() == QToolButton::InstantPopup ||
                            toolButton->popupMode() == QToolButton::MenuButtonPopup);
            if (isPopup) {
                toolButton->setAttribute(Qt::WA_Hover, true);
            }
        }

        if (auto *menuBar = qobject_cast<QMenuBar*>(widget)) {
            menuBar->setAttribute(Qt::WA_Hover, true);
            menuBar->setMouseTracking(true); // Enable mouse tracking natively to trigger QEvent::MouseMove on sweeps
        }

        // 2. Identify and cache Dock Widget parent association
        const QString className = widget->metaObject()->className();
        if (qobject_cast<const QAbstractButton*>(widget)) {
            bool isDockTitleButton = false;
            if (className == "QDockWidgetTitleButton") {
                isDockTitleButton = true;
            }

            if (isDockTitleButton) {
                widget->setProperty(PROP_IS_DOCK_TITLE_BUTTON, true);
            }
        }


        // 3. Identify and cache Window Title Button association
        const bool isWindowTitleButton = (className == "QMdiSubWindow" || className == "QTitleBar" ||
                                    className == "QMdi::ControllerWidget" || className == "ControllerWidget" ||
                                    widget->inherits("QMdiSubWindow") || widget->inherits("QTitleBar") ||
                                    widget->inherits("QMdi::ControllerWidget") ||
                                    (widget->parentWidget() &&
                                     (widget->parentWidget()->inherits("QMdiSubWindow") ||
                                      widget->parentWidget()->inherits("QTitleBar") ||
                                      widget->parentWidget()->inherits("QMdi::ControllerWidget") ||
                                      widget->parentWidget()->metaObject()->className() == "QMdi::ControllerWidget" ||
                                      widget->parentWidget()->metaObject()->className() == "QTitleBar")));
        if (isWindowTitleButton) {
            widget->setProperty(PROP_IS_WINDOW_TITLE_BUTTON, true);
        }

        // 4. Identify and cache Item View row highlight behavior
            if (auto *itemView = qobject_cast<QAbstractItemView*>(widget)) {
                bool highlightWholeRow = false;
                const QVariant prop = itemView->property(PROP_DO_HOVER_ROW);
                if (prop.isValid()) {
                    highlightWholeRow = prop.toBool();
                } else {
                    if (qobject_cast<const QTableView*>(itemView)) {
                        highlightWholeRow = (itemView->selectionBehavior() == QAbstractItemView::SelectRows);
                    } else if (const auto *treeView = qobject_cast<const QTreeView*>(itemView)) {
                        highlightWholeRow = !m_isClassic || treeView->allColumnsShowFocus();
                    }
                }
                itemView->setProperty(PROP_CACHED_HIGHLIGHT_ROW, highlightWholeRow);
            }

        // 5. Install event filter on standard labels to capture QEvent::Leave
        if (auto *label = qobject_cast<QLabel*>(widget)) {
            connect(label, &QLabel::linkHovered, this, [label](const QString &link) {
                const bool alreadyHovered = label->property(PROP_LINK_ACTIVE_HOVER).toBool();

                if (!link.isEmpty()) {
                    if (alreadyHovered) {
                        return;
                    }

                    QTimer::singleShot(0, label, [label]() {
                        label->blockSignals(true);
                        label->setProperty(PROP_LINK_ACTIVE_HOVER, true);

                        const QString currentText = label->text();
                        if (!currentText.startsWith("<style>a { color:")) {
                            label->setProperty(PROP_ORIGINAL_HTML, currentText);
                        }

                        QString originalHtml = label->property(PROP_ORIGINAL_HTML).toString();
                        const QString hoverColor = label->palette().color(QPalette::Active, QPalette::Highlight).name();
                        const QString hoveredHtml = QString("<style>a { color: %1; text-decoration: underline; }</style>").arg(hoverColor) + originalHtml;

                        label->setText(hoveredHtml);
                        label->blockSignals(false);
                        label->update();
                    });
                } else {
                    if (!alreadyHovered) {
                        return;
                    }

                    QTimer::singleShot(0, label, [label]() {
                        label->blockSignals(true);
                        label->setProperty(PROP_LINK_ACTIVE_HOVER, false);

                        const QString originalHtml = label->property(PROP_ORIGINAL_HTML).toString();

                        label->setText(originalHtml);
                        label->blockSignals(false);
                        label->update();
                    });
                }
            });
            return;
        }

        // Symmetrical Dialog / Window flash overrides
        const Qt::WindowFlags flags = widget->windowFlags();
        const bool isTargetWindow = widget->isWindow() &&
                              !flags.testFlag(Qt::Popup) &&
                              !flags.testFlag(Qt::ToolTip);

        if (isTargetWindow) {
            if (!widget->property("lcfs_styleEventFilterInstalled").toBool()) {
                widget->installEventFilter(this);
                widget->setProperty("lcfs_styleEventFilterInstalled", true);
            }

            const QPalette::ColorGroup group = (widget->isActiveWindow()) ? QPalette::Active : QPalette::Inactive;
            const QColor winColor = widget->palette().color(group, QPalette::Window);
            const bool isDarkTheme = (winColor.value() < 120);

#ifdef Q_OS_WIN
            const auto hwnd = reinterpret_cast<HWND>(widget->winId());
            if (hwnd) {
                // A. Dynamically load dwmapi.dll to apply Immersive Dark Mode to the OS titlebar/borders
                const HMODULE dwmDll = ::LoadLibraryW(L"dwmapi.dll");
                if (dwmDll) {
                    const auto setWindowAttr = reinterpret_cast<DwmSetWindowAttributePtr>(
                        ::GetProcAddress(dwmDll, "DwmSetWindowAttribute"));

                    if (setWindowAttr) {
                        const BOOL useDarkMode = isDarkTheme ? TRUE : FALSE;
                        constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
                        setWindowAttr(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));

                        // Pre-Cloak the window immediately inside polish() so it is already invisible
                        // long before QWidget::show_sys() calls ShowWindow()
                        if (isDarkTheme && !widget->isVisible()) {
                            constexpr BOOL cloak = TRUE;
                        constexpr DWORD DWMWA_CLOAK = 13;
                            setWindowAttr(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));
                            widget->setProperty(PROP_STYLE_PRE_CLOAKED, true); // Mark that we pre-cloaked this window
                        }
                    }
                    ::FreeLibrary(dwmDll);
                }

                // B. Override the Win32 class background brush with a dark brush
                if (isDarkTheme) {
                    HBRUSH darkBrush = ::CreateSolidBrush(RGB(winColor.red(), winColor.green(), winColor.blue()));
                    const auto oldBrush = reinterpret_cast<HBRUSH>(::SetClassLongPtrW(
                        hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(darkBrush)));

                    if (oldBrush) {
                        ::DeleteObject(oldBrush);
                    }
                }
            }
#endif
            return;
        }

        if (widget->inherits("QFontComboBox")) {
            if (auto *fontCombo = qobject_cast<QFontComboBox*>(widget)) {
                // Prevent Qt from querying and sizing every font in the OS on startup
                fontCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

                // Enable uniform, batched popup rendering for large system font databases
                if (auto *view = qobject_cast<QListView*>(fontCombo->view())) {
                    view->setUniformItemSizes(true);
                    view->setLayoutMode(QListView::Batched);
                }
            }
        }

        if (auto *dock = qobject_cast<QDockWidget*>(widget)) {
            setupPermanentTitleBar(dock);
        }

        if (m_useFloatingHUD) {
            if (auto *menu = qobject_cast<QMenu*>(widget)) {
                // Respect and preserve original CAD menu layout configurations; do NOT force tearable flags.
                // Install the filter to intercept and swap QTornOffMenu when instantiated
                menu->installEventFilter(new LC_EventFilterFloatingHUD(menu, this));
                return;
            }

            const bool isTearOff = (className == "QTornOffMenu" || className == "QTearOffMenu" ||
                                    widget->inherits("QTornOffMenu") || widget->inherits("QTearOffMenu"));

            if (isTearOff) {
                // Direct event trapping for freshly-polished native tear-off windows
                widget->installEventFilter(new LC_EventFilterFloatingHUD(widget, this));
                return;
            }
        }
    }
}

// ================= METRICS & SIZE INTERCEPTS =================

int LC_ProxyStyle::pixelMetric(const PixelMetric metric, const QStyleOption *option, const QWidget *widget) const {
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(widget);
    switch (metric) {
        case PM_ToolBarExtensionExtent:
        if (m_autoPopupToolbarOverflow) {
            auto toolBar = qobject_cast<const QToolBar*>(widget);
            if (!toolBar && isToolbarExtensionButton(widget)) {
                toolBar = qobject_cast<const QToolBar*>(widget->parentWidget());
            }

            if (toolBar && toolBar->isMovable()) {
                return 0;
            }
        }
            break;

        case PM_ButtonMargin:
        return geoms.scaledMetrics.buttonPadding;

        case PM_SpinBoxFrameWidth:
        case PM_ComboBoxFrameWidth:
        case PM_DefaultFrameWidth:
        return m_isClassic ? 2 : 3;

        case PM_ButtonShiftHorizontal:
        case PM_ButtonShiftVertical:
        return m_isFlat ? 0 : 1;

        case PM_MenuButtonIndicator:
        return m_isClassic ? 14 : (12 + geoms.scaledMetrics.buttonPadding);

        case PM_TabCloseIndicatorWidth:
        case PM_TabCloseIndicatorHeight:
        return geoms.scaledMetrics.tabCloseIndicatorSize;

        case PM_DockWidgetSeparatorExtent:
        return geoms.scaledMetrics.splitterWidth;

        case PM_DockWidgetFrameWidth:
        return (m_boxDecoration == BoxDecoration::Frameless) ? 0 : 1;

        case PM_ScrollBarExtent:
        if (m_metrics.scrollBarWidth < 0) {
            return QProxyStyle::pixelMetric(PM_ScrollBarExtent, option, widget);
        }
        return geoms.scaledMetrics.scrollBarWidth;

        case PM_ScrollBarSliderMin:
            return geoms.scaledMetrics.scrollBarMinLength;

        case PM_SliderControlThickness:
            return geoms.scaledMetrics.sliderControlThickness;

        case PM_SliderLength:
            return geoms.scaledMetrics.sliderHandleLength;

        case PM_SplitterWidth:
            return geoms.scaledMetrics.splitterWidth;

        case PM_TitleBarHeight: {
        if (m_metrics.dockTitleBarHeight < 0) {
            const int fontHeight = option ? option->fontMetrics.height() :
                             (widget ? widget->fontMetrics().height() : QApplication::fontMetrics().height());
            return fontHeight + geoms.ints.scale8;
        }
        return geoms.scaledMetrics.dockTitleBarHeight;
    }

        case PM_TitleBarButtonSize:
            return geoms.scaledMetrics.titleBarButtonSize;

        case PM_TitleBarButtonIconSize:
        return qMax(8, geoms.scaledMetrics.titleBarButtonSize - 6);

        case PM_DockWidgetTitleBarButtonMargin: {
        if (m_metrics.dockWidgetTitleBarButtonMargin < 0) {
            int activeTitleBarH = geoms.scaledMetrics.dockTitleBarHeight;
            if (activeTitleBarH < 0) {
                const int fontHeight = option ? option->fontMetrics.height() :
                                 (widget ? widget->fontMetrics().height() : QApplication::fontMetrics().height());
                activeTitleBarH = fontHeight + geoms.ints.scale8;;
            }
            return qMax(0, (activeTitleBarH - geoms.scaledMetrics.titleBarButtonSize) / 2);
        }
        return geoms.scaledMetrics.dockWidgetTitleBarButtonMargin;
    }

        case PM_TabBarTabOverlap:
            return geoms.scaledMetrics.tabBarTabOverlap;

        case PM_TabBarTabHSpace:
            return geoms.scaledMetrics.tabBarTabHSpace;

        case PM_TabBarTabVSpace:
            return geoms.scaledMetrics.tabBarTabVSpace;

        case PM_TabBarBaseOverlap:
            return geoms.scaledMetrics.tabBarTabBaseOverlap;

        case PM_MenuVMargin:
            return geoms.scaledMetrics.menuVerticalPadding;

        case PM_MenuHMargin:
            return geoms.scaledMetrics.menuHorizontalPadding;

        case PM_MenuPanelWidth:
            return geoms.scaledMetrics.menuBorderWidth;

        case PM_HeaderDefaultSectionSizeVertical:
            return geoms.scaledMetrics.headerDefaultHeight;

        case PM_FocusFrameHMargin:
            return geoms.scaledMetrics.focusFrameHMargin;

        case PM_FocusFrameVMargin:
            return geoms.scaledMetrics.focusFrameVMargin;

        case PM_LayoutLeftMargin:
        case PM_LayoutTopMargin:
        case PM_LayoutRightMargin:
        case PM_LayoutBottomMargin:
        return geoms.scaledMetrics.layoutMargin;

        case PM_LayoutHorizontalSpacing:
        case PM_LayoutVerticalSpacing:
        return geoms.scaledMetrics.layoutSpacing;

        case PM_ToolBarItemSpacing:
            return geoms.scaledMetrics.toolbarItemSpacing;

        case PM_ToolBarSeparatorExtent:
            return geoms.scaledMetrics.toolbarSeparatorWidth;

        case PM_MenuBarItemSpacing:
            return geoms.scaledMetrics.menuBarItemSpacing;

        case PM_MenuBarVMargin:
            return geoms.scaledMetrics.menuBarVerticalMargin;

        case PM_IndicatorWidth:
        case PM_ExclusiveIndicatorWidth:
        case PM_IndicatorHeight:
        case PM_ExclusiveIndicatorHeight:
        return geoms.scaledMetrics.indicatorBoxSize;

        case PM_CheckBoxLabelSpacing:
        case PM_RadioButtonLabelSpacing:
        return geoms.scaledMetrics.indicatorLabelSpacing;

        case PM_SubMenuOverlap:
            return geoms.scaledMetrics.subMenuOverlap;

        case PM_DockWidgetTitleMargin: {
        const int fontHeight = option ? option->fontMetrics.height() : (widget ? widget->fontMetrics().height() : 14);
        return qMax(0, (geoms.scaledMetrics.dockTitleBarHeight - fontHeight) / 2);
    }

        case PM_TreeViewIndentation:
            return geoms.scaledMetrics.treeIndentation;

        default:
            break;
    }

    return QProxyStyle::pixelMetric(metric, option, widget);
}

int LC_ProxyStyle::styleHint(const StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const {
    switch (hint) {
        case SH_EtchDisabledText:
        return m_metrics.flatDisabledText ? 0 : 1;

        case SH_UnderlineShortcut:
        if (m_metrics.hideShortcutUnderlines) {
            if (QApplication::activePopupWidget() || (m_mnemonicFilter && m_mnemonicFilter->showUnderlines())) {
                return 1;
            }
            return 0;
        }
        return 1; // Always show if the setting is disabled

        case SH_Menu_AllowActiveAndDisabled:
        return m_metrics.menuAllowActiveAndDisabled ? 1 : 0;

        default:
            break;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

QSize LC_ProxyStyle::sizeFromContents(const ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const {
    QSize calculatedSize = QProxyStyle::sizeFromContents(type, option, size, widget);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(widget);

    switch (type) {
        case CT_ItemViewItem:
        calculatedSize.setHeight(calculatedSize.height() + geoms.scaledMetrics.itemViewRowPadding);
            break;

        case CT_TabBarTab: {
        const qreal totalExtraHeight = geoms.tab.activeExtraHeight + geoms.scaledMetrics.tabBarTabBaseOverlap;
        calculatedSize.setHeight(calculatedSize.height() + static_cast<int>(totalExtraHeight));
            break;
    }

        case CT_HeaderSection:
        if (option && (option->state & State_Horizontal)) {
            calculatedSize.setHeight(qMax(calculatedSize.height(), geoms.scaledMetrics.headerDefaultHeight));
        } else {
            calculatedSize.setWidth(qMax(calculatedSize.width(), geoms.scaledMetrics.headerDefaultHeight));
        }
            break;

        case CT_ToolButton: {
        if (isToolbarExtensionButton(widget)) {
            if (m_customToolbarOverflowGrip && m_autoPopupToolbarOverflow) {
                if (const auto *button = qobject_cast<const QToolButton*>(widget)) {
                    if (const auto *toolBar = qobject_cast<const QToolBar*>(button->parentWidget())) {
                        if (toolBar->isMovable()) {
                            return QSize(0, 0);
                        }
                    }
                }
            }
        }
        if (const auto *toolOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
            const bool hasMenu = (toolOpt->features & QStyleOptionToolButton::HasMenu);
            const bool isMenuButtonPopup = (toolOpt->features & QStyleOptionToolButton::MenuButtonPopup);

            if (hasMenu && !isMenuButtonPopup) {
                const int indicatorWidth = pixelMetric(PM_MenuButtonIndicator, option, widget);
                const int offset = indicatorWidth / 2;
                calculatedSize.setWidth(calculatedSize.width() + offset);
            }
        }
            break;
    }

        case CT_MenuItem: {
        if (m_showMenuCommandAliases) {
            int maxCmdWidth = 0;
            int maxShortcutWidth = 0;
            const QFont baseFont = widget ? widget->font() : QApplication::font();
            if (getMenuCommandAliasInfo(widget, baseFont, maxCmdWidth, maxShortcutWidth)) {
                if (maxCmdWidth > 0) {
                    calculatedSize.setWidth(calculatedSize.width() + maxCmdWidth + geoms.ints.scale16);
                }
            }
        }
            break;
    }

        default:
            break;
    }

    return calculatedSize;
}
// ================= COHESIVE DISPATCHING PASSTHROUGH ENGINES =================

void LC_ProxyStyle::drawPrimitive(const PrimitiveElement element,
                                        const QStyleOption *option,
                                        QPainter *painter,
                                        const QWidget *widget) const {
    if (m_isClassic) {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        return;
    }

    switch (element) {
        case PE_FrameTabWidget: {
            drawCustomTabWidgetFrame(option, painter, widget);
            return;
        }
        case PE_Frame: {
            drawCustomLineEditFrame(option, painter, widget);
            return;
        }
        case PE_FrameLineEdit: {
            drawCustomLineEditFrame(option, painter, widget);
            return;
        }
        case PE_PanelLineEdit: {
            drawCustomLineEditPanel(option, painter, widget);
            return;
        }
        case PE_FrameGroupBox: {
            if (const auto* groupOpt = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
                drawCustomGroupBoxFrame(groupOpt, painter, widget);
                return;
            }
            break;
        }
        case PE_PanelButtonCommand: {
            const bool isToolButton = (widget && qobject_cast<const QToolButton*>(widget)) || (qstyleoption_cast<const
                QStyleOptionToolButton*>(option));

            if (isToolButton && m_useToolButtonUnderline) {
                drawCustomPanelButtonTool(option, painter, widget);
                return;
            }
            drawCustomPanelButton(option, painter, widget);
            return;
        }
        case PE_PanelButtonTool: {
            const bool isTabScrollButton = widget && widget->parentWidget() && qobject_cast<const QTabBar*>(widget->parentWidget());
            if (isTabScrollButton) {
                if (option->state & State_MouseOver) {
                    const SkinColors desc = getStyleDescriptor(option);
                    painter->fillRect(option->rect, desc.button.bgHovered);
                }
                return;
            }
            drawCustomPanelButtonTool(option, painter, widget);
            return;
        }
        case PE_IndicatorTabTearLeft:
        case PE_IndicatorTabTearRight: {
            drawCustomIndicatorTabTear(element, option, painter, widget);
            return;
        }
        case PE_IndicatorArrowDown:
        case PE_IndicatorArrowUp:
        case PE_IndicatorArrowLeft:
        case PE_IndicatorArrowRight: {
            drawCustomIndicatorArrow(element, option, painter, widget);
            return;
        }
        case PE_IndicatorCheckBox: {
            drawCustomIndicatorCheckBox(option, painter, widget);
            return;
        }
        case PE_IndicatorRadioButton: {
            drawCustomIndicatorRadioButton(option, painter, widget);
            return;
        }
        case PE_IndicatorToolBarSeparator: {
            drawCustomIndicatorToolBarSeparator(option, painter, widget);
            return;
        }
        case PE_PanelItemViewRow: {
            if (const auto* vopt = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
                drawCustomPanelItemViewRow(vopt, painter, widget);
                return;
            }
            break;
        }
        case PE_IndicatorTabClose: {
            if (m_customVectorIconsInButtons) {
                drawCustomIndicatorTabClose(option, painter);
                return;
            }
            break;
        }
        case PE_IndicatorHeaderArrow: {
            if (m_customVectorIconsInButtons) {
                drawCustomIndicatorHeaderArrow(option, painter);
                return;
            }
            break;
        }
        case PE_IndicatorBranch: {
            drawCustomIndicatorBranch(option, painter, widget);
            return;
        }
        case PE_IndicatorToolBarHandle: {
            const bool isStatusPill = isStatusPillNeeded(widget);
            if (isStatusPill) {
                drawCustomStatusPillToolbarHandle(option, painter, widget);
                return;
            }
            drawCustomIndicatorToolBarHandle(option, painter, widget);
            return;
        }
        case PE_IndicatorDockWidgetResizeHandle: {
            if (!m_customSplitterAnToolbarGrip) {
                QStyleOption copy = *option;
                const SkinColors desc = getStyleDescriptor(option);
                copy.palette.setColor(QPalette::Mid, desc.splitter.splitterGripColor);
                copy.palette.setColor(QPalette::Dark, desc.splitter.splitterGripDark);
                QProxyStyle::drawPrimitive(element, &copy, painter, widget);
            }
            else {
                drawCustomSplitter(option, painter, widget);
            }
            return;
        }
        case PE_PanelTipLabel: {
            if (m_customToolTipCard) {
                drawCustomToolTipCard(option, painter, widget);
                return;
            }
            break;
        }
        case PE_FrameMenu: {
            if (m_useFloatingHUD && widget && widget->isWindow()) {
                // Background and titlebar are drawn on-demand by LC_EventFilterFloatingHUD
                return;
            }
            break;
        }
        case PE_FrameDockWidget: {
            const auto *dock = qobject_cast<const QDockWidget*>(widget);
            if (m_useFloatingHUD && dock && dock->isFloating()) {
                // Background and titlebar are drawn on-demand by LC_EventFilterFloatingHUD
                return;
            }
            break;
        }
        default:
            break;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool LC_ProxyStyle::isStatusPillNeeded(const QWidget* widget) const {
    return m_useStatusPillChips && widget && widget->property(PROP_USE_STATUS_PILL_CHIPS).toBool();
}

void LC_ProxyStyle::drawControl(const ControlElement element,
                                const QStyleOption *option,
                                QPainter *painter,
                                const QWidget *widget) const {

    if (element == CE_DockWidgetTitle) {
        if (const auto *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(option)) {
            // Apply custom draw if not Classic, OR if Classic but customDockTitleBar is active
            const bool shouldDrawCustom = (!m_isClassic || m_customDockTitleBar);

            if (shouldDrawCustom) {
                drawCustomDockTitleBar(dwOpt, painter, widget);
                return;
            }
        }
    }


    if (m_isClassic) {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    switch (element) {
        case CE_MenuBarEmptyArea: {
            drawCustomMenuBarEmptyArea(option, painter, widget);
            return;
        }
        case CE_MenuBarItem: {
            if (const auto* menuItemOpt = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
                drawCustomMenuBarItem(menuItemOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_MenuItem: {
            if (const auto* menuItemOpt = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
                drawCustomMenuItem(menuItemOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_ToolButtonLabel: {
            if (m_customToolbarOverflowGrip && isToolbarExtensionButton(widget)) {
                drawCustomToolbarOverflowIndicator(option, painter, widget);
                return;
            }
            if (const auto *toolOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
                const bool hasMenu = (toolOpt->features & QStyleOptionToolButton::HasMenu);
                const bool isMenuButtonPopup = (toolOpt->features & QStyleOptionToolButton::MenuButtonPopup);

                if (hasMenu && !isMenuButtonPopup) {
                    QStyleOptionToolButton copy = *toolOpt;
                    const int indicatorWidth = pixelMetric(PM_MenuButtonIndicator, option, widget);
                    const int offset = indicatorWidth / 2;

                    if (copy.direction == Qt::RightToLeft) {
                        copy.rect.setLeft(copy.rect.left() + offset);
                    } else {
                        copy.rect.setRight(copy.rect.right() - offset);
                    }

                    QProxyStyle::drawControl(element, &copy, painter, widget);
                    return;
                }
            }
            break;
        }
        case CE_ItemViewItem: {
            if (const auto* vopt = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
                drawCustomItemViewItem(vopt, painter, widget);
                return;
            }
            break;
        }
        case CE_PushButton: {
            if (const auto* btnOpt = qstyleoption_cast<const QStyleOptionButton*>(option)) {
                drawCustomPushButton(btnOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_TabBarTab: {
            if (const auto* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                drawCustomTabBarTab(tabOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_HeaderSection: {
            if (const auto* headerOpt = qstyleoption_cast<const QStyleOptionHeader*>(option)) {
                drawCustomHeaderSection(headerOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_ProgressBar: {
            if (const auto* barOpt = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
                drawCustomProgressBar(barOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_ToolBar: {
            if (const auto toolBarOpt = qstyleoption_cast<const QStyleOptionToolBar*>(option)) {
                if (isStatusPillNeeded(widget)) {
                    drawCustomStatusPillToolbar(toolBarOpt, painter, widget);
                    return;
                }
                drawCustomToolBar(toolBarOpt, painter, widget);
                return;
            }
            break;
        }
        case CE_Splitter: {
            // Determine if custom overrides are enabled
            if (!m_customSplitterAnToolbarGrip) {
                // Fallback to base style: Delegate directly to QProxyStyle's control drawing
                QStyleOption copy = *option; // Apply palette mods for native drawing
                const SkinColors desc = getCachedStyleDescriptor(option->palette, QPalette::Active);
                // Group doesn't matter for palette mods
                copy.palette.setColor(QPalette::Mid, desc.splitter.splitterGripColor);
                copy.palette.setColor(QPalette::Dark, desc.splitter.splitterGripDark);
                QProxyStyle::drawControl(element, &copy, painter, widget);
            }
            else {
                // Custom drawing: Call our unified custom splitter renderer
                drawCustomSplitter(option, painter, widget);
            }
            return;
        }
        default:
            break;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

void LC_ProxyStyle::drawComplexControl(const ComplexControl control,
                                              const QStyleOptionComplex *option,
                                              QPainter *painter,
                                              const QWidget *widget) const {
    if (control == CC_ScrollBar && (m_accentedScrollbars || m_transparentScrollbars)) {
        if (const auto *scrollOpt = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            drawCustomScrollBar(scrollOpt, painter, widget);
            return;
        }
    }

    if (m_isClassic) {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    switch (control) {
        case CC_ToolButton:
        if (const auto *toolOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
            const QVariant neighborsVar = widget ? widget->property(PROP_GROUP_NEIGHBORS) : QVariant();
                const bool isSegmented = m_useSegmentedToolButtons && neighborsVar.isValid() && !isTitleOrDockButton(widget);

            if (isSegmented) {
                // 1. Draw the segmented background well, outer borders, and internal separators
                drawSegmentedToolButton(toolOpt, painter, widget, neighborsVar.toInt());

                // 2. Execute base drawing with re-entry guard active so it only paints labels (icons/text)
                g_inSegmentedDraw = true;
                QProxyStyle::drawComplexControl(control, option, painter, widget);
                g_inSegmentedDraw = false;
                return;
            }
        }
            break;

        case CC_GroupBox:
        if (const auto *gbOpt = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
            const bool shouldDrawCustom = (!m_isClassic || m_customGroupBoxBar);
            if (shouldDrawCustom) {
                drawCustomGroupBox(gbOpt, painter, widget);
                return;
            }
        }
            break;

        case CC_Slider:
        if (const auto *sliderOpt = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            drawCustomSlider(sliderOpt, painter, widget);
            return;
        }
            break;

        case CC_ComboBox:
        if (const auto *cbOpt = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            drawCustomComboBox(cbOpt, painter, widget);
            return;
        }
            break;

        case CC_SpinBox:
        if (const auto *spinOpt = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
            drawCustomSpinBox(spinOpt, painter, widget);
            return;
        }
            break;

        default:
            break;
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

// ================= ESTABLISHED CACHE & VECTOR RESOLUTION ENGINE =================

SkinColors LC_ProxyStyle::getCachedStyleDescriptor(const QPalette &palette, const QPalette::ColorGroup group) const {
    return m_skinColorsResolver.getDescriptor(palette, group);
}

const QFont& LC_ProxyStyle::getResolvedMonoFont(const QFont &baseFont) const {
    return m_skinColorsResolver.getResolvedMonoFont(baseFont);
}

void LC_ProxyStyle::precomputeSegmentedGroupColors(QWidget *widget, const int totalGroups) const {
    m_skinColorsResolver.precomputeSegmentedGroupColors(widget, totalGroups);
}

// Delegate standard icons through the descriptor cache
QIcon LC_ProxyStyle::standardIcon(const StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const {
    // QIcon customIcon = m_skinColorsResolver.getStandardIcon(standardIcon, option, widget);
    // if (!customIcon.isNull()) {
    //     return customIcon;
    // }
    // return QProxyStyle::standardIcon(standardIcon, option, widget);

    QStyleOption fallbackOpt;
    const QStyleOption *actualOption = option;

    // Globally sanitize null option pointers passed by Qt's own internal widgets (e.g., QMdiSubWindow)
    if (!actualOption) {
        if (widget) {
            fallbackOpt.initFrom(widget);
        } else {
            fallbackOpt.palette = QApplication::palette();
            fallbackOpt.state = QStyle::State_Enabled | QStyle::State_Active;
        }
        actualOption = &fallbackOpt;
    }

    QIcon customIcon = m_skinColorsResolver.getStandardIcon(standardIcon, actualOption, widget);
    if (!customIcon.isNull()) {
        return customIcon;
    }

    return QProxyStyle::standardIcon(standardIcon, actualOption, widget);
}

const SkinScaledGeometries& LC_ProxyStyle::getGeometries(const QWidget* widget) const {
    return m_scaledGeometryProvider.getGeometries(widget);
}

void LC_ProxyStyle::drawParameterizedBox(QPainter *painter, const QRect &rect, const SkinColors &desc, const bool isVertical) const {

    // LC_ERR << "[drawParameterizedBox]"
    //       << " style=" << this
    //       << " x=" << rect.x()
    //       << " y=" << rect.y()
    //       << " w=" << rect.width()
    //       << " h=" << rect.height()
    //       << " bgStart=" << desc.bgStart.name(QColor::HexArgb)
    //       << " bgEnd=" << desc.bgEnd.name(QColor::HexArgb)
    //       << " useGlassyGloss=" << desc.useGlassyGloss
    //       << " useGradient=" << desc.useGradient
    //       << " hasFullBorder=" << desc.hasFullBorder;


    if (desc.common.useGlassyGloss) {
        QLinearGradient glassGrad(rect.topLeft(), rect.bottomLeft());
        // Softened stops stretching across 13% of the control's height
        glassGrad.setColorAt(0.0, desc.button.glassStart);
        glassGrad.setColorAt(0.42, desc.button.glassMidStart);
        glassGrad.setColorAt(0.55, desc.button.glassMidEnd);
        glassGrad.setColorAt(1.0, desc.button.glassEnd);
        painter->fillRect(rect, glassGrad);
    } else if (desc.common.useGradient) {
        QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
        grad.setColorAt(0.0, desc.common.bgStart);
        grad.setColorAt(1.0, desc.common.bgEnd);
        painter->fillRect(rect, grad);
    } else {
        painter->fillRect(rect, desc.common.bgStart);
    }

    if (desc.frame.hasFullBorder) {
        painter->setPen(desc.frame.borderTop);
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
    } else if (isVertical) {
        // Rotated Border Mapping:
        // Top/Bottom of horizontal bar map to Left/Right of vertical bar
        if (desc.frame.borderTop.isValid()) {
            painter->setPen(desc.frame.borderTop);
            painter->drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
        }
        if (desc.frame.borderBottom.isValid()) {
            painter->setPen(desc.frame.borderBottom);
            painter->drawLine(rect.right() - 1, rect.top(), rect.right() - 1, rect.bottom());
        }
        // Left/Right of horizontal bar map to Bottom/Top of vertical bar
        if (desc.frame.borderLeft.isValid()) {
            painter->setPen(desc.frame.borderLeft);
            painter->drawLine(rect.left(), rect.bottom() - 1, rect.right(), rect.bottom() - 1);
        }
        if (desc.frame.borderRight.isValid()) {
            painter->setPen(desc.frame.borderRight);
            painter->drawLine(rect.left(), rect.top(), rect.right(), rect.top());
        }
    } else {
        // Standard horizontal border drawing
        if (desc.frame.hasTopBottomBorder) {
        if (desc.frame.borderTop.isValid()) {
            painter->setPen(desc.frame.borderTop);
            painter->drawLine(rect.left(), rect.top(), rect.right(), rect.top());
        }
        if (desc.frame.borderBottom.isValid()) {
            painter->setPen(desc.frame.borderBottom);
            painter->drawLine(rect.left(), rect.bottom() - 1, rect.right(), rect.bottom() - 1);
        }
    } else if (desc.frame.hasSideBorders) {
        if (desc.frame.borderLeft.isValid()) {
            painter->setPen(desc.frame.borderLeft);
            painter->drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
        }
        if (desc.frame.borderRight.isValid()) {
            painter->setPen(desc.frame.borderRight);
            painter->drawLine(rect.right() - 1, rect.top(), rect.right() - 1, rect.bottom());
        }
    }
    }

    // Align the left IDE-like accent bar to the bottom of the widget in vertical mode
    if (desc.frame.hasLeftAccentBar && desc.frame.accentBarColor.isValid()) {
        const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
        const int thickness = geoms.ints.scale3;
        if (isVertical) {
            const QRect sidebarRect(rect.left(), rect.bottom() - thickness, rect.width(), thickness);
            painter->fillRect(sidebarRect, desc.frame.accentBarColor);
        } else {
            const QRect sidebarRect(rect.left(), rect.top(), thickness, rect.height());
            painter->fillRect(sidebarRect, desc.frame.accentBarColor);
        }
    }
}

// ================= EXTRACTED WIDGET RENDERING HELPERS =================

void LC_ProxyStyle::drawCustomPushButton(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const {
    // LC_ERR << "[drawCustomPushButton]"
    //       << " style=" << this
    //       << " x=" << option->rect.x()
    //       << " y=" << option->rect.y()
    //       << " w=" << option->rect.width()
    //       << " h=" << option->rect.height()
    //       << " text=" << option->text;
    const SkinColors buttonDesc = m_skinColorsResolver.resolveStandardButtonDescriptor(option, widget);

    drawParameterizedBox(painter, option->rect, buttonDesc);
    QProxyStyle::drawControl(CE_PushButtonLabel, option, painter, widget);
}

void LC_ProxyStyle::drawCustomPanelButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    if (g_inSegmentedDraw) {
        return;
    }
    LCPainterGuard guard(painter, false);

    const SkinColors buttonDesc = m_skinColorsResolver.resolveToolButtonDescriptor(option, widget);

    QRect drawRect = option->rect;
    const bool isTitleBarButton = widget && widget->property(PROP_IS_DOCK_TITLE_BUTTON).toBool();

    if (isTitleBarButton && widget) {
        const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
        const int margin = geoms.ints.scale1;
        drawRect.adjust(margin, margin, -margin, -margin);
    }

    drawParameterizedBox(painter, drawRect, buttonDesc);
}

void LC_ProxyStyle::drawCustomHeaderSection(const QStyleOptionHeader *option, QPainter *painter, const QWidget *widget) const {
    Q_UNUSED(widget);
    const SkinColors desc = m_skinColorsResolver.resolveHeaderSectionDescriptor(option);

    drawParameterizedBox(painter, option->rect, desc);

    LCPainterGuard guard(painter, false);
    painter->setPen(desc.header.headerSeparator);
    painter->drawLine(option->rect.topRight(), option->rect.bottomRight() - QPoint(0, 1));
}

void LC_ProxyStyle::drawCustomProgressBar(const QStyleOptionProgressBar *option, QPainter *painter, const QWidget *widget) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const SkinColors desc = getStyleDescriptor(option);

    // Resolve progress bar layout
    const LC_SkinWidgetsLayoutResolver::ProgressBarLayout layout = LC_SkinWidgetsLayoutResolver::resolveProgressBarLayout(option, geoms);

    painter->setPen(QPen(desc.progressBar.borderColor, 1));
    painter->setBrush(desc.progressBar.trackColor);
    painter->drawRoundedRect(crispRect(layout.rect, geoms.crispOffset), geoms.progressBar.trackRadius, geoms.progressBar.trackRadius);

    if (layout.fillRect.isValid()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(desc.progressBar.fillColor);
        painter->drawRoundedRect(layout.fillRect, geoms.progressBar.fillRadius, geoms.progressBar.fillRadius);
    }

    QProxyStyle::drawControl(CE_ProgressBarLabel, option, painter, widget);
}

void LC_ProxyStyle::drawCustomLineEditFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    // Skip drawing duplicate borders and focus glows for nested LineEdit editors inside SpinBoxes
    // to prevent duplicate bevel and nested borderlines inside the edit area.
    if (widget && widget->parentWidget() &&
          (qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()) ||
           qobject_cast<const QComboBox*>(widget->parentWidget()))) {
        return;
    }

    const QWidget *actualWidget = widget;
    if (!actualWidget && painter && painter->device() && painter->device()->devType() == QInternal::Widget) {
        actualWidget = static_cast<const QWidget*>(painter->device());
    }

    const SkinColors desc = m_skinColorsResolver.resolveLineEditDescriptor(option, actualWidget);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    // Robust focus tracking fallback for composite controls (spinboxes, comboboxes)
    bool hasFocus = (option->state & State_HasFocus);
    if (!hasFocus && actualWidget) {
        hasFocus = actualWidget->hasFocus() || actualWidget->isAncestorOf(QApplication::focusWidget());
    }

    // Determine input category to apply rounded corners cleanly
    bool isInput = false;
    if (actualWidget) {
        isInput = actualWidget->property(PROP_IS_INPUT_COMPONENT).toBool();
        if (!isInput && actualWidget->parentWidget()) {
            isInput = actualWidget->parentWidget()->property(PROP_IS_INPUT_COMPONENT).toBool();
        }
    }

    const QPalette::ColorGroup finalGroup = resolveColorGroup(option->state);

    // Unconditional diagnostic debug logging to trace exact palette behavior and color roles
    // LC_ERR << "[LineEdit Paint Debug] widget=" << actualWidget
    //        << " WA_SetPalette=" << (actualWidget ? actualWidget->testAttribute(Qt::WA_SetPalette) : false)
    //        << " optionBase_Active=" << option->palette.color(QPalette::Active, QPalette::Base).name()
    //        << " optionBase_Inactive=" << option->palette.color(QPalette::Inactive, QPalette::Base).name()
    //        << " optionWindow_Active=" << option->palette.color(QPalette::Active, QPalette::Window).name()
    //        << " appBase_Active=" << QApplication::palette().color(QPalette::Active, QPalette::Base).name()
    //        << " desc_bgInput=" << desc.input.bgInput.name()
    //        << " desc_bgStart=" << desc.common.bgStart.name()
    //        << " finalGroup=" << finalGroup
    //        << " hasFullBorder=" << desc.frame.hasFullBorder;

    {
        LCPainterGuard guard(painter, true); // AA allows smooth rounded corners / clean angled lines

        if (hasFocus) {
            // Focus state: Draw full highlighted closed border and match focus glow
            const QColor focusColor = option->palette.color(finalGroup, QPalette::Highlight);
            painter->setPen(QPen(focusColor, 1));
            painter->setBrush(Qt::NoBrush);

            if (isInput && !m_isClassic) {
                painter->drawRoundedRect(crispRect(option->rect), 2.0, 2.0);
            } else {
                painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
            }
        } else {
            // Unfocused state: Respect the precise individual border parameters
            if (desc.frame.hasFullBorder) {
                QColor borderColor = desc.frame.borderButton;
                if (desc.frame.borderTop.isValid()) {
                    borderColor = desc.frame.borderTop;
                }

                if ((option->state & State_MouseOver) && (option->state & State_Enabled)) {
                    borderColor = desc.frame.borderHovered;
                }

                painter->setPen(QPen(borderColor, 1));
                painter->setBrush(Qt::NoBrush);

                if (isInput && !m_isClassic) {
                    painter->drawRoundedRect(crispRect(option->rect), 2.0, 2.0);
                } else {
                    painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
                }
            } else {
                // Draw individual borders as specified by the active BoxDecoration (e.g. omitting left border)
                QColor tc = desc.frame.borderTop;
                QColor bc = desc.frame.borderBottom;
                QColor lc = desc.frame.borderLeft;
                QColor rc = desc.frame.borderRight;

                if ((option->state & State_MouseOver) && (option->state & State_Enabled)) {
                    if (tc.isValid()) tc = desc.frame.borderHovered;
                    if (bc.isValid()) bc = desc.frame.borderHovered;
                    if (lc.isValid()) lc = desc.frame.borderHovered;
                    if (rc.isValid()) rc = desc.frame.borderHovered;
                }

                const QRectF rect = crispRect(option->rect);

                if (desc.frame.hasTopBottomBorder) {
                    if (tc.isValid()) {
                        painter->setPen(tc);
                        painter->drawLine(rect.topLeft(), rect.topRight());
                    }
                    if (bc.isValid()) {
                        painter->setPen(bc);
                        painter->drawLine(rect.bottomLeft(), rect.bottomRight());
                    }
                }
                if (desc.frame.hasSideBorders) {
                    if (lc.isValid()) {
                        painter->setPen(lc);
                        painter->drawLine(rect.topLeft(), rect.bottomLeft());
                    }
                    if (rc.isValid()) {
                        painter->setPen(rc);
                        painter->drawLine(rect.topRight(), rect.bottomRight());
                    }
                }
            }
        }
    }

    // Draw localized interactive outer glows only on the focused frame
    if (hasFocus && m_useFocusedInputGlow && isInput) {
        LCPainterGuard guard(painter, true);

        painter->setPen(QPen(desc.input.inputFocusGlowOuter, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(crispRect(option->rect, geoms.focusGlow.radius1), geoms.focusGlow.radius1, geoms.focusGlow.radius1);

        painter->setPen(QPen(desc.input.inputFocusGlowInner, 1));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(geoms.focusGlow.margin2, geoms.focusGlow.margin2,
                                                              -geoms.focusGlow.margin2, -geoms.focusGlow.margin2),
                                 geoms.focusGlow.radius2, geoms.focusGlow.radius2);

        painter->setPen(QPen(option->palette.color(finalGroup, QPalette::Highlight), 1));
        painter->drawRoundedRect(QRectF(option->rect).adjusted(geoms.focusGlow.margin3, geoms.focusGlow.margin3,
                                                              -geoms.focusGlow.margin3, -geoms.focusGlow.margin3),
                                 geoms.focusGlow.radius3, geoms.focusGlow.radius3);
    }
}

void LC_ProxyStyle::drawSpinBoxProgressBar(QPainter *painter, const QRect &rect, const SkinScaledGeometries &geoms, const SkinColors &desc,
    qreal pct, bool hasFocus, const QWidget *widget, const QAbstractSpinBox *spinBox) const {

    // Resolve sub-control regions and progress offsets
    const LC_SkinWidgetsLayoutResolver::SpinBoxProgressBarLayout layout = LC_SkinWidgetsLayoutResolver::resolveSpinBoxProgressBarLayout(
        this, rect, geoms, pct, hasFocus, widget, spinBox, m_useFocusedInputGlow);

    LCPainterGuard progressGuard(painter, true);
    painter->setClipping(false);

    QColor highlightCol = desc.common.highlightColor.isValid()
                        ? desc.common.highlightColor
                        : (widget ? widget->palette().color(QPalette::Active, QPalette::Highlight)
                                  : QApplication::palette().color(QPalette::Active, QPalette::Highlight));

    // 1. Draw Empty Track Background (Boosted to 18% opacity when focused, 6% when idle)
    QColor trackColor = hasFocus ? desc.spinBox.spinBoxProgressBarTrackFocused
                             : desc.spinBox.spinBoxProgressBarTrack;
    painter->fillRect(layout.bgRect, trackColor);

    // 2. Draw Progress Fill Bar
    if (layout.fillRect.width() > 0) {
        if (m_isSoftSatin) {
            QLinearGradient fillGrad(layout.fillRect.topLeft(), layout.fillRect.bottomLeft());
            if (hasFocus) {
                fillGrad.setColorAt(0.0, desc.spinBox.spinBoxProgressBarSatinFocusedTop);
                fillGrad.setColorAt(1.0, desc.spinBox.spinBoxProgressBarSatinFocusedBottom);
            }
            else {
                fillGrad.setColorAt(0.0, desc.spinBox.spinBoxProgressBarSatinTop);
                fillGrad.setColorAt(1.0, desc.spinBox.spinBoxProgressBarSatinBottom);
            }
            painter->fillRect(layout.fillRect, fillGrad);
        }
        else if (m_isGlossy) {
            QLinearGradient fillGrad(layout.fillRect.topLeft(), layout.fillRect.bottomLeft());
            if (hasFocus) {
                fillGrad.setColorAt(0.0, desc.spinBox.spinBoxProgressBarGlassFocused0);
                fillGrad.setColorAt(0.42, desc.spinBox.spinBoxProgressBarGlassFocused42);
                fillGrad.setColorAt(0.55, desc.spinBox.spinBoxProgressBarGlassFocused55);
                fillGrad.setColorAt(1.0, desc.spinBox.spinBoxProgressBarGlassFocused100);
            }
            else {
                fillGrad.setColorAt(0.0, desc.spinBox.spinBoxProgressBarGlass0);
                fillGrad.setColorAt(0.42, desc.spinBox.spinBoxProgressBarGlass42);
                fillGrad.setColorAt(0.55, desc.spinBox.spinBoxProgressBarGlass55);
                fillGrad.setColorAt(1.0, desc.spinBox.spinBoxProgressBarGlass100);
            }
            painter->fillRect(layout.fillRect, fillGrad);
        }
        else {
            QColor solidFill = hasFocus ? desc.spinBox.spinBoxProgressBarFocusedFill : desc.spinBox.spinBoxProgressBarFill;
            painter->fillRect(layout.fillRect, solidFill);
        }

        // 3. Draw fully opaque (100%) vertical leading-edge line at progress tip
        painter->fillRect(layout.cursorRect, highlightCol);
    }
}

void LC_ProxyStyle::drawCustomGroupBoxFrame(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const {
    Q_UNUSED(widget);
    LCPainterGuard guard(painter, false);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const SkinColors desc = getStyleDescriptor(option);

    painter->setPen(QPen(desc.groupBox.groupBoxFrameColor, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(option->rect.adjusted(0, geoms.ints.scale8, -1, -1));
}

void LC_ProxyStyle::drawCustomGroupBox(const QStyleOptionGroupBox *option,
                                             QPainter *painter,
                                             const QWidget *widget) const {
    const QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
    const QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);

    const QPalette::ColorGroup group = resolveColorGroup(option->state);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    // Resolve dynamic group box layouts cleanly
    const GroupBoxLayout layout = LC_SkinWidgetsLayoutResolver::resolveGroupBoxLayout(this, option, desc, geoms, textRect, checkBoxRect, widget);

    LCPainterGuard guard(painter, false);

    // Pass 2: ClipRegion precomputation
    QRegion clipRegion;
    const bool hasBreak = (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::BreakBorder);

    if (hasBreak && (option->subControls & SC_GroupBoxLabel) && !option->text.isEmpty()) {
        clipRegion = QRegion(option->rect.adjusted(-geoms.ints.scale2, -geoms.ints.scale2, geoms.ints.scale2, geoms.ints.scale2));

        QRect clipOut = layout.drawTextRect;
        if (option->subControls & SC_GroupBoxCheckBox) {
            clipOut.setLeft(layout.drawCheckBoxRect.left());
        }

        const int lineGap = geoms.scaledMetrics.groupBoxTitleLineGap;
        clipOut.adjust(-lineGap, 0, lineGap, 0);
        clipRegion -= clipOut;
    }

    // Pass 3: Draw Frame Boundaries
    if (desc.groupBox.groupBoxHeaderStyle != GroupBoxHeaderStyle::HeaderBanner) {
        drawGroupBoxBoundary(painter, option, desc, layout, clipRegion);
    }

    // Pass 4: Draw Headers & Underlines
    drawGroupBoxHeader(painter, option, desc, layout, textRect, checkBoxRect);

    // Pass 5: Draw Overlay Checkbox Control
    if (option->subControls & SC_GroupBoxCheckBox) {
        QStyleOptionButton boxOpt;
        boxOpt.QStyleOption::operator=(*option);
        boxOpt.rect = layout.drawCheckBoxRect;
        boxOpt.state = option->state;
        boxOpt.state &= ~(State_MouseOver | State_Sunken | State_On | State_Off);

        if (option->state & State_On) {
            boxOpt.state |= State_On;
        } else if (option->state & State_Off) {
            boxOpt.state |= State_Off;
        }
        if (option->state & State_Sunken) boxOpt.state |= State_Sunken;
        if (option->state & State_MouseOver) boxOpt.state |= State_MouseOver;

        proxy()->drawPrimitive(PE_IndicatorCheckBox, &boxOpt, painter, widget);
    }

    // Pass 6: Draw Text Caption Label (Uses localized RAII block to restore pen color)
    if (option->subControls & SC_GroupBoxLabel && !option->text.isEmpty()) {
        const QColor txtColor = (option->state & State_Enabled && option->state & State_MouseOver)
                          ? desc.common.highlightColor
                          : desc.groupBox.groupBoxTextColor;

        {
            LCPainterGuard textGuard(painter);
            painter->setPen(txtColor);

            constexpr int textFlags = Qt::TextShowMnemonic | Qt::AlignLeft | Qt::AlignVCenter;
            proxy()->drawItemText(painter, layout.drawTextRect, textFlags, option->palette, option->state & State_Enabled,
                                  option->text, QPalette::NoRole);
        }
    }
}

void LC_ProxyStyle::drawCustomSlider(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const SkinColors desc = getStyleDescriptor(option);

    // Resolve Slider geometries completely out of the paint pass
    const LC_SkinWidgetsLayoutResolver::SliderLayout layout = LC_SkinWidgetsLayoutResolver::resolveSliderLayout(this, option, geoms, widget);

    if ((option->subControls & SC_SliderGroove) && layout.grooveRect.isValid()) {
        const QColor trackColor  = desc.slider.grooveBg;
        const QColor borderColor = desc.slider.grooveBorder;
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(trackColor);
        painter->drawRoundedRect(layout.grooveRect, 1.5, 1.5);
    }

    if ((option->subControls & SC_SliderHandle) && layout.handleRect.isValid()) {
        const QColor handleColor = (option->state & State_MouseOver) ? desc.slider.sliderHandleColorHovered
                                                               : desc.slider.sliderHandleColor;
        const QColor borderColor  = desc.slider.handleBorder;

        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(handleColor);
        painter->drawEllipse(layout.circleRect);

        painter->setPen(Qt::NoPen);
        painter->setBrush(desc.slider.sliderInnerDotColor);
        painter->drawEllipse(layout.innerDotRect);
    }
}

void LC_ProxyStyle::drawCustomSplitter(const QStyleOption *option,
                                             QPainter *painter,
                                             const QWidget *widget) const {
    const QPalette::ColorGroup group = resolveColorGroup(option->state);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const bool isDockSplitter = !widget || !qobject_cast<const QSplitter*>(widget);
    const QRect rect = option->rect;

    const bool handleIsHorizontal = isDockSplitter ? (option->state & State_Horizontal) : !(option->state & State_Horizontal);

    painter->save();

    if (m_highlightSplitterOnDrag && (option->state & State_Sunken)) {
        painter->fillRect(rect, desc.common.highlightColor);
        painter->restore();
        return;
    }

    const QPoint cx = rect.center();
    const bool isIdleDockSplitter = isDockSplitter && !m_persistentDockSplitter &&
                              !(option->state & State_MouseOver) && !(option->state & State_Sunken);

    if (!isIdleDockSplitter) {
        painter->fillRect(rect, desc.common.bgStart);

        if (option->state & State_MouseOver) {
            painter->fillRect(rect, desc.splitter.splitterHoverGlowColor);
        }

        const QColor lineColor = (option->state & (State_MouseOver | State_Sunken)) ? desc.common.highlightColor : desc.splitter.splitterGripColor;

        painter->setPen(QPen(lineColor, 1));
        if (handleIsHorizontal) {
            painter->drawLine(rect.left(), cx.y(), rect.right(), cx.y());
        } else {
            painter->drawLine(cx.x(), rect.top(), cx.x(), rect.bottom());
        }
    }

    const QColor gripColor = (m_accentGrips || (option->state & State_MouseOver)) ? desc.common.highlightColor : desc.splitter.splitterGripColorIdle;

    int baseHandleLen = geoms.scaledMetrics.splitterHandleLength;
    if (baseHandleLen < 0) {
        baseHandleLen = handleIsHorizontal ? rect.width() : rect.height();
    } else {
        baseHandleLen = qMin(baseHandleLen, handleIsHorizontal ? rect.width() : rect.height());
    }

    // Resolve grip and background well geometries
    const LC_SkinWidgetsLayoutResolver::GripLayout layout = LC_SkinWidgetsLayoutResolver::resolveGripLayout(option, geoms, m_showGripBackgroundWell, baseHandleLen, handleIsHorizontal);

    if (layout.showWell) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(desc.splitter.splitterGripWellColor);
        painter->drawRoundedRect(layout.wellRect, layout.wellRadius, layout.wellRadius);
    }

    drawUnifiedGripPattern(painter, cx, rect, handleIsHorizontal, gripColor, m_splitterGripStyle, layout.handleLength, geoms);

    painter->restore();
}

void LC_ProxyStyle::drawCustomPanelButtonTool(const QStyleOption *option,
                                                    QPainter *painter,
                                                    const QWidget *widget) const {
    if (g_inSegmentedDraw) {
        return;
    }

    if (!widget || !m_useToolButtonUnderline || isTitleOrDockButton(widget)) {
        drawCustomPanelButton(option, painter, widget);
        return;
    }

    QPalette::ColorGroup group = resolveColorGroup(option->state);
    SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    QRect rect = option->rect;
    bool selected = (option->state & State_On);
    bool hovered = (option->state & State_MouseOver);
    bool sunken = (option->state & State_Sunken);
    bool autoRaise = (option->state & State_AutoRaise);

    // Delegate accent boundaries and wel geometry straight to the layout resolver
    const  LC_SkinWidgetsLayoutResolver::ToolButtonAccessoryLayout layout = LC_SkinWidgetsLayoutResolver::resolveToolButtonAccessoryLayout(
        option, geoms, m_useToolButtonUnderline, m_toolButtonIndicatorStyle, widget, autoRaise, m_showGripBackgroundWell);

    LCPainterGuard guard(painter);

    if (selected) {
        if (layout.drawWell) {
            QColor wellBgColor = hovered ? desc.toolButton.toolButtonWellColorHover : desc.toolButton.toolButtonWellColor;
            painter->setPen(Qt::NoPen);
            painter->setBrush(wellBgColor);
            painter->drawRoundedRect(layout.wellRect, 2, 2);
        } else {
            QColor activeBgColor = hovered ? desc.toolButton.toolButtonBgCheckedHover : desc.button.bgChecked;
            painter->fillRect(rect, activeBgColor);
        }
    }
    else if (sunken) {
        painter->fillRect(rect, desc.button.bgSunken);
    }
    else if (hovered) {
        painter->fillRect(rect, desc.button.bgHovered);
    }
    else if (!autoRaise) {
        painter->fillRect(rect, desc.button.bgButton);
    }

    if (layout.drawIndicator) {
        QColor lineCol = hovered ? desc.toolButton.toolButtonIndicatorColorHover : desc.toolButton.toolButtonIndicatorColor;
        painter->setPen(Qt::NoPen);
        painter->setBrush(lineCol);

        if (layout.isIndicatorDot) {
            painter->drawEllipse(layout.indicatorDotCenter, geoms.ints.scale2, geoms.ints.scale2);
        } else if (m_toolButtonIndicatorStyle == ToolButtonIndicatorStyle::ContextStripe) {
            painter->drawRoundedRect(layout.indicatorRect, 1.0, 1.0);
        } else if (m_toolButtonIndicatorStyle == ToolButtonIndicatorStyle::AccentFrame) {
            painter->setPen(QPen(lineCol, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(layout.indicatorRect, geoms.ints.scale3, geoms.ints.scale3);
        }
    }
}

void LC_ProxyStyle::drawSegmentedToolButton(const QStyleOption *option,
                                                 QPainter *painter,
                                                 const QWidget *widget,
                                                 int mask) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    QPalette::ColorGroup group = (option->state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(option->state & State_Active)) group = QPalette::Inactive;

    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);

    const bool selected = (option->state & State_On);
    const bool hovered = (option->state & State_MouseOver);
    const bool sunken = (option->state & State_Sunken);

    // Delegate corner-radii math and indicators directly to the resolver
    const LC_SkinWidgetsLayoutResolver::SegmentedToolButtonLayout layout = LC_SkinWidgetsLayoutResolver::resolveSegmentedToolButtonLayout(
        option, geoms, mask, m_useToolButtonUnderline, m_toolButtonIndicatorStyle, widget);

    QColor bgCol = Qt::transparent;
    if (selected) {
        bgCol = hovered ? desc.toolButton.toolButtonBgCheckedHover : desc.button.bgChecked;
    } else if (sunken) {
        bgCol = desc.button.bgSunken;
    } else if (hovered) {
        bgCol = desc.button.bgHovered;
    }

    if (bgCol != Qt::transparent) {
        painter->fillPath(layout.buttonPath, bgCol);
    }

    if (!layout.dividerLines.isEmpty()) {
        painter->setPen(QPen(desc.splitter.splitterGripColorIdle, 1.0));
        for (const QLineF &divider : layout.dividerLines) {
            painter->drawLine(divider);
        }
    }

    if (layout.drawIndicator) {
        const QColor lineCol = hovered ? desc.toolButton.toolButtonIndicatorColorHover : desc.toolButton.toolButtonIndicatorColor;
        painter->setPen(Qt::NoPen);
        painter->setBrush(lineCol);

        if (layout.isIndicatorDot) {
            painter->drawEllipse(layout.indicatorDotCenter, geoms.ints.scale2, geoms.ints.scale2);
        } else if (m_toolButtonIndicatorStyle == ToolButtonIndicatorStyle::ContextStripe) {
            painter->drawRoundedRect(layout.indicatorRect, 1.0, 1.0);
        } else if (m_toolButtonIndicatorStyle == ToolButtonIndicatorStyle::AccentFrame) {
            painter->setPen(QPen(lineCol, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(layout.indicatorRect, geoms.ints.scale3, geoms.ints.scale3);
        }
    }
}


void LC_ProxyStyle::drawSegmentedGroupBackdrops(QPainter *painter, const QWidget *widget) const {
    if (!m_useSegmentedToolButtons) {
        return;
    }

    if (!widget || !painter) return;

    const QPalette::ColorGroup group = widget->isEnabled() ? QPalette::Active : QPalette::Disabled;
    const SkinColors desc = getCachedStyleDescriptor(widget->palette(), group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    QMap<int, QList<QToolButton*>> groups;
    const QList<QToolButton*> widgets = widget->findChildren<QToolButton*>();
    for (QToolButton *w : widgets) {
        if (!w->isVisible()) continue;
        QVariant groupVal = w->property(PROP_BUTTON_GROUP);
        if (groupVal.isValid()) {
            groups[groupVal.toInt()].append(w);
        }
    }

    if (groups.isEmpty()) return;

    // Self-healing synchronization check: recompute if current paint-time palette differs from cached properties
    const QColor currentWinColor = widget->palette().color(QPalette::Window);
    const QColor cachedWinColor = widget->property(PROP_GROUP_PALETTE_COLOR).value<QColor>();

    if (cachedWinColor != currentWinColor) {
        precomputeSegmentedGroupColors(const_cast<QWidget*>(widget), groups.size());
    }

    QVariantMap bgStartMap = widget->property(PROP_GROUP_BG_START).toMap();
    QVariantMap bgEndMap   = widget->property(PROP_GROUP_BG_END).toMap();

    QVariantMap stop0Map   = widget->property(PROP_GROUP_STOP0).toMap();
    QVariantMap stop42Map  = widget->property(PROP_GROUP_STOP42).toMap();
    QVariantMap stop55Map  = widget->property(PROP_GROUP_STOP55).toMap();
    QVariantMap stop100Map = widget->property(PROP_GROUP_STOP100).toMap();

    QVariantMap bgStartHoveredMap = widget->property(PROP_GROUP_BG_START_HOVERED).toMap();
    QVariantMap bgEndHoveredMap   = widget->property(PROP_GROUP_BG_END_HOVERED).toMap();

    QVariantMap stop0HoveredMap   = widget->property(PROP_GROUP_STOP0_HOVERED).toMap();
    QVariantMap stop42HoveredMap  = widget->property(PROP_GROUP_STOP42_HOVERED ).toMap();
    QVariantMap stop55HoveredMap  = widget->property(PROP_GROUP_STOP55_HOVERED).toMap();
    QVariantMap stop100HoveredMap = widget->property(PROP_GROUP_STOP100_HOVERED).toMap();

    LCPainterGuard guard(painter, true);

    for (auto it = groups.begin(); it != groups.end(); ++it) {
        const int groupId = it.key();
        const QList<QToolButton*>& buttons = it.value();
        if (buttons.isEmpty()) continue;

        bool groupHasAutoRaise = false;
        if (!buttons.isEmpty()) {
            groupHasAutoRaise = buttons.first()->autoRaise();
        }

        const QVariant parentHoverVar = widget->property(PROP_HOVERED_GROUP_ID);
        const bool isGroupHovered = parentHoverVar.isValid() && (parentHoverVar.toInt() == groupId);

        bool drawBg = true;
        bool drawBorder = true;

        if (groupHasAutoRaise) {
            if (m_segmentedSeparationStyle == SegmentedSeparationStyle::TransparentIdle) {
                drawBg = isGroupHovered;
                drawBorder = isGroupHovered;
            } else if (m_segmentedSeparationStyle == SegmentedSeparationStyle::MinimalBorder) {
                drawBg = isGroupHovered;
                drawBorder = true;
            } else if (m_segmentedSeparationStyle == SegmentedSeparationStyle::ContinuousCard) {
                drawBg = true;
                drawBorder = true;
            }
        }

        if (!drawBg && !drawBorder) {
            continue;
        }

        // Delegate dynamic, math-heavy path merging to the layout resolver
        const QPainterPath groupPath = LC_SkinWidgetsLayoutResolver::resolveSegmentedGroupPath(buttons, geoms.dpr);

        const QString key = QString::number(groupId);
        const QColor bgStart = isGroupHovered ? bgStartHoveredMap.value(key, desc.tab.bgTabInactive).value<QColor>()
                                        : bgStartMap.value(key, desc.tab.bgTabInactive).value<QColor>();
        const QColor bgEnd   = isGroupHovered ? bgEndHoveredMap.value(key, desc.tab.bgTabInactive).value<QColor>()
                                        : bgEndMap.value(key, desc.tab.bgTabInactive).value<QColor>();

        if (drawBg) {
            const bool useActiveTabGradients = (m_isSoftSatin || m_isGlossy);

            if (useActiveTabGradients && m_isGlossy) {
                QLinearGradient glassGrad(groupPath.boundingRect().topLeft(), groupPath.boundingRect().bottomLeft());

                glassGrad.setColorAt(0.0,   isGroupHovered ? stop0HoveredMap.value(key, bgStart).value<QColor>()   : stop0Map.value(key, bgStart).value<QColor>());
                glassGrad.setColorAt(0.42,  isGroupHovered ? stop42HoveredMap.value(key, bgStart).value<QColor>()  : stop42Map.value(key, bgStart).value<QColor>());
                glassGrad.setColorAt(0.55,  isGroupHovered ? stop55HoveredMap.value(key, bgEnd).value<QColor>()    : stop55Map.value(key, bgEnd).value<QColor>());
                glassGrad.setColorAt(1.0,   isGroupHovered ? stop100HoveredMap.value(key, bgEnd).value<QColor>()   : stop100Map.value(key, bgEnd).value<QColor>());

                painter->fillPath(groupPath, glassGrad);
            } else if (useActiveTabGradients && m_isSoftSatin) {
                QLinearGradient grad(groupPath.boundingRect().topLeft(), groupPath.boundingRect().bottomLeft());
                grad.setColorAt(0.0, bgStart);
                grad.setColorAt(1.0, bgEnd);
                painter->fillPath(groupPath, grad);
            } else {
                painter->fillPath(groupPath, bgStart);
            }
        }

        if (drawBorder) {
            const QColor borderCol = isGroupHovered ? desc.frame.borderHovered : desc.frame.borderButton;
            painter->setPen(QPen(borderCol, 1.0));
            painter->drawPath(groupPath);
        }
    }
}

void LC_ProxyStyle::drawCustomScrollBar(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget) const {
    LCPainterGuard guard(painter, false);

    QPalette::ColorGroup group = (option->state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(option->state & State_Active)) group = QPalette::Inactive;

    SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    // Fetch precomputed ScrollBar layout coordinates and handle boundaries
    const LC_SkinWidgetsLayoutResolver::ScrollBarLayout layout = LC_SkinWidgetsLayoutResolver::resolveScrollBarLayout(
        this, option, geoms, widget, m_transparentScrollbars);

    // 1. Resolve and render background track from cache
    const QColor trackColor = desc.scrollBar.scrollBarTrackColor;

    if (layout.subPageRect.isValid()) painter->fillRect(layout.subPageRect, trackColor);
    if (layout.addPageRect.isValid()) painter->fillRect(layout.addPageRect, trackColor);

    // 2. Resolve and render the handle
    if (layout.handleValid) {
        const QColor handleColor = (option->state & State_MouseOver) ? desc.scrollBar.scrollBarHandleColorHovered
                                                               : desc.scrollBar.scrollBarHandleColor;

        const QColor borderColor = (option->state & State_MouseOver) ? desc.scrollBar.scrollBarBorderColorHovered
                                                               : desc.scrollBar.scrollBarBorderColor;

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(handleColor);
        painter->drawRoundedRect(layout.adjustedHandleRect, layout.handleRadius, layout.handleRadius);
    }
}

void LC_ProxyStyle::drawCustomToolBar(const QStyleOptionToolBar *option, QPainter *painter, const QWidget *widget) const {
    Q_UNUSED(widget);
    QPalette::ColorGroup group = (option->state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(option->state & State_Active))
        group = QPalette::Inactive;

    LCPainterGuard guard(painter, false);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);

    // Fill the toolbar panel with its Window background from the precomputed cache
    painter->fillRect(option->rect, desc.toolBar.bg);

    // If Light == Dark (Flat Edge mode), we bypass drawing borderlines completely
    const bool isFlatOrNone = m_isFlat || (desc.toolBar.borderLight == desc.toolBar.borderDark);

    if (!isFlatOrNone) {
        if (m_isFlat) {
            const QColor flatBorder = desc.toolBar.flatBorder;
            painter->setPen(flatBorder);
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        }
        else {
            // Traditional styles receive a physically consistent 3D chiseled frame
            const QColor lightBorder = desc.toolBar.borderLight;
            const QColor darkBorder = desc.toolBar.borderDark;

            painter->setPen(lightBorder);
            painter->drawLine(option->rect.topLeft(), option->rect.topRight());

            painter->setPen(darkBorder);
            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
        }
    }
}

void LC_ProxyStyle::drawCustomDockTitleBar(const QStyleOptionDockWidget *option,
                                                 QPainter *painter,
                                                 const QWidget *widget) const {

    LCPainterGuard guard(painter, false);
    const SkinColors desc = m_skinColorsResolver.resolveDockTitleBarDescriptor(option, widget);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const DockTitleBarStyle activeStyle = m_skinColorsResolver.activeDockTitleStyle();

    if (activeStyle == DockTitleBarStyle::Native) {
        if (m_isClassic) {
            QProxyStyle::drawControl(CE_DockWidgetTitle, option, painter, widget);
            return;
        }
    }

    drawParameterizedBox(painter, option->rect, desc, option->verticalTitleBar);

    if (activeStyle != DockTitleBarStyle::CustomSolid &&
        activeStyle != DockTitleBarStyle::CustomAccentOutline &&
        activeStyle != DockTitleBarStyle::Native) {

        const QColor highlightBorder = desc.dockTitleBar.highlightBorder;
        painter->setPen(highlightBorder);

        if (option->verticalTitleBar) {
            painter->drawLine(option->rect.left() + 1, option->rect.top(), option->rect.left() + 1, option->rect.bottom());
        } else {
            painter->drawLine(option->rect.left(), option->rect.top() + 1, option->rect.right(), option->rect.top() + 1);
        }
    }

    const bool isCustomWidget = widget && widget->inherits("LC_CustomTitleBarWidget");

    if (!isCustomWidget) {
        // Draw standard native title text
        const QString titleText = option->title;
        if (!titleText.isEmpty()) {
            QFont font = widget ? widget->font() : painter->font();
            font.setBold(false);
            painter->setFont(font);

            const auto buttonLayout = LC_SkinWidgetsLayoutResolver::resolveTitleBarButtonLayout(
                option->rect, geoms, option->closable ? (QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable) : QDockWidget::NoDockWidgetFeatures, option->verticalTitleBar);

            const int leftSpacing = desc.frame.hasLeftAccentBar ? geoms.groupBox.titleLeftSpacing + geoms.ints.scale4
                                                    : geoms.groupBox.titleLeftSpacing;

            if (option->verticalTitleBar) {
                painter->translate(option->rect.left(), option->rect.bottom());
                painter->rotate(-90);
                const QRect textRect(leftSpacing, 0, buttonLayout.textRect.height(), option->rect.width());
                painter->setPen(desc.common.textColor);
                painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, titleText);
            } else {
                const QRect textRect(buttonLayout.textRect.left(), option->rect.top(),
                                     buttonLayout.textRect.width(), option->rect.height());
                painter->setPen(desc.common.textColor);
                painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, titleText);
            }
        }
    } else {
        // Draw the unified grip handle inside the reserved layout margin area [74]
        const QPoint cx = option->verticalTitleBar
            ? QPoint(option->rect.center().x(), option->rect.top() + geoms.ints.scale8)
            : QPoint(option->rect.left() + geoms.ints.scale8, option->rect.center().y());

        const QColor gripColor = desc.splitter.splitterGripColorIdle;
        const int handleLen = geoms.ints.scale12;

        drawUnifiedGripPattern(painter, cx, option->rect, option->verticalTitleBar, gripColor, m_splitterGripStyle, handleLen, geoms);
    }
}


void LC_ProxyStyle::drawCustomToolTipCard(const QStyleOption *option,
                                                QPainter *painter,
                                                const QWidget *widget) const {
    Q_UNUSED(widget);
    const SkinColors desc = getStyleDescriptor(option);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    LCPainterGuard guard(painter, true);

    painter->setPen(Qt::NoPen);
    painter->setBrush(desc.tooltip.tooltipBg);
    painter->drawRoundedRect(crispRect(option->rect, geoms.crispOffset), geoms.tooltip.cornerRadius, geoms.tooltip.cornerRadius);

    painter->setPen(QPen(desc.tooltip.tooltipBorder, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(crispRect(option->rect, geoms.crispOffset), geoms.tooltip.cornerRadius, geoms.tooltip.cornerRadius);
}


void LC_ProxyStyle::drawCustomTabWidgetFrame(const QStyleOption *option, QPainter *painter,const QWidget *widget) const {
    const SkinColors desc = getStyleDescriptor(option);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    LCPainterGuard guard(painter, false);
    painter->setPen(QPen(desc.tab.tabWidgetFrameBorder, 1));
    painter->setBrush(Qt::NoBrush);

    // Resolve viewport frame masks cleanly from the resolver
    const  LC_SkinWidgetsLayoutResolver::TabWidgetFrameClipLayout layout = LC_SkinWidgetsLayoutResolver::resolveTabWidgetFrameClipLayout(option, geoms, widget, m_tabStripeAtBottom);

    if (layout.needClipping) {
        painter->setClipRegion(layout.clipRegion);
    }

    painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
    if (layout.needClipping) {
        painter->setClipping(false);
    }
}

void LC_ProxyStyle::drawCustomTabBarTab(const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const {
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const bool selected = (option->state & State_Selected);
    const bool hovered = (option->state & State_MouseOver);

    // Resolve tab borders and geometric paths
    const LC_SkinWidgetsLayoutResolver::TabLayout layout = LC_SkinWidgetsLayoutResolver::resolveTabLayout(option, widget, geoms, this, m_boxDecoration == BoxDecoration::LeftAccentBar, m_tabStripeAtBottom);
    const SkinColors tabDesc = m_skinColorsResolver.resolveTabBorders(option, widget);

    LCPainterGuard guard(painter, true);

    paintTabBackground(painter, layout.adjustedRect, option, tabDesc, layout.paths, selected);
    paintTabBorders(painter, option, tabDesc, layout.paths, selected);

    if (layout.hasActiveStripe) {
        painter->setPen(QPen(tabDesc.common.highlightColor, geoms.tab.stripeThickness, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(layout.stripeLine);
    }

    QStyleOptionTab tabCopy = *option;
    tabCopy.rect = layout.adjustedRect.toRect();
    if (m_isFlatModern && !selected) {
        tabCopy.palette.setColor(QPalette::WindowText, tabDesc.tab.textTabInactive);
        tabCopy.palette.setColor(QPalette::ButtonText, tabDesc.tab.textTabInactive);
    }
    QProxyStyle::drawControl(CE_TabBarTabLabel, &tabCopy, painter, widget);
}


void LC_ProxyStyle::drawCustomComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const {
    SkinColors desc = getStyleDescriptor(option);

    // Map interactive hover and sunken states dynamically inside the cached boundaries
    if (option->state & State_Sunken) {
        desc.common.bgStart = desc.button.bgSunken;
        desc.common.bgEnd = desc.button.bgSunken;
    } else if (option->state & State_MouseOver) {
        desc.common.bgStart = desc.button.bgHovered;
        desc.common.bgEnd = desc.button.bgHovered;
    }

    // Ensure ComboBoxes always maintain full, closed borders to match LineEdits and Push Buttons
    desc.frame.hasFullBorder = true;
    desc.frame.hasTopBottomBorder = false;
    desc.frame.hasSideBorders = false;
    desc.frame.hasLeftAccentBar = false;
    desc.frame.borderTop = desc.frame.borderBottom = desc.frame.borderLeft = desc.frame.borderRight = desc.frame.borderButton;

    drawParameterizedBox(painter, option->rect, desc);

    QProxyStyle::drawComplexControl(CC_ComboBox, option, painter, widget);
}

void LC_ProxyStyle::drawCustomSpinBox(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget) const {
    // Copy option and disable standard frame and 3D edit field background drawing
    // to prevent duplicate 3D bevel lines from rendering
    QStyleOptionSpinBox copy = *option;
    copy.subControls &= ~SC_SpinBoxFrame;
    copy.subControls &= ~SC_SpinBoxEditField;
    copy.frame = false;

    // 1. Let the base style draw the arrow buttons only
    QProxyStyle::drawComplexControl(CC_SpinBox, &copy, painter, widget);

    // 2. NOW draw our custom frame ON TOP of the buttons, but CLIP the painter
    // to exclude the buttons area so we do NOT paint over them!
    if (option->frame && (option->subControls & SC_SpinBoxFrame)) {
        const QRect upRect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
        const QRect downRect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

        LCPainterGuard guard(painter, false);

        // Exclude the button rectangles from the drawing region to prevent our background fill from covering them
        QRegion clipRegion(option->rect);
        if (!upRect.isEmpty())   clipRegion -= upRect;
        if (!downRect.isEmpty()) clipRegion -= downRect;
        painter->setClipRegion(clipRegion);

        QStyleOptionFrame frameOpt;
        frameOpt.rect = option->rect;
        frameOpt.palette = option->palette;
        frameOpt.state = option->state;
        drawCustomLineEditFrame(&frameOpt, painter, widget);
    }
}

void LC_ProxyStyle::drawCustomIndicatorArrow(PrimitiveElement element,
                                                   const QStyleOption *option,
                                                   QPainter *painter,
                                                   const QWidget *widget) const {
    LCPainterGuard guard(painter, true);

    const SkinColors desc = getStyleDescriptor(option);
    const QColor arrowColor = (option->state & State_MouseOver) ? desc.arrow.arrowColorHovered
                                                                : desc.arrow.arrowColor;

    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    // Fetch precomputed Arrow indicator polylines/lines
    const LC_SkinWidgetsLayoutResolver::ArrowLayout layout = LC_SkinWidgetsLayoutResolver::resolveArrowLayout(
        element, option, geoms, widget, m_customVectorIconsInButtons);

    if (layout.isPolygon) {
        QColor strokeColor = desc.groupBox.groupBoxFrameColor;

        if (option->state & State_Selected) {
            strokeColor = desc.arrow.menuArrowColorSelected;
        } else if (option->state & State_MouseOver) {
            strokeColor = desc.arrow.arrowColorHovered;
        }

        painter->setPen(QPen(strokeColor, 1.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPolyline(layout.polygon);
    } else if (layout.isLines) {
        painter->setPen(QPen(arrowColor, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(layout.line1);
        painter->drawLine(layout.line2);
    }
}


void LC_ProxyStyle::drawCustomIndicatorCheckBox(const QStyleOption *option,
                                                      QPainter *painter,
                                                      const QWidget *widget) const {
    LCPainterGuard guard(painter);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const SkinColors desc = getStyleDescriptor(option);

    const QRect rect = option->rect;
    const bool isCheckable = isWidgetCheckable(widget);
    const bool isHovered = (option->state & State_MouseOver);

    const QColor highlightColor = desc.checkbox.checkMark;
    QColor borderColor          = desc.checkbox.border;

    if (isHovered && isCheckable) {
        borderColor = desc.frame.borderHovered;
    }

    const QRectF boxRect = crispRect(rect, geoms.crispOffset);

    if (!isCheckable) {
        QPen borderPen(desc.frame.borderButton, 1);
        borderPen.setStyle(Qt::DashLine);
        painter->setPen(borderPen);
        painter->setBrush(desc.tab.bgTabInactive);
    } else {
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(desc.checkbox.bg);
    }

    painter->drawRect(boxRect);

    const int inset = geoms.checkbox.checkInset;

    if (option->state & State_On) {
        if (isCheckable) {
            painter->fillRect(rect.adjusted(1, 1, -1, -1), highlightColor);
            painter->setPen(QPen(Qt::white, geoms.checkbox.checkPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        } else {
            painter->fillRect(rect.adjusted(1, 1, -1, -1), desc.frame.borderButton);
            painter->setPen(QPen(desc.tab.bgTabInactive, geoms.checkbox.checkPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
        // Coordinate alignment relative to logical bounds
        painter->drawLine(rect.left() + inset, rect.top() + rect.height() / 2,
                          rect.left() + rect.width() / 2 - 1, rect.bottom() - inset);
        painter->drawLine(rect.left() + rect.width() / 2 - 1, rect.bottom() - inset,
                          rect.right() - inset, rect.top() + inset);
    }
    else if (option->state & State_NoChange) {
        if (isCheckable) {
            painter->fillRect(rect.adjusted(1, 1, -1, -1), highlightColor);
            painter->setPen(QPen(Qt::white, geoms.checkbox.checkPenWidth, Qt::SolidLine, Qt::RoundCap));
        } else {
            painter->fillRect(rect.adjusted(1, 1, -1, -1), desc.frame.borderButton);
            painter->setPen(QPen(desc.tab.bgTabInactive, geoms.checkbox.checkPenWidth, Qt::SolidLine, Qt::RoundCap));
        }
        painter->drawLine(rect.left() + inset, rect.center().y(), rect.right() - inset, rect.center().y());
    }
}

void LC_ProxyStyle::drawCustomIndicatorRadioButton(const QStyleOption *option,
                                                         QPainter *painter,
                                                         const QWidget *widget) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const SkinColors desc = getStyleDescriptor(option);

    const QRect rect = option->rect;
    const bool isCheckable = isWidgetCheckable(widget);
    const bool isHovered = (option->state & State_MouseOver);

    QColor borderColor          = desc.radioButton.border;

    if (isHovered && isCheckable) {
        borderColor = desc.frame.borderHovered;
    }

    const QRectF circleRect = crispRect(rect, geoms.crispOffset);

    if (!isCheckable) {
        QPen borderPen(desc.frame.borderButton, 1);
        borderPen.setStyle(Qt::DashLine);
        painter->setPen(borderPen);
        painter->setBrush(desc.tab.bgTabInactive);
    } else {
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(desc.radioButton.bg);
    }

    painter->drawEllipse(circleRect);

    if (option->state & State_On) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(isCheckable ? desc.radioButton.dot : desc.frame.borderButton);
        const int dotOffset = geoms.radioButton.dotOffset;
        painter->drawEllipse(circleRect.adjusted(dotOffset, dotOffset, -dotOffset, -dotOffset));
    }
}

void LC_ProxyStyle::drawCustomIndicatorToolBarSeparator(const QStyleOption *option,
                                                               QPainter *painter,
                                                               const QWidget *widget) const {
    Q_UNUSED(widget);
    LCPainterGuard guard(painter, false);
    const bool horizontal = option->state & State_Horizontal;
    const QRect rect = option->rect;
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const SkinColors desc = getStyleDescriptor(option);
    painter->setPen(desc.toolBar.toolbarSeparatorPen);

    const int offset1 = geoms.ints.scale1;
    const int offset2 = geoms.ints.scale2;

    if (m_isFlat) {
        if (horizontal) {
            painter->drawLine(rect.left(), rect.top() + offset2, rect.left(), rect.bottom() - offset2);
        } else {
            painter->drawLine(rect.left() + offset2, rect.top(), rect.right() - offset2, rect.top());
        }
    } else {
        if (horizontal) {
            painter->drawLine(rect.left(), rect.top() + offset2, rect.left(), rect.bottom() - offset2);
            painter->setPen(desc.toolBar.toolbarSeparatorHighlightPen);
            painter->drawLine(rect.left() + offset1, rect.top() + offset2, rect.left() + offset1, rect.bottom() - offset2);
        } else {
            painter->drawLine(rect.left() + offset2, rect.top(), rect.right() - offset2, rect.top());
            painter->setPen(desc.toolBar.toolbarSeparatorHighlightPen);
            painter->drawLine(rect.left() + offset2, rect.top() + offset1, rect.right() - 2, rect.top() + offset1);
        }
    }
}

void LC_ProxyStyle::drawCustomToolbarOverflowIndicator(const QStyleOption *option,
                                                            QPainter *painter,
                                                            const QWidget *widget) const {
    LCPainterGuard guard(painter, true);

    const SkinColors desc = getStyleDescriptor(option);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    QColor strokeColor = desc.toolBar.toolbarOverflowIndicatorColor;
    if (option->state & State_MouseOver) {
        strokeColor = desc.common.highlightColor;
    }

    const qreal strokeWidth = option->rect.width() >= 24 ? 1.8 : 1.25;
    painter->setPen(QPen(strokeColor, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const QPoint cx = option->rect.center();
    const int cy = cx.y();

    bool horizontal = true;
    if (widget && widget->parentWidget()) {
        if (const auto *toolbar = qobject_cast<const QToolBar*>(widget->parentWidget())) {
            horizontal = (toolbar->orientation() == Qt::Horizontal);
        }
    }

    const int s1 = geoms.ints.scale1;
    const int s3 = geoms.ints.scale3;
    const int s4 = geoms.ints.scale4;

    if (horizontal) {
        const bool rtl = (option->direction == Qt::RightToLeft);
        if (rtl) { // Point Left (RTL layouts)
            // Inner chevron
            painter->drawLine(QPointF(cx.x() + s4, cy - s4), QPointF(cx.x() + s1, cy));
            painter->drawLine(QPointF(cx.x() + s1, cy),     QPointF(cx.x() + s4, cy + s4));
            // Outer chevron
            painter->drawLine(QPointF(cx.x(),     cy - s4), QPointF(cx.x() - s3, cy));
            painter->drawLine(QPointF(cx.x() - s3, cy),     QPointF(cx.x(),     cy + s4));
        } else { // Point Right (LTR layouts)
            // Inner chevron
            painter->drawLine(QPointF(cx.x() - s4, cy - s4), QPointF(cx.x() - s1, cy));
            painter->drawLine(QPointF(cx.x() - s1, cy),     QPointF(cx.x() - s4, cy + s4));
            // Outer chevron
            painter->drawLine(QPointF(cx.x(),     cy - s4), QPointF(cx.x() + s3, cy));
            painter->drawLine(QPointF(cx.x() + s3, cy),     QPointF(cx.x(),     cy + s4));
        }
    } else { // Vertical: Point Down
        // Inner chevron
        painter->drawLine(QPointF(cx.x() - s4, cy - s4), QPointF(cx.x(), cy - s1));
        painter->drawLine(QPointF(cx.x(),     cy - s1), QPointF(cx.x() + s4, cy - s4));
        // Outer chevron
        painter->drawLine(QPointF(cx.x() - s4, cy),     QPointF(cx.x(), cy + s3));
        painter->drawLine(QPointF(cx.x(),     cy + s3), QPointF(cx.x() + s4, cy));
    }
}


void LC_ProxyStyle::drawCustomIndicatorToolBarHandle(const QStyleOption *option,
                                                           QPainter *painter,
                                                           const QWidget *widget) const {
    const QPalette::ColorGroup group = resolveColorGroup(option->state);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    bool isWrapped = false;
    if (widget && m_autoPopupToolbarOverflow) {
        if (auto *toolBar = qobject_cast<const QToolBar*>(widget)) {
            if (const auto *extButton = findToolbarExtensionButton(toolBar)) {
                isWrapped = extButton->isVisible() && !extButton->isChecked();
            }
        }
    }

    const bool customOverflowGrip = m_customToolbarOverflowGrip && isWrapped;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRect rect = option->rect;
    const QPoint cx = rect.center();
    const bool horizontal = option->state & State_Horizontal;

    if (customOverflowGrip) {
        QRegion clipRegion(rect);
        const int halfGap = geoms.ints.scale6;
        const int gap = geoms.ints.scale12;
        const QRect gapRect = horizontal ? QRect(rect.left(), cx.y() - halfGap, rect.width(), gap)
                                   : QRect(cx.x() - halfGap, rect.top(), gap, rect.height());
        clipRegion -= gapRect;
        painter->setClipRegion(clipRegion);
    }

    if (!m_customSplitterAnToolbarGrip) {
        QStyleOption copy = *option;
        copy.palette.setColor(QPalette::Mid, desc.splitter.splitterGripColor);
        copy.palette.setColor(QPalette::Midlight, desc.splitter.splitterGripLight);
        QProxyStyle::drawPrimitive(PE_IndicatorToolBarHandle, &copy, painter, widget);
    } else {
        const int baseHandleLen = qMin(geoms.ints.scale24, (horizontal ? rect.height() : rect.width()) - geoms.ints.scale4);

        // Resolve dynamic grip well bounding rects cleanly from the layout resolver
        const LC_SkinWidgetsLayoutResolver::GripLayout layout = LC_SkinWidgetsLayoutResolver::resolveGripLayout(option, geoms, m_showGripBackgroundWell, baseHandleLen, !horizontal);

        if (layout.showWell) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(desc.splitter.splitterGripWellColor);
            painter->drawRoundedRect(layout.wellRect, layout.wellRadius, layout.wellRadius);
        }

        const QColor gripColor = m_accentGrips ? desc.common.highlightColor : desc.splitter.splitterGripColorIdle;

        int gripLen = 0;
        if (m_splitterGripStyle == SplitterGripStyle::RoundedPill) {
            gripLen = geoms.ints.scale16;
        } else if (m_splitterGripStyle == SplitterGripStyle::DoubleRidges || m_splitterGripStyle == SplitterGripStyle::BorderHairline) {
            gripLen = -1;
        } else {
            gripLen = geoms.ints.scale12;
        }
        if (gripLen >= 0) {
            gripLen = layout.handleLength;
        }

        drawUnifiedGripPattern(painter, cx, rect, !horizontal, gripColor, m_splitterGripStyle, gripLen, geoms);
    }

    painter->restore();

    if (customOverflowGrip) {
        LCPainterGuard guard(painter, true);
        painter->setPen(QPen(desc.toolBar.toolbarOverflowIndicatorColor, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const int cy = cx.y();

        Qt::ToolBarArea area = Qt::NoToolBarArea;
        if (widget && widget->parentWidget()) {
            if (const auto *mainWindow = qobject_cast<QMainWindow*>(widget->parentWidget())) {
                area = mainWindow->toolBarArea(qobject_cast<const QToolBar*>(widget));
            }
        }

        if (horizontal) {
            const bool pointUp = (area == Qt::BottomToolBarArea);
            if (pointUp) {
                painter->drawLine(cx.x() - 3, cy + 2, cx.x(), cy - 1);
                painter->drawLine(cx.x(), cy - 1, cx.x() + 3, cy + 2);
            } else {
                painter->drawLine(cx.x() - 3, cy - 2, cx.x(), cy + 1);
                painter->drawLine(cx.x(), cy + 1, cx.x() + 3, cy - 2);
            }
        } else {
            const bool pointLeft = (area == Qt::RightToolBarArea);
            if (pointLeft) {
                painter->drawLine(cx.x() + 2, cy - 3, cx.x() - 1, cy);
                painter->drawLine(cx.x() - 1, cy, cx.x() + 2, cy + 3);
            } else {
                painter->drawLine(cx.x() - 2, cy - 3, cx.x() + 1, cy);
                painter->drawLine(cx.x() + 1, cy, cx.x() - 2, cy + 3);
            }
        }
    }
}


void LC_ProxyStyle::drawCustomIndicatorBranch(const QStyleOption *option,
                                                    QPainter *painter,
                                                    const QWidget *widget) const {
    Q_UNUSED(widget);
    QRect rect = option->rect;
    QPalette::ColorGroup group = (option->state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(option->state & State_Active)) group = QPalette::Inactive;

    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    // Resolve tree guideline lines and center coordinates mathematically
    const  LC_SkinWidgetsLayoutResolver::BranchLayout layout = LC_SkinWidgetsLayoutResolver::resolveBranchLayout(option, geoms);

    QColor indicatorColor = desc.groupBox.groupBoxFrameColor;
    QColor lineColor = desc.groupBox.groupBoxFrameColor;

    if (option->state & State_Selected) {
        indicatorColor = desc.itemView.branchIndicatorSelectedColor;
        lineColor = desc.itemView.branchLineSelectedColor;
    }
    else if (option->state & State_MouseOver) {
        indicatorColor = desc.itemView.branchIndicatorHoverColor;
    }

    LCPainterGuard guard(painter, false);

    // 1. Draw Connecting Guidelines
    if (m_showTreeConnectingLines && m_branchIndicatorStyle != BranchIndicatorStyle::None) {
        QPen linePen(desc.groupBox.groupBoxFrameColor, 1, Qt::DotLine);
        painter->setPen(linePen);
        if (layout.drawLines) {
        if (option->state & State_Sibling) {
                painter->drawLine(layout.verticalLine);
        }

        if (option->state & State_Item) {
                painter->drawLine(layout.horizontalLine);
            }
        }
    }

    // 2. Draw Expand/Collapse Indicator
    if (option->state & State_Children && m_branchIndicatorStyle != BranchIndicatorStyle::None &&
        m_branchIndicatorStyle != BranchIndicatorStyle::ClassicLinesOnly) {

        painter->setRenderHint(QPainter::Antialiasing, true);

        if (m_branchIndicatorStyle == BranchIndicatorStyle::MutedChevrons) {
            QPen pen(indicatorColor, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPolyline(layout.chevronPolygon);
        }
        else if (m_branchIndicatorStyle == BranchIndicatorStyle::PlusMinusBoxes) {
            QPen pen(indicatorColor, 1, Qt::SolidLine);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(layout.plusMinusBox);

            const int inset = qMax(1, geoms.ints.scale2);
            painter->drawLine(layout.plusMinusBox.left() + inset, layout.center.y(), layout.plusMinusBox.right() - inset, layout.center.y());

            if (!layout.isExpanded) {
                painter->drawLine(layout.center.x(), layout.plusMinusBox.top() + inset, layout.center.x(), layout.plusMinusBox.bottom() - inset);
            }
        }
        else if (m_branchIndicatorStyle == BranchIndicatorStyle::TactileCircles) {
            QPen pen(indicatorColor, 1, Qt::SolidLine);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(layout.center, layout.size / 2, layout.size / 2);

            const int inset = qMax(1, qRound(layout.size / 4.0));
            painter->drawLine(layout.center.x() - layout.size / 2 + inset, layout.center.y(), layout.center.x() + inset, layout.center.y());
            if (!layout.isExpanded) {
                painter->drawLine(layout.center.x(), layout.center.y() - inset, layout.center.x(), layout.center.y() + inset);
            }
        }
    }
}

void LC_ProxyStyle::drawCustomItemViewItem(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const {
    QPalette::ColorGroup group = (option->state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(option->state & State_Active))
        group = QPalette::Inactive;

    SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    bool selected = (option->state & State_Selected);
    bool hovered = (option->state & State_MouseOver);

    // Suppress hover calculations if disabled by the user (Point 2)
    if (!m_showItemViewHover) {
        hovered = false;
    }

    const auto* treeView  = qobject_cast<const QTreeView*>(widget);
    const auto* listView  = qobject_cast<const QListView*>(widget);
    const auto* tableView = qobject_cast<const QTableView*>(widget);

    // Retrieve row-level hover states instantly from the polished cache
    if (widget && widget->isEnabled() && m_showItemViewHover) {
        bool highlightWholeRow = widget->property(PROP_CACHED_HIGHLIGHT_ROW).toBool();

        if (highlightWholeRow) {
            const auto* itemView = qobject_cast<const QAbstractItemView*>(widget);
            if (itemView) {
                QWidget* viewport = itemView->viewport();
                QPoint localMousePos = viewport ? viewport->mapFromGlobal(QCursor::pos()) : widget->mapFromGlobal(QCursor::pos());
                QModelIndex hoveredIndex = itemView->indexAt(localMousePos);

                if (hoveredIndex.isValid() && hoveredIndex.row() == option->index.row() && hoveredIndex.parent() == option->index.
                    parent()) {
                    hovered = true;
                }
            }
        }
    }

    QStyleOptionViewItem copy = *option;
    if (!m_showItemViewHover) {
        copy.state &= ~State_MouseOver;
    }

    // --- REFINED: SOFT SELECTION BACKGROUND & TEXT ALIGNMENTS FOR TABLES & LISTS  ---
    if (selected && m_showActiveRowSpotlight) {
        copy.state &= ~State_Selected;

        // Shift cell bounds rightwards proportionally using scale5 register
        if (option->index.column() == 0) {
            copy.rect.adjust(geoms.ints.scale5, 0, 0, 0);
        }

        // Draw soft selection background behind non-tree cells (QTableView / QListView)
        if (!treeView) {
            LCPainterGuard guard(painter, false);
            QColor softSelectionBg = desc.itemView.bgItemHovered;
            painter->fillRect(option->rect, softSelectionBg);
        }
    }

    if (hovered && !selected && (option->state & State_Enabled)) {
        if (!treeView) {
            LCPainterGuard guard(painter, false);
            bool isAlternate = (option->features & QStyleOptionViewItem::Alternate);
            QColor hoverBg = isAlternate ? desc.itemView.bgItemAlternateHovered : desc.itemView.bgItemHovered;
            painter->fillRect(option->rect, hoverBg);
        }

        // Clear State_MouseOver so the base style skips drawing its own cell-level hover highlights
        copy.state &= ~State_MouseOver;
    }

    QProxyStyle::drawControl(CE_ItemViewItem, &copy, painter, widget);

    // --- UNIFIED SPOTLIGHT ACCENT OVERLAY FOR TREES, TABLES, AND LIST VIEWS (Point 2) ---
    if (selected && m_showActiveRowSpotlight) {
        bool isRowSelected = true;
        if (tableView) {
            isRowSelected = (tableView->selectionBehavior() == QAbstractItemView::SelectRows);
        }
        else if (listView) {
            isRowSelected = true; // Standard list views always highlight full row selections
        }

        if (option->index.column() == 0 && isRowSelected) {
            LCPainterGuard guard(painter, false);

            // Draw spotlight stripe with dynamic width using scale3 register
            QRect stripeRect(option->rect.left(), option->rect.top(), geoms.ints.scale3, option->rect.height());
            painter->fillRect(stripeRect, desc.common.highlightColor);
        }
    }
}

void LC_ProxyStyle::drawCustomMenuBarEmptyArea(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    Q_UNUSED(widget);
    const SkinColors desc = getStyleDescriptor(option);

    LCPainterGuard guard(painter, false);

    // Fill empty menubar area with standard Window background
    painter->fillRect(option->rect, desc.menuBar.bg);

    // Draw a bottom dividing hairline only if not flat and not in flat-edge (None) mode
    const bool isFlatOrNone = m_isFlat || (desc.toolBar.borderLight == desc.toolBar.borderDark);
    if (!isFlatOrNone) {
        painter->setPen(desc.menuBar.borderBottom);
        painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
    }
}

void LC_ProxyStyle::drawCustomMenuItem(const QStyleOptionMenuItem* option, QPainter* painter, const QWidget* widget) const {
    QStyleOptionMenuItem copy = *option;

    const QPalette::ColorGroup group = resolveColorGroup(option->state);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);

    if (option->state & State_Selected) {
        copy.palette.setBrush(QPalette::Highlight, desc.common.selectionHighlight);
        copy.palette.setColor(QPalette::HighlightedText, desc.common.textColor);
    }

    auto menu = qobject_cast<const QMenu*>(widget);
    bool hasAliases = m_showMenuCommandAliases && menu;
    int maxCmdWidth = 0;
    int maxShortcutWidth = 0;

    const QFont baseFont = widget ? widget->font() : QApplication::font();

    if (hasAliases) {
        hasAliases = getMenuCommandAliasInfo(widget, baseFont, maxCmdWidth, maxShortcutWidth);
    }

    if (hasAliases) {
        copy.text = ""; // Suppress standard text drawing inside base CE_MenuItem
    }

    // Standard drawing pass (handles background wells, checkmarks, sub-menu arrows)
    QProxyStyle::drawControl(CE_MenuItem, &copy, painter, widget);

    // Draw custom aligned columns if alias is active
    if (hasAliases) {
        drawMenuItemColumns(painter, option, desc, maxCmdWidth, maxShortcutWidth, baseFont, widget);
    }
}

void LC_ProxyStyle::drawCustomPanelItemViewRow(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const {
    const SkinColors desc = getStyleDescriptor(option);

    bool selected = (option->state & State_Selected);
    bool hovered = (option->state & State_MouseOver);
    if (!m_showItemViewHover) {
        hovered = false;
    }

    if (widget && widget->isEnabled() && m_showItemViewHover) {
        const bool highlightWholeRow = widget->property(PROP_CACHED_HIGHLIGHT_ROW).toBool();

        if (highlightWholeRow) {
            const auto* itemView = qobject_cast<const QAbstractItemView*>(widget);
            if (itemView) {
                QWidget* viewport = itemView->viewport();
                const QPoint localMousePos = viewport ? viewport->mapFromGlobal(QCursor::pos()) : widget->mapFromGlobal(QCursor::pos());
                const QModelIndex hoveredIndex = itemView->indexAt(localMousePos);

                if (hoveredIndex.isValid() && hoveredIndex.row() == option->index.row() && hoveredIndex.parent() == option->index.parent()) {
                    hovered = true;
                }
            }
        }
    }

    if (selected && m_showActiveRowSpotlight) {
        LCPainterGuard guard(painter, false);
        const QColor softSelectionBg = desc.itemView.bgItemHovered;
        painter->fillRect(option->rect, softSelectionBg);
        return;
    }

    if (hovered && !selected && (option->state & State_Enabled)) {
        LCPainterGuard guard(painter, false);
        const bool isAlternate = (option->features & QStyleOptionViewItem::Alternate);
        const QColor hoverBg = isAlternate ? desc.itemView.bgItemAlternateHovered : desc.itemView.bgItemHovered;
        painter->fillRect(option->rect, hoverBg);

        return;
    }

    QStyleOptionViewItem copy = *option;
    if (!m_showItemViewHover) {
        copy.state &= ~State_MouseOver;
    }

    QProxyStyle::drawPrimitive(PE_PanelItemViewRow, &copy, painter, widget);
}


void LC_ProxyStyle::drawCustomMenuBarItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const {
    QStyleOptionMenuItem copy = *option;

    const SkinColors desc = getStyleDescriptor(option);

    bool isHovered = (option->state & State_Selected);

    // Retrieve the hovered action from our extracted auto-popup event controller
    const LC_EventFilterAutoPopupController *controller = autoPopupController();
    const QPointer<QAction> hoveredAction = controller ? controller->hoveredMenuBarAction() : nullptr;

    // Symmetrically determine if the menu bar item is currently hovered in our state-machine
    if (!isHovered && m_useMenuBarHoverCard && hoveredAction) {
        if (cleanMnemonic(hoveredAction->text()) == cleanMnemonic(option->text)) {
            isHovered = true;
        }
    }

    QColor bgCol = desc.common.bgStart;
    if (option->state & State_Sunken) {
        bgCol = desc.button.bgSunken;
    } else if (isHovered) {
        bgCol = desc.common.selectionHighlight; // Hover state represents the soft selection highlight tint
        copy.state |= State_Selected;   // Force selection state so QCommonStyle draws the background highlight
    }

    copy.palette.setBrush(QPalette::Highlight, bgCol);
    copy.palette.setBrush(QPalette::Button, bgCol);

    // Force text color on selection to use standard window text color so it remains readable
    copy.palette.setColor(QPalette::HighlightedText, desc.common.textColor);
    copy.palette.setColor(QPalette::ButtonText, desc.common.textColor);
    copy.palette.setColor(QPalette::WindowText, desc.common.textColor);

    QProxyStyle::drawControl(CE_MenuBarItem, &copy, painter, widget);
}

QRect LC_ProxyStyle::subControlRect(const ComplexControl control,
                                          const QStyleOptionComplex *option, const SubControl subControl,
                                          const QWidget *widget) const {
    QRect rect = QProxyStyle::subControlRect(control, option, subControl, widget);

    if (control == CC_GroupBox) {
        if (subControl == SC_GroupBoxLabel && !m_isClassic) {
            const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(widget);
            // Apply pre-scaled margin spacing around text block
            const int lineGap = geoms.scaledMetrics.groupBoxTitleLineGap;
            rect.adjust(-lineGap, 0, lineGap, 0);
        }
    }
    return rect;
}


void LC_ProxyStyle::drawCustomIndicatorHeaderArrow(const QStyleOption *option, QPainter *painter) const {
    const QRect rect = option->rect;
    const QPalette::ColorGroup group = resolveColorGroup(option->state);

    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    LCPainterGuard guard(painter, true);

    QColor strokeColor = desc.groupBox.groupBoxFrameColor;
    if (option->state & State_Selected) {
        strokeColor = desc.common.highlightColor;
    }

    painter->setPen(QPen(strokeColor, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const QPoint cx = rect.center();
    const int size = geoms.ints.scale6; // Scale sorting arrow geometry dynamically

    const double quadSize = size/4.0;
    if (option->state & State_UpArrow) {
        QPolygonF chevron;
        chevron << QPointF(cx.x() - size/2.0, cx.y() + quadSize)
                << QPointF(cx.x(), cx.y() - quadSize)
                << QPointF(cx.x() + size/2.0, cx.y() + quadSize);
        painter->drawPolyline(chevron);
    } else if (option->state & State_DownArrow) {
        QPolygonF chevron;
        chevron << QPointF(cx.x() - size/2.0, cx.y() - quadSize)
                << QPointF(cx.x(), cx.y() + quadSize)
                << QPointF(cx.x() + size/2.0, cx.y() - quadSize);
        painter->drawPolyline(chevron);
    }
}

void LC_ProxyStyle::drawCustomIndicatorTabClose(const QStyleOption *option, QPainter *painter) const {
    const QRect rect = option->rect;
    const QPalette::ColorGroup group = resolveColorGroup(option->state);

    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    LCPainterGuard guard(painter);

    QColor strokeColor = desc.splitter.splitterGripColorIdle;
    if (option->state & State_MouseOver) {
        strokeColor = option->palette.color(group, QPalette::Highlight);

        painter->setPen(Qt::NoPen);
        painter->setBrush(desc.itemView.indicatorHoverWellColor);
        painter->drawEllipse(crispRect(rect, geoms.crispOffset));
    }

    // Standard uniform logical pen thickness
    painter->setPen(QPen(strokeColor, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);

    const QPoint cx = rect.center();
    const int hSize = qMax(2, rect.width() / 5);

    painter->drawLine(cx.x() - hSize, cx.y() - hSize, cx.x() + hSize, cx.y() + hSize);
    painter->drawLine(cx.x() - hSize, cx.y() + hSize, cx.x() + hSize, cx.y() - hSize);
}

void LC_ProxyStyle::drawCustomIndicatorTabTear(const PrimitiveElement element,
                                                      const QStyleOption *option,
                                                      QPainter *painter,
                                                      const QWidget *widget) const {
    Q_UNUSED(widget);
    LCPainterGuard guard(painter);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const QRect rect = option->rect;
    const SkinColors desc = getStyleDescriptor(option);
    QColor strokeColor = desc.tab.tearIndicatorColor;

    if (option->state & State_MouseOver) {
        strokeColor = desc.tab.tearIndicatorColorHover;
        painter->setPen(Qt::NoPen);
        painter->setBrush(desc.itemView.indicatorHoverWellColor);
        painter->drawEllipse(crispRect(rect, geoms.crispOffset));
    }

    // Standard uniform logical pen thickness
    painter->setPen(QPen(strokeColor, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const QPoint cx = rect.center();
    const int size = qMin(geoms.ints.scale5, rect.width() / 3);

    bool isLeft = (element == PE_IndicatorTabTearLeft);
    bool isVertical = false;

    if (element == PE_IndicatorTabTear) {
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
            isLeft = (tab->direction == Qt::RightToLeft);
            isVertical = (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::TriangularWest ||
                          tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast);
        }
    } else {
        isVertical = (element == PE_IndicatorTabTearLeft || element == PE_IndicatorTabTearRight);
    }

    const int halfSize = size / 2;
    if (isVertical) {
        if (isLeft) { // Point Up
            painter->drawLine(cx.x() - size, cx.y() + halfSize, cx.x(), cx.y() - halfSize);
            painter->drawLine(cx.x(), cx.y() - halfSize, cx.x() + size, cx.y() + halfSize);
        } else { // Point Down
            painter->drawLine(cx.x() - size, cx.y() - halfSize, cx.x(), cx.y() + halfSize);
            painter->drawLine(cx.x(), cx.y() + halfSize, cx.x() + size, cx.y() - halfSize);
        }
    } else {
        if (isLeft) { // Point Left
            painter->drawLine(cx.x() + halfSize, cx.y() - size, cx.x() - halfSize, cx.y());
            painter->drawLine(cx.x() - halfSize, cx.y(), cx.x() + halfSize, cx.y() + size);
        } else { // Point Right
            painter->drawLine(cx.x() - halfSize, cx.y() - size, cx.x() + halfSize, cx.y());
            painter->drawLine(cx.x() + halfSize, cx.y(), cx.x() - halfSize, cx.y() + size);
        }
    }
}

void LC_ProxyStyle::paintTabBackground(QPainter *painter, const QRectF &rect, const QStyleOptionTab *option, const SkinColors &desc, const TabPaths &paths, bool selected) const {
    Q_UNUSED(option);

    // Symmetrically draw standard Satin/Glossy gradients on the active selected tab face
    bool useActiveTabGradients = selected && (m_isSoftSatin || m_isGlossy);

    if (useActiveTabGradients && m_isGlossy) {
        QLinearGradient glassGrad(rect.topLeft(), rect.bottomLeft());
        glassGrad.setColorAt(0.0, desc.button.glassStart);
        glassGrad.setColorAt(0.42, desc.button.glassMidStart);
        glassGrad.setColorAt(0.55, desc.button.glassMidEnd);
        glassGrad.setColorAt(1.0, desc.button.glassEnd);
        painter->fillPath(paths.fillPath, glassGrad);
    } else if (useActiveTabGradients && m_isSoftSatin) {
        QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
        grad.setColorAt(0.0, desc.button.bgButton);
        grad.setColorAt(1.0, desc.button.bgButtonEnd);
        painter->fillPath(paths.fillPath, grad);
    } else if (desc.common.useGlassyGloss) {
        QLinearGradient glassGrad(rect.topLeft(), rect.bottomLeft());
        glassGrad.setColorAt(0.0, desc.button.glassStart);
        glassGrad.setColorAt(0.42, desc.button.glassMidStart);
        glassGrad.setColorAt(0.55, desc.button.glassMidEnd);
        glassGrad.setColorAt(1.0, desc.button.glassEnd);
        painter->fillPath(paths.fillPath, glassGrad);
    } else if (desc.common.useGradient) {
        QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
        grad.setColorAt(0.0, desc.common.bgStart);
        grad.setColorAt(1.0, desc.common.bgEnd);
        painter->fillPath(paths.fillPath, grad);
    } else {
        painter->fillPath(paths.fillPath, desc.common.bgStart);
    }
}

void LC_ProxyStyle::paintTabBorders(QPainter *painter, const QStyleOptionTab *option, const SkinColors &desc, const TabPaths &paths, bool selected) const {
    Q_UNUSED(option);

    if (m_isFlatModern) {
        if (desc.frame.hasFullBorder) {
            painter->setPen(QPen(desc.frame.borderTop, 1));
            painter->drawPath(paths.fillPath);
        } else {
            painter->setPen(QPen(desc.frame.borderButton, 1));
            painter->drawPath(paths.borderPath);
        }
    } else {
        if (desc.frame.hasFullBorder) {
            painter->setPen(QPen(desc.frame.borderTop, 1));
            painter->drawPath(paths.fillPath);
        } else {
            const QColor softBorder = desc.frame.borderTop.isValid() ? desc.frame.borderTop : desc.frame.borderHovered;
            painter->setPen(QPen(softBorder, 1));
            painter->drawPath(paths.borderPath);
        }
    }
}

void LC_ProxyStyle::paintTabActiveStripe(QPainter *painter, const QRectF &rect, const QStyleOptionTab *option, const SkinColors &desc) const {
    const bool isAccentOutline = m_isAccentOnline;
    if (isAccentOutline && (m_boxDecoration == BoxDecoration::Frameless || m_boxDecoration == BoxDecoration::DividingHairline)) {
        return;
    }

    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const QColor highlightColor = desc.common.highlightColor;
    painter->setPen(QPen(highlightColor, geoms.tab.stripeThickness, Qt::SolidLine, Qt::RoundCap));

    const qreal r = geoms.tab.roundedRadius;
    const qreal inset = geoms.tab.beveledInset;

    const TabShape shape = (option->shape == QTabBar::TriangularNorth || option->shape == QTabBar::TriangularSouth ||
                      option->shape == QTabBar::TriangularWest || option->shape == QTabBar::TriangularEast)
                     ? TabShape::Beveled : TabShape::Rounded;

    if (m_boxDecoration == BoxDecoration::LeftAccentBar && !m_tabStripeAtBottom) {
        const bool isHorizontal = (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth ||
                             option->shape == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth);

        if (isHorizontal) {
            if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
                if (shape == TabShape::Beveled) {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.stripeOffset, rect.bottom() - geoms.tab.marginDefault),
                                      QPointF(rect.left() + inset + geoms.tab.stripeOffset, rect.top() + geoms.tab.marginDefault));
                } else {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.stripeOffset, rect.bottom() - geoms.tab.marginDefault),
                                      QPointF(rect.left() + geoms.tab.stripeOffset, rect.top() + r));
                }
            } else { // South
                if (shape == TabShape::Beveled) {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.stripeOffset, rect.top() + geoms.tab.marginDefault),
                                      QPointF(rect.left() + inset + geoms.tab.stripeOffset, rect.bottom() - geoms.tab.marginDefault));
                } else {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.stripeOffset, rect.top() + geoms.tab.marginDefault),
                                      QPointF(rect.left() + geoms.tab.stripeOffset, rect.bottom() - r));
                }
            }
        } else {
            if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
                if (shape == TabShape::Beveled) {
                    painter->drawLine(QPointF(rect.right() - geoms.tab.marginDefault, rect.top() + geoms.tab.stripeOffset),
                                      QPointF(rect.left() + geoms.tab.marginDefault, rect.top() + inset + geoms.tab.stripeOffset));
                } else {
                    painter->drawLine(QPointF(rect.right() - geoms.tab.marginDefault, rect.top() + geoms.tab.stripeOffset),
                                      QPointF(rect.left() + r, rect.top() + geoms.tab.stripeOffset));
                }
            } else { // East
                if (shape == TabShape::Beveled) {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.marginDefault, rect.top() + geoms.tab.stripeOffset),
                                      QPointF(rect.right() - geoms.tab.marginDefault, rect.top() + inset + geoms.tab.stripeOffset));
                } else {
                    painter->drawLine(QPointF(rect.left() + geoms.tab.marginDefault, rect.top() + geoms.tab.stripeOffset),
                                      QPointF(rect.right() - r, rect.top() + geoms.tab.stripeOffset));
                }
            }
        }
    } else {
        const bool drawAtBase = m_tabStripeAtBottom;
        const bool isHorizontal = (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth ||
                             option->shape == QTabBar::RoundedSouth || option->shape == QTabBar::TriangularSouth);

        if (isHorizontal) {
            qreal y = 0.0;
            bool stripeIsAtBaseEdge = false;

            if (option->shape == QTabBar::RoundedNorth || option->shape == QTabBar::TriangularNorth) {
                if (drawAtBase) {
                    y = rect.bottom() - geoms.tab.stripeOffset;
                    stripeIsAtBaseEdge = true;
                } else {
                    y = rect.top() + geoms.tab.stripeOffset;
                }
            } else { // South
                if (drawAtBase) {
                    y = rect.top() + geoms.tab.stripeOffset;
                    stripeIsAtBaseEdge = true;
                } else {
                    y = rect.bottom() - geoms.tab.stripeOffset;
                }
            }

            qreal xStart = rect.left() + geoms.tab.marginDefault;
            qreal xEnd   = rect.right() - geoms.tab.marginDefault;

            if (!stripeIsAtBaseEdge) {
                if (shape == TabShape::Rounded) {
                    xStart = rect.left() + r;
                    xEnd   = rect.right() - r;
                } else if (shape == TabShape::Beveled) {
                    xStart = rect.left() + inset;
                    xEnd   = rect.right() - inset;
                }
            }
            painter->drawLine(QPointF(xStart, y), QPointF(xEnd, y));
        } else { // Vertical
            qreal x = 0.0;
            bool stripeIsAtBaseEdge = false;

            if (option->shape == QTabBar::RoundedWest || option->shape == QTabBar::TriangularWest) {
                if (drawAtBase) {
                    x = rect.right() - geoms.tab.stripeOffset;
                    stripeIsAtBaseEdge = true;
                } else {
                    x = rect.left() + geoms.tab.stripeOffset;
                }
            } else { // East
                if (drawAtBase) {
                    x = rect.left() + geoms.tab.stripeOffset;
                    stripeIsAtBaseEdge = true;
                } else {
                    x = rect.right() - geoms.tab.stripeOffset;
                }
            }

            qreal yStart = rect.top() + geoms.tab.marginDefault;
            qreal yEnd   = rect.bottom() - geoms.tab.marginDefault;

            if (!stripeIsAtBaseEdge) {
                if (shape == TabShape::Rounded) {
                    yStart = rect.top() + r;
                    yEnd   = rect.bottom() - r;
                } else if (shape == TabShape::Beveled) {
                    yStart = rect.top() + inset;
                    yEnd   = rect.bottom() - inset;
                }
            }
            painter->drawLine(QPointF(x, yStart), QPointF(x, yEnd));
        }
    }
}

bool LC_ProxyStyle::observeToolbarExtensionEvent(QObject *watched, QEvent *event) const {
    const QEvent::Type type = event->type();

    if (auto *extButton = qobject_cast<QToolButton*>(watched)) {
        if (isToolbarExtensionButton(extButton)) { // Refactored helper
            if (auto *toolBar = qobject_cast<QToolBar*>(extButton->parentWidget())) {
                if (m_customToolbarOverflowGrip && m_autoPopupToolbarOverflow && toolBar->isMovable()) {
                    if (type == QEvent::Show) {
                        toolBar->setProperty(PROP_IS_WRAPPED, true);
                        extButton->hide(); // Collapse the trailing layout gap natively
                        event->accept();
                        return true; // Mark as consumed
                    } else if (type == QEvent::Hide) {
                        toolBar->setProperty(PROP_IS_WRAPPED, false);
                        return true; // Mark as consumed
                    }
                } else if (type == QEvent::Show) {
                    extButton->setMinimumWidth(0);
                    extButton->setMaximumWidth(QWIDGETSIZE_MAX);
                }
            }
        }
    }
    return false;
}


bool LC_ProxyStyle::getMenuCommandAliasInfo(const QWidget *widget, const QFont &baseFont, int &maxCmdWidth, int &maxShortcutWidth) const {
    maxCmdWidth = 0;
    maxShortcutWidth = 0;
    const auto *menu = qobject_cast<const QMenu*>(widget);
    if (!menu) {
        return false;
    }

    const QVariant hasAliasesProp = menu->property(PROP_HAS_CMD_ALIASES);
    if (!hasAliasesProp.isValid()) {
        bool hasAliases = false;
        const QFont &monoFont = getResolvedMonoFont(baseFont); // Zero-allocation cache lookup
        const QFontMetrics fmCmd(monoFont);
        const QFontMetrics fmBase(baseFont);

        for (const QAction *action : menu->actions()) {
            // 1. Measure Command Alias Width
            QVariant cmdLine = action->property(PROP_CMD_LINE);
            if (cmdLine.isValid() && !cmdLine.toString().isEmpty()) {
                hasAliases = true;
                const int width = fmCmd.size(Qt::TextSingleLine, QString("[%1]").arg(cmdLine.toString())).width();
                if (width > maxCmdWidth) {
                    maxCmdWidth = width;
                }
            }

            // 2. Measure Shortcut Text Width natively directly from QAction's shortcut
            if (!action->shortcut().isEmpty()) {
                QString shortcut = action->shortcut().toString(QKeySequence::NativeText);
                const int sw = fmBase.size(Qt::TextSingleLine, shortcut).width();
                if (sw > maxShortcutWidth) {
                    maxShortcutWidth = sw;
                }
            }
        }

        // Cache the lookup and width retSkinults directly on the parent QMenu
        auto *mutableMenu = const_cast<QMenu*>(menu);
        mutableMenu->setProperty(PROP_HAS_CMD_ALIASES, hasAliases);
        mutableMenu->setProperty(PROP_MAX_CMD_WIDTH, maxCmdWidth);
        mutableMenu->setProperty(PROP_MAX_SHORTCUT_WIDTH, maxShortcutWidth);
        return hasAliases;
    }

    maxCmdWidth = menu->property(PROP_MAX_CMD_WIDTH).toInt();
    maxShortcutWidth = menu->property(PROP_MAX_SHORTCUT_WIDTH).toInt();
    return hasAliasesProp.toBool();
}


QPalette::ColorGroup LC_ProxyStyle::resolveColorGroup(const QStyle::State state) const {
    QPalette::ColorGroup group = (state & State_Enabled) ? QPalette::Active : QPalette::Disabled;
    if (!(state & State_Active)) {
        group = QPalette::Inactive;
    }
    return group;
}

SkinColors LC_ProxyStyle::getStyleDescriptor(const QStyleOption *option) const {
    return getCachedStyleDescriptor(option->palette, resolveColorGroup(option->state));
}

void LC_ProxyStyle::drawUnifiedGripPattern(QPainter *painter,
                                                 const QPoint &cx,
                                                 const QRect &rect, const bool horizontalPattern,
                                                 const QColor &gripColor, const SplitterGripStyle style, const int handleLen,
                                                 const SkinScaledGeometries &geoms) const {
    LCPainterGuard guard(painter);

    int realLen = handleLen;
    if (realLen < 0) {
        realLen = horizontalPattern ? rect.width() - geoms.ints.scale4
                                    : rect.height() - geoms.ints.scale4;
    }
    const int halfLen = realLen / 2;

    switch (style) {
        case SplitterGripStyle::MutedDots: {
            painter->setPen(Qt::NoPen);
            painter->setBrush(gripColor);
            const int dotOffset = (handleLen > 0) ? qMax(geoms.ints.scale4, halfLen - geoms.ints.scale3)
                                            : geoms.ints.scale6;
            constexpr qreal dotRadius = 1.5;

            if (horizontalPattern) {
                painter->drawEllipse(QPointF(cx.x() - dotOffset, cx.y()), dotRadius, dotRadius);
                painter->drawEllipse(QPointF(cx.x(),             cx.y()), dotRadius, dotRadius);
                painter->drawEllipse(QPointF(cx.x() + dotOffset, cx.y()), dotRadius, dotRadius);
            } else {
                painter->drawEllipse(QPointF(cx.x(), cx.y() - dotOffset), dotRadius, dotRadius);
                painter->drawEllipse(QPointF(cx.x(), cx.y()),             dotRadius, dotRadius);
                painter->drawEllipse(QPointF(cx.x(), cx.y() + dotOffset), dotRadius, dotRadius);
            }
            break;
        }

        case SplitterGripStyle::RoundedPill: {
            painter->setPen(Qt::NoPen);
            painter->setBrush(gripColor);
            const qreal pillHeight = geoms.splitterGrip.gripPillHeight;
            const qreal halfPill = pillHeight / 2.0;

            if (horizontalPattern) {
                painter->drawRoundedRect(QRectF(cx.x() - halfLen, cx.y() - halfPill, realLen, pillHeight), halfPill, halfPill);
            } else {
                painter->drawRoundedRect(QRectF(cx.x() - halfPill, cx.y() - halfLen, pillHeight, realLen), halfPill, halfPill);
            }
            break;
        }

        case SplitterGripStyle::RibbedGrip: {
            painter->setPen(QPen(gripColor, 1));
            const int spacing = geoms.ints.scale4;

            if (horizontalPattern) {
                painter->drawLine(cx.x() - spacing, cx.y() - spacing, cx.x() - spacing, cx.y() + spacing);
                painter->drawLine(cx.x(),           cx.y() - spacing, cx.x(),           cx.y() + spacing);
                painter->drawLine(cx.x() + spacing, cx.y() - spacing, cx.x() + spacing, cx.y() + spacing);
            } else {
                painter->drawLine(cx.x() - spacing, cx.y() - spacing, cx.x() + spacing, cx.y() - spacing);
                painter->drawLine(cx.x() - spacing, cx.y(),           cx.x() + spacing, cx.y());
                painter->drawLine(cx.x() - spacing, cx.y() + spacing, cx.x() + spacing, cx.y() + spacing);
            }
            break;
        }

        case SplitterGripStyle::DoubleRidges: {
            painter->setPen(QPen(gripColor, 1));
            const int spacing = geoms.ints.scale2;

            if (horizontalPattern) {
                painter->drawLine(cx.x() - halfLen, cx.y() - spacing, cx.x() + halfLen, cx.y() - spacing);
                painter->drawLine(cx.x() - halfLen, cx.y() + spacing, cx.x() + halfLen, cx.y() + spacing);
            } else {
                painter->drawLine(cx.x() - spacing, cx.y() - halfLen, cx.x() - spacing, cx.y() + halfLen);
                painter->drawLine(cx.x() + spacing, cx.y() - halfLen, cx.x() + spacing, cx.y() + halfLen);
            }
            break;
        }

        case SplitterGripStyle::BorderHairline: {
            painter->setPen(QPen(gripColor, 1));
            if (horizontalPattern) {
                painter->drawLine(cx.x() - halfLen, cx.y(), cx.x() + halfLen, cx.y());
            } else {
                painter->drawLine(cx.x(), cx.y() - halfLen, cx.x(), cx.y() + halfLen);
            }
            break;
        }
    }
}

void LC_ProxyStyle::drawGroupBoxBoundary(QPainter *painter,
                                                const QStyleOptionGroupBox *option,
                                                const SkinColors &desc,
                                                const GroupBoxLayout &layout,
                                                const QRegion &clipRegion) const {
    const QRect rect = option->rect;
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const bool hasBreak = (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::BreakBorder);

    if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::Full) {
        painter->setPen(desc.groupBox.groupBoxFrameColor);
        painter->setBrush(Qt::NoBrush);

        if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::HeaderUnderline) {
            painter->drawLine(layout.drawFrameRect.left(), layout.underlineY, layout.drawFrameRect.left(), layout.drawFrameRect.bottom());
            painter->drawLine(layout.drawFrameRect.right(), layout.underlineY, layout.drawFrameRect.right(), layout.drawFrameRect.bottom());
            painter->drawLine(layout.drawFrameRect.left(), layout.drawFrameRect.bottom(), layout.drawFrameRect.right(), layout.drawFrameRect.bottom());
        } else {
            if (hasBreak && !clipRegion.isEmpty()) {
                painter->setClipRegion(clipRegion);
            }
            if (m_isSoftSatin) {
                LCPainterGuard roundedGuard(painter, true);
                painter->drawRoundedRect(layout.drawFrameRect, 4.0, 4.0);
            } else {
                painter->drawRect(layout.drawFrameRect);
            }
            painter->setClipping(false);
        }
    }
    else if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::LeftStripe) {
        const QRect stripeRect(rect.left(), rect.top(), geoms.ints.scale3, rect.height());
        painter->fillRect(stripeRect, desc.common.highlightColor);
    }
    else if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::TopLine) {
        painter->setPen(desc.groupBox.groupBoxFrameColor);
        if (hasBreak && !clipRegion.isEmpty()) {
            painter->setClipRegion(clipRegion);
        }
        painter->drawLine(rect.left(), layout.topY, rect.right(), layout.topY);
        painter->setClipping(false);
    }
}

void LC_ProxyStyle::drawGroupBoxHeader(QPainter *painter,
                                              const QStyleOptionGroupBox *option,
                                              const SkinColors &desc,
                                              const GroupBoxLayout &layout,
                                              const QRect &textRect,
                                              const QRect &checkBoxRect) const {
    const QRect rect = option->rect;

    if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::HeaderBanner) {
        painter->setPen(desc.groupBox.groupBoxFrameColor);
        painter->setBrush(Qt::NoBrush);

        if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::Full) {
            if (m_isSoftSatin) {
                LCPainterGuard roundedGuard(painter, true);
                painter->drawRoundedRect(layout.drawTotalRect, 4.0, 4.0);
            } else {
                painter->drawRect(layout.drawTotalRect);
            }
        } else if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::LeftStripe) {
            const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
            QRect stripeRect(rect.left(), rect.top(), geoms.ints.scale3, layout.headerRect.height());
            painter->fillRect(stripeRect, desc.common.highlightColor);

            painter->drawLine(layout.headerRect.topLeft(), layout.headerRect.topRight());
            painter->drawLine(layout.headerRect.topRight(), layout.headerRect.bottomRight());
            painter->drawLine(layout.headerRect.topLeft(), layout.headerRect.bottomLeft());
        } else {
            painter->drawLine(layout.headerRect.topLeft(), layout.headerRect.topRight());
        }

        {
            LCPainterGuard bgGuard(painter, true);

            if (desc.groupBox.groupBoxBoundaryStyle == GroupBoxBoundaryStyle::Full && m_isSoftSatin) {
                QPainterPath roundedPath;
                roundedPath.addRoundedRect(layout.drawTotalRect, 4.0, 4.0);
                painter->setClipPath(roundedPath);
            }

            painter->setPen(Qt::NoPen);

            if (desc.common.useGlassyGloss) {
                QLinearGradient glassGrad(layout.fillHeaderRect.topLeft(), layout.fillHeaderRect.bottomLeft());
                glassGrad.setColorAt(0.0, desc.groupBox.groupBoxGlassStart);
                glassGrad.setColorAt(0.42, desc.groupBox.groupBoxGlassMidStart);
                glassGrad.setColorAt(0.55, desc.groupBox.groupBoxGlassMidEnd);
                glassGrad.setColorAt(1.0, desc.groupBox.groupBoxGlassEnd);
                painter->fillRect(layout.fillHeaderRect, glassGrad);
            } else if (desc.common.useGradient) {
                QLinearGradient grad(layout.fillHeaderRect.topLeft(), layout.fillHeaderRect.bottomLeft());
                grad.setColorAt(0.0, desc.groupBox.groupBoxBannerFillColor);
                grad.setColorAt(1.0, desc.groupBox.groupBoxBannerFillColor.darker(102));
                painter->fillRect(layout.fillHeaderRect, grad);
            } else {
                painter->fillRect(layout.fillHeaderRect, desc.groupBox.groupBoxBannerFillColor);
            }
        }

        painter->setPen(desc.groupBox.groupBoxFrameColor);
        painter->drawLine(layout.headerRect.bottomLeft(), layout.headerRect.bottomRight());
    }
    else if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::HeaderUnderline) {
        painter->setPen(desc.groupBox.groupBoxFrameColor);
        painter->drawLine(rect.left(), layout.underlineY, rect.right(), layout.underlineY);
    }
    else if (desc.groupBox.groupBoxHeaderStyle == GroupBoxHeaderStyle::Overlapping) {
        if (option->subControls & SC_GroupBoxLabel && !option->text.isEmpty()) {
            painter->fillRect(layout.bgCoverRect, desc.common.bgStart);
        }
    }
}

void LC_ProxyStyle::drawMenuItemColumns(QPainter* painter, const QStyleOptionMenuItem* option, const SkinColors& desc,
                                              int maxCmdWidth, int maxShortcutWidth, const QFont& baseFont, const QWidget* widget) const {
    LCPainterGuard guard(painter);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const QAction *action = findActionForOption(option, widget);

    QString labelText = option->text;
    QString shortcutText;

    if (action) {
        labelText = action->text();
        if (!action->shortcut().isEmpty()) {
            shortcutText = action->shortcut().toString(QKeySequence::NativeText);
        }
    } else {
        const int tabIdx = labelText.indexOf(QLatin1Char('\t'));
        if (tabIdx >= 0) {
            shortcutText = labelText.mid(tabIdx + 1);
            labelText = labelText.left(tabIdx);
        }
    }

    // Resolve column alignment coordinates mathematically
    const LC_SkinWidgetsLayoutResolver::MenuItemColumnsLayout layout = LC_SkinWidgetsLayoutResolver::resolveMenuItemLayout(option, geoms, maxCmdWidth, maxShortcutWidth);

    const bool isEnabled = (option->state & State_Enabled);
    const bool isSelected = (option->state & State_Selected);

    const QString elidedLabel = option->fontMetrics.elidedText(labelText, Qt::ElideRight, layout.labelRect.width());
    constexpr int labelFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
    proxy()->drawItemText(painter, layout.labelRect, labelFlags, option->palette, isEnabled, elidedLabel,
                          isSelected ? QPalette::HighlightedText : QPalette::WindowText);

        if (action) {
        const QVariant cmdLineProp = action->property(PROP_CMD_LINE);
        if (cmdLineProp.isValid() && !cmdLineProp.toString().isEmpty()) {
            painter->setFont(getResolvedMonoFont(baseFont));

            const QColor aliasColor = isSelected ? desc.itemView.menuAliasColorSelected
                                           : desc.itemView.menuAliasColorNormal;
            painter->setPen(aliasColor);

            painter->drawText(layout.aliasRect, Qt::AlignLeft | Qt::AlignVCenter, QString("%1").arg(cmdLineProp.toString()));
        }
    }

    if (!shortcutText.isEmpty()) {
        painter->setFont(baseFont);
        constexpr int scFlags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextShowMnemonic;
        proxy()->drawItemText(painter, layout.shortcutRect, scFlags, option->palette, isEnabled, shortcutText,
                              isSelected ? QPalette::HighlightedText : QPalette::WindowText);
    }
}

bool LC_ProxyStyle::getSpinBoxProgress(const QWidget* widget, qreal& pct, const QAbstractSpinBox* &resolvedSpinBox) const {
    resolvedSpinBox = nullptr;
    if (!m_useSpinBoxProgressBar || !widget) {
        return false;
    }

    auto spinBox = qobject_cast<const QAbstractSpinBox*>(widget);
    if (!spinBox && widget->parentWidget()) {
        spinBox = qobject_cast<const QAbstractSpinBox*>(widget->parentWidget());
    }

    if (!spinBox) {
        return false;
    }

    const QVariant valVar = spinBox->property(PROP_SPINBOX_VALUE);
    const QVariant minVar = spinBox->property(PROP_SPINBOX_MIN);
    const QVariant maxVar = spinBox->property(PROP_SPINBOX_MAX);

    if (!valVar.isValid() || !minVar.isValid() || !maxVar.isValid()) {
        return false;
    }

    const double val = valVar.toDouble();
    const double min = minVar.toDouble();
    const double max = maxVar.toDouble();

    if (max <= min) {
        return false;
    }

    pct = (val - min) / (max - min);
    pct = qBound(0.0, pct, 1.0);

    // Save the pointer reference correctly to the caller
    resolvedSpinBox = spinBox;
    return true;
}


void LC_ProxyStyle::drawCustomLineEditPanel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    const SkinColors desc = m_skinColorsResolver.resolveLineEditDescriptor(option, widget);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);

    const QWidget *actualWidget = widget;
    if (!actualWidget && painter && painter->device() && painter->device()->devType() == QInternal::Widget) {
        actualWidget = static_cast<const QWidget*>(painter->device());
    }

    bool isInput = false;
    if (actualWidget) {
        isInput = actualWidget->property(PROP_IS_INPUT_COMPONENT).toBool();
        if (!isInput && actualWidget->parentWidget()) {
            isInput = actualWidget->parentWidget()->property(PROP_IS_INPUT_COMPONENT).toBool();
        }
    }

    // 1. Draw Background Fill (Satin, Glossy, or Flat depending on style archetype)
    {
        LCPainterGuard guard(painter, true); // AA prevents background bleeding through rounded corners
        painter->setPen(Qt::NoPen);

        QBrush bgBrush;
        const bool isGlassy = desc.common.useGlassyGloss;

        if (isGlassy) {
            QLinearGradient glassGrad(option->rect.topLeft(), option->rect.bottomLeft());
            // Invert glassy stops for the recessed/inner gloss look (Dark at top, Light at bottom)
            glassGrad.setColorAt(0.0, desc.button.glassEnd);
            glassGrad.setColorAt(0.45, desc.button.glassMidEnd);
            glassGrad.setColorAt(0.58, desc.button.glassMidStart);
            glassGrad.setColorAt(1.0, desc.button.glassStart);
            bgBrush = glassGrad;
        } else if (desc.common.useGradient) {
            QLinearGradient grad(option->rect.topLeft(), option->rect.bottomLeft());
            // Invert stops (bgEnd at top, bgStart at bottom) to create a sunken inner shadow
            grad.setColorAt(0.0, desc.common.bgEnd);
            grad.setColorAt(1.0, desc.common.bgStart);
            bgBrush = grad;
        } else {
            bgBrush = desc.common.bgStart;
        }

        painter->setBrush(bgBrush);
        // Symmetrical background rounding matches the outer frame geometry
        if (isInput && !m_isClassic && desc.frame.hasFullBorder) {
            painter->drawRoundedRect(QRectF(option->rect), 2.0, 2.0);
        } else {
            painter->drawRect(option->rect);
        }
    }

    // 2. Draw the Interactive SpinBox Progress Bar overlay on top of the background panel
    qreal pct = 0.0;
    const QAbstractSpinBox *spinBox = nullptr;

    bool hasFocus = (option->state & State_HasFocus);
    if (!hasFocus && actualWidget) {
        hasFocus = actualWidget->hasFocus() || actualWidget->isAncestorOf(QApplication::focusWidget());
    }

    if (getSpinBoxProgress(widget, pct, spinBox)) {
        drawSpinBoxProgressBar(painter, option->rect, geoms, desc, pct, hasFocus, widget, spinBox);
    }

    // 3. Delegate the border frame drawing explicitly to PE_FrameLineEdit
    if (const auto *frameOpt = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
        if (frameOpt->lineWidth > 0) {
            proxy()->drawPrimitive(PE_FrameLineEdit, option, painter, widget);
        }
    } else {
        proxy()->drawPrimitive(PE_FrameLineEdit, option, painter, widget);
    }
}

void LC_ProxyStyle::drawCustomStatusPillToolbar(const QStyleOptionToolBar *option, QPainter *painter, const QWidget *widget) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const QPalette::ColorGroup group = resolveColorGroup(option->state);
    const SkinColors desc = getCachedStyleDescriptor(option->palette, group);

    const bool isHovered = (option->state & State_MouseOver);
    const bool isActive = widget && widget->property(PROP_ACTIVE).toBool();

    QColor bgCol;
    if (isActive) {
        bgCol = desc.common.highlightColor;
        bgCol.setAlpha(35); // Subtle transmissive 13-15% active tint overlay
    } else if (isHovered) {
        bgCol = desc.splitter.splitterGripWellColor;
        bgCol.setAlpha(70); // Enhanced contrast during active hover sweeps
    } else {
        bgCol = desc.splitter.splitterGripWellColor; // Uses baseline low-alpha well color
    }

    const qreal r = geoms.ints.scale4; // Subtle radius mapping

    painter->setPen(Qt::NoPen);
    painter->setBrush(bgCol);
    painter->drawRoundedRect(crispRect(option->rect, geoms.crispOffset), r, r);

    if (isHovered) {
        painter->setPen(QPen(desc.frame.borderHovered, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(crispRect(option->rect, geoms.crispOffset), r, r);
    }
}

void LC_ProxyStyle::drawCustomStatusPillToolbarHandle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    LCPainterGuard guard(painter, true);
    const SkinScaledGeometries &geoms = m_scaledGeometryProvider.getGeometries(painter);
    const SkinColors desc = getStyleDescriptor(option);

    const QRect rect = option->rect;
    const QPoint cx = rect.center();
    const bool horizontal = option->state & State_Horizontal;

    // Apply integrated subtle contrast modifier to the status bar handles
    painter->setOpacity(0.9);

    const int baseHandleLen = qMin(geoms.ints.scale20, (horizontal ? rect.height() : rect.width()) - geoms.ints.scale2);
    // const QColor gripColor = desc.splitter.splitterGripColorIdle;

    const QColor gripColor = (m_accentGrips || (option->state & State_MouseOver)) ? desc.common.highlightColor : desc.splitter.splitterGripColorIdle;

    drawUnifiedGripPattern(painter, cx, rect, !horizontal, gripColor, m_splitterGripStyle, baseHandleLen, geoms);
}


void LC_ProxyStyle::setupPermanentTitleBar(QDockWidget *dock) const  {
    // if (!dock) return;
    //
    // const bool needCustom = m_useFloatingHUD || m_customDockTitleBar;
    // if (needCustom) {
    //     if (!dock->titleBarWidget() || !dock->titleBarWidget()->inherits("LC_DockTitleBar")) {
    //         if (dock->titleBarWidget()) {
    //             QWidget *old = dock->titleBarWidget();
    //             if (old->inherits("LC_DockTitleBar")) {
    //                 dock->setTitleBarWidget(nullptr);
    //                 old->deleteLater();
    //             }
    //         }
    //         auto *titleBar = new LC_DockTitleBar(dock, this);
    //         dock->setTitleBarWidget(titleBar);
    //         // Removed titleBar->show(); to let QDockWidgetLayout manage dynamic tab visibility automatically
    //     }
    // } else {
    //     if (dock->titleBarWidget() && dock->titleBarWidget()->inherits("LC_DockTitleBar")) {
    //         QWidget *old = dock->titleBarWidget();
    //         dock->setTitleBarWidget(nullptr);
    //         old->deleteLater();
    //     }
    // }

    if (!dock) return;

    const bool needCustom = m_useFloatingHUD || m_customDockTitleBar;
    if (needCustom) {
        if (!dock->titleBarWidget() || !dock->titleBarWidget()->inherits("LC_CustomTitleBarWidget")) {
            if (dock->titleBarWidget()) {
                QWidget *old = dock->titleBarWidget();
                if (old->inherits("LC_CustomTitleBarWidget") || old->inherits("LC_DockTitleBar")) {
                    dock->setTitleBarWidget(nullptr);
                    old->deleteLater();
                }
            }
            // Pass the dock's title directly to the custom constructor as its default text [74]
            auto *titleBar = new LC_CustomTitleBarWidget(dock->windowTitle(), dock->windowTitle(), "", dock);
            dock->setTitleBarWidget(titleBar);
        }
    } else {
        if (dock->titleBarWidget() && (dock->titleBarWidget()->inherits("LC_CustomTitleBarWidget") || dock->titleBarWidget()->inherits("LC_DockTitleBar"))) {
            QWidget *old = dock->titleBarWidget();
            dock->setTitleBarWidget(nullptr);
            old->deleteLater();
        }
    }
}
