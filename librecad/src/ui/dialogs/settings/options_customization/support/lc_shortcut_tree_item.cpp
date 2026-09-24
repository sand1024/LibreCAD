/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "lc_shortcut_tree_item.h"

#include <QAction>

#include "lc_action.h"
#include "lc_shortcut_info.h"

LC_ShortcutTreeItem::LC_ShortcutTreeItem(LC_ShortcutTreeItem* parent, const QAction* action, LC_ShortcutInfo* shortcutInfo)
    : LC_ActionMappingItemBase(parent,
                               action != nullptr ? action->text().remove('&') : QString(),
                               action != nullptr ? action->icon() : QIcon(),
                               false)
    , m_description(action != nullptr ? action->toolTip() : QString())
    , m_shortcutInfo(shortcutInfo) {
    if (action != nullptr) {
        const auto* lcAct = dynamic_cast<const LC_Action*>(action);
        if (lcAct != nullptr){
            if (!lcAct->description().isEmpty()) {
                m_description = lcAct->description();
            }
        }
        else {
            const auto property = action->property(LC_ActionKeys::PROP_DESCRIPTION);
            if (property.isValid()) {
                m_description = property.toString();
            }
        }
    }
}

LC_ShortcutTreeItem::LC_ShortcutTreeItem(LC_ShortcutTreeItem* parent, const QIcon& ic, const QString& name, const QString& desc)
    : LC_ActionMappingItemBase(parent, name, ic, true)
    , m_description(desc)
    , m_shortcutInfo(nullptr) {
}

void LC_ShortcutTreeItem::clearShortcut() const {
    if (m_shortcutInfo != nullptr) {
        m_shortcutInfo->clear();
    }
}

void LC_ShortcutTreeItem::resetShortcutToDefault() const {
    if (m_shortcutInfo != nullptr) {
        m_shortcutInfo->resetToDefault();
    }
}

LC_ShortcutInfo* LC_ShortcutTreeItem::getShortcutInfo() const {
    return m_shortcutInfo;
}

LC_ShortcutTreeItem* LC_ShortcutTreeItem::addChild(const QAction* action, LC_ShortcutInfo* shortcutInfo) {
    auto* childItem = new LC_ShortcutTreeItem(this, action, shortcutInfo);
    appendChild(childItem);
    return childItem;
}

QString LC_ShortcutTreeItem::identifier() const {
    return (m_shortcutInfo != nullptr) ? m_shortcutInfo->getName() : QString();
}


QString LC_ShortcutTreeItem::getShortcutViewString() const {
    if (m_isGroup || m_shortcutInfo == nullptr) {
        return QString();
    }
    return m_shortcutInfo->getKeyAsString();
}
