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

#ifndef LC_FUSIONPROXYSTYLE_H
#define LC_FUSIONPROXYSTYLE_H

#include <QProxyStyle>
#include <QPalette>
#include <QColor>
#include <QElapsedTimer>
#include <QPainterPath>
#include <qpointer.h>
#include <QRect>
#include <QStyleOptionComboBox>

#include "lc_proxy_style_shared.h"
#include "lc_palette_editor_shared.h"
#include "lc_skin_colors_resolver.h"
#include "lc_skin_scaled_geometry_provider.h"

class QDockWidget;
class LC_EventFilterWin32WindowCloaking;
class LC_EventFilterAutoPopupController;
class LC_EventFilterToolTip;
class LC_EventFilterMnemonic;
class QToolBar;
class QStyleOptionButton;
class QStyleOptionTab;
class QStyleOptionHeader;
class QStyleOptionProgressBar;
class QStyleOptionSlider;
class QStyleOptionFrame;
class QStyleOptionDockWidget;

namespace DEBUG_PROXY {
    // Expose the global profiling trackers to all translation units via extern linkage [3]
    extern QElapsedTimer g_globalPerfTimer;
    extern bool g_perfTimerStarted;
}

#ifdef Q_OS_WIN
#define USE_WIN_NATIVE_RESIZE_FILTER_
#endif

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
class LC_WinResizeNativeEventFilter;
#endif



class LC_ProxyStyle : public QProxyStyle {
    Q_OBJECT
public:
    explicit LC_ProxyStyle(QStyle *baseStyle = nullptr, const StyleMetricsConfig &metrics = StyleMetricsConfig());

    void setMetrics(const StyleMetricsConfig& metrics);
    void setSkin(const SkinConfig& skin);

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget = nullptr) const override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override;
    bool isStatusPillNeeded(const QWidget* widget) const;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const override;

    void polish(QApplication *app) override;
    void polish(QPalette &palette) override;
    void polish(QWidget *widget) override;
    void unpolish(QApplication *app) override;
    void unpolish(QWidget *widget) override;

    QRect subControlRect(ComplexControl control,
                    const QStyleOptionComplex *option,
                    SubControl subControl,
                    const QWidget *widget) const override;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const override;
    SkinColors getCachedStyleDescriptor(const QPalette &palette, QPalette::ColorGroup group) const;


    bool useSegmentedToolButtons() const {return m_useSegmentedToolButtons;}
    void precomputeSegmentedGroupColors(QWidget *widget, int totalGroups) const;

    const StyleMetricsConfig& metrics() const { return m_metrics; }
    bool customToolTipCardEnabled() const { return m_customToolTipCard; }
    bool autoPopupMenuBarEnabled() const { return m_autoPopupMenuBar; }
    bool autoPopupInstantButtonsEnabled() const { return m_autoPopupInstantButtons; }
    bool autoPopupToolbarOverflowEnabled() const { return m_autoPopupToolbarOverflow; }
    const LC_EventFilterAutoPopupController* autoPopupController() const { return m_autoPopupController.get(); }
    const SkinScaledGeometries &getGeometries(const QWidget *widget) const;

    void drawSegmentedGroupBackdrops(QPainter *painter, const QWidget *widget) const;
    void drawCustomDockTitleBar(const QStyleOptionDockWidget *option, QPainter *painter, const QWidget *widget) const;
private:
    void drawParameterizedBox(QPainter *painter, const QRect &rect, const SkinColors &desc, bool isVertical = false) const;
    void invalidateCache() const;
    // Extracted widget-specific drawing helpers [3]
    void drawCustomPushButton(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomGroupBox(const QStyleOptionGroupBox *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomTabBarTab(const QStyleOptionTab *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomHeaderSection(const QStyleOptionHeader *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomProgressBar(const QStyleOptionProgressBar *option, QPainter *painter, const QWidget *widget) const;

    void drawCustomTabWidgetFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomToolBar(const QStyleOptionToolBar *option, QPainter *painter, const QWidget *widget = nullptr) const;
    void drawCustomLineEditFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawSpinBoxProgressBar(QPainter* painter, const QRect& rect, const SkinScaledGeometries& geoms, const SkinColors& desc, qreal pct, bool hasFocus, const QWidget*
                                widget, const QAbstractSpinBox* spinBox) const;
    void drawCustomGroupBoxFrame(const QStyleOptionFrame *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomPanelButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomPanelButtonTool(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomSlider(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomScrollBar(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomSpinBox(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorArrow(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorToolBarSeparator(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomItemViewItem(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomPanelItemViewRow(const QStyleOptionViewItem *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomMenuBarEmptyArea(const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;
    void drawCustomMenuItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorBranch(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorToolBarHandle(const QStyleOption *option,QPainter *painter,const QWidget *widget) const;
    void drawCustomToolbarOverflowIndicator(const QStyleOption *option,QPainter *painter,const QWidget *widget) const;
    void drawCustomSplitter(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomToolTipCard(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomIndicatorHeaderArrow(const QStyleOption *option, QPainter *painter) const;
    void drawCustomIndicatorTabClose(const QStyleOption* option, QPainter* painter) const;
    void drawCustomIndicatorTabTear(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawSegmentedToolButton(const QStyleOption *option, QPainter *painter, const QWidget *widget, int mask) const;

    void paintTabBackground(QPainter *painter, const QRectF &rect, const QStyleOptionTab *option, const SkinColors &desc, const TabPaths &paths, bool selected) const;
    void paintTabBorders(QPainter *painter, const QStyleOptionTab *option, const SkinColors &desc, const TabPaths &paths, bool selected) const;
    void paintTabActiveStripe(QPainter *painter, const QRectF &rect, const QStyleOptionTab *option, const SkinColors &desc) const;
    bool observeToolbarExtensionEvent(QObject *watched, QEvent *event) const;
    void drawCustomMenuBarItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const;
    void drawUnifiedGripPattern(QPainter* painter, const QPoint& cx, const QRect& rect, bool horizontalPattern, const QColor& gripColor,
                                SplitterGripStyle style, int handleLen, const SkinScaledGeometries &geoms) const;
    bool getMenuCommandAliasInfo(const QWidget *widget, const QFont &baseFont, int &maxCmdWidth, int &maxShortcutWidth) const;
    const QFont& getResolvedMonoFont(const QFont &baseFont) const;

    QPalette::ColorGroup resolveColorGroup(QStyle::State state) const;
    SkinColors getStyleDescriptor(const QStyleOption *option) const;


    void drawGroupBoxBoundary(QPainter* painter, const QStyleOptionGroupBox* option, const SkinColors& desc,
                              const GroupBoxLayout& layout, const QRegion& clipRegion) const;

    void drawGroupBoxHeader(QPainter* painter, const QStyleOptionGroupBox* option, const SkinColors& desc,
                            const GroupBoxLayout& layout, const QRect& textRect, const QRect& checkBoxRect) const;

    void drawMenuItemColumns(QPainter *painter,
                             const QStyleOptionMenuItem *option,
                             const SkinColors &desc,
                             int maxCmdWidth,
                             int maxShortcutWidth,
                             const QFont &baseFont,
                             const QWidget *widget) const;

    bool getSpinBoxProgress(const QWidget* widget, qreal& pct, const QAbstractSpinBox*& spinBox) const;
    void drawCustomLineEditPanel(const QStyleOption* option, QPainter* painter, const QWidget* widget) const;

    void doSetStyleArtefact(StyleArchetype archetype);

    void drawCustomStatusPillToolbar(const QStyleOptionToolBar *option, QPainter *painter, const QWidget *widget) const;
    void drawCustomStatusPillToolbarHandle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    void setupPermanentTitleBar(QDockWidget *dock) const;

    bool m_isFlat = false;
    bool m_isClassic = false;
    bool m_isFlatModern = false;
    bool m_isGlossy = false;
    bool m_isSoftSatin = false;
    bool m_isAccentOnline = false;

    StyleMetricsConfig m_metrics;

    StyleArchetype    m_styleArchetype = StyleArchetype::ClassicFusion;
    BoxDecoration     m_boxDecoration  = BoxDecoration::DividingHairline;
    bool              m_customDockTitleBar = false;
    DockTitleBarStyle m_dockTitleBarStyle  = DockTitleBarStyle::Native;
    bool              m_accentedScrollbars = false;
    bool              m_transparentScrollbars = false;

    bool m_customGroupBoxBar = false;
    GroupBoxHeaderStyle m_groupBoxHeaderStyle = GroupBoxHeaderStyle::Plain;
    GroupBoxBoundaryStyle m_groupBoxBoundaryStyle = GroupBoxBoundaryStyle::Full;
    bool m_groupBoxUseAccent = false;
    bool m_showActiveRowSpotlight = false;
    bool m_showItemViewHover = true;

    BranchIndicatorStyle m_branchIndicatorStyle = BranchIndicatorStyle::MutedChevrons;
    bool m_showTreeConnectingLines = false;
    bool m_customSplitterAnToolbarGrip = false;
    SplitterGripStyle m_splitterGripStyle = SplitterGripStyle::MutedDots;
    bool m_highlightSplitterOnDrag = false;
    bool m_showGripBackgroundWell = false;
    bool m_accentGrips = false;
    bool m_persistentDockSplitter = true;
    bool m_useFocusedInputGlow = false;
    bool m_useToolButtonUnderline = false;
    ToolButtonIndicatorStyle m_toolButtonIndicatorStyle = ToolButtonIndicatorStyle::ContextStripe;
    bool m_customToolTipCard = false;
    bool m_customVectorIconsInButtons = false;
    bool m_tabStripeAtBottom = false;
    bool m_customToolbarOverflowGrip = false;
    bool m_autoPopupToolbarOverflow = false;
    bool m_autoPopupInstantButtons = false;
    bool m_autoPopupMenuBar = false;
    bool m_showMenuCommandAliases = true;
    bool m_useMenuBarHoverCard = true;
    bool m_useSegmentedToolButtons = true;
    bool m_useStatusPillChips = false;
    SegmentedSeparationStyle m_segmentedSeparationStyle = SegmentedSeparationStyle::ContinuousCard;
    SegmentedColorPolicy m_segmentedColorPolicy = SegmentedColorPolicy::HarmonizedAnalogous;

    bool m_useSpinBoxProgressBar = true;

    bool m_useFloatingHUD = false;
    CloseButtonColorPolicy m_closeButtonColorPolicy = CloseButtonColorPolicy::AccentColor;
    mutable LC_SkinScaledGeometryProvider m_scaledGeometryProvider;
    mutable LC_SkinColorsResolver m_skinColorsResolver;

    std::unique_ptr<LC_EventFilterMnemonic> m_mnemonicFilter;
    std::unique_ptr<LC_EventFilterToolTip>  m_toolTipFilter;
    std::unique_ptr<LC_EventFilterAutoPopupController> m_autoPopupController;
#ifdef Q_OS_WIN
    std::unique_ptr<LC_EventFilterWin32WindowCloaking> m_win32CloakingFilter;
#endif

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
    std::unique_ptr<LC_WinResizeNativeEventFilter> m_winResizeFilter;
#endif

    friend class LC_EventFilterFloatingHUD;
    friend class LC_DockTitleBar;
};

#endif
