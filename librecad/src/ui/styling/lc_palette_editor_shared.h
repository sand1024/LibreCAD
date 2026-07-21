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

#ifndef LC_PALETTE_EDITOR_SHARED_H
#define LC_PALETTE_EDITOR_SHARED_H


#include <QString>
#include <QPalette>
#include <QMap>
#include <QColor>
#include <QList>

inline const QString DEFAULT_THEME_KEY = "VIRTUAL_DEFAULT";

inline const QString SKIN_EXTENSION = ".lcsk";
inline const QString ICON_STYLE_EXTENSION = ".lcis";
inline const QString FONT_EXTENSION = ".lcft";
inline const QString METRICS_EXTENSION = ".lcms";
inline const QString PROFILE_EXTENSION = ".lcws";


inline const QString SKIN_FILE_IDENTIFIER = "LibreCAD Skin Config v_1";
inline const QString FONT_FILE_IDENTIFIER = "LibreCAD Font Config v_1";
inline const QString METRICS_FILE_IDENTIFIER = "LibreCAD Metrics Config v_1";
inline const QString PROFILE_FILE_IDENTIFIER = "LibreCAD Workspace Profile v_1";


inline const QString FILE_IDENTIFIER = "LibreCAD Fusion Skin v_1";
inline const QString CONFIG_EXTENSION = ".lcfs";

// Global visual style engine rendering archetypes
enum class StyleArchetype {
    FlatModern = 0,
    SoftSatin,
    GlassyGloss,
    AccentOutline,
    ClassicFusion // Restores standard native Fusion chiseled geometry
};

enum class ContrastPolicy {
    Standard = 0,     // Standard un-shifted theme palette
    RecessedWindow,   // Window background deep recess, buttons standard
    ElevatedWindow,   // Window background elevated, buttons recessed
    AccentTinted      // Soft highlight/accent color tint blended over background
};

enum class BoxDecoration {
    Frameless = 0,              // Flat container faces, zero borders
    DividingHairline,           // 1: Horizontal divider (line in flat/accent, 3D bevel in traditional)
    BoxOutline,                 // 2: Full 1px bounding outline box
    RecessedWell,               // 3: Simulates an inset groove container (shaded in flat/bevel in traditional)
    LeftAccentBar,              // 4: Left vertical 3px active accent indicator sidebar
    ActiveAccentFrame           // 5: Highlights the entire panel with a 1px accent-colored border
};


enum class TabShape {
    Rectangular = 0, // Clean, 90-degree straight boundaries (Modern VS Code style)
    Rounded,         // Soft, rounded corners on outer edges (Default)
    Beveled          // Trapezoidal, angular beveled tabs (Classic CAD/Excel look)
};


// Global visual contrast weight multipliers
enum class ContrastWeight {
    None = 0,   // Flat (0% contrast / completely flat edges)
    Soft,       // Soft contrast (35% of baseline contrast)
    Balanced,   // Balanced contrast (Default, 100%)
    Hard        // Hard contrast (185% of baseline contrast)
};

enum class DockTitleBarStyle {
   Native = 0,             // Fusion standard native window-color background
   CustomSolid,            // Custom flat solid background
   CustomSatin,            // Soft satin gradient
   CustomGlassy,           // Polished glassy reflection
   CustomAccentOutline,    // Translucent highlight-colored style
   CustomAccentLine        // Active Left Sidebar style (Accent line on the left/bottom)
};

enum class GroupBoxHeaderStyle {
    Plain = 0,       // Standard text sitting on background canvas
    Overlapping,     // Text overlaps top border frame line
    BreakBorder,     // Text breaks top border (creates a physical gap)
    HeaderUnderline, // Title text is followed immediately by a horizontal divider
    HeaderBanner     // Title area is enclosed in a horizontal filled block
};

enum class GroupBoxBoundaryStyle {
    None = 0,        // No boundary frame (frameless)
    Full,            // Complete bounding box enclosing header and contents
    LeftStripe,      // Vertical 3px sidebar stripe on the left edge
    TopLine,         // Horizontal line only at the top of the content
};

struct PaletteRoleMapping {
    QString name;
    QPalette::ColorRole role;
};


struct PaletteStateMapping {
    QString name;
    QPalette::ColorGroup group;
};

enum class BevelStyle {
    Flat = 0,
    Soft,
    Sharp,
    Custom
};

enum class MnemonicUnderlineMode {
    PressAndHold = 0, // Underlines are visible only while holding ALT [cite: 74]
    StickyToggle      // A lone tap of ALT toggles underline visibility state [cite: 74]
};

enum class ToolButtonIndicatorStyle {
    ContextStripe = 0, // Bottom line for horizontal toolbars, Left line for vertical [cite: 74]
    AccentDot,         // Centered 4px dot along the bottom edge [cite: 74]
    AccentFrame        // Soft 1px rounded outline box [cite: 74]
};



struct ColorSchemeData {
    QMap<QString, QMap<QString, QColor>> palette; // Role -> State -> Color
    QString qss;
    ContrastPolicy contrastPolicy = ContrastPolicy::Standard;
    ContrastWeight contrastWeight = ContrastWeight::Balanced;

    bool autoCalculate3DHelpers = true;
    QPalette::ColorRole bevelSeedRole = QPalette::Button;
};


// Reusable structural block representing offsets and font weights
struct FontRoleConfig {
    int sizeOffset = 0;
    bool bold = false;
    bool italic = false;

    void setup(int offset, bool isBold, bool isItalic) {
        sizeOffset = offset;
        bold = isBold;
        italic = isItalic;
    }
};

// Consolidated typography data structure
struct FontConfig {
    QString name; // Unique descriptor name
    // Role 1: Baseline
    QString mainFamily;
    int mainSize = 10;

    // Relative offset configurations
    FontRoleConfig headings;   // Role 2: Group titles, column headers
    FontRoleConfig menuBar;   // Role 3: Main horizontal menu bar
    FontRoleConfig menus;     // Role 4: Popup dropdown context menus
    FontRoleConfig buttons;   // Role 5: Clickable buttons and actions
    FontRoleConfig inputs;     // Role 6: Input fields, dropdowns, spinners

    FontRoleConfig genericDockTitle; // Font for generic dock widget title bar
    FontRoleConfig specialDockTitle; // Font for special (CAD-related) dock widget title bar

    // Role 6: Code & Technical (includes independent monospaced family selection)
    QString techFamily;
    FontRoleConfig technical;
};

enum class ThemeModeOverride {
    FollowSystem = 0, // Follow Operating System (Default)
    ForceLight,       // Force Light Mode
    ForceDark         // Force Dark Mode
};

enum class SegmentedSeparationStyle {
    ContinuousCard = 0, // Unified background well (Figma/Blender Style)
    MinimalBorder,      // 1px border contour contour around group, transparent inside
    TransparentIdle     // Transparent idle, active/hover elements render segmented shapes
};

enum class DragCursorStyle {
    StandardArrow = 0, // Default arrow pointer (Qt::ArrowCursor)
    OpenHand,          // Sleek, modern grab hand (Qt::OpenHandCursor) [3]
    SizeAll            // Classic, highly visible 4-way move arrows (Qt::SizeAllCursor) [3]
};

struct StyleMetricsConfig {
    QString name; // Unique descriptor name

    int scrollBarWidth = -1;          // PM_ScrollBarExtent
    int treeIndentation = 12;         // PM_TreeViewIndentation
    int buttonPadding = 5;            // PM_ButtonMargin
    int scrollBarMinLength = 25;      // PM_ScrollBarSliderMin
    int dockTitleBarHeight = -1;      // PM_TitleBarHeight
    int splitterWidth = 4;            // PM_SplitterWidth
    int itemViewRowPadding = 4;       // CT_ItemViewItem vertical spacer
    bool flatDisabledText = true;     // SH_EtchDisabledText
    bool hideShortcutUnderlines = true;// SH_UnderlineShortcut

    int tabBarTabOverlap = 1;         // Horizontal tab bar overlap value
    int tabBarTabBaseOverlap = 1;     // PM_TabBarBaseOverlap
    int tabBarTabHSpace = 8;                   // PM_TabBarTabHSpace
    int tabBarTabVSpace = 4;                   // PM_TabBarTabVSpace
    int tabActiveExtraHeight = 13;    // Taller active tab popped-out height in pixels
    int tabCloseIndicatorSize = 14;   // Size of tab close 'X' buttons (PM_TabCloseIndicatorWidth/Height)

    int menuVerticalPadding = 3;      // Spacing inside context menu items (PM_MenuVMargin)
    int menuHorizontalPadding = 4;    // Horizontal padding inside menus (PM_MenuHMargin)
    int menuBorderWidth = 1;          // Border around popup menu panels (PM_MenuPanelWidth)
    int headerDefaultHeight = 22;     // Default height of table column headers (PM_HeaderDefaultSectionSizeVertical)
    int sliderHandleLength = 15;      // Physical width/length of draggable slider handles (PM_SliderLength)


    // IMPORTANT: value of -1 means auto center
    int dockWidgetTitleBarButtonMargin = -1;    // PM_DockWidgetTitleBarButtonMargin.
    int sliderControlThickness = 12;           // PM_SliderControlThickness
    int focusFrameHMargin = 1;                 // PM_FocusFrameHMargin
    int focusFrameVMargin = 1;                 // PM_FocusFrameVMargin
    int layoutMargin = 6;                      // PM_LayoutLeftMargin, etc.
    int layoutSpacing = 6;                     // PM_LayoutHorizontalSpacing, etc.
    int toolbarItemSpacing = 3;       // PM_ToolBarItemSpacing
    int toolbarSeparatorWidth = 4;    // PM_ToolBarSeparatorExtent
    int menuBarItemSpacing = 6;       // PM_MenuBarItemSpacing
    int menuBarVerticalMargin = 2;    // PM_MenuBarVMargin

    int indicatorBoxSize = 12;        // PM_IndicatorWidth & PM_ExclusiveIndicatorWidth
    int indicatorLabelSpacing = 4;    // PM_CheckBoxLabelSpacing & PM_RadioButtonLabelSpacing

    int titleBarButtonSize = 16;      // PM_TitleBarButtonSize
    int titleBarButtonSpacing = 4;     // Custom gap between title bar buttons in pixels 
    int subMenuOverlap = -2;          // PM_SubMenuOverlap

    bool menuSupportsIcons = true;             // SH_Menu_SupportsIcons
    bool menuAllowActiveAndDisabled = false;  // SH_Menu_AllowActiveAndDisabled

    int groupBoxTitleLeftPadding = 8;
    int groupBoxTitleLineGap = 6;

    MnemonicUnderlineMode mnemonicUnderlineMode = MnemonicUnderlineMode::StickyToggle;

    int treeBranchIndicatorSize = -1;
    int splitterHandleLength = -1;

    int toolbarPopupDelay = 400; // default delay in milliseconds
    DragCursorStyle dragCursorStyle = DragCursorStyle::SizeAll; // Unified drag cursor style
};

enum class BranchIndicatorStyle {
    MutedChevrons = 0,    // Sleek 1px vector arrow chevrons (IDE Style) [cite: 74]
    PlusMinusBoxes,       // Square outline boxes with plus/minus lines [cite: 74]
    TactileCircles,       // Soft circular badges with plus/minus indicators
    ClassicLinesOnly,     // Dotted guidelines only, no icons [cite: 74]
    None                  // Entirely open, indented layout [cite: 74]
};

enum class SplitterGripStyle {
    MutedDots = 0,   // Centered 3-dot vector grip (Modern IDE Style) [cite: 74]
    RoundedPill,     // Centered rounded capsule bar (Modern OS Style) [cite: 74]
    RibbedGrip,      // Centered short 3-ridge ribbed grip (Classic Tactile Style) [cite: 74]
    DoubleRidges,    // Two parallel full-length vector lines [cite: 74]
   BorderHairline   // Minimalist continuous 1px hairline divider (Pro Creative Suite Style) [cite: 74]
};

enum class SegmentedColorPolicy {
    NeutralAlternating = 0, // Alternates between 3 polished neutral slate shades
    HarmonizedAnalogous,    // Analogous spectrum derived from QPalette::Highlight
    VibrantSpectrum,        // Complete vibrant pastel color-wheel spectrum
    AnalogousHueShift       // New: Rotating hue within a safe 90° analogous arc (constant luminance)
};

enum class CloseButtonColorPolicy {
    AccentColor = 0, // Highlights in the active accent color (Highlight)
    MutedRed,        // Soft red on hover, neutral when idle
    VibrantRed,      // High-contrast warning red
    MutedNeutral     // Neutral border color matching standard controls
};

// Skin Configuration representing visual colors and aesthetic anchors (.lcsk)
struct SkinConfig {
    QString name;
    ColorSchemeData light;
    ColorSchemeData dark;

    StyleArchetype styleArchetype = StyleArchetype::ClassicFusion;
    BoxDecoration boxDecoration = BoxDecoration::DividingHairline;

    bool useSegmentedToolButtons = false;
    SegmentedSeparationStyle segmentedSeparationStyle = SegmentedSeparationStyle::ContinuousCard;
    SegmentedColorPolicy segmentedColorPolicy = SegmentedColorPolicy::HarmonizedAnalogous;

    bool customDockTitleBar = false;
    DockTitleBarStyle dockTitleBarStyle = DockTitleBarStyle::Native;

    bool accentedScrollbars = false;
    bool transparentScrollbars = false;

    QString linkedIconStyleName = "Default";
    bool useThemeDefaultIcons = false;

    bool customGroupBoxBar = false;
    GroupBoxHeaderStyle groupBoxHeaderStyle = GroupBoxHeaderStyle::Plain;
    GroupBoxBoundaryStyle groupBoxBoundaryStyle = GroupBoxBoundaryStyle::Full;
    bool groupBoxUseAccent = false;

    bool showActiveRowSpotlight = false;
    bool showItemViewHover = true;

    BranchIndicatorStyle branchIndicatorStyle = BranchIndicatorStyle::MutedChevrons;
    bool showTreeConnectingLines = false;

    bool customSplitterGrip = false;
    SplitterGripStyle splitterGripStyle = SplitterGripStyle::MutedDots;
    bool highlightSplitterOnDrag = false;
    bool showGripBackgroundWell = false;
    bool accentGrips = false;
    bool persistentDockSplitter = true;

    bool useFocusedInputGlow = false;

    bool useToolButtonUnderline = false;
    ToolButtonIndicatorStyle toolButtonIndicatorStyle = ToolButtonIndicatorStyle::ContextStripe;
    bool customToolTipCard = false;
    bool customVectorIconsInButtons = false;
    bool tabStripeAtBottom = false;

    bool customToolbarOverflowGrip = false;
    bool autoPopupToolbarOverflow = false;

    bool autoPopupInstantButtons = false;

    bool autoPopupMenuBar = false;

    bool useMenuBarHoverCard = false;
    bool showMenuCommandAliases = false;

    bool useSpinBoxProgressBar = false;

    bool useStatusPillChips = false;

    bool customMenuTearOff = false;
    bool syncCheckedMenuState = false;

    bool useFloatingHUD = false;

    bool showGenericDockIcons = true; // Show icons on generic dock title bars
    bool showSpecialDockIcons = true; // Show icons on special (CAD) dock title bars

    CloseButtonColorPolicy closeButtonColorPolicy = CloseButtonColorPolicy::AccentColor;

    bool customDialogTitleBar = false;
};

// Consolidated Workspace Profile linking the 4 decoupled configurations (.lcws)
struct WorkspaceProfile {
    QString name;
    QString activeSkinFile;
    QString activeIconStyleFile;
    QString activeTypographyFile;
    QString activeMetricsFile;
};

// Left for legacy code migration compatibility until deprecated
struct FusionSkinConfig {
    QString name;
    FontConfig font;
    StyleMetricsConfig metrics;
    ColorSchemeData light;
    ColorSchemeData dark;
    QString linkedIconStyleName = "Default";
    bool useThemeDefaultIcons = true;
};


inline const QList<PaletteRoleMapping> BASE_INTERFACE_ROLES = {
    {"Window", QPalette::Window},
    {"WindowText", QPalette::WindowText},
    {"Base", QPalette::Base},
    {"AlternateBase", QPalette::AlternateBase},
    {"ToolTipBase", QPalette::ToolTipBase},
    {"ToolTipText", QPalette::ToolTipText},
    {"Text", QPalette::Text},
    {"Button", QPalette::Button},
    {"ButtonText", QPalette::ButtonText},
    {"BrightText", QPalette::BrightText},
    {"Link", QPalette::Link},
    {"Highlight", QPalette::Highlight},
    {"HighlightedText", QPalette::HighlightedText},
    {"PlaceholderText", QPalette::PlaceholderText}
};

// Structural 3D edge and border roles (5 roles)
inline const QList<PaletteRoleMapping> BEVEL_HELPER_ROLES = {
    {"Light", QPalette::Light},
    {"Midlight", QPalette::Midlight},
    {"Dark", QPalette::Dark},
    {"Mid", QPalette::Mid},
    {"Shadow", QPalette::Shadow}
};

inline const QList<PaletteStateMapping> PALETTE_STATES = {
    {"Active", QPalette::Active},
    {"Inactive", QPalette::Inactive},
    {"Disabled", QPalette::Disabled}
};

// Unified template iterator: Executes a given functor sequentially on every single role
template <typename Functor>
inline void forEachRole(Functor func) {
    for (const auto &row : BASE_INTERFACE_ROLES) {
        func(row);
    }
    for (const auto &row : BEVEL_HELPER_ROLES) {
        func(row);
    }
}

// Unified template iterator: Executes a given functor sequentially on every role and state combination
template <typename Functor>
inline void forEachRoleState(Functor func) {
    auto processList = [&](const QList<PaletteRoleMapping> &rolesList) {
        for (const auto &row : rolesList) {
            for (const auto &state : PALETTE_STATES) {
                func(row, state);
            }
        }
    };
    processList(BASE_INTERFACE_ROLES);
    processList(BEVEL_HELPER_ROLES);
}

struct IndexEntry {
    QString displayName;
    QString lastModified; // ISO 8601 string
};
typedef QMap<QString, IndexEntry> StyleIndexMap;

class LC_RepositoryBase {
public:
    virtual ~LC_RepositoryBase() = default;
    virtual bool exists(const QString& name) const = 0;
    virtual bool removeByKey(const QString& name) = 0;
    virtual QStringList getAvailableNames() = 0;
    virtual QList<QPair<QString, QString>> getPresetChoices() const = 0;
};

#endif
