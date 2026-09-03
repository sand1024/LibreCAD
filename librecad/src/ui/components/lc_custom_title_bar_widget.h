
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

#ifndef LC_TITLE_BAR_WIDGET_H
#define LC_TITLE_BAR_WIDGET_H

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QList>
#include <QMetaObject>

class LC_IconLabel;
class QLabel;
class QDockWidget;
class QToolButton;
class QTimer;
class QPixmap;
class QFontMetrics;

namespace Constants {
    constexpr int UPDATE_DELAY_MS = 50;
    constexpr int TOOLTIP_DELAY_MS = 500;
    constexpr int BASE_ICON_SIZE = 16;
    constexpr int BASE_PADDING_SMALL = 2;
    constexpr int BASE_PADDING_MEDIUM = 4;
    constexpr int BASE_PADDING_LARGE = 6;
    constexpr int MINIMUM_HEIGHT = 20;
    constexpr int CHAR_WIDTH_FACTOR = 2;
    constexpr int CACHE_TOLERANCE_FACTOR = 4;
    constexpr int BASE_PADDING_EXTRA = 2;
    constexpr int MAX_TEXT_LENGTH = 100;
}

class LC_CustomTitleBarWidget : public QWidget {
    Q_OBJECT

public:
    enum DisplayMode {
        TextOnly = 0,
        IconOnly,
        IconAndText
    };

    enum class TitleTextDirection {
        Horizontal = 0,  // Standard horizontal layout (0 degrees)
        Vertical,        // Bottom-to-top layout (-90 degrees)
        VerticalAlt      // Top-to-bottom layout (+90 degrees)
    };

    enum class TitleTextAlignment {
        Start = 0, // Aligns to Left (Horizontal), Bottom (Vertical), or Top (VerticalAlt)
        Center,    // Centered in the title bar (Default)
        End        // Aligns to Right (Horizontal), Top (Vertical), or Bottom (VerticalAlt)
    };

    TitleTextAlignment textAlignment() const { return m_textAlignment; }
    void setTextAlignment(TitleTextAlignment alignment);

    TitleTextDirection textDirection() const { return m_textDirection; }
    void setTextDirection(TitleTextDirection direction);

    Q_ENUM(DisplayMode)
    Q_ENUM(TitleTextDirection)

    struct VerticalTextInfo {
        QString displayText;
        bool isElided = false;
    };

    // Constructors
    LC_CustomTitleBarWidget(const QString& horizontalTitle, const QString& verticalTitle, const QString& iconName,
                             QWidget* parent = nullptr, DisplayMode mode = IconAndText);

    LC_CustomTitleBarWidget(const QString& title, const QString& iconName,
                             QWidget* parent = nullptr, DisplayMode mode = IconAndText);

    ~LC_CustomTitleBarWidget() override;

    // Getters / Setters
    DisplayMode displayMode() const;
    QString horizontalTitle() const;
    QString verticalTitle() const;
    QString iconName() const;
    void setDisplayMode(DisplayMode mode);

    // Override sizeHint to return the correct PM_TitleBarHeight [74]
    QSize sizeHint() const override;
    void updateButtonIcons() const;
    void updateTitleBar();
    void showEvent(QShowEvent* event);

signals:
    void displayModeChanged(LC_CustomTitleBarWidget::DisplayMode mode);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool event(QEvent* event) override;
    virtual bool checkOrientationFromSettings() const;

private slots:
    void onDockWidgetFeaturesChanged();
    void onScreenChanged();
    void onLogicalDotsPerInchChanged(qreal dpi);
    void onApplicationFontChanged();
    void delayedUpdate();
    void showTooltip();

private:

    TitleTextDirection m_textDirection = TitleTextDirection::Vertical;
    TitleTextAlignment m_textAlignment = TitleTextAlignment::Start;
    // Updated helper signature to accept the parameterized text direction
    QPixmap createRotatedTextPixmap(const QString& text, TitleTextDirection direction) const;
    // Layout and Label Instantiation Helpers
    QLabel* createTitleLabel(const QString& text);
    QString getCurrentTitle() const;
    void createIconLabel(const QString& iconName);
    void loadIcon(const QString& iconName) const;
    void updateIconForMode();
    void updateLabelAlignment() const;

    // Pixel-perfect manual positioning [74]
    void updateButtonAndLabelGeometries();

    // Dock Specific Components and Configurations
    void createDockButtons();
    void updateDockButtonsVisibility();
    void updateDockWidgetPointer();
    void updateOrientation();

    // Event connections & cleanups
    void setupConnections();
    void disconnectAllConnections();

    // Scaling
    void updateIconSize();
    QSize getScaledIconSize() const;
    QSize minimumSizeHint() const;
    int scaleToDpi(int value) const;
    qreal getLogicalDpiX() const;
    void updateFontMetrics();

    // Symmetrical Title Orientation Renderers
    void updateTitleForCurrentOrientation();
    void clearVerticalCache();
    void hideTooltip();
    void scheduleUpdate();

    // Rotated/Elided Geometry Math
    VerticalTextInfo getVerticalDisplayText(const QString& text, int availableHeight) const;
    void createVerticalText(const QString& text);
    int getSafeAvailableHeight() const;
    QString safeUnicodeLeft(const QString& text, int maxChars) const;
    bool isVerticalCacheValid() const;
    QString getElidedText(const QString& text, int width);
    QString safeElideText(const QString& text, int width) const;
    bool isTextElided() const;

    void updateCursor();

    QLabel* m_titleLabel;
    LC_IconLabel* m_iconLabel;
    QDockWidget* m_dockWidget;

    // Symmetrical Dock control buttons
    QToolButton* m_closeButton;
    QToolButton* m_floatButton;

    QString m_horizontalTitle;
    QString m_verticalTitle;
    QString m_iconName;

    Qt::Orientation m_currentOrientation;
    DisplayMode m_displayMode;
    bool m_isTextElided;
    QFontMetrics* m_fontMetrics;
    bool m_blockRebuild;

    // Pixmap Caching for rotated vertical layouts
    QPixmap* m_verticalPixmapCache;

    // Symmetrical Timers
    QTimer* m_updateTimer;
    QTimer* m_tooltipTimer;
    QPoint m_lastTooltipPos;

    // Connection Groups
    QList<QMetaObject::Connection> m_dockWidgetConnections;
    QList<QMetaObject::Connection> m_screenConnections;
    QList<QMetaObject::Connection> m_windowConnections;
    QList<QMetaObject::Connection> m_appConnections;

    QPoint m_dragStartPos;    // Tracks the relative mouse offset during drag starts
    bool m_isDragging = false;
};

#endif
