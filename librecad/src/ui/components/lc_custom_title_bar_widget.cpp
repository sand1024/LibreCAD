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
#include "lc_custom_title_bar_widget.h"

#include <QApplication>
#include <QToolButton>
#include <QWidget>
#include <QLabel>
#include <QStyle>
#include <QPainter>
#include <QDockWidget>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QPointer>
#include <QTimer>
#include <QToolTip>
#include <QWindow>
#include <QStyleOptionDockWidget>

// Constructor with separate strings for horizontal and vertical orientation
LC_CustomTitleBarWidget::LC_CustomTitleBarWidget(const QString& horizontalTitle, const QString& verticalTitle, const QString& iconName,
                                                 QWidget* parent, DisplayMode mode)
    : QWidget(parent), m_titleLabel(createTitleLabel(horizontalTitle)), m_iconLabel(nullptr), m_dockWidget(nullptr),
      m_closeButton(nullptr), m_floatButton(nullptr),
      m_horizontalTitle(horizontalTitle), m_verticalTitle(verticalTitle), m_iconName(iconName), m_currentOrientation(Qt::Horizontal),
      m_displayMode(mode), m_isTextElided(false), m_fontMetrics(nullptr), m_blockRebuild(false), m_verticalPixmapCache(nullptr),
      m_updateTimer(nullptr), m_tooltipTimer(nullptr), m_lastTooltipPos(QPoint()) {

    // Initialize font metrics
    updateFontMetrics();

    bool hasIcon = !iconName.isEmpty();
    if (mode != TextOnly && hasIcon) {
        createIconLabel(iconName);
    }

    // Resolve parent dock widget and initialize buttons / handlers
    updateDockWidgetPointer();

    // Setup connections
    setupConnections();

    setMouseTracking(true);
}

// Backwards compatibility constructor override
LC_CustomTitleBarWidget::LC_CustomTitleBarWidget(const QString& title, const QString& iconName, QWidget* parent, DisplayMode mode)
    : LC_CustomTitleBarWidget(title, title, iconName, parent, mode) {
}

LC_CustomTitleBarWidget::~LC_CustomTitleBarWidget() {
    disconnectAllConnections();

    if (m_updateTimer) m_updateTimer->stop();
    if (m_tooltipTimer) m_tooltipTimer->stop();

    delete m_fontMetrics;
    delete m_verticalPixmapCache;
}

// Getters / Setters
LC_CustomTitleBarWidget::DisplayMode LC_CustomTitleBarWidget::displayMode() const { return m_displayMode; }
QString LC_CustomTitleBarWidget::horizontalTitle() const { return m_horizontalTitle; }
QString LC_CustomTitleBarWidget::verticalTitle() const { return m_verticalTitle; }
QString LC_CustomTitleBarWidget::iconName() const { return m_iconName; }

void LC_CustomTitleBarWidget::setDisplayMode(DisplayMode mode) {
    if (m_displayMode == mode || m_blockRebuild) {
        return;
    }

    m_displayMode = mode;

    updateIconForMode();
    updateButtonAndLabelGeometries();

    if (m_displayMode != IconOnly) {
        updateTitleForCurrentOrientation();
    }

    updateGeometry();
    hideTooltip();

    emit displayModeChanged(m_displayMode);
}

QSize LC_CustomTitleBarWidget::sizeHint() const {
    QStyleOptionDockWidget opt;
    opt.initFrom(this);
    const int height = style()->pixelMetric(QStyle::PM_TitleBarHeight, &opt, this);

    if (m_currentOrientation == Qt::Vertical) {
        return QSize(height, 100);
    }
    return QSize(100, height);
}

// Protected Event Overrides
void LC_CustomTitleBarWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_blockRebuild) return;

    updateButtonAndLabelGeometries();
}

void LC_CustomTitleBarWidget::mouseDoubleClickEvent(QMouseEvent* event) { event->ignore(); }
void LC_CustomTitleBarWidget::mousePressEvent(QMouseEvent* event) { hideTooltip(); event->ignore(); }

void LC_CustomTitleBarWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_displayMode != IconOnly && isTextElided()) {
        QPoint globalPos = mapToGlobal(event->pos());
        if (globalPos != m_lastTooltipPos) {
            m_lastTooltipPos = globalPos;
            m_tooltipTimer->start();
        }
    } else {
        hideTooltip();
    }
    event->ignore();
}

void LC_CustomTitleBarWidget::mouseReleaseEvent(QMouseEvent* event) { event->ignore(); }
void LC_CustomTitleBarWidget::leaveEvent(QEvent* event) { hideTooltip(); QWidget::leaveEvent(event); }

void LC_CustomTitleBarWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    QStyleOptionDockWidget opt;
    opt.initFrom(this);
    opt.rect = rect();
    opt.title = getCurrentTitle();
    opt.verticalTitleBar = (m_currentOrientation == Qt::Vertical);

    if (m_dockWidget) {
        opt.closable = (m_dockWidget->features() & QDockWidget::DockWidgetClosable);
        opt.floatable = (m_dockWidget->features() & QDockWidget::DockWidgetFloatable);
    }

    style()->drawControl(QStyle::CE_DockWidgetTitle, &opt, &painter, this);
}

void LC_CustomTitleBarWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange) {
        updateFontMetrics();
        clearVerticalCache();
        if (m_displayMode != IconOnly) {
            scheduleUpdate();
        }
    }
    QWidget::changeEvent(event);
}

bool LC_CustomTitleBarWidget::event(QEvent* event) {
    if (event->type() == QEvent::ParentChange) {
        updateDockWidgetPointer();
        setupConnections();
    }
    return QWidget::event(event);
}

// Private Slots
void LC_CustomTitleBarWidget::onDockWidgetFeaturesChanged() {
    if (m_dockWidget && !m_blockRebuild) {
        updateDockButtonsVisibility();
        updateOrientation();
    }
}

void LC_CustomTitleBarWidget::onScreenChanged() {
    clearVerticalCache();
    updateIconSize();
    if (m_displayMode != IconOnly) {
        scheduleUpdate();
    }
    updateGeometry();
}

void LC_CustomTitleBarWidget::onLogicalDotsPerInchChanged(qreal dpi) {
    Q_UNUSED(dpi);
    onScreenChanged();
}

void LC_CustomTitleBarWidget::onApplicationFontChanged() {
    updateFontMetrics();
    clearVerticalCache();
    if (m_displayMode != IconOnly) {
        scheduleUpdate();
    }
}

void LC_CustomTitleBarWidget::delayedUpdate() {
    if (m_blockRebuild) return;
    if (m_displayMode != IconOnly) {
        updateTitleForCurrentOrientation();
    }
}

void LC_CustomTitleBarWidget::showTooltip() {
    if (!m_lastTooltipPos.isNull()) {
        QString tooltipText = getCurrentTitle();
        QToolTip::showText(m_lastTooltipPos, tooltipText, this, rect());
    }
}

// Private Helpers
QLabel* LC_CustomTitleBarWidget::createTitleLabel(const QString& text) {
    auto label = new QLabel(text, this);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return label;
}

QString LC_CustomTitleBarWidget::getCurrentTitle() const {
    return (m_currentOrientation == Qt::Horizontal) ? m_horizontalTitle : m_verticalTitle;
}

void LC_CustomTitleBarWidget::createIconLabel(const QString& iconName) {
    m_iconLabel = new QLabel(this);
    updateIconSize();
    m_iconLabel->setScaledContents(true);
    loadIcon(iconName);
}

void LC_CustomTitleBarWidget::loadIcon(const QString& iconName) {
    QIcon icon(iconName);
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(getScaledIconSize()));
    } else {
        qWarning() << "LC_CustomTitleBarWidget: Failed to load icon:" << iconName;
        if (m_displayMode != IconOnly) {
            QStyleOption opt;
            opt.initFrom(this);
            m_iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileIcon, &opt, this).pixmap(getScaledIconSize()));
        }
    }
}

void LC_CustomTitleBarWidget::updateIconForMode() {
    if (m_displayMode == TextOnly) {
        if (m_iconLabel) {
            m_iconLabel->deleteLater();
            m_iconLabel = nullptr;
        }
    } else if (!m_iconName.isEmpty()) {
        if (!m_iconLabel) {
            createIconLabel(m_iconName);
        } else {
            m_iconLabel->show();
        }
    }
}

void LC_CustomTitleBarWidget::updateButtonAndLabelGeometries() {
    if (!m_dockWidget || m_blockRebuild) return;

    QStyleOptionDockWidget opt;
    opt.initFrom(this);
    opt.rect = rect();
    opt.verticalTitleBar = (m_currentOrientation == Qt::Vertical);

    QDockWidget::DockWidgetFeatures features = m_dockWidget->features();

    // Resolve metrics parameters directly from active style [74]
    const int btnSize = style()->pixelMetric(QStyle::PM_TitleBarButtonSize, &opt, this);
    const int margin = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, &opt, this);
    const int spacing = scaleToDpi(2);
    const int leftSpacing = scaleToDpi(8);

    const QRect titleRect = rect();
    const int thickness = m_currentOrientation == Qt::Vertical ? titleRect.width() : titleRect.height();
    const int actualMargin = margin >= 0 ? margin : qMax(0, (thickness - btnSize) / 2);

    QRect closeRect;
    QRect floatRect;
    QRect textRect;

    if (m_currentOrientation == Qt::Vertical) {
        const int left = titleRect.left() + qMax(0, (titleRect.width() - btnSize) / 2);
        int top = titleRect.top() + actualMargin;

        if (features & QDockWidget::DockWidgetClosable) {
            closeRect = QRect(left, top, btnSize, btnSize);
            top += (btnSize + spacing);
        }
        if (features & QDockWidget::DockWidgetFloatable) {
            floatRect = QRect(left, top, btnSize, btnSize);
            top += (btnSize + spacing);
        }

        // Top margin matches the double height margin space of the handle pattern
        const int verticalGripOffset = scaleToDpi(16);
        textRect = QRect(titleRect.left(), top + verticalGripOffset,
                         titleRect.width(), qMax(0, titleRect.bottom() - top - verticalGripOffset));
    } else {
        const int top = titleRect.top() + actualMargin;
        int right = titleRect.right() - actualMargin;

        if (features & QDockWidget::DockWidgetClosable) {
            closeRect = QRect(right - btnSize, top, btnSize, btnSize);
            right -= (btnSize + spacing);
        }
        if (features & QDockWidget::DockWidgetFloatable) {
            floatRect = QRect(right - btnSize, top, btnSize, btnSize);
            right -= (btnSize + spacing);
        }

        // Horizontal Grip handle margin offset spacing
        const int gripOffset = scaleToDpi(16);
        int iconWidth = 0;

        if (m_iconLabel && m_iconLabel->isVisible()) {
            iconWidth = getScaledIconSize().width();
            m_iconLabel->setGeometry(QRect(titleRect.left() + gripOffset, top + actualMargin, iconWidth, iconWidth));
        }

        const int textLeft = titleRect.left() + gripOffset + iconWidth + (iconWidth > 0 ? scaleToDpi(4) : 0);
        textRect = QRect(textLeft, titleRect.top(),
                         qMax(0, right - textLeft), titleRect.height());
    }

    // Set absolute geometries directly, preventing layout caching issues [74]
    m_blockRebuild = true;
    if (m_closeButton) {
        m_closeButton->setGeometry(closeRect);
        m_closeButton->setFixedSize(btnSize, btnSize);
        m_closeButton->setIconSize(QSize(btnSize - scaleToDpi(4), btnSize - scaleToDpi(4)));
    }
    if (m_floatButton) {
        m_floatButton->setGeometry(floatRect);
        m_floatButton->setFixedSize(btnSize, btnSize);
        m_floatButton->setIconSize(QSize(btnSize - scaleToDpi(4), btnSize - scaleToDpi(4)));
    }
    if (m_titleLabel) {
        m_titleLabel->setGeometry(textRect);
    }
    m_blockRebuild = false;
}

void LC_CustomTitleBarWidget::createDockButtons() {
    if (!m_dockWidget) return;

    QStyleOption opt;
    opt.initFrom(this);

    // Create Close Button and connect standard click trigger
    if (!m_closeButton) {
        m_closeButton = new QToolButton(this);
        m_closeButton->setObjectName("lc_titlebar_close_btn");
        m_closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton, &opt, this));
        connect(m_closeButton, &QToolButton::clicked, m_dockWidget, &QDockWidget::close);
    }

    // Create Float Button and connect dynamic toggle trigger
    if (!m_floatButton) {
        m_floatButton = new QToolButton(this);
        m_floatButton->setObjectName("lc_titlebar_float_btn");
        m_floatButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton, &opt, this));
        connect(m_floatButton, &QToolButton::clicked, this, [this]() {
            if (m_dockWidget) {
                m_dockWidget->setFloating(!m_dockWidget->isFloating());
            }
        });
    }

    updateDockButtonsVisibility();
}

void LC_CustomTitleBarWidget::updateDockButtonsVisibility() {
    if (!m_dockWidget) return;

    QDockWidget::DockWidgetFeatures features = m_dockWidget->features();

    if (m_closeButton) {
        m_closeButton->setVisible(features & QDockWidget::DockWidgetClosable);
    }
    if (m_floatButton) {
        m_floatButton->setVisible(features & QDockWidget::DockWidgetFloatable);
    }
}

void LC_CustomTitleBarWidget::updateDockWidgetPointer() {
    disconnectAllConnections();

    m_dockWidget = qobject_cast<QDockWidget*>(parent());

    if (m_dockWidget) {
        if (!m_dockWidget->property("lcfs_originalFeatures").isValid()) {
            m_dockWidget->setProperty("lcfs_originalFeatures", static_cast<int>(m_dockWidget->features()));
        }
        createDockButtons();
        updateOrientation();
    }
}

void LC_CustomTitleBarWidget::updateOrientation() {
    if (!m_dockWidget || m_blockRebuild) return;

    Qt::Orientation newOrientation = (m_dockWidget->features() & QDockWidget::DockWidgetVerticalTitleBar) ? Qt::Vertical : Qt::Horizontal;

    m_currentOrientation = newOrientation;
    clearVerticalCache();

    // Dynamically adjust text alignment based on horizontal/vertical layout [74]
    if (m_titleLabel) {
        if (m_currentOrientation == Qt::Horizontal) {
            m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            m_titleLabel->setAlignment(Qt::AlignCenter);
        }
    }

    updateButtonAndLabelGeometries();
}

void LC_CustomTitleBarWidget::updateTitleForCurrentOrientation() {
    if (!m_titleLabel || m_displayMode == IconOnly || !m_fontMetrics) return;

    QString currentTitle = getCurrentTitle();

    if (m_currentOrientation == Qt::Horizontal) {
        m_titleLabel->setText(currentTitle);
        m_titleLabel->setPixmap(QPixmap());

        int availableWidth = m_titleLabel->width();
        if (availableWidth > 0) {
            if (availableWidth < m_fontMetrics->averageCharWidth()) {
                m_titleLabel->setText("…");
                m_isTextElided = true;
                return;
            }

            QString elidedText = getElidedText(currentTitle, availableWidth);
            m_isTextElided = (elidedText != currentTitle);
            m_titleLabel->setText(elidedText);
        }
    } else {
        createVerticalText(currentTitle);
    }
}

void LC_CustomTitleBarWidget::clearVerticalCache() {
    delete m_verticalPixmapCache;
    m_verticalPixmapCache = nullptr;
}

void LC_CustomTitleBarWidget::hideTooltip() {
    if (m_tooltipTimer) m_tooltipTimer->stop();
    m_lastTooltipPos = QPoint();
    QToolTip::hideText();
}

void LC_CustomTitleBarWidget::scheduleUpdate() {
    if (m_updateTimer && !m_blockRebuild) m_updateTimer->start();
}

LC_CustomTitleBarWidget::VerticalTextInfo LC_CustomTitleBarWidget::getVerticalDisplayText(const QString& text, int availableHeight) const {
    VerticalTextInfo result;
    result.displayText = text;
    result.isElided = false;

    if (!m_fontMetrics || text.isEmpty()) return result;

    int charHeight = m_fontMetrics->height();
    int charWidth = m_fontMetrics->maxWidth();
    int effectiveCharHeight = qMax(charHeight, charWidth / Constants::CHAR_WIDTH_FACTOR);

    if (availableHeight < effectiveCharHeight / 2) {
        result.displayText = "…";
        result.isElided = true;
        return result;
    }

    qint64 textHeight = static_cast<qint64>(effectiveCharHeight) * static_cast<qint64>(text.length());
    if (textHeight > availableHeight) {
        int maxChars = availableHeight / effectiveCharHeight;
        if (maxChars < text.length()) {
            if (maxChars >= 2) {
                result.displayText = safeUnicodeLeft(text, maxChars - 1) + "…";
            } else {
                result.displayText = "…";
            }
            result.isElided = true;
        }
    }

    return result;
}

void LC_CustomTitleBarWidget::createVerticalText(const QString& text) {
    if (!m_titleLabel || text.isEmpty() || !m_fontMetrics) return;

    if (m_verticalPixmapCache && isVerticalCacheValid()) {
        m_titleLabel->setPixmap(*m_verticalPixmapCache);
        m_titleLabel->setText(QString());
        return;
    }

    int availableHeight = getSafeAvailableHeight();
    VerticalTextInfo textInfo = getVerticalDisplayText(text, availableHeight);

    if (textInfo.displayText.isEmpty()) {
        m_titleLabel->setPixmap(QPixmap());
        m_titleLabel->setText(QString());
        m_isTextElided = true;
        return;
    }

    QPixmap pixmap = createRotatedTextPixmap(textInfo.displayText);

    // Safely delete the old cached pixmap before assigning a new one to prevent memory leaks
    clearVerticalCache();
    m_verticalPixmapCache = new QPixmap(pixmap);

    m_titleLabel->setPixmap(pixmap);
    m_titleLabel->setText(QString());
    m_isTextElided = textInfo.isElided;
}

int LC_CustomTitleBarWidget::getSafeAvailableHeight() const {
    if (!m_titleLabel) return scaleToDpi(Constants::MINIMUM_HEIGHT);

    int availableHeight = m_titleLabel->height();
    if (availableHeight <= 0) {
        availableHeight = m_titleLabel->sizeHint().height();
        if (availableHeight <= 0) {
            availableHeight = m_titleLabel->minimumHeight();
            if (availableHeight <= 0) {
                availableHeight = scaleToDpi(Constants::MINIMUM_HEIGHT);
            }
        }
    }

    return availableHeight;
}

QString LC_CustomTitleBarWidget::safeUnicodeLeft(const QString& text, int maxChars) const {
    if (maxChars <= 0) return QString();
    if (maxChars >= text.length()) return text;

    QString result;
    result.reserve(maxChars * 2);
    int count = 0;

    for (int i = 0; i < text.length() && count < maxChars; ++i) {
        if (text.at(i).isHighSurrogate() && i + 1 < text.length()) {
            result.append(text.at(i));
            result.append(text.at(i + 1));
            i++;
        } else {
            result.append(text.at(i));
        }
        count++;
    }

    return result;
}

QPixmap LC_CustomTitleBarWidget::createRotatedTextPixmap(const QString& text) const {
    if (!m_fontMetrics) return QPixmap();

    QSize textSize = m_fontMetrics->size(Qt::TextSingleLine, text);
    int padding = scaleToDpi(Constants::BASE_PADDING_EXTRA);
    int width = textSize.height() + padding;
    int height = textSize.width() + padding;

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(devicePixelRatioF());

    QPainter painter(&pixmap);
    painter.setFont(m_titleLabel ? m_titleLabel->font() : font());
    painter.setPen(m_titleLabel ? m_titleLabel->palette().text().color() : palette().text().color());

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    painter.translate(pixmap.width() / 2, pixmap.height() / 2);
    painter.rotate(-90);

    QRectF textRect(-textSize.width() / 2, -textSize.height() / 2, textSize.width(), textSize.height());

    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    painter.drawText(textRect, text, textOption);
    painter.end();

    return pixmap;
}

bool LC_CustomTitleBarWidget::isVerticalCacheValid() const {
    if (!m_verticalPixmapCache || !m_titleLabel) return false;

    QSize currentSize = m_titleLabel->size();
    QSize cacheSize = m_verticalPixmapCache->size();
    int tolerance = scaleToDpi(Constants::CACHE_TOLERANCE_FACTOR);

    return !m_verticalPixmapCache->isNull() && qAbs(cacheSize.width() - currentSize.width()) <= tolerance && qAbs(
        cacheSize.height() - currentSize.height()) <= tolerance;
}

QString LC_CustomTitleBarWidget::getElidedText(const QString& text, int width) {
    if (!m_fontMetrics) return text;
    return safeElideText(text, width);
}

QString LC_CustomTitleBarWidget::safeElideText(const QString& text, int width) const {
    if (!m_fontMetrics) return text;

    QString safeText = text;
    bool wasTruncated = false;

    if (text.length() > Constants::MAX_TEXT_LENGTH) {
        safeText = text.left(Constants::MAX_TEXT_LENGTH) + "…";
        wasTruncated = true;
    }

    int fullWidth = m_fontMetrics->horizontalAdvance(safeText);
    if (fullWidth <= width && !wasTruncated) return safeText;

    int left = 0;
    int right = safeText.length();
    int best = 0;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        QString candidate = safeUnicodeLeft(safeText, mid);
        if (mid < safeText.length() || wasTruncated) candidate += "…";

        int candidateWidth = m_fontMetrics->horizontalAdvance(candidate);
        if (candidateWidth <= width) {
            best = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (best > 0) {
        QString result = safeUnicodeLeft(safeText, best);
        if (best < safeText.length() || wasTruncated) result += "…";
        return result;
    }

    return "…";
}

bool LC_CustomTitleBarWidget::isTextElided() const {
    if (!m_titleLabel || m_displayMode == IconOnly) return false;

    QString currentTitle = getCurrentTitle();
    if (currentTitle.isEmpty()) return false;

    if (m_currentOrientation == Qt::Horizontal) {
        return m_titleLabel->text() != currentTitle;
    }

    return m_isTextElided;
}

void LC_CustomTitleBarWidget::setupConnections() {
    if (m_dockWidget) {
        m_dockWidgetConnections << connect(m_dockWidget, &QDockWidget::featuresChanged, this,
                                           &LC_CustomTitleBarWidget::onDockWidgetFeaturesChanged);

        // Setup parent floating orientation feature toggle connection [1.2.2]
        m_dockWidgetConnections << connect(m_dockWidget, &QDockWidget::topLevelChanged, this, [this](bool floating) {
            if (m_dockWidget) {
                QDockWidget::DockWidgetFeatures features = static_cast<QDockWidget::DockWidgetFeatures>(
                    m_dockWidget->property("lcfs_originalFeatures").toInt());

                if (floating) {
                    features &= ~QDockWidget::DockWidgetVerticalTitleBar;
                }

                m_dockWidget->blockSignals(true);
                m_dockWidget->setFeatures(features);
                m_dockWidget->blockSignals(false);

                updateOrientation();
                m_dockWidget->updateGeometry();
            }
        });
    }

    if (QScreen* currentScreen = screen()) {
        QPointer<QScreen> screenPtr(currentScreen);
        m_screenConnections << connect(currentScreen, &QScreen::logicalDotsPerInchChanged, this, [this, screenPtr](qreal dpi) {
            if (!screenPtr.isNull()) {
                onLogicalDotsPerInchChanged(dpi);
            }
        });
    }

    if (QWindow* window = windowHandle()) {
        m_windowConnections << connect(window, &QWindow::screenChanged, this, &LC_CustomTitleBarWidget::onScreenChanged);
    }

    m_appConnections << connect(qApp, &QApplication::fontChanged, this, &LC_CustomTitleBarWidget::onApplicationFontChanged);
}

void LC_CustomTitleBarWidget::disconnectAllConnections() {
    for (const auto& connection : std::as_const(m_dockWidgetConnections)) disconnect(connection);
    m_dockWidgetConnections.clear();

    for (const auto& connection : std::as_const(m_screenConnections)) disconnect(connection);
    m_screenConnections.clear();

    for (const auto& connection : std::as_const(m_windowConnections)) disconnect(connection);
    m_windowConnections.clear();

    for (const auto& connection : std::as_const(m_appConnections)) disconnect(connection);
    m_appConnections.clear();
}

void LC_CustomTitleBarWidget::updateIconSize() {
    if (m_iconLabel) m_iconLabel->setFixedSize(getScaledIconSize());
    updateButtonAndLabelGeometries();
}

QSize LC_CustomTitleBarWidget::getScaledIconSize() const {
    int size = scaleToDpi(Constants::BASE_ICON_SIZE);
    return QSize(size, size);
}

int LC_CustomTitleBarWidget::scaleToDpi(int value) const {
    qreal dpiScale = getLogicalDpiX() / 96.0;
    return qMax(1, qRound(value * dpiScale));
}

qreal LC_CustomTitleBarWidget::getLogicalDpiX() const {
    if (const QScreen* scr = screen()) return scr->logicalDotsPerInchX();
    return 96.0;
}


void LC_CustomTitleBarWidget::updateFontMetrics() {
    auto newMetrics = new QFontMetrics(m_titleLabel ? m_titleLabel->font() : font());
    QFontMetrics* oldMetrics = m_fontMetrics;
    m_fontMetrics = newMetrics;
    delete oldMetrics;
}
