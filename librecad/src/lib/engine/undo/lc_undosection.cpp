/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2018 A. Stebich (librecad@mail.lordofbikes.de)
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

#include "lc_graphicviewport.h"
#include "lc_undosection.h"
#include "rs_document.h"
#include "rs_graphic.h"
#include "rs_insert.h"
#include "rs_layer.h"

LC_UndoSection::LC_UndoSection(RS_Document *doc, LC_GraphicViewport* view) :
    m_document( doc),
    m_viewport(view){
    Q_ASSERT(nullptr != doc && nullptr != view);
    m_document->startUndoCycle();
}

LC_UndoSection::~LC_UndoSection(){
    try {
        RS_Undoable* relativeZeroUndoable = m_viewport->getRelativeZeroUndoable();
        if (relativeZeroUndoable != nullptr) {
            m_document->addUndoable(relativeZeroUndoable);
        }
        m_document->endUndoCycle();
    }
    catch (...) {
    }
}

void LC_UndoSection::undoableDelete(RS_Entity* e) const {
    m_document->undoableDelete(e);
}

void LC_UndoSection::undoableAdd(RS_Entity* e) const {
    m_document->undoableAdd(e);
}

void LC_UndoSection::addUndoable(RS_Undoable* u) const {
    m_document->addUndoable(u);
}

void LC_UndoSection::undoableReplace(RS_Entity* entityToDelete, RS_Entity* entityToAdd) const {
     m_document->undoableDelete(entityToDelete);
     m_document->undoableAdd(entityToAdd);
}

bool LC_UndoSection::undoableExecute(FunUndoable doUndoable) {
    return undoableExecute(doUndoable, {});
}

bool LC_UndoSection::undoableExecute(FunUndoable doUndoable, FunSelection doSelection) {
    LC_DocumentModificationBatch ctx;
    bool success = doUndoable(ctx);
    ctx.success = success;
    if (success) {
        if (!ctx.entitiesToDelete.isEmpty()) {
            for (const auto e: ctx.entitiesToDelete) {
                auto layer = e->getLayer(true);
                if (!layer->isLocked()) {
                      m_document->undoableDelete(e);
                }
            }
        }
        if (!ctx.entitiesToAdd.isEmpty()) {
            setupAndUndoableAdd(ctx.entitiesToAdd, ctx.m_setActiveLayer, ctx.m_setActivePen);
        }
    }
    doSelection(ctx, m_document);
    return success;
}

void LC_UndoSection::setupAndUndoableAdd(const QList<RS_Entity*>& entitiesToInsert, bool setActiveLayer, bool setActivePen) const {
    auto graphic = m_document->getGraphic();
    RS_Layer *activeLayer = setActiveLayer ? graphic->getActiveLayer() : nullptr;
    RS_Pen activePen      = setActivePen ? graphic->getActivePen() : RS_Pen();
    for (auto ent: entitiesToInsert) {
        undoableAdd(ent);
        if (setActiveLayer) {
            ent->setLayer(activeLayer);
        }
        if (setActivePen){
            ent->setPen(activePen);
        }
        auto rtti = ent->rtti();
        if (rtti == RS2::EntityInsert || RS2::isDimensionalEntity(rtti)) {
            static_cast<RS_Insert*>(ent)->update();
        }
    }
}
