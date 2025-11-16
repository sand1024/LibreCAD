/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * ********************************************************************************
 */


#include "lc_undoabledocumentmodificationaction.h"

#include "rs_graphic.h"
#include "rs_insert.h"
#include "rs_layer.h"
#include "rs_preview.h"

void LC_UndoableDocumentModificationAction::doTrigger() {
    bool success  = triggerModifyDocumentContent();
    doTriggerCompletion(success);
}

bool LC_UndoableDocumentModificationAction::triggerModifyDocumentContent() {
    LC_DocumentModificationBatch ctx;
    prepareDocumentModificationContext(ctx);
    bool success = doTriggerModificationsPrepare(ctx);
    ctx.success = success;
    if (success) {
        if (!ctx.entitiesToAdd.isEmpty()) {
            setupAndUndoableAdd(ctx);   // FIXME - WHAT IF THE THE LAYER IS LOCKED???
        }

        if (!ctx.entitiesToDelete.isEmpty()) {
            for (const auto e: ctx.entitiesToDelete) {
                auto layer = e->getLayer(true);
                if (!layer->isLocked()) {   // FIXME - review this logic. We can't let removing from locked layer, yet how to inform the user????
                    undoableDeleteEntity(e);
                }
            }
        }
    }
    clearDocumentModificationContext(ctx);
    return success;
}

void LC_UndoableDocumentModificationAction::clearDocumentModificationContext(LC_DocumentModificationBatch& ctx) {
    ctx.entitiesToAdd.clear();
    ctx.entitiesToDelete.clear();
}

void LC_UndoableDocumentModificationAction::setupAndUndoableAdd(const QList<RS_Entity*>& entitiesToInsert, bool setActiveLayer, bool setActivePen) {
    RS_Layer *activeLayer = setActiveLayer ? m_graphic->getActiveLayer() : nullptr;
    RS_Pen activePen      = setActivePen ? m_graphic->getActivePen() : RS_Pen();
    for (auto ent: entitiesToInsert) {
        undoableAdd(ent);
        if (setActiveLayer) {
            ent->setLayer(activeLayer);
        }
        if (setActivePen){
            ent->setPen(activePen);
        }
        auto rtti = ent->rtti();
        if (rtti == RS2::EntityInsert || RS2::isDimensionalEntity(rtti)) { // fixme - sand - review and check, to ensure that this is performed only once during trigger!!!! (DIMS, etc)
            static_cast<RS_Insert*>(ent)->update();
        }
    }
}

void LC_UndoableDocumentModificationAction::setupAndUndoableAdd(LC_DocumentModificationBatch &ctx) {
    setupAndUndoableAdd(ctx.entitiesToAdd, ctx.m_setActiveLayer, ctx.m_setActivePen);
}

void LC_UndoableDocumentModificationAction::previewEntitiesToAdd(LC_DocumentModificationBatch &ctx) const {
    for (const auto e: ctx.entitiesToAdd) {
        RS2::EntityType rtti = e->rtti();
        if (rtti == RS2::EntityInsert || RS2::isDimensionalEntity(rtti)) {
            e->update();
        }
        else{
            e->calculateBorders();
        }
        m_preview->addEntity(e);
    }
}

bool LC_SingleEntityCreationAction::doTriggerModificationsPrepare(LC_DocumentModificationBatch& ctx) {
    RS_Entity* e = doTriggerCreateEntity();
    if (e == nullptr) {
        return false;
    }
    ctx += e;
    return true;
}
