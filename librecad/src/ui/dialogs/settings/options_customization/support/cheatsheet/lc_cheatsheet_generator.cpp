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

#include "lc_cheatsheet_generator.h"
#include <algorithm>
#include <QApplication>
#include <QCollator>
#include <QCursor>
#include <QDate>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include "lc_action.h"
#include "lc_commands_tree_item.h"
#include "lc_commands_tree_model.h"
#include "lc_shortcuts_manager.h"
#include "lc_shortcut_tree_item.h"
#include "lc_shortcuts_tree_model.h"
#include "main.h"

namespace {
    struct LC_WaitCursorGuard {
        LC_WaitCursorGuard() {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        }

        ~LC_WaitCursorGuard() {
            QApplication::restoreOverrideCursor();
        }
    };

    QString getActionDescription(const QAction* action) {
        if (action == nullptr) {
            return QString();
        }
        const auto* lcAction = dynamic_cast<const LC_Action*>(action);
        if (lcAction != nullptr && !lcAction->description().isEmpty()) {
            return lcAction->description();
        }
        return LC_ShortcutsManager::getPlainActionToolTip(action);
    }

    class LC_CheatsheetHtmlHelper {
    public:
        static QString cleanPlainText(const QString& raw) {
            if (raw.isEmpty()) {
                return QString();
            }

            QString text = raw;
            if (text.contains('<') || text.contains('&')) {
                QTextDocument doc;
                doc.setHtml(text);
                text = doc.toPlainText();
            }

            text.replace(QChar(0x00A0), ' ');
            text.replace("&nbsp;", " ", Qt::CaseInsensitive);
            text.replace("nbsp;", " ", Qt::CaseInsensitive);

            return text.simplified();
        }

        static QString cleanDescription(const QString& rawDesc, const QString& actionName, const QString& shortcut = QString()) {
            QString desc = cleanPlainText(rawDesc);
            const QString cleanName = cleanPlainText(actionName);

            if (desc.isEmpty() || desc.compare(cleanName, Qt::CaseInsensitive) == 0) {
                return QString();
            }

            if (!shortcut.isEmpty()) {
                const QString cleanShortcut = cleanPlainText(shortcut);
                if (desc.endsWith(cleanShortcut, Qt::CaseInsensitive)) {
                    desc.chop(cleanShortcut.length());
                    desc = desc.trimmed();
                }
            }

            if (desc.startsWith(cleanName + ":", Qt::CaseInsensitive)) {
                desc = desc.mid(cleanName.length() + 1).trimmed();
            }
            else if (desc.startsWith(cleanName + " -", Qt::CaseInsensitive)) {
                desc = desc.mid(cleanName.length() + 2).trimmed();
            }
            else if (desc.startsWith(cleanName + " —", Qt::CaseInsensitive)) {
                desc = desc.mid(cleanName.length() + 2).trimmed();
            }

            if (desc.isEmpty() || desc.compare(cleanName, Qt::CaseInsensitive) == 0) {
                return QString();
            }

            if (!desc.isEmpty()) {
                desc[0] = desc[0].toUpper();
            }

            return desc;
        }

        static QString iconCell(const QString& iconUrl) {
            if (iconUrl.isEmpty()) {
                return "<td class=\"icon-col\" width=\"22\" valign=\"middle\"></td>";
            }
            return QString(
                    "<td class=\"icon-col\" width=\"22\" valign=\"middle\"><img src=\"%1\" width=\"18\" height=\"18\" style=\"vertical-align: middle;\"></td>")
               .arg(iconUrl);
        }

        static QString actionCell(const QString& rawName, const QString& groupNameSuffix = QString()) {
            const QString name = cleanPlainText(rawName);
            QString cellContent = QString("<span class=\"action-name\" style=\"vertical-align: middle;\">%1</span>").arg(
                name.toHtmlEscaped());

            if (!groupNameSuffix.isEmpty()) {
                cellContent += QString("&nbsp;<span class=\"continued-text\">(%1)</span>").arg(
                    cleanPlainText(groupNameSuffix).toHtmlEscaped());
            }

            return QString("<td class=\"action-cell\" width=\"100%\" valign=\"middle\">%1</td>").arg(cellContent);
        }

        static QString shortcutBadgeCell(const QString& shortcutText) {
            const QString clean = cleanPlainText(shortcutText);
            if (clean.isEmpty() || clean == "—") {
                return
                    "<td align=\"right\" valign=\"middle\" class=\"trigger-cell\" width=\"1%\" nowrap><span class=\"empty-badge\">—</span></td>";
            }
            return QString(
                    "<td align=\"right\" valign=\"middle\" class=\"trigger-cell\" width=\"1%\" nowrap><span class=\"keycap\" style=\"vertical-align: middle;\">%1</span></td>")
               .arg(clean.toHtmlEscaped());
        }

        static QString itemRow(const QString& iconCellHtml, const QString& actionCellHtml, const QString& triggerCellHtml,
                               const QString& aliasSubRowHtml, const QString& descSubRowHtml) {
            QString rowHtml;
            rowHtml += "<tr class=\"item-row\">";
            rowHtml += iconCellHtml;
            rowHtml += actionCellHtml;
            rowHtml += triggerCellHtml;
            rowHtml += "</tr>";

            if (!aliasSubRowHtml.isEmpty()) {
                rowHtml += aliasSubRowHtml;
            }

            if (!descSubRowHtml.isEmpty()) {
                rowHtml += descSubRowHtml;
            }

            return rowHtml;
        }

        static QString plainTriggerCell(const QString& triggerText) {
            const QString clean = cleanPlainText(triggerText);
            if (clean.isEmpty() || clean == "—") {
                return
                    "<td align=\"right\" valign=\"middle\" class=\"trigger-cell\" width=\"1%\" nowrap><span class=\"empty-badge\">—</span></td>";
            }
            return QString(
                    "<td align=\"right\" valign=\"middle\" class=\"trigger-cell\" width=\"1%\" nowrap><span class=\"cmd-trigger\" style=\"vertical-align: middle;\">%1</span></td>")
               .arg(clean.toHtmlEscaped());
        }

        static QString cmdAliasSubRow(const QStringList& aliases, int colSpan) {
            if (aliases.isEmpty()) {
                return QString();
            }
            return QString(
                "<tr class=\"cmd-alias-row\"><td></td><td colspan=\"%1\" align=\"right\" class=\"cmd-alias-cell\">"
                "<span class=\"cmd-alias-text\">%2</span></td></tr>").arg(colSpan).arg(aliases.join(", ").toHtmlEscaped());
        }

        static QString descSubRow(const QString& desc, int colSpan) {
            if (desc.isEmpty()) {
                return QString();
            }
            return QString("<tr class=\"desc-row\"><td></td><td colspan=\"%1\" class=\"action-desc\" valign=\"top\">%2</td></tr>").
                   arg(colSpan).arg(desc.toHtmlEscaped());
        }

        static QString categoryHeaderRow(const QString& title, const QString& iconUrl, int colSpan, bool isContinuation) {
            if (title.isEmpty()) {
                return QString();
            }

            QString iconHtml;
            if (!iconUrl.isEmpty()) {
                iconHtml = QString("<img src=\"%1\" width=\"12\" height=\"12\" style=\"vertical-align: middle;\">&nbsp;").arg(iconUrl);
            }

            QString titleHtml = cleanPlainText(title).toHtmlEscaped();
            if (isContinuation) {
                titleHtml += QString("&nbsp;<span class=\"continued-text\">%1</span>").arg(QObject::tr("(Continued)"));
            }

            return QString("<tr class=\"category-header-row\"><td colspan=\"%1\" class=\"category-header-cell\">%2%3</td></tr>").
                   arg(colSpan).arg(iconHtml, titleHtml);
        }
    };

    struct LC_CheatsheetItemData {
        QString m_name;
        QString m_groupTitle;
        QString m_iconResUrl;
        QString m_triggerCellHtml;
        QString m_aliasSubRowHtml;
        QString m_desc;
    };

    struct LC_CategoryData {
        QString m_title;
        QString m_iconResUrl;
        QList<LC_CheatsheetItemData> m_items;
        QList<QString> m_itemRows;
    };

    struct LC_ColumnData {
        QString m_contentHtml;
        qreal m_currentHeight{0.0};
    };

    struct LC_PageData {
        QVector<LC_ColumnData> m_columns;
        explicit LC_PageData(int cols) : m_columns(cols) {
        }
    };

    QString getBaseCss() {
        return "<style>" "body { font-family: sans-serif; font-size: 7.5pt; color: #1f2937; margin: 0; padding: 0; }"
            "table.data-table { width: 100%; border-collapse: collapse; margin-bottom: 6px; background-color: #ffffff; }"
            ".category-header-cell { font-size: 7.0pt; font-weight: bold; color: #374151; text-transform: uppercase; letter-spacing: 0.5px; "
            "   background-color: #f3f4f6; padding: 2.5px 5px; border-bottom: 1px solid #d1d5db; border-top: 1px solid #e5e7eb; vertical-align: middle; }"
            "tr.item-row td { padding: 2px 3px; border-bottom: 0.5pt solid #f1f5f9; vertical-align: middle; background-color: #ffffff; }"
            "tr.cmd-alias-row td { padding: 0px 3px 2.5px 3px; border-bottom: 0.5pt solid #f1f5f9; vertical-align: top; background-color: #ffffff; }"
            "tr.desc-row td { padding: 0px 3px 2.5px 3px; border-bottom: 0.5pt solid #f1f5f9; vertical-align: top; background-color: #ffffff; }"
            ".icon-col { width: 22px; text-align: center; vertical-align: middle; padding: 2px 2px; }"
            ".action-cell { vertical-align: middle; padding: 2px 3px; }"
            ".action-name { font-weight: normal; color: #111827; font-size: 7.5pt; }"
            ".action-desc { font-size: 6.5pt; color: #6b7280; line-height: 1.1; }"
            ".trigger-cell { vertical-align: middle; padding: 2px 3px; text-align: right; white-space: nowrap; }"
            ".cmd-alias-cell { vertical-align: top; text-align: right; }"
            ".cmd-trigger { font-family: monospace; font-size: 6.0pt; font-weight: bolder; color: #111827; letter-spacing: 2px; }"
            ".cmd-alias-text { font-family: monospace; font-size: 6.0pt; font-weight: normal; color: #4b5563; letter-spacing: 2px; }"
            ".keycap { font-family: monospace; font-size: 6.0pt; font-weight: bolder; color: #111827; letter-spacing: 2px; "
            "          background-color: #f3f4f6; border: 1px solid #d1d5db; border-radius: 2px; padding: 0.5px 3px; }"
            ".empty-badge { color: #9ca3af; font-size: 6.8pt; font-family: monospace; }"
            ".continued-text { font-size: 6.0pt; font-weight: normal; color: #6b7280; text-transform: none; letter-spacing: normal; }"
            "</style>";
    }

    qreal measureBlockHeight(const QString& tableHtml, qreal colWidth, QTextDocument& probeDoc) {
        probeDoc.setTextWidth(colWidth);
        probeDoc.setHtml(getBaseCss() + tableHtml);
        return probeDoc.size().height();
    }

    QString assemblePageHtml(const LC_PageData& pageData, int numCols) {
        QString html = getBaseCss();
        html += "<table width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" style=\"border: none;\"><tr>";

        const QString colWidthPercent = QString("%1%").arg(100.0 / numCols - 1.5, 0, 'f', 1);
        for (int c = 0; c < numCols; ++c) {
            if (c > 0) {
                html += "<td width=\"2%\"></td>";
            }
            html += QString("<td width=\"%1\" valign=\"top\">%2</td>").arg(colWidthPercent, pageData.m_columns[c].m_contentHtml);
        }

        html += "</tr></table>";
        return html;
    }

    QList<LC_CategoryData> extractRawCategories(const QAbstractItemModel* model, const LC_CheatsheetOptions& options,
                                                QMap<QUrl, QPixmap>& docResources) {
        QList<LC_CategoryData> categories;
        const int categoriesCount = model->rowCount();

        for (int category = 0; category < categoriesCount; ++category) {
            const QModelIndex groupIdx = model->index(category, 0);
            const QString groupTitle = groupIdx.data(Qt::DisplayRole).toString();

            const QVariant iconVar = groupIdx.data(Qt::DecorationRole);
            const QIcon groupIcon = iconVar.isValid() ? iconVar.value<QIcon>() : QIcon();
            const QString groupResUrl = QString("icon://group_%1").arg(category);
            if (!groupIcon.isNull()) {
                docResources.insert(QUrl(groupResUrl), groupIcon.pixmap(32, 32));
            }

            LC_CategoryData catData;
            catData.m_title = groupTitle;
            catData.m_iconResUrl = !groupIcon.isNull() ? groupResUrl : QString();

            const int childCount = model->rowCount(groupIdx);
            for (int child = 0; child < childCount; ++child) {
                const QModelIndex itemIdx = model->index(child, 0, groupIdx);

                if (options.type == CheatsheetType::Keymap) {
                    const auto* sModel = qobject_cast<const LC_ShortcutsTreeModel*>(model);
                    auto* item = (sModel != nullptr) ? sModel->getItemForIndex(itemIdx) : nullptr;
                    if (item == nullptr) {
                        continue;
                    }

                    const QString shortcutText = item->getShortcutViewString();
                    if (options.onlyAssignedShortcuts && shortcutText.isEmpty()) {
                        continue;
                    }

                    const QString actionName = item->getName();
                    const QIcon icon = item->getIcon();
                    const QString resUrl = QString("icon://act_%1_%2").arg(category).arg(child);
                    if (!icon.isNull()) {
                        docResources.insert(QUrl(resUrl), icon.pixmap(48, 48));
                    }

                    QString desc;
                    if (options.includeDescriptions) {
                        desc = LC_CheatsheetHtmlHelper::cleanDescription(item->getDescription(), actionName, shortcutText);
                    }

                    LC_CheatsheetItemData itemData;
                    itemData.m_name = actionName;
                    itemData.m_groupTitle = groupTitle;
                    itemData.m_iconResUrl = !icon.isNull() ? resUrl : QString();
                    itemData.m_triggerCellHtml = LC_CheatsheetHtmlHelper::shortcutBadgeCell(shortcutText);
                    itemData.m_desc = desc;

                    catData.m_items.append(itemData);
                }
                else {
                    const auto* cModel = qobject_cast<const LC_CommandsTreeModel*>(model);
                    auto* item = (cModel != nullptr) ? cModel->getItemForIndex(itemIdx) : nullptr;
                    if (item == nullptr) {
                        continue;
                    }

                    if (item->isKeyword() && !options.includeKeywords) {
                        continue;
                    }

                    const QString name = item->name();
                    const QIcon icon = item->icon();
                    const QString resUrl = QString("icon://cmd_%1_%2").arg(category).arg(child);
                    if (!icon.isNull()) {
                        docResources.insert(QUrl(resUrl), icon.pixmap(48, 48));
                    }

                    QString desc;
                    if (options.includeDescriptions) {
                        desc = LC_CheatsheetHtmlHelper::cleanDescription(item->description(), name);
                    }

                    const QString cleanKey = LC_CheatsheetHtmlHelper::cleanPlainText(item->getKeyCode());
                    const bool hasKeycode = (!cleanKey.isEmpty() && cleanKey != "—");
                    const QStringList& allTriggers = item->effectiveTriggers();

                    QString row1TriggerHtml;
                    QStringList remainingTriggers;

                    if (hasKeycode) {
                        row1TriggerHtml = LC_CheatsheetHtmlHelper::shortcutBadgeCell(cleanKey);
                        for (const auto& trig : allTriggers) {
                            if (trig.compare(cleanKey, Qt::CaseInsensitive) != 0) {
                                remainingTriggers.append(trig);
                            }
                        }
                    }
                    else if (!allTriggers.isEmpty()) {
                        row1TriggerHtml = LC_CheatsheetHtmlHelper::plainTriggerCell(allTriggers.first());
                        for (int i = 1; i < allTriggers.size(); ++i) {
                            remainingTriggers.append(allTriggers.at(i));
                        }
                    }
                    else {
                        row1TriggerHtml = LC_CheatsheetHtmlHelper::shortcutBadgeCell(QString());
                    }

                    LC_CheatsheetItemData itemData;
                    itemData.m_name = name;
                    itemData.m_groupTitle = groupTitle;
                    itemData.m_iconResUrl = !icon.isNull() ? resUrl : QString();
                    itemData.m_triggerCellHtml = row1TriggerHtml;
                    itemData.m_aliasSubRowHtml = LC_CheatsheetHtmlHelper::cmdAliasSubRow(remainingTriggers, 2);
                    itemData.m_desc = desc;

                    catData.m_items.append(itemData);
                }
            }

            if (!catData.m_items.isEmpty()) {
                categories.append(catData);
            }
        }

        return categories;
    }

    QList<LC_CategoryData> prepareFlatLayout(const QList<LC_CategoryData>& rawCategories, const LC_CheatsheetOptions& options) {
        QList<LC_CheatsheetItemData> allItems;
        QHash<QString, int> nameCounts;

        for (const auto& cat : rawCategories) {
            for (const auto& item : cat.m_items) {
                allItems.append(item);
                nameCounts[item.m_name]++;
            }
        }

        if (options.sortByName) {
            QCollator collator;
            collator.setCaseSensitivity(Qt::CaseInsensitive);
            std::sort(allItems.begin(), allItems.end(), [&collator](const LC_CheatsheetItemData& a, const LC_CheatsheetItemData& b) {
                const int nameCmp = collator.compare(a.m_name, b.m_name);
                if (nameCmp != 0) {
                    return nameCmp < 0;
                }
                return collator.compare(a.m_groupTitle, b.m_groupTitle) < 0;
            });
        }

        LC_CategoryData flatCat;
        flatCat.m_title = QString();
        flatCat.m_iconResUrl = QString();

        for (const auto& item : allItems) {
            const QString disambiguation = (nameCounts.value(item.m_name) > 1) ? item.m_groupTitle : QString();
            const QString iconCell = LC_CheatsheetHtmlHelper::iconCell(item.m_iconResUrl);
            const QString actCell = LC_CheatsheetHtmlHelper::actionCell(item.m_name, disambiguation);
            const QString descRow = LC_CheatsheetHtmlHelper::descSubRow(item.m_desc, 2);

            flatCat.m_itemRows.append(LC_CheatsheetHtmlHelper::itemRow(iconCell, actCell, item.m_triggerCellHtml,
                                                                       item.m_aliasSubRowHtml, descRow));
        }

        QList<LC_CategoryData> result;
        if (!flatCat.m_itemRows.isEmpty()) {
            result.append(flatCat);
        }
        return result;
    }

    QList<LC_CategoryData> prepareCategoriesForLayout(const QList<LC_CategoryData>& rawCategories, const LC_CheatsheetOptions& options) {
        if (!options.groupByCategory) {
            return prepareFlatLayout(rawCategories, options);
        }

        QList<LC_CategoryData> categories = rawCategories;

        for (auto& cat : categories) {
            for (const auto& item : cat.m_items) {
                const QString iconCell = LC_CheatsheetHtmlHelper::iconCell(item.m_iconResUrl);
                const QString actCell = LC_CheatsheetHtmlHelper::actionCell(item.m_name);
                const QString descRow = LC_CheatsheetHtmlHelper::descSubRow(item.m_desc, 2);

                cat.m_itemRows.append(LC_CheatsheetHtmlHelper::itemRow(iconCell, actCell, item.m_triggerCellHtml,
                                                                       item.m_aliasSubRowHtml, descRow));
            }
        }

        return categories;
    }

    QList<LC_PageData> packPages(const QList<LC_CategoryData>& categories, int numCols, qreal colWidth, qreal maxColHeight,
                                 QTextDocument& probeDoc) {
        QList<LC_PageData> pages;
        pages.append(LC_PageData(numCols));
        int curPage = 0;
        int curCol = 0;

        auto advanceColumn = [&]() {
            curCol++;
            if (curCol >= numCols) {
                curCol = 0;
                curPage++;
                pages.append(LC_PageData(numCols));
            }
        };

        for (const auto& cat : categories) {
            int itemIdx = 0;
            bool isContinuation = false;

            while (itemIdx < cat.m_itemRows.size()) {
                qreal availHeight = maxColHeight - pages[curPage].m_columns[curCol].m_currentHeight;

                const qreal minAdvanceThreshold = !cat.m_title.isEmpty() ? 55.0 : 25.0;
                if (availHeight < minAdvanceThreshold && pages[curPage].m_columns[curCol].m_currentHeight > 0.0) {
                    advanceColumn();
                    availHeight = maxColHeight;
                }

                int fitCount = 0;
                QString bestTableHtml;
                qreal bestTableHeight = 0.0;

                for (int count = 1; itemIdx + count <= cat.m_itemRows.size(); ++count) {
                    QString testRows;
                    for (int i = 0; i < count; ++i) {
                        testRows += cat.m_itemRows[itemIdx + i];
                    }
                    const QString headerHtml = LC_CheatsheetHtmlHelper::categoryHeaderRow(cat.m_title, cat.m_iconResUrl, 3, isContinuation);
                    const QString testTable = "<table class=\"data-table\" width=\"100%\">" + headerHtml + testRows + "</table>";

                    const qreal h = measureBlockHeight(testTable, colWidth, probeDoc);
                    if (h <= availHeight) {
                        fitCount = count;
                        bestTableHtml = testTable;
                        bestTableHeight = h;
                    }
                    else {
                        break;
                    }
                }

                if (fitCount == 0) {
                    if (pages[curPage].m_columns[curCol].m_currentHeight > 0.0) {
                        advanceColumn();
                        continue;
                    }
                    else {
                        fitCount = 1;
                        const QString headerHtml = LC_CheatsheetHtmlHelper::categoryHeaderRow(
                            cat.m_title, cat.m_iconResUrl, 3, isContinuation);
                        bestTableHtml = "<table class=\"data-table\" width=\"100%\">" + headerHtml + cat.m_itemRows[itemIdx] + "</table>";
                        bestTableHeight = measureBlockHeight(bestTableHtml, colWidth, probeDoc);
                    }
                }

                pages[curPage].m_columns[curCol].m_contentHtml += bestTableHtml;
                pages[curPage].m_columns[curCol].m_currentHeight += bestTableHeight + 6.0;
                itemIdx += fitCount;
                isContinuation = true;

                if (itemIdx < cat.m_itemRows.size()) {
                    advanceColumn();
                }
            }
        }

        return pages;
    }

    void renderCheatsheetPages(QPainter& painter, QPrinter& printer, const QList<LC_PageData>& pages,
                               const QMap<QUrl, QPixmap>& docResources, const LC_CheatsheetOptions& options, qreal contentWidth,
                               qreal maxColHeight, qreal headerHeight, qreal footerHeight, const QRectF& pageRect, int numCols) {
        const int totalPages = pages.size();
        const QPixmap logoPix(":/images/librecad.png");
        const QString docTitle = (options.type == CheatsheetType::Keymap)
                                     ? QObject::tr("LibreCAD Keyboard Shortcuts")
                                     : QObject::tr("LibreCAD Command Aliases");

        const QString dateStr = QLocale().toString(QDate::currentDate(), QLocale::LongFormat);
        const QString versionStr = QString("LibreCAD %1").arg(XSTR(LC_VERSION));

        for (int currentPage = 0; currentPage < totalPages; ++currentPage) {
            if (currentPage > 0) {
                printer.newPage();
            }

            // Running Header
            if (!logoPix.isNull()) {
                painter.drawPixmap(QRectF(0, 0, 22, 22), logoPix.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation),
                                   QRectF(0, 0, 44, 44));
            }

            QFont titleFont = painter.font();
            titleFont.setPointSize(11);
            titleFont.setBold(true);

            painter.setFont(titleFont);
            painter.setPen(QColor("#111827"));
            painter.drawText(QRectF(28, 0, contentWidth - 210, 16), Qt::AlignLeft | Qt::AlignVCenter, docTitle);

            QFont subFont = painter.font();
            subFont.setPointSize(8);
            subFont.setBold(false);

            painter.setFont(subFont);
            painter.setPen(QColor("#4b5563"));
            const QString schemeSubtitle = QObject::tr("Scheme: %1").arg(
                !options.schemeName.isEmpty() ? options.schemeName : QObject::tr("Default"));
            painter.drawText(QRectF(28, 16, contentWidth - 210, 14), Qt::AlignLeft | Qt::AlignVCenter, schemeSubtitle);

            painter.drawText(QRectF(contentWidth - 180, 0, 180, 16), Qt::AlignRight | Qt::AlignVCenter, dateStr);
            painter.drawText(QRectF(contentWidth - 180, 16, 180, 14), Qt::AlignRight | Qt::AlignVCenter, versionStr);

            painter.setPen(QPen(QColor("#9ca3af"), 0.75));
            painter.drawLine(QPointF(0, 36), QPointF(contentWidth, 36));

            // Page Content
            QTextDocument pageDoc;
            for (auto it = docResources.constBegin(); it != docResources.constEnd(); ++it) {
                pageDoc.addResource(QTextDocument::ImageResource, it.key(), it.value());
            }
            pageDoc.setHtml(assemblePageHtml(pages[currentPage], numCols));
            pageDoc.setPageSize(QSizeF(contentWidth, maxColHeight));

            painter.save();
            painter.translate(0, headerHeight);
            pageDoc.drawContents(&painter, QRectF(0, 0, contentWidth, maxColHeight));
            painter.restore();

            // Running Footer
            const qreal footerY = pageRect.height() - footerHeight + 4;
            painter.setPen(QPen(QColor("#9ca3af"), 0.75));
            painter.drawLine(QPointF(0, footerY), QPointF(contentWidth, footerY));

            painter.setFont(subFont);
            painter.setPen(QColor("#6b7280"));
            painter.drawText(QRectF(0, footerY + 3, contentWidth / 2, 14), Qt::AlignLeft | Qt::AlignVCenter,
                             QObject::tr("LibreCAD Cheatsheet"));

            const QString pageStr = QObject::tr("Page %1 of %2").arg(currentPage + 1).arg(totalPages);
            painter.drawText(QRectF(contentWidth / 2, footerY + 3, contentWidth / 2, 14), Qt::AlignRight | Qt::AlignVCenter, pageStr);
        }
    }
}

void LC_CheatsheetGenerator::showCompletionConfirmation(QWidget* parent, const LC_CheatsheetOptions& options) {
    if (options.destination == CheatsheetDestination::PdfFile) {
        const QFileInfo fileInfo(options.pdfFilePath);
        const QString nativePath = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
        QMessageBox msgBox(parent);
        msgBox.setWindowTitle(tr("Export Complete"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Cheatsheet PDF generated successfully."));
        msgBox.setInformativeText(nativePath);

        auto* btnOpenFolder = msgBox.addButton(tr("Open Folder"), QMessageBox::ActionRole);
        auto* btnOpenFile = msgBox.addButton(tr("Open PDF"), QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Close);

        msgBox.exec();

        if (msgBox.clickedButton() == btnOpenFolder) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
        }
        else if (msgBox.clickedButton() == btnOpenFile) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
        }
    }
    else {
        QMessageBox::information(parent, tr("Print Complete"), tr("Cheatsheet was sent to the printer successfully."));
    }
}

bool LC_CheatsheetGenerator::generate(QWidget* parent, const LC_CheatsheetOptions& options, const QAbstractItemModel* model) {
    if (model == nullptr) {
        return false;
    }
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setResolution(96);

    const auto pageSize = (options.paperSize == CheatsheetPaperSize::Letter) ? QPageSize(QPageSize::Letter) : QPageSize(QPageSize::A4);
    printer.setPageSize(pageSize);

    printer.setPageOrientation(options.isLandscape ? QPageLayout::Orientation::Landscape : QPageLayout::Orientation::Portrait);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);

    if (options.destination == CheatsheetDestination::PdfFile) {
        if (options.pdfFilePath.isEmpty()) {
            return false;
        }
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(options.pdfFilePath);
    }
    else {
        QPrintDialog printDialog(&printer, parent);
        printDialog.setWindowTitle(tr("Print Cheatsheet"));
        if (printDialog.exec() != QDialog::Accepted) {
            return false;
        }
    }

    {
        const LC_WaitCursorGuard cursorGuard;

        const QRectF pageRect = printer.pageLayout().paintRectPixels(96);
        const qreal contentWidth = pageRect.width();
        constexpr qreal headerHeight = 44.0;
        constexpr qreal footerHeight = 26.0;
        const qreal maxColHeight = pageRect.height() - headerHeight - footerHeight;

        const int numCols = options.isLandscape ? 3 : 2;
        const qreal colGapWidth = contentWidth * 0.02;
        const qreal colWidth = (contentWidth - (numCols - 1) * colGapWidth) / numCols;

        QMap<QUrl, QPixmap> docResources;
        const QList<LC_CategoryData> rawCategories = extractRawCategories(model, options, docResources);
        const QList<LC_CategoryData> preparedCategories = prepareCategoriesForLayout(rawCategories, options);

        QTextDocument probeDoc;
        for (auto it = docResources.constBegin(); it != docResources.constEnd(); ++it) {
            probeDoc.addResource(QTextDocument::ImageResource, it.key(), it.value());
        }

        const QList<LC_PageData> pages = packPages(preparedCategories, numCols, colWidth, maxColHeight, probeDoc);

        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);

        renderCheatsheetPages(painter, printer, pages, docResources, options, contentWidth, maxColHeight, headerHeight, footerHeight,
                              pageRect, numCols);

        painter.end();
    }

    showCompletionConfirmation(parent, options);

    return true;
}
