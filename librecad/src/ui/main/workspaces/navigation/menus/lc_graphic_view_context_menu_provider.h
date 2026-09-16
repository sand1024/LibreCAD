
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

#ifndef LC_GRAPHICVIEWCONTEXTMENUPROVIDER_H
#define LC_GRAPHICVIEWCONTEXTMENUPROVIDER_H
#include <QStringList>

#include "lc_menu_builder_base.h"
#include "lc_repository_graphic_view_context_menus.h"

class LC_ActionGroupManager;
class QG_GraphicView;
class LC_MenuActivator;
class RS_Entity;
class QMouseEvent;

class LC_GraphicViewContextMenuProvider :  public QObject, public LC_MenuBuilderBase {
public:
    explicit LC_GraphicViewContextMenuProvider(  LC_ActionGroupManager* actionGroupManager,LC_SpecialMenuServiceInterface* specialMenuService = nullptr);
    ~LC_GraphicViewContextMenuProvider() override;

    QMenu* createContextMenu(QG_GraphicView* graphicView, RS_Entity* entity, const RS_Vector& pos, const QMouseEvent* event);

    LC_RepositoryGraphicViewContextMenus* getMenusRepository() const {
        return m_menusRepository.get();
    }
    void applyCustomMenusScheme(const ContextMenusConfig& config);
    void loadActiveScheme();

    const ContextMenusConfig& getActiveConfig() const {
        return m_activeConfig;
    }

    QAction* getAction(const QString& key) const override;
protected:
    bool shouldIncludeNode(const ActionNode& node) const override;
    void appendActionItem(QMenu* parentMenu, const ActionNode& node) override;
private:
    bool isDefaultMenuInvokerEvent(const QMouseEvent* event) const;
    const ContextMenuDef* findMatchingMenu(const QMouseEvent* event, const RS_Entity* entity) const;

    QG_GraphicView* m_currentGraphicView{nullptr};
    RS_Entity* m_currentEntity{nullptr};
    RS_Vector m_currentPos;
    bool m_hasSelection{false};
    bool m_undoAvailable{false};
    bool m_redoAvailable{false};

    QList<LC_MenuActivator*> m_menuActivators;
    ContextMenusConfig m_activeConfig;
    std::unique_ptr<LC_RepositoryGraphicViewContextMenus> m_menusRepository;
    LC_ActionGroupManager* m_actionGroupManager;
};


#endif
