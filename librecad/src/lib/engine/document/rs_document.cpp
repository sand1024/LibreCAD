/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2018 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include "rs_document.h"

#include <set>

#include "rs_debug.h"

/**
 * Constructor.
 *
 * @param parent Parent of the document. Often that's NULL but
 *        for blocks it's the blocklist.
 */
RS_Document::RS_Document(RS_EntityContainer* parent)
    : RS_EntityContainer{parent}
    , activePen {RS_Color{RS2::FlagByLayer}, RS2::WidthByLayer, RS2::LineByLayer}
    , m_selectedSet{std::make_unique<LC_SelectedSet>()}

{
    RS_DEBUG->print("RS_Document::RS_Document() ");
}

void RS_Document::addEntity(RS_Entity* entity) {
    entity->parent = this;
    RS_EntityContainer::addEntity(entity);
}

/**
 * Overwritten to set modified flag when undo cycle finished with undoable(s).
 */
void RS_Document::endUndoCycle(){
    if (hasUndoable()) {
        setModified(true);
    }
    m_selectedSet->enableListeners();
    RS_Undo::endUndoCycle();
    setAutoUpdateBorders(m_savedAutoUpdateBorders);
    calculateBorders();
}

void RS_Document::startUndoCycle() {
    m_selectedSet->disableListeners();
    RS_Undo::startUndoCycle();
    m_savedAutoUpdateBorders = getAutoUpdateBorders();
    setAutoUpdateBorders(false);
}

void RS_Document::startBulkUndoablesCleanup() {
    m_savedAutoUpdateBorders = getAutoUpdateBorders();
    setAutoUpdateBorders(false);
}

void RS_Document::endBulkUndoablesCleanup() {
    setAutoUpdateBorders(m_savedAutoUpdateBorders);
    calculateBorders();
}

bool RS_Document::isSingleEntitySelected() const {
    QList<RS_Entity*> entitiesList;
    collectSelected(entitiesList);
    return entitiesList.size() == 1;
}

bool RS_Document::collectSelected(QList<RS_Entity*>& entitiesList) const {
    auto selection = getSelection();
    if (selection->isEmpty()) {
        return false;
    }
    return selection->collectSelectedEntities(entitiesList);
}

RS_Document::LC_SelectionInfo RS_Document::getSelectionInfo(const QList<RS2::EntityType> &types) const {
    LC_SelectionInfo result;
    std::set<RS2::EntityType> type{types.cbegin(), types.cend()};
    QList<RS_Entity*> selection;

    if (collectSelected(selection)) {
        for (RS_Entity* e : selection) {
            if (types.empty() || type.count(e->rtti())) {
                result.count++;
                double entityLength = e->getLength();
                if (entityLength >= 0.) {
                    result.length += entityLength;
                }
            }
        }
    }
    return result;
}

/**
 * Counts the selected entities in this container.
 */
[[deprecated]]
unsigned RS_Document::countSelected(bool deep, QList<RS2::EntityType> const &types) {
    unsigned count = 0;
    std::set<RS2::EntityType> type{types.cbegin(), types.cend()};

    for (RS_Entity *entity: *this) {
        if (entity->isSelected()) {  // fixme - SELECTION - selection collection! Review where it's used and rework
            if (!types.size() || type.count(entity->rtti())) {
                count++;
            }
        }
        /*if (entity->isContainer()) {
            count += dynamic_cast<RS_EntityContainer *>(entity)->countSelected(deep); // fixme - hm... - what about entity types there? and deep flag?
        }*/
    }
    return count;
}

void RS_Document::collectSelected(QList<RS_Entity*> &collect, bool deep, QList<RS2::EntityType> const &types) {
    std::set<RS2::EntityType> type{types.cbegin(), types.cend()};
    QList<RS_Entity*> m_entities; // FIXME _ COMPLETE!!!
    for (RS_Entity *e: m_entities) {
        if (e != nullptr) {
            if (e->isSelected()) { // fixme - rework!!! // fixme - SELECTION - selection collection!
                if (types.empty() || type.count(e->rtti())) {
                    collect.push_back(e);
                }
                /*if (deep && e->isContainer()) {
                    auto *container = dynamic_cast<RS_EntityContainer *>(e);
                    container->collectSelected(collect, false); // todo - check whether we need deep and types?
                }*/
            }
        }
    }
}
