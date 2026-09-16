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

#include "lc_action_tooltip_builder.h"

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QPalette>

#include "lc_action.h"
#include "lc_proxy_style.h"
#include "lc_settings_appearance.h"
#include "lc_settings_commands_promotion.h"
#include "shortcuts/lc_shortcuts_manager.h"

QString LC_ActionTooltipBuilder::buildTooltip(const QAction* action,
                                              const bool showTitle,
                                              const bool showDesc,
                                              const bool showShortcut,
                                              const bool showCommand) {
    if (action == nullptr) {
        return QString();
    }

    const QString title = action->text().remove('&').trimmed();

    QString desc;
    const auto* lcAct = dynamic_cast<const LC_Action*>(action);
    if (lcAct != nullptr && !lcAct->description().isEmpty()) {
        desc = lcAct->description().trimmed();
    }
    else {
        desc = LC_ShortcutsManager::getPlainActionToolTip(action).trimmed();
    }

    // Omit description if it merely duplicates the action title
    if (desc.compare(title, Qt::CaseInsensitive) == 0) {
        desc.clear();
    }

    const QString shortcut = action->shortcut().toString(QKeySequence::NativeText).trimmed();
    const QString command  = action->property(PROP_CMD_LINE).toString().trimmed();

    const bool hasTitle    = showTitle && !title.isEmpty();
    const bool hasDesc     = showDesc && !desc.isEmpty();
    const bool hasShortcut = showShortcut && !shortcut.isEmpty();
    const bool hasCommand  = showCommand && !command.isEmpty();

    // 1. Single-Line Metadata Footer (Shortcut | Command)
    QStringList metaParts;
    if (hasShortcut) {
        metaParts.append(QString("%1: <b>%2</b>").arg(tr("Shortcut"), shortcut.toHtmlEscaped()));
    }
    if (hasCommand) {
        metaParts.append(QString("%1: <b>%2</b>").arg(tr("Command"), command.toHtmlEscaped()));
    }
    const QString metaFooter = metaParts.join(" &nbsp;|&nbsp; ");

    if (!hasTitle && !hasDesc && metaParts.isEmpty()) {
        return QString();
    }

    // 2. Palette-driven colors (No hardcoded hex strings)
    const QPalette pal = QApplication::palette();
    QColor metaColor = pal.color(QPalette::PlaceholderText);
    if (metaColor.value() == 0 && pal.color(QPalette::ToolTipText).value() == 0) {
        metaColor = QColor("gray");
    }

    // 3. Assemble HTML Card matching LC_EventFilterToolTip card structure
    QString html;

    if (hasTitle) {
        html += QString("<b>%1</b>").arg(title.toHtmlEscaped());
    }

    if (hasTitle && (hasDesc || !metaParts.isEmpty())) {
        html += "<hr>";
    }

    if (hasDesc) {
        html += QString("<div>%1</div>").arg(desc.toHtmlEscaped());
    }

    if (!metaParts.isEmpty()) {
        const QString marginTop = hasDesc ? "margin-top: 4px;" : "";
        html += QString("<div style=\"%1 color: %2; font-size: small;\">%3</div>")
                    .arg(marginTop, metaColor.name(), metaFooter);
    }

    return html;
}

void LC_ActionTooltipBuilder::updateActionTooltip(QAction* action) {
    if (action == nullptr) {
        return;
    }

    using namespace CFG_Appearance;
    const bool showTitle    = o_ShowTooltipActionName;
    const bool showDesc     = o_ShowTooltipDescription;
    const bool showShortcut = o_ShowKeyboardShortcutsInTooltips;
    const bool showCommand  = CFG_CommandsPromotion::o_ShowCommandsInTooltips;

    const QString tooltip = buildTooltip(action, showTitle, showDesc, showShortcut, showCommand);
    action->setToolTip(tooltip);
}

void LC_ActionTooltipBuilder::updateAllTooltips(const QMap<QString, QAction*>& actionsMap) {
    using namespace CFG_Appearance;
    const bool showTitle    = o_ShowTooltipActionName;
    const bool showDesc     = o_ShowTooltipDescription;
    const bool showShortcut = o_ShowKeyboardShortcutsInTooltips;
    const bool showCommand  = CFG_CommandsPromotion::o_ShowCommandsInTooltips;

    for (auto* action : actionsMap) {
        if (action != nullptr) {
            const QString tooltip = buildTooltip(action, showTitle, showDesc, showShortcut, showCommand);
            action->setToolTip(tooltip);
        }
    }
}
