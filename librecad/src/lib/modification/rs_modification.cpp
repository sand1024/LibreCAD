/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
** Copyright LibreCAD librecad.org
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
// File: rs_modification.cpp
#include <QSet>

#include "lc_containertraverser.h"
#include "lc_graphicviewport.h"
#include "lc_linemath.h"
#include "lc_splinepoints.h"
#include "lc_undosection.h"
#include "rs_arc.h"
#include "rs_block.h"
#include "rs_circle.h"
#include "rs_clipboard.h"
#include "rs_creation.h"
#include "rs_debug.h"
#include "rs_ellipse.h"
#include "rs_graphic.h"
#include "rs_information.h"
#include "rs_insert.h"
#include "rs_layer.h"
#include "rs_line.h"
#include "rs_math.h"
#include "rs_modification.h"
#include "rs_mtext.h"
#include "rs_polyline.h"
#include "rs_selection.h"
#include "rs_settings.h"
#include "rs_text.h"
#include "rs_units.h"

#ifdef EMU_C99
#include "emu_c99.h"
#endif

class LC_SplinePoints;

namespace {
// fixme - hm, is it actually needed to mix the logic of modification and ui/undo?
/**
 * @brief getPasteScale - find scaling factor for pasting
 * @param const RS_PasteData& data - RS_PasteData
 * @param RS_Graphic *& source - source graphic. If source is nullptr, the graphic on the clipboard is used instead
 * @param const RS_Graphic& graphic - the target graphic
 * @return
 */
    RS_Vector getPasteScale(const RS_PasteData &data, RS_Graphic *&source, const RS_Graphic &graphic){

        // adjust scaling factor for units conversion in case of clipboard paste
        double factor = (RS_TOLERANCE < std::abs(data.factor)) ? data.factor : 1.0;
        // select source for paste
        if (source == nullptr){
            RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::paste: add graphic source from clipboard");
            source = RS_CLIPBOARD->getGraphic();
            // graphics from the clipboard need to be scaled. From the part lib not:
            const RS2::Unit sourceUnit = source->getUnit();
            const RS2::Unit targetUnit = graphic.getUnit();
            factor = RS_Units::convert(factor, sourceUnit, targetUnit);
        }
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::paste: pasting scale factor: %g", factor);
        // scale factor as vector
        return {factor, factor};
    }

/**
 * @brief addNewBlock() - create a new block
 * @param name - name of the new block to create
 * @param graphic - the target graphic
 * @return RS_Block - the block created
 */
    RS_Block *addNewBlock(const QString &name, RS_Graphic &graphic){
        const auto db = RS_BlockData(name, {0.0, 0.0}, false);
        const auto b = new RS_Block(&graphic, db);
        b->reparent(&graphic);
        graphic.addBlock(b);
        return b;
    }

    RS_VectorSolutions findIntersection(const RS_Entity &trimEntity, const RS_Entity &limitEntity, double tolerance = 1e-4){

        RS_VectorSolutions sol;
        if (limitEntity.isAtomic()){
            // intersection(s) of the two entities:
            return RS_Information::getIntersection(&trimEntity, &limitEntity, false);
        }
        if (limitEntity.isContainer()){
            const auto ec = static_cast<const RS_EntityContainer *>(&limitEntity);

            for(RS_Entity* e: lc::LC_ContainerTraverser{*ec, RS2::ResolveAll}.entities()) {
                RS_VectorSolutions s2 = RS_Information::getIntersection(&trimEntity,
                                                                        e, false);

                std::copy_if(s2.begin(), s2.end(), std::back_inserter(sol), [e, tolerance](const RS_Vector &vp){
                    return vp.valid && e->isPointOnEntity(vp, tolerance);
                });
            }
        }
        return sol;
    }

    RS_Arc *trimCircle(RS_Circle *circle, const RS_Vector &trimCoord, const RS_VectorSolutions &sol){
        double aStart = 0.;
        double aEnd = 2. * M_PI;
        switch (sol.size()) {
            case 0:
                break;
            case 1:
                aStart = circle->getCenter().angleTo(sol.at(0));
                aEnd = aStart + 2. * M_PI;
                break;
            default:
            case 2:
                //trim according to intersections
                const RS_Vector& center0 = circle->getCenter();
                const std::vector<double> angles { {center0.angleTo(sol[0]), center0.angleTo(sol[1])}};
                const double a0 = center0.angleTo(trimCoord);
                aStart = angles.front();
                aEnd = angles.back();
                if (!RS_Math::isAngleBetween(a0, aStart, aEnd, false)) {
                    std::swap(aStart, aEnd);
                }
                break;
        }
        const RS_ArcData arcData(circle->getCenter(),
                           circle->getRadius(),
                           aStart,
                           aEnd,
                           false);
        return new RS_Arc(circle->getParent(), arcData);
    }

/**
 * @brief getIdFlagString create a string by the entity and ID and type ID.
 * @param entity - entity, could be nullptr
 * @return std::string - "ID/typeID", or an empty string, if the input entity is nullptr
 */
    std::string getIdFlagString(RS_Entity *entity){
        if (entity == nullptr) return {};
        return std::to_string(entity->getId()) + "/" + std::to_string(entity->rtti());
    }

// Support fillet trimming for whole ellipses
    RS_AtomicEntity *trimEllipseForRound(RS_AtomicEntity *entity, const RS_Arc &arcFillet){
        if (entity == nullptr) {
            return entity;
        }
        if (entity->rtti() != RS2::EntityEllipse) {
            return entity;
        }
        const auto ellipse = dynamic_cast<RS_Ellipse *>(entity);
        if (ellipse->isEllipticArc()) {
            return entity;
        }
        RS_Vector tangent = entity->getNearestPointOnEntity(arcFillet.getCenter(), false);
        const RS_Line line{nullptr, {arcFillet.getCenter(), tangent}};
        const RS_Vector middle = arcFillet.getMiddlePoint();
        const RS_Vector opposite = arcFillet.getCenter() + (arcFillet.getCenter() - middle).normalized() * ellipse->getMinorRadius() * 0.01;
        const RS_Vector trimCoord = ellipse->getNearestPointOnEntity(opposite, false);
        const RS_VectorSolutions sol = RS_Information::getIntersection(entity, &line, false);
        ellipse->prepareTrim(trimCoord, sol);
        return entity;
    }

// A quick fix for rounding on circles
    RS_AtomicEntity *trimCircleForRound(RS_AtomicEntity *entity, const RS_Arc &arcFillet){
        if (entity == nullptr) {
            return entity;
        }
        if (entity->rtti() == RS2::EntityEllipse) {
            return trimEllipseForRound(entity, arcFillet);
        }
        if (entity->rtti() != RS2::EntityCircle) {
            return entity;
        }
        const RS_Line line{nullptr, {arcFillet.getCenter(), entity->getCenter()}};
        const RS_Vector middle = arcFillet.getMiddlePoint();
        // prefer acute angle for fillet
        // Use a trimCoord at the opposite side of the arc wrt to the
        const RS_Vector opposite = arcFillet.getCenter() + (arcFillet.getCenter() - middle).normalized() * entity->getRadius() * 0.01;
        const RS_Vector trimCoord = entity->getNearestPointOnEntity(opposite, true);
        const RS_VectorSolutions sol = RS_Information::getIntersection(entity, &line, false);
        RS_Arc *arc = trimCircle(dynamic_cast<RS_Circle *>(entity), trimCoord, sol);
        delete entity;
        return arc;
    }

    inline bool isOneOfPoints(const RS_Vector& candidate, const RS_Vector& point1, const RS_Vector& point2) {
       const bool result = point1.distanceTo(candidate) < RS_TOLERANCE || point2.distanceTo(candidate) < RS_TOLERANCE;
       return result;
    }

    /**
 * @brief getUniqueBlockName - Generates unique block name like "PASTE_0"
 * @param graphic - Target graphic
 * @param baseName - Base ("PASTE" default)
 * @return Unique name
 */
    QString getUniqueBlockName(RS_Graphic* graphic, const QString& baseName = QStringLiteral("PASTE")) {
        if (!graphic) return baseName;
        RS_BlockList* bl = graphic->getBlockList();
        if (!bl) return baseName;
        int i = 0;
        QString candidate;
        do {
            candidate = QString("%1_%2").arg(baseName).arg(i++);
        } while (bl->find(candidate) != nullptr);
        return candidate;
    }
} // namespace


RS_PasteData::RS_PasteData(RS_Vector _insertionPoint,
		double _factor,
		double _angle,
		bool _asInsert,
		const QString& _blockName):
		insertionPoint(_insertionPoint)
		,factor(_factor)
		,angle(_angle)
		,asInsert(_asInsert)
		,blockName(_blockName)
{
}

/**
 * Default constructor.
 *
 * @param container The container to which we will add
 *        entities. Usually that's an RS_Graphic entity but
 *        it can also be a polyline, text, ...
 * @param graphicView Pointer to graphic view or nullptr if you don't want the
 *        any views to be updated.
 * @param handleUndo true: Handle undo functionalitiy.
 */
RS_Modification::RS_Modification(RS_EntityContainer* container,
                                 LC_GraphicViewport* vp,
                                 bool handleUndo) {

    Q_ASSERT(container != nullptr && vp != nullptr);
    this->m_handleUndo = handleUndo;
    m_viewport = vp;
    graphic = container->getGraphic();
    m_document = container->getDocument();
}

void RS_Modification::revertDirection(QList<RS_Entity*>& originalEntities, LC_DocumentModificationBatch &ctx){
    for (const auto e : originalEntities) {
        RS_Entity* clone = e->clone();
        clone->revertDirection();
        ctx += clone;
    }
    ctx -= originalEntities;
    ctx.success = true;
}

/**
 * Changes the attributes of all selected
 */
bool RS_Modification::changeAttributesOld(RS_AttributesData& data, const bool keepSelected){
    return changeAttributesOld(data, m_document, keepSelected);
}

bool RS_Modification::changeAttributesOld(
    RS_AttributesData& data,
    RS_EntityContainer* cont,
    const bool keepSelected) {

    if (cont == nullptr) {
        return false;
    }
    QList<RS_Entity *> selectedEntities;
    m_document->collectSelected(selectedEntities, false);
    return changeAttributesOld(data, selectedEntities, keepSelected);
}

bool RS_Modification::changeAttributes(const QList<RS_Entity*>& originalEntities, RS_AttributesData& data, LC_DocumentModificationBatch& ctx) {
    QSet<RS_Block*> blocks;

    for (const auto en: originalEntities) {
        if  (en == nullptr) {
            continue;
        }
        auto rtti = en->rtti();
        if (data.applyBlockDeep && rtti == RS2::EntityInsert) {
            RS_Block *bl = dynamic_cast<RS_Insert *>(en)->getBlockForInsert();
            blocks << bl;
        }

        RS_Entity *clone = en->clone();
        RS_Pen pen = clone->getPen(false);
        if (data.changeLayer) {
            clone->setLayer(data.layer);
        }
        if (data.changeColor) {
            pen.setColor(data.pen.getColor());
        }
        if (data.changeLineType) {
            pen.setLineType(data.pen.getLineType());
        }
        if (data.changeWidth) {
            pen.setWidth(data.pen.getWidth());
        }
        clone->setPen(pen);
        clone->calculateBorders();

        ctx += clone;
        ctx -= en;

        if (RS2::isDimensionalEntity(rtti)) {
            clone->update();
        }
    }

     for (const auto block: blocks) {
         changeAttributes(block->getEntityList(), data, ctx);
    }
    return true;

}

bool RS_Modification::changeAttributesOld(RS_AttributesData& data, const QList<RS_Entity*> &entitiesList, bool keepSelected){
    const LC_UndoSection  undo(m_document, m_viewport);
    QList<RS_Entity*> clones;
    QSet<RS_Block*> blocks;

    const auto selection = m_document->getSelectedSet();
    selection->disableListeners();

    for (const auto en: entitiesList) {
        if (data.applyBlockDeep && en->rtti() == RS2::EntityInsert) {
            RS_Block *bl = dynamic_cast<RS_Insert *>(en)->getBlockForInsert();
            blocks << bl;
        }

        RS_Entity *clone = en->clone();
        RS_Pen pen = clone->getPen(false);
        if (data.changeLayer) {
            clone->setLayer(data.layer);
        }
        if (data.changeColor) {
            pen.setColor(data.pen.getColor());
        }
        if (data.changeLineType) {
            pen.setLineType(data.pen.getLineType());
        }
        if (data.changeWidth) {
            pen.setWidth(data.pen.getWidth());
        }
        clone->setPen(pen);
        clones << clone;

        undo.undoableDelete(en);
    }

    for (const auto block: blocks) {
        for (const auto en: *block) {
            if (en == nullptr) {
                continue;
            }
            en->setSelectionFlag(true);  // actual selection will be performed later
        }
        changeAttributesOld(data, block, keepSelected);
    }

    for (const auto clone: clones) {
        m_document->select(clone,keepSelected);
        const RS2::EntityType rtti = clone->rtti();
        if (RS2::isDimensionalEntity(rtti)) {
            clone->update();
        }
        undo.undoableAdd(clone);
    }

    if (graphic != nullptr) {
        graphic->updateInserts();
    }

    m_document->calculateBorders();

    selection->enableListeners();
    m_viewport->notifyChanged();
    return true;
}

/**
 * Copies all selected entities from the given container to the clipboard.
 * Layers and blocks that are needed are also copied if the container is
 * or is part of an RS_Graphic.
 *
 * @param container The entity container.
 * @param ref Reference point. The entities will be moved by -ref.
 * @param cut true: cut instead of copying, false: copy
 */
void RS_Modification::copy(const RS_Vector& ref, const bool cut) {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copy");

    RS_CLIPBOARD->clear();
    if (graphic) {
        RS_CLIPBOARD->getGraphic()->setUnit(graphic->getUnit());
    } else {
        RS_CLIPBOARD->getGraphic()->setUnit(RS2::None);
    }

    // start undo cycle for the container if we're cutting
    LC_UndoSection undo( m_document, m_viewport, cut && m_handleUndo);
    const auto selection = m_document->getSelectedSet();

    if (!selection->isEmpty()) {
        RS_Vector refPoint;
        if (ref.valid) {
            refPoint = ref;
        } else { // no ref-point set, determine center of selection
            RS_BoundData bound = getBoundingRect(selection);
            refPoint =  bound.getCenter();
        }
        for (const auto e : *selection) {
            doCopyEntity(e, refPoint);
            if (cut) {
                RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyEntity: cut ID/flag: %s", getIdFlagString(e).c_str());
                e->setFlag(RS2::FlagDeleted);
                m_document->addUndoable(e);
            }
        }
        RS_Selection::unselectAllInDocument(m_document, m_viewport);
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copy: OK");
    }
}

// fixme - selection - rework based on selected set!
 void RS_Modification::collectSelectedEntities(QList<RS_Entity *> &selected) const{
    for (auto e: *m_document) {
        if (e != nullptr  && e->isSelected()) {
            selected.push_back(e);
        }
    }
}

RS_BoundData RS_Modification::getBoundingRect(QList<RS_Entity *> &selected)  {
    RS_Vector min = RS_Vector(10e10, 10e10,0);
    RS_Vector max = RS_Vector(-10e10, -10e10,0);
    for (const auto e: selected) {
        const RS_Vector &entityMin = e->getMin();
        const RS_Vector &entityMax = e->getMax();

        min.x = std::min(min.x, entityMin.x);
        min.y = std::min(min.y, entityMin.y);
        max.x = std::max(max.x, entityMax.x);
        max.y = std::max(max.y, entityMax.y);
    }

    const RS_BoundData result(min, max);
    return result;
}

RS_BoundData RS_Modification::getBoundingRect(LC_SelectedSet* selected)  {
    RS_Vector min = RS_Vector(10e10, 10e10,0);
    RS_Vector max = RS_Vector(-10e10, -10e10,0);
    for (const auto e: *selected) {
        const RS_Vector &entityMin = e->getMin();
        const RS_Vector &entityMax = e->getMax();

        min.x = std::min(min.x, entityMin.x);
        min.y = std::min(min.y, entityMin.y);
        max.x = std::max(max.x, entityMax.x);
        max.y = std::max(max.y, entityMax.y);
    }

    const RS_BoundData result(min, max);
    return result;
}

/**
 * Copies the given entity from the given container to the clipboard.
 * Layers and blocks that are needed are also copied if the container is
 * or is part of an RS_Graphic.
 *
 * @param e The entity.
 * @param ref Reference point. The entities will be moved by -ref.
 */
void RS_Modification::doCopyEntity(RS_Entity* e, const RS_Vector& ref) {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyEntity");

    if (e == nullptr || !e->isSelected()) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyEntity: no entity is selected");
        return;
    }

    // Ensure the insert is updated before copying to populate the container with transformed entities
    if (e->rtti() == RS2::EntityInsert) {
        dynamic_cast<RS_Insert*>(e)->update();
    }

    // add entity to clipboard:
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyEntity: to clipboard: %s", getIdFlagString(e).c_str());
    RS_Entity* clone = e->clone();
    clone->move(-ref);

    // issue #1616: copy&paste a rotated block results in a double rotated block
    // At this point the copied block entities are already rotated, but at
    // pasting, RS_Insert::update() would still rotate the entities again and
    // cause double rotation.
    const bool isBlock = clone->rtti() == RS2::EntityInsert;
    const double angle = isBlock ? dynamic_cast<RS_Insert*>(clone)->getAngle() : 0.;
    // issue #1616: A quick fix: rotate back all block entities in the clipboard back by the
    // rotation angle before pasting
    if (isBlock && std::abs(std::remainder(angle, 2. * M_PI)) > RS_TOLERANCE_ANGLE){
        auto* insert = dynamic_cast<RS_Insert*>(clone);
        //insert->rotate(insert->getData().insertionPoint, - angle);
        insert->setAngle(0.);
    }

    RS_CLIPBOARD->addEntity(clone);
    doCopyLayers(e);
    doCopyBlocks(e);

    // set layer to the layer clone:
    const auto originalLayer = e->getLayer();
    // layer could be null if copy is performed in font file, where block is open. LibreCAD#2110
    if (originalLayer != nullptr) {
        clone->setLayer(e->getLayer()->getName());
    }
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyEntity: OK");
}

/**
 * Copies all layers of the given entity to the clipboard.
 */
void RS_Modification::doCopyLayers(RS_Entity* e) {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyLayers");

	if (e == nullptr) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyLayers: no entity is selected");
        return;
    }

    // add layer(s) of the entity insert can also be into any layer
    const RS_Layer* l = e->getLayer();
    if (!l) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyLayers: no valid layer found");
        return;
    }

    if (!RS_CLIPBOARD->hasLayer(l->getName())) {
        RS_CLIPBOARD->addLayer(l->clone());
    }

    // special handling of inserts:
    if (e->rtti()==RS2::EntityInsert) {
        // insert: add layer(s) of subentities:
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyLayers: copy insert entity ID/flag layers: %s", getIdFlagString(e).c_str());
        RS_Block* b = static_cast<RS_Insert*>(e)->getBlockForInsert();
        if (!b) {
            RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyLayers: could not find block for insert entity");
            return;
        }
        for(const auto e2: *b) {
            doCopyLayers(e2);
        }
    } else {
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyLayers: skip noninsert entity");
    }

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyLayers: OK");
}


/**
 * Copies all blocks of the given entity to the clipboard.
 */
void RS_Modification::doCopyBlocks(RS_Entity* e) {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks");

	if (e == nullptr) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyBlocks: no entity to process");
        return;
    }

    // add block of the entity only if it's an insert
    if (e->rtti()!=RS2::EntityInsert) {
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks: skip non-insert entity");
        return;
    }

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks: get insert entity ID/flag block: %s", getIdFlagString(e).c_str());
    RS_Block* b = ((RS_Insert*)e)->getBlockForInsert();
    if (!b) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::copyBlocks: could not find block for insert entity");
        return;
    }
    // add block of an insert
    const QString bn = b->getName();
    if (!RS_CLIPBOARD->hasBlock(bn)) {
        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks: add block name: %s", bn.toLatin1().data());
        RS_CLIPBOARD->addBlock((RS_Block*)b->clone());
    }
    //find insert into insert
    for(const auto e2: *b) {
        //call copyBlocks only if entity are insert
        if (e2->rtti()==RS2::EntityInsert) {
            RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks: process insert-into-insert blocks for %s", getIdFlagString(e).c_str());
            doCopyBlocks(e2);
        }
    }

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::copyBlocks: OK");
}

/**
 * Pastes all entities from the clipboard into the container.
 * Layers and blocks that are needed are also copied if the container is
 * or is part of an RS_Graphic.
 *
 * @param data Paste data.
 * @param source The source from where to paste. nullptr means the source
 *      is the clipboard.
 */
void RS_Modification::paste(const RS_PasteData& data, RS_Graphic* source) {
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::paste:");

    if (container == nullptr || container->isLocked() || !container->isVisible()) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::paste: invalid container");
        return;
    }

    RS_Graphic* src = (source != nullptr) ? source : RS_CLIPBOARD->getGraphic();
    if (src == nullptr) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::paste: no source");
        return;
    }

    // Scale (units)
    RS_Graphic* srcRef = source;
    RS_Vector scaleV = getPasteScale(data, srcRef, *graphic);
    src = srcRef;

    src->calculateBorders();
    RS_Vector center = (src->getMin() + src->getMax()) * 0.5;
    RS_Vector offset = data.insertionPoint - center;

    LC_UndoSection undo(document, viewport, handleUndo);

    if (data.asInsert) {
        // === BLOCK: Bake → angle=0 ===
        QString bname = data.blockName.isEmpty() ? getUniqueBlockName(graphic) : data.blockName;
        RS_Block* block = addNewBlock(bname, *graphic);

        auto entities = lc::LC_ContainerTraverser{*src, RS2::ResolveAll}.entities();
        for (RS_Entity* e : entities) {
            if (e == nullptr || e->isUndone()) continue;
            RS_Entity* clone = e->clone();
            // Bake: center→0 → scale/rot → block@0
            clone->move(-center);
            clone->scale(RS_Vector{}, scaleV);
            clone->rotate(RS_Vector{}, data.angle);
            block->addByBlockEntity(clone);  // **ByBlock** 👌
        }

        // Insert (baked, **angle=0**)
        RS_InsertData idata(bname, data.insertionPoint, {1., 1.}, 0.0, 1, 1, {});
        RS_Insert* insert = new RS_Insert(container, idata);
        insert->reparent(container);
        container->addEntity(insert);

        // Props (inherit)
        RS_Entity* first = src->firstEntity(RS2::ResolveNone);
        if (first) {
            insert->setLayer(first->getLayer());
            insert->setPen(first->getPen(true));
        }
        insert->setSelected(true);
        insert->update();

        undo.addUndoable(block);
        undo.addUndoable(insert);

        RS_DEBUG->print(RS_Debug::D_DEBUGGING, "paste: block '%s'", bname.toLatin1().data());

    } else {
        // === EXPLODED ===
        std::vector<RS_Entity*> newEnts;
        auto entities = lc::LC_ContainerTraverser{*src, RS2::ResolveAll}.entities();
        for (RS_Entity* e : entities) {
            if (e == nullptr || e->isUndone()) continue;
            RS_Entity* clone = e->clone();
            // **Symmetric**: scale/rot **around center** → move
            clone->scale(center, scaleV);
            clone->rotate(center, data.angle);
            clone->move(offset);
            clone->setSelected(true);
            newEnts.push_back(clone);
        }
        // Add (match explode/moveRef)
        for (RS_Entity* ne : newEnts) {
            ne->reparent(container);
            container->addEntity(ne);
            undo.addUndoable(ne);
        }
    }

    graphic->updateInserts();
    viewport->notifyChanged();
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "paste: OK ✅");
}

/**
 * Create layers in destination graphic corresponding to entity to be copied
 *
 **/
bool RS_Modification::pasteLayers(RS_Graphic* source) const {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteLayers");

    if (!source) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteLayers: no valid graphic found");
        return false;
    }

    RS_LayerList* lrs=source->getLayerList();
    for(const RS_Layer* layer: *lrs) {

        if(!layer) {
            RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::pasteLayers: nullptr layer in source");
            continue;
        }

        // add layers if absent
        QString ln = layer->getName();
        if (!graphic->findLayer(ln)) {
            graphic->addLayer(layer->clone());
            RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteLayers: layer added: %s", ln.toLatin1().data());
        }
    }

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteLayers: OK");
    return true;
}

/**
 * Create inserts and blocks in destination graphic corresponding to entity to be copied
 *
 **/
bool RS_Modification::pasteContainer(RS_Entity* entity, RS_EntityContainer* containerToPaste, QHash<QString, QString>blocksDict, RS_Vector insertionPoint) {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert");

    auto* insert = dynamic_cast<RS_Insert*>(entity);

    // get block for this insert object
    const RS_Block* insertBlock = insert->getBlockForInsert();
    if (insertBlock == nullptr) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: no block to process");
        return false;
    }
    // get name for this insert object
    const QString name_old = insertBlock->getName();
    QString name_new = name_old;
    if (name_old != insert->getName()) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: block and insert names don't coincide");
        return false;
    }
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: processing container: %s", name_old.toLatin1().data());
    // rename if needed
    if (graphic->findBlock(name_old)) {
        if (insertBlock->getParent() == graphic) {
            // If block is already in graphic, only paste a new insert
            pasteEntity(entity, m_document);
            return true;
        } else {
            name_new = graphic->getBlockList()->newName(name_old);
            RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: new block name: %s", name_new.toLatin1().data());
        }
    }
    blocksDict[name_old] = name_new;
    // make new block in the destination
    RS_Block* blockClone = addNewBlock(name_new, *graphic);
    // create insert for the new block
    const auto di = RS_InsertData(name_new, insertionPoint, RS_Vector(1.0, 1.0), 0.0, 1, 1, RS_Vector(0.0,0.0));
    auto* insertClone = new RS_Insert(containerToPaste, di);
    insertClone->reparent(containerToPaste);
    containerToPaste->addEntity(insertClone);

    // set the same layer in clone as in source
    const QString layerName = entity->getLayer()->getName();
    RS_Layer* layer = graphic->getLayerList()->find(layerName);
    if (!layer) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: unable to select layer to paste in");
        return false;
    }
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: selected layer: %s", layer->getName().toLatin1().data());
    insertClone->setLayer(layer);
    insertClone->setPen(entity->getPen(false));

    // get relative insertion point
    RS_Vector ip{0.0, 0.0};
    if (containerToPaste->getId() != graphic->getId()) {
        ip = blockClone->getBasePoint();
    }

    // copy content of block/insert to destination
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: copy content to the subcontainer");
    for(auto* e: *insert) {
        if(e == nullptr) {
            RS_DEBUG->print(RS_Debug::D_NOTICE, "RS_Modification::pasteInsert: nullptr entity in block");
            continue;
        }
        if (e->rtti() == RS2::EntityInsert) {
            RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: process sub-insert for %s", ((RS_Insert*)e)->getName().toLatin1().data());
            if (!pasteContainer(e, blockClone, blocksDict, ip)) {
                RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: unable to paste entity to sub-insert");
                return false;
            }
        } else {
            if (!pasteEntity(e, blockClone)) {
                RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: unable to paste entity");
                return false;
            }
        }
    }

    insertClone->update();
    insertClone->clearSelectionFlag();

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: OK");
    return true;
}

/**
 * Paste entity in supplied container
 *
 **/
bool RS_Modification::pasteEntity(RS_Entity* entity, RS_EntityContainer* containerToPaste) const {

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteEntity");
    if (!entity) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteEntity: no entity to process");
        return false;
    }
    // create entity copy to paste
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteEntity ID/flag: %s", getIdFlagString(entity).c_str());


    // set the same layer in clone as in source
    const QString ln = entity->getLayer()->getName();
    RS_Layer* layer = graphic->getLayerList()->find(ln); // fixme - perf- layer search is not needed if copy paste within the same document
    if (!layer) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Modification::pasteInsert: unable to select layer to paste in");
        return false;
    }
    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteInsert: selected layer: %s", layer->getName().toLatin1().data());

    RS_Entity* clone = entity->clone();
    clone->setLayer(layer);
    clone->setPen(entity->getPen(false));

    // scaling entity doesn't needed as it scaled with insert object
    // paste entity
    clone->reparent(containerToPaste);
    clone->clearSelectionFlag();
    containerToPaste->addEntity(clone);

    RS_DEBUG->print(RS_Debug::D_DEBUGGING, "RS_Modification::pasteEntity: OK");
    return true;
}

/**
 * Splits a polyline into two leaving out a gap.
 *
 * @param polyline The original polyline
 * @param e1 1st entity on which the first cutting point is.
 * @param v1 1st cutting point.
 * @param e2 2nd entity on which the first cutting point is.
 * @param v2 2nd cutting point.
 * @param polyline1 Pointer to a polyline pointer which will hold the
 *        1st resulting new polyline. Pass nullptr if you don't
 *        need those pointers.
 * @param polyline2 Pointer to a polyline pointer which will hold the
 *        2nd resulting new polyline. Pass nullptr if you don't
 *        need those pointers.
 *
 * @todo Support arcs in polylines, check for wrong parameters
 *
 * @return true
 */
bool RS_Modification::splitPolyline(RS_Polyline* polyline,
                                    RS_Entity& e1, RS_Vector v1,
                                    RS_Entity& e2, RS_Vector v2,
                                    RS_Polyline** polyline1,
                                    RS_Polyline** polyline2) const {

    RS_Entity* firstEntity = polyline->firstEntity();
    RS_Vector firstPoint(false);
    if (firstEntity->rtti()==RS2::EntityLine) {
        firstPoint = static_cast<RS_Line*>(firstEntity)->getStartpoint();
    }
    auto* pl1 = new RS_Polyline(m_document,
                        RS_PolylineData(firstPoint, RS_Vector(0.0,0.0), 0));
    auto* pl2 = new RS_Polyline(m_document);
    RS_Polyline* pl = pl1;	// Current polyline
	const RS_Line* line = nullptr;
	[[maybe_unused]] const RS_Arc* arc = nullptr;

    if (polyline1) {
        *polyline1 = pl1;
    }
    if (polyline2) {
        *polyline2 = pl2;
    }

    for(const auto e: *polyline){
        if (e->rtti()==RS2::EntityLine) {
            line = static_cast<RS_Line*>(e);
            arc  = nullptr;
        } else if (e->rtti()==RS2::EntityArc) {
            arc  = static_cast<RS_Arc*>(e);
            line = nullptr;
        } else {
            line = nullptr;
            arc = nullptr;
        }

        if (line != nullptr /*|| arc*/) {

            if (e==&e1 && e==&e2) {
                // Trim within a single entity:
                RS_Vector sp = line->getStartpoint();
                const double dist1 = (v1-sp).magnitude();
                const double dist2 = (v2-sp).magnitude();
                pl->addVertex(dist1<dist2 ? v1 : v2, 0.0);
                pl = pl2;
                pl->setStartpoint(dist1<dist2 ? v2 : v1);
                pl->addVertex(line->getEndpoint(), 0.0);
            } else if (e==&e1 || e==&e2) {
                // Trim entities:
                RS_Vector v = (e==&e1 ? v1 : v2);
                if (pl==pl1) {
                    // Trim endpoint of entity to first vector
                    pl->addVertex(v, 0.0);
                } else {
                    // Trim startpoint of entity to second vector
                    pl = pl2;
                    pl->setStartpoint(v);
                    pl->addVertex(line->getEndpoint(), 0.0);
                }
            } else {
                // Add entities to polylines
                if (line && pl) {
                    pl->addVertex(line->getEndpoint(), 0.0);
                }
            }
        }
    }


    if (m_handleUndo) {
        const LC_UndoSection undo(m_document,m_viewport);
        undo.undoableAdd(pl1);
        undo.undoableAdd(pl2);
        undo.undoableDelete(polyline);
    }
    else {
        m_document->addEntity(pl1);
        m_document->addEntity(pl2);
    }

    return true;
}

void RS_Modification::selectClone(RS_Entity* original, RS_Entity* clone) const {
    const bool select = original->isSelected();
    if (select) {
        m_document->select(clone,select);
    }
    else {
        clone->clearSelectionFlag();
    }
}

/**
 * Adds a node to the given polyline. The new node is placed between
 * the start and end point of the given segment.
 *
 * @param node The position of the new node.
 *
 * @return Pointer to the new polyline or nullptr.
 */
RS_Polyline* RS_Modification::addPolylineNode(RS_Polyline* polyline,
        const RS_AtomicEntity& segment,
        const RS_Vector& node) const {
    RS_DEBUG->print("RS_Modification::addPolylineNode");
    if (segment.getParent()!=polyline) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::addPolylineNode: segment not part of the polyline");
		return nullptr;
    }

    const auto newPolyline = new RS_Polyline(m_document);
    newPolyline->setClosed(polyline->isClosed());
    selectClone(polyline, newPolyline);
    newPolyline->setLayer(polyline->getLayer());
    newPolyline->setPen(polyline->getPen(false));

    // copy polyline and add new node:
    bool first = true;
    const RS_Entity* lastEntity = polyline->lastEntity();
    for(const auto e: *polyline){
        if (e->isAtomic()) {
            const auto ae = (RS_AtomicEntity*)e;
            double bulge = 0.0;
            if (ae->rtti()==RS2::EntityArc) {
                RS_DEBUG->print("RS_Modification::addPolylineNode: arc segment");
                bulge = ((RS_Arc*)ae)->getBulge();
            } else {
                RS_DEBUG->print("RS_Modification::addPolylineNode: line segment");
                bulge = 0.0;
            }

            if (first) {
                RS_DEBUG->print("RS_Modification::addPolylineNode: first segment: %f/%f",
                                ae->getStartpoint().x, ae->getStartpoint().y);

                newPolyline->setNextBulge(bulge);
                newPolyline->addVertex(ae->getStartpoint());
                first = false;
            }

            // segment to split:
            if (ae==&segment) {
                RS_DEBUG->print("RS_Modification::addPolylineNode: split segment found");

                RS_DEBUG->print("RS_Modification::addPolylineNode: node: %f/%f",
                                node.x, node.y);

                newPolyline->setNextBulge(0.0);
                newPolyline->addVertex(node);

                RS_DEBUG->print("RS_Modification::addPolylineNode: after node: %f/%f",
                                ae->getEndpoint().x, ae->getEndpoint().y);

                if (ae!=lastEntity || !polyline->isClosed()) {
                    newPolyline->setNextBulge(0.0);
                    newPolyline->addVertex(ae->getEndpoint());
                }
            } else {
                RS_DEBUG->print("RS_Modification::addPolylineNode: normal vertex found: %f/%f",
                                ae->getEndpoint().x, ae->getEndpoint().y);

                if (ae!=lastEntity || !polyline->isClosed()) {
                    newPolyline->setNextBulge(bulge);
                    newPolyline->addVertex(ae->getEndpoint());
                }
            }
        } else {
            RS_DEBUG->print(RS_Debug::D_WARNING,
                            "RS_Modification::addPolylineNode: Polyline contains non-atomic entities");
        }
    }

    newPolyline->setNextBulge(polyline->getClosingBulge());
    newPolyline->endPolyline();

    // add new polyline:
    if (m_handleUndo) {
        const LC_UndoSection undo(m_document,m_viewport);
        undo.undoableReplace(polyline, newPolyline);
    }
    else {
        m_document->addEntity(newPolyline);
    }

    m_viewport->notifyChanged();
    return newPolyline;
}

/**
 * Deletes a node from a polyline.
 *
 * @param node The node to delete.
 *
 * @return Pointer to the new polyline or nullptr.
 */
RS_Polyline* RS_Modification::deletePolylineNode(RS_Polyline* polyline,
        const RS_Vector& node, bool createOnly) const {

    RS_DEBUG->print("RS_Modification::deletePolylineNode");
    if (!node.valid){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::deletePolylineNode: node not valid");
        return nullptr;
    }

    // check if the polyline is no longer there after deleting the node:
    if (polyline->count() == 1){
        RS_Entity *e = polyline->firstEntity();
        if (e != nullptr && e->isAtomic()){
            const auto atomic = dynamic_cast<RS_AtomicEntity *>(e);
            if (node.distanceTo(atomic->getStartpoint()) < 1.0e-6 ||
                node.distanceTo(atomic->getEndpoint()) < 1.0e-6){

                if (m_handleUndo){
                    const LC_UndoSection undo(m_document,m_viewport);
                    undo.undoableDelete(polyline);
                }
            }
        }
        return nullptr;
    }

    auto* newPolyline = new RS_Polyline(m_document);
    newPolyline->setClosed(polyline->isClosed());
    if (!createOnly){
        selectClone(polyline, newPolyline);
        newPolyline->setLayer(polyline->getLayer());
        newPolyline->setPen(polyline->getPen(false));
    }

    // copy polyline and drop deleted node:
    bool first = true;
    bool lastDropped = false;
    const RS_Entity* lastEntity = polyline->lastEntity();
    for (const auto e: *polyline) {
        if (e->isAtomic()){
            const auto ae = dynamic_cast<RS_AtomicEntity *>(e);
            double bulge = 0.0;
            if (ae->rtti() == RS2::EntityArc){
                RS_DEBUG->print("RS_Modification::deletePolylineNode: arc segment");
                bulge = static_cast<RS_Arc*>(ae)->getBulge();
            } else {
                RS_DEBUG->print("RS_Modification::deletePolylineNode: line segment");
                bulge = 0.0;
            }

            // last entity is closing entity and will be added below with endPolyline()
            if (e == lastEntity && polyline->isClosed()){
                continue;
            }

            // first vertex (startpoint)
            if (first && node.distanceTo(ae->getStartpoint()) > 1.0e-6){
                RS_DEBUG->print("RS_Modification::deletePolylineNode: first node: %f/%f",
                                ae->getStartpoint().x, ae->getStartpoint().y);

                newPolyline->setNextBulge(bulge);
                newPolyline->addVertex(ae->getStartpoint());
                first = false;
            }

            // normal node (not deleted):
            if (first == false && node.distanceTo(ae->getEndpoint()) > 1.0e-6){
                RS_DEBUG->print("RS_Modification::deletePolylineNode: normal vertex found: %f/%f",
                                ae->getEndpoint().x, ae->getEndpoint().y);
                if (lastDropped){
                    //bulge = 0.0;
                }
                newPolyline->setNextBulge(bulge);
                newPolyline->addVertex(ae->getEndpoint());
                lastDropped = false;
            }
            // drop deleted node:
            else {
                RS_DEBUG->print("RS_Modification::deletePolylineNode: deleting vertex: %f/%f",
                                ae->getEndpoint().x, ae->getEndpoint().y);
                lastDropped = true;
            }
        } else {
            RS_DEBUG->print(RS_Debug::D_WARNING,
                            "RS_Modification::deletePolylineNode: Polyline contains non-atomic entities");
        }
    }

    RS_DEBUG->print("RS_Modification::deletePolylineNode: ending polyline");
    newPolyline->setNextBulge(polyline->getClosingBulge());
    newPolyline->endPolyline();

    // add new polyline:
    RS_DEBUG->print("RS_Modification::deletePolylineNode: adding new polyline");

    if (createOnly){
        m_document->addEntity(newPolyline);
    }
    else {
        RS_DEBUG->print("RS_Modification::deletePolylineNode: handling undo");
        if (m_handleUndo){
            const LC_UndoSection undo(m_document, m_viewport);
            undo.undoableReplace(polyline, newPolyline);
        }
    }
    m_viewport->notifyChanged();
    return newPolyline;
}

/**
 * Deletes all nodes between the two given nodes (exclusive).
 *
 * @param node1 First limiting node.
 * @param node2 Second limiting node.
 *
 * @return Pointer to the new polyline or nullptr.
 */
RS_Polyline *RS_Modification::deletePolylineNodesBetween(
    RS_Polyline* polyline,
    const RS_Vector &node1, const RS_Vector &node2) const {

    RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween");

    if (node1.valid == false || node2.valid == false){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::deletePolylineNodesBetween: node not valid");
        return nullptr;
    }

    if (node1.distanceTo(node2) < 1.0e-6){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::deletePolylineNodesBetween: nodes are identical");
        return nullptr;
    }

    // check if there's nothing to delete:
    for (const auto e: *polyline) {

        if (e->isAtomic()){
            const auto *atomic = dynamic_cast<RS_AtomicEntity *>(e);
            /// FIXME- RS_TOLERANCE?
            if ((node1.distanceTo(atomic->getStartpoint()) < RS_TOLERANCE  &&
                 node2.distanceTo(atomic->getEndpoint()) < 1.0e-6) ||
                (node2.distanceTo(atomic->getStartpoint()) < 1.0e-6 &&
                 node1.distanceTo(atomic->getEndpoint()) < 1.0e-6)){

                RS_DEBUG->print(RS_Debug::D_WARNING,
                                "RS_Modification::deletePolylineNodesBetween: nothing to delete");
                return nullptr;
            }
        }
    }

    // check if the start point is involved:
    const RS_Vector &polylineStartpoint = polyline->getStartpoint();
    const bool startpointInvolved = isOneOfPoints(polylineStartpoint, node1, node2);

    // check which part of the polyline has to be deleted:
    bool deleteStart = false;
    if (polyline->isClosed()){
        bool found = false;
        double length1 = 0.0;
        double length2 = 0.0;
        RS_Entity *e = polyline->firstEntity();

        if (startpointInvolved){
            if (e->isAtomic()){
                const auto *atomic = dynamic_cast<RS_AtomicEntity *>(e);
                length1 += atomic->getLength();
            }
            e = polyline->nextEntity();
        }
        for (; e; e = polyline->nextEntity()) {

            if (e->isAtomic()){
                const auto *atomic = dynamic_cast<RS_AtomicEntity *>(e);

                if (isOneOfPoints(atomic->getStartpoint(), node1, node2)){
                    found = !found;
                }

                if (found){
                    length2 += atomic->getLength();
                } else {
                    length1 += atomic->getLength();
                }
            }
        }
        if (length1 < length2){
            deleteStart = true;
        } else {
            deleteStart = false;
        }
    }

    auto *newPolyline = new RS_Polyline(m_document);
    newPolyline->setClosed(polyline->isClosed());
    if (startpointInvolved && deleteStart && polyline->isClosed()){
        newPolyline->setNextBulge(0.0);
        newPolyline->addVertex(polylineStartpoint);
    }

    // copy polyline and drop deleted nodes:
    bool first = true;
    bool removing = deleteStart;
    bool done = false;
    bool nextIsStraight = false;
    const RS_Entity *lastEntity = polyline->lastEntity();
    int i = 0;
    double bulge = 0.0;

    for (const auto e: *polyline) {
        RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: entity: %d", i++);
        RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: removing: %d", (int) removing);

        if (e->isAtomic()){
            const auto atomic = dynamic_cast<RS_AtomicEntity *>(e);
            if (atomic->rtti() == RS2::EntityArc){
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: arc segment");
                const auto arc = dynamic_cast<RS_Arc *>(atomic);
                bulge = arc->getBulge();
            } else {
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: line segment");
                bulge = 0.0;
            }

            const RS_Vector &endpoint = atomic->getEndpoint();
            const RS_Vector &startpoint = atomic->getStartpoint();

            // last entity is closing entity and will be added below with endPolyline()
            if (e == lastEntity && polyline->isClosed()){
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: dropping last vertex of closed polyline");
                continue;
            }

            // first vertex (startpoint)
            if (first){
                if (!removing){
                    RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: first node: %f/%f", startpoint.x, startpoint.y);
                    newPolyline->setNextBulge(bulge);
                    newPolyline->addVertex(startpoint);
                    first = false;
                }
            }

            // stop removing nodes:
            if (removing == true && isOneOfPoints(endpoint, node1, node2)){
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: stop removing at: %f/%f",endpoint.x, endpoint.y);
                removing = false;
                done = true;
                if (first == false){
                    nextIsStraight = true;
                }
            }

            // normal node (not deleted):
            if (removing == false && (done == false || deleteStart == false)){
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: normal vertex shouldRemove: %f/%f", endpoint.x, endpoint.y);
                if (nextIsStraight){
                    bulge = 0.0;
                    nextIsStraight = false;
                }
                newPolyline->setNextBulge(bulge);
                newPolyline->addVertex(endpoint);
            }
                // drop deleted node:
            else {
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: deleting vertex: %f/%f", endpoint.x, endpoint.y);
            }

            // start to remove nodes from now on:
            if (done == false && removing == false && isOneOfPoints(endpoint, node1, node2)){
                RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: start removing at: %f/%f", endpoint.x, endpoint.y);
                removing = true;
            }

            if (done){
                done = false;
            }
        } else {
            RS_DEBUG->print(RS_Debug::D_WARNING,"RS_Modification::deletePolylineNodesBetween: Polyline contains non-atomic entities");
        }
    }

    RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: ending polyline");
    newPolyline->setNextBulge(polyline->getClosingBulge());
    newPolyline->endPolyline();

    // add new polyline:
    RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: adding new polyline");
    if (m_handleUndo) {
        RS_DEBUG->print("RS_Modification::deletePolylineNodesBetween: handling undo");
        selectClone(polyline, newPolyline);
        newPolyline->setLayer(polyline->getLayer());
        newPolyline->setPen(polyline->getPen(false));

        const LC_UndoSection undo(m_document, m_viewport);
        undo.undoableReplace(polyline,newPolyline);
    }
    else {
        m_document->addEntity(newPolyline);
    }


    m_viewport->notifyChanged();
    return newPolyline;
}

/**
 * Trims two segments of a polyline all nodes between the two trim segments
 * are removed.
 *
 * @param polyline The polyline entity.
 * @param segment1 First segment to trim.
 * @param segment2 Second segment to trim.
 *
 * @return Pointer to the new polyline or nullptr.
 */
RS_Polyline *RS_Modification::polylineTrim(
    RS_Polyline *polyline,
    RS_AtomicEntity &segment1,
    RS_AtomicEntity &segment2,
    bool createOnly) const {

    RS_DEBUG->print("RS_Modification::polylineTrim");

    if (segment1.getParent() != polyline || segment2.getParent() != polyline){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::polylineTrim: segments not in polyline");
        return nullptr;
    }

    if (&segment1 == &segment2){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::polylineTrim: segments are identical");
        return nullptr;
    }

    RS_VectorSolutions sol;
    sol = RS_Information::getIntersection(&segment1, &segment2, false);

    if (sol.getNumber() == 0){
        RS_DEBUG->print(RS_Debug::D_WARNING,
                        "RS_Modification::polylineTrim: segments cannot be trimmed");
        return nullptr;
    }

    // check which segment comes first in the polyline:
    RS_AtomicEntity *firstSegment;
    if (polyline->findEntity(&segment1) > polyline->findEntity(&segment2)){
        firstSegment = &segment2;
    } else {
        firstSegment = &segment1;
    }

    // find out if we need to trim towards the open part of the polyline
    bool reverseTrim;
    reverseTrim = !RS_Math::isSameDirection(firstSegment->getDirection1(),
                                            firstSegment->getStartpoint().angleTo(sol.get(0)), M_PI_2);
    //reverseTrim = reverseTrim || !RS_Math::isSameDirection(segment2.getDirection1(),
//	segment2.getStartpoint().angleTo(sol.get(0)), M_PI_2);

    auto *newPolyline = new RS_Polyline(m_document);
    newPolyline->setClosed(polyline->isClosed());
    if (!createOnly){
        selectClone(polyline, newPolyline);
        newPolyline->setLayer(polyline->getLayer());
        newPolyline->setPen(polyline->getPen(false));
    }

    // normal trimming: start removing nodes at trim segment. ends stay the same
    if (!reverseTrim){
        // copy polyline, trim segments and drop between nodes:
        bool first = true;
        bool removing = false;
        bool nextIsStraight = false;
        const RS_Entity *lastEntity = polyline->lastEntity();
        for (const auto e: *polyline) {

            if (e->isAtomic()){
                const auto ae = dynamic_cast<RS_AtomicEntity *>(e);
                double bulge = 0.0;
                if (ae->rtti() == RS2::EntityArc){
                    RS_DEBUG->print("RS_Modification::polylineTrim: arc segment");
                    bulge = static_cast<RS_Arc*>(ae)->getBulge();
                } else {
                    RS_DEBUG->print("RS_Modification::polylineTrim: line segment");
                    bulge = 0.0;
                }

                // last entity is closing entity and will be added below with endPolyline()
                if (e == lastEntity && polyline->isClosed()){
                    RS_DEBUG->print("RS_Modification::polylineTrim: "
                                    "dropping last vertex of closed polyline");
                    continue;
                }

                // first vertex (startpoint)
                if (first){
                    RS_DEBUG->print("RS_Modification::polylineTrim: first node: %f/%f",
                                    ae->getStartpoint().x, ae->getStartpoint().y);

                    newPolyline->setNextBulge(bulge);
                    newPolyline->addVertex(ae->getStartpoint());
                    first = false;
                }

                // trim and start removing nodes:
                if (!removing && (ae == &segment1 || ae == &segment2)){
                    RS_DEBUG->print("RS_Modification::polylineTrim: "
                                    "start removing at trim point %f/%f",
                                    sol.get(0).x, sol.get(0).y);
                    newPolyline->setNextBulge(0.0);
                    newPolyline->addVertex(sol.get(0));
                    removing = true;
                    nextIsStraight = true;
                }

                    // stop removing nodes:
                else if (removing && (ae == &segment1 || ae == &segment2)){
                    RS_DEBUG->print("RS_Modification::polylineTrim: stop removing at: %f/%f",
                                    ae->getEndpoint().x, ae->getEndpoint().y);
                    removing = false;
                }

                // normal node (not deleted):
                if (!removing){
                    RS_DEBUG->print("RS_Modification::polylineTrim: normal vertex found: %f/%f",
                                    ae->getEndpoint().x, ae->getEndpoint().y);
                    if (nextIsStraight){
                        newPolyline->setNextBulge(0.0);
                        nextIsStraight = false;
                    } else {
                        newPolyline->setNextBulge(bulge);
                    }
                    newPolyline->addVertex(ae->getEndpoint());
                }
            } else {
                RS_DEBUG->print(RS_Debug::D_WARNING,
                                "RS_Modification::polylineTrim: Polyline contains non-atomic entities");
            }
        }
    }

        // reverse trimming: remove nodes at the ends and keep those in between
    else {
        // copy polyline, trim segments and drop between nodes:
        //bool first = true;
        bool removing = true;
        bool nextIsStraight = false;
        const RS_Entity *lastEntity = polyline->lastEntity();
        for (const auto e: *polyline) {

            if (e->isAtomic()){
                auto *ae = dynamic_cast<RS_AtomicEntity *>(e);
                double bulge = 0.0;
                if (ae->rtti() == RS2::EntityArc){
                    RS_DEBUG->print("RS_Modification::polylineTrim: arc segment");
                    const auto arc = dynamic_cast<RS_Arc *>(ae);
                    bulge = arc ->getBulge();
                } else {
                    RS_DEBUG->print("RS_Modification::polylineTrim: line segment");
                    bulge = 0.0;
                }

                // last entity is closing entity and will be added below with endPolyline()
                if (e == lastEntity && polyline->isClosed()){
                    RS_DEBUG->print("RS_Modification::polylineTrim: dropping last vertex of closed polyline");
                    continue;
                }

                // trim and stop removing nodes:
                if (removing == true && (ae == &segment1 || ae == &segment2)){
                    RS_DEBUG->print("RS_Modification::polylineTrim: "
                                    "stop removing at trim point %f/%f",
                                    sol.get(0).x, sol.get(0).y);
                    newPolyline->setNextBulge(0.0);
                    // start of new polyline:
                    newPolyline->addVertex(sol.get(0));
                    removing = false;
                    nextIsStraight = true;
                }

                    // start removing nodes again:
                else if (removing == false && (ae == &segment1 || ae == &segment2)){
                    RS_DEBUG->print("RS_Modification::polylineTrim: start removing at: %f/%f",
                                    ae->getEndpoint().x, ae->getEndpoint().y);
                    newPolyline->setNextBulge(0.0);
                    // start of new polyline:
                    newPolyline->addVertex(sol.get(0));
                    removing = true;
                }

                // normal node (not deleted):
                if (removing == false){
                    RS_DEBUG->print("RS_Modification::polylineTrim: normal vertex found: %f/%f",
                                    ae->getEndpoint().x, ae->getEndpoint().y);
                    if (nextIsStraight){
                        newPolyline->setNextBulge(0.0);
                        nextIsStraight = false;
                    } else {
                        newPolyline->setNextBulge(bulge);
                    }
                    newPolyline->addVertex(ae->getEndpoint());
                }
            } else {
                RS_DEBUG->print(RS_Debug::D_WARNING,
                                "RS_Modification::polylineTrim: Polyline contains non-atomic entities");
            }
        }
    }

    RS_DEBUG->print("RS_Modification::polylineTrim: ending polyline");
    newPolyline->setNextBulge(polyline->getClosingBulge());
    newPolyline->endPolyline();

    // add new polyline:
    RS_DEBUG->print("RS_Modification::polylineTrim: adding new polyline");

    if (createOnly){
        m_document->addEntity(newPolyline);
    }
    else {
        RS_DEBUG->print("RS_Modification::polylineTrim: handling undo");
        if (m_handleUndo){
            const LC_UndoSection undo(m_document, m_viewport);
            undo.undoableReplace(polyline, newPolyline);
        }
    }

    m_viewport->notifyChanged();
    return newPolyline;
}

/**
 * Moves all selected entities with the given data for the move
 * modification.
 */

void RS_Modification::addClonesAndDeleteOriginals(const LC_ModifyOperationFlags& data, const QList<RS_Entity*>& entitiesList,
        bool forPreviewOnly, bool keepSelected, QList<RS_Entity*> clonesList) const {
    setupModifiedClones(clonesList, data, forPreviewOnly, keepSelected);
    deleteOriginalAndAddNewEntities(clonesList, entitiesList, forPreviewOnly, !data.keepOriginals);
    clonesList.clear();
}

void RS_Modification::move(RS_MoveData& data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx) {
    const int numberOfCopies = data.obtainNumberOfCopies();
    QList<RS_Entity*> clonesList;

    for(const auto e: entitiesList){
        // Create new entities
        for (int num = 1; num <= numberOfCopies; num++) {
            RS_Entity* clone = getClone(forPreviewOnly, e);
            clone->move(data.offset*num);
            ctx += clone;
        }
    }

    if (!data.keepOriginals) {
        ctx -= entitiesList;
    }
    ctx.success = true;
}

RS_Entity *RS_Modification::getClone(bool forPreviewOnly, const RS_Entity *e){
    RS_Entity* result = nullptr;
    if (forPreviewOnly){
        const int rtti = e->rtti();
        switch (rtti){
            case RS2::EntityText:
            case RS2::EntityMText:{
                // fixme - sand - ucs - BAD dependency, rework.
                const bool  drawTextAsDraftInPreview = LC_GET_ONE_BOOL("Render","DrawTextsAsDraftInPreview", true);
                if (drawTextAsDraftInPreview) {
                    result = e->cloneProxy();
                }
                else{
                    result = e->clone();
                }
                break;
            }
            case RS2::EntityImage:{
                result = e->cloneProxy();
                break;
            }
            default:
                result = e->clone();
        }
    }
    else{
        result = e->clone();
    }

    return result;
}

void RS_Modification::setupModifiedClones(QList<RS_Entity *> &clonesList,
        const LC_ModifyOperationFlags &data, bool forPreviewOnly, bool keepSelected) const {

    if (!forPreviewOnly && (data.useCurrentLayer || data.useCurrentAttributes)){
        RS_Layer* layer = nullptr;
        RS_Pen pen;
        if (data.useCurrentLayer) {
            layer = m_viewport->getGraphic()->getActiveLayer();
        }
        if (data.useCurrentAttributes) {
            pen = m_document->getActivePen();
        }
        for (const auto clone: clonesList){
            if (data.useCurrentLayer) {
                clone->setLayer(layer);
            }
            if (data.useCurrentAttributes) {
                clone->setPen(pen);
            }
        }
    }

    for (const auto clone: clonesList){
        if (clone->rtti()==RS2::EntityInsert) {
            static_cast<RS_Insert*>(clone)->update();
        }
        // since 2.0.4.0: keep selection
        clone->setSelectionFlag(keepSelected);
    }
}

bool RS_Modification::alignRef(LC_AlignRefData & data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, bool keepSelected) {
    const int numberOfCopies = 1; /*data.obtainNumberOfCopies();*/
    QList<RS_Entity*> clonesList;
    const RS_Vector offset = data.offset;
    // too slow:
    for(const auto e: entitiesList){
        // Create new entities
        for (int num = 1; num <= numberOfCopies; num++) {
            RS_Entity* clone = getClone(forPreviewOnly, e);
            clone->rotate(data.rotationCenter, data.rotationAngle);
            if (data.scale && LC_LineMath::isMeaningful(data.scaleFactor - 1.0)){
                clone->scale(data.rotationCenter, data.scaleFactor);
            }
            clone->move(offset*num);
            clonesList.push_back(clone);
        }
    }
    addClonesAndDeleteOriginals(data, entitiesList, forPreviewOnly, keepSelected, clonesList);
    return true;
}

/**
 * Offset all selected entities with the given mouse position and distance
 *
 *@Author: Dongxu Li
 */
bool RS_Modification::offset(const RS_OffsetData& data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, bool keepSelected) {
    QList<RS_Entity*> clonesList;

    const int numberOfCopies = data.obtainNumberOfCopies();
    // Create new entities
    // too slow:
    for(const auto e: entitiesList){
        for (int num=1; num<= numberOfCopies; num++) {
            auto ec = e->clone();
            //highlight is used by trim actions. do not carry over flag
            ec->setHighlighted(false);

            if (!ec->offset(data.coord, num*data.distance)) {
                delete ec;
                continue;
            }

            clonesList.push_back(ec);
        }
    }
    addClonesAndDeleteOriginals(data, entitiesList, forPreviewOnly, keepSelected, clonesList);
    return true;
}

/**
 * Rotates all selected entities with the given data for the rotation.
 */
bool RS_Modification::rotate(RS_RotateData& data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx) {
        // Create new entities
    const int numberOfCopies = data.obtainNumberOfCopies();
    for (const auto e: entitiesList) {
        for (int num = 1; num <= numberOfCopies; num++) {
            RS_Entity* clone = getClone(forPreviewOnly, e);

            const double rotationAngle = data.angle * num;
            clone->rotate(data.center, rotationAngle);

            bool rotateTwice = data.twoRotations;
            const double distance = data.refPoint.distanceTo(data.center);
            if (distance < RS_TOLERANCE){
                rotateTwice = false;
            }

            if (rotateTwice) {
                RS_Vector rotatedRefPoint = data.refPoint;
                rotatedRefPoint.rotate(data.center, rotationAngle);

                double secondRotationAngle = data.secondAngle;
                if (data.secondAngleIsAbsolute){
                    secondRotationAngle -= rotationAngle;
                }
                clone->rotate(rotatedRefPoint, secondRotationAngle);
            }
            ctx += clone;
        }
    }
    if (!data.keepOriginals) {
        ctx -= entitiesList;
    }
    return true;
}


/**
 * Moves all selected entities with the given data for the scale
 * modification.
 */
bool RS_Modification::scale(RS_ScaleData& data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx) {
    QList<RS_Entity*> entitiesToScale;
    QList<RS_Entity*> createdEntities;

    for(auto e: entitiesList){
        RS_Entity* entityToScale = e;
        if (e->rtti() == RS2::EntityImage) {
            entityToScale = getClone(forPreviewOnly, e);
            createdEntities.append(entityToScale);
        }
        else if ( data.isotropicScaling ) {
            // do nothing, scale original entity
        }
        else {
            const RS2::EntityType rtti = e->rtti();
            if (rtti == RS2::EntityCircle ) {
                //non-isotropic scaling, replacing selected circles with ellipses
                const auto* c =dynamic_cast<RS_Circle*>(e);
                entityToScale = new RS_Ellipse{nullptr,
                                               {c->getCenter(), {c->getRadius(),0.},
                                                1.,
                                                0., 0., false}};
                createdEntities.append(entityToScale);
            } else if (rtti == RS2::EntityArc ) {
                //non-isotropic scaling, replacing selected arcs with ellipses
                const auto *c =dynamic_cast<RS_Arc*>(e);
                entityToScale = new RS_Ellipse{nullptr,
                                               {c->getCenter(),
                                                {c->getRadius(),0.},
                                                1.0,
                                                c->getAngle1(),
                                                c->getAngle2(),
                                                c->isReversed()}};
                createdEntities.append(entityToScale);
            }
            // todo - non isotropicScaling - blocks? splines? images?  Should they be also affected?
        }
        entitiesToScale.push_back(entityToScale);
    }

    const int numberOfCopies = data.obtainNumberOfCopies();

    // Create new entities
    for(const RS_Entity* e: entitiesToScale) {
        if (e != nullptr) {
            for (int num= 1; num <= numberOfCopies; num++) {
                RS_Entity* clone = getClone(forPreviewOnly, e);
                clone->scale(data.referencePoint, RS_Math::pow(data.factor, num));
                ctx += clone;
            }
        }
    }

    // delete temporary entities created for scale
    for (const auto e:createdEntities) {
        delete e;
    }

    if (!data.keepOriginals){
       ctx -= entitiesList;
    }

    return true;
}

/**
 * Mirror all selected entities with the given data for the mirror
 * modification.
 */

bool RS_Modification::mirror(RS_MirrorData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx) {
    //    int numberOfCopies = obtainNumberOfCopies(data);
    const int numberOfCopies = 1;
    // fixme - think about support of multiple copies.... may it be be something like moving the central point of selection? Like mirror+move?

    // Create new entities
    for (const auto e : entitiesList) {
        for (int num = 1; num <= numberOfCopies; ++num) {
            RS_Entity* clone = getClone(forPreviewOnly, e);
            clone->mirror(data.axisPoint1, data.axisPoint2);
            ctx += clone;
        }
    }

    if (!data.keepOriginals) {
        ctx -= entitiesList;
    }

    return true;
}

/**
 * Rotates entities around two centers with the given parameters.
 */
bool RS_Modification::rotate2(RS_Rotate2Data& data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx) {

    const int numberOfCopies = data.obtainNumberOfCopies();

    // Create new entities
    for(const auto e: entitiesList){
        for (int num= 1; num <= numberOfCopies; num++) {
            RS_Entity* clone = getClone(forPreviewOnly, e);

            const double angle1ForCopy = /*data.sameAngle1ForCopies ?  data.angle1 :*/ data.angle1 * num;
            const double angle2ForCopy = data.sameAngle2ForCopies ?  data.angle2 : data.angle2 * num;
            clone->rotate(data.center1, angle1ForCopy);

            RS_Vector center2 = data.center2;
            center2.rotate(data.center1, angle1ForCopy);
            clone->rotate(center2, angle2ForCopy);

            ctx += clone;
        }
    }

    if (!data.keepOriginals) {
        ctx -= entitiesList;
    }
    return true;
}

void RS_Modification::deleteOriginalAndAddNewEntities(const QList<RS_Entity*>& clonesList, const QList<RS_Entity*>& originalEntities, bool addOnly,
                                                      bool deleteOriginals) const {
    LC_UndoSection undo(m_document, m_viewport, m_handleUndo); // bundle remove/add entities in one undoCycle
    if (deleteOriginals) {
        for (const auto e : originalEntities) {
            undo.undoableDelete(e);
        }
    }
    else {
        for (const auto e : originalEntities) {
            m_document->unselect(e);
        }
    }
    addNewEntities(clonesList, undo, addOnly);
}

/**
 * Moves and rotates entities with the given parameters.
 */
bool RS_Modification::moveRotate(RS_MoveRotateData &data, const QList<RS_Entity*> &entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx){
     const int numberOfCopies = data.obtainNumberOfCopies();

    // Create new entities
    for (const auto entity : entitiesList) {
        for (int num = 1; num <= numberOfCopies; ++num) {
            RS_Entity* clone = getClone(forPreviewOnly, entity);

            const RS_Vector& offset = data.offset * num;
            clone->move(offset);
            const double angleForCopy = data.sameAngleForCopies ? data.angle : data.angle * num;
            clone->rotate(data.referencePoint + offset, angleForCopy);

            ctx += clone;
        }
    }

    if (!data.keepOriginals) {
        ctx -= entitiesList;
    }
    return true;
}

/**
 * Adds the given entities to the container and draws the entities if
 * there's a graphic view available.
 *
 * @param addList Entities to add.
 */
void RS_Modification::addNewEntities(const QList<RS_Entity*>& addList, LC_UndoSection& undo, bool addOnly) const {
    for (RS_Entity* e: addList) {
        if (e != nullptr) {
            undo.undoableAdd(e);
            if (!addOnly && e->getFlag(RS2::FlagSelected)) {
                e->clearSelectionFlag(); // clear flag to force subsequent selection
                m_document->select(e);
            }
        }
    }
    if (!addOnly) {
        m_document->calculateBorders();
        m_viewport->notifyChanged();
    }
}

/**
 * Trims or extends the given trimEntity to the intersection point of the
 * trimEntity and the limitEntity.
 *
 * @param trimCoord Coordinate which defines which endpoint of the
 *   trim entity to trim.
 * @param trimEntity Entity which will be trimmed.
 * @param limitCoord Coordinate which defines the intersection to which the
 *    trim entity will be trimmed.
 * @param limitEntity Entity to which the trim entity will be trimmed.
 * @param both true: Trim both entities. false: trim trimEntity only.
 * @param forPreview true: used in preview, no entities are added to the document.
 *
 */
LC_TrimResult RS_Modification::trim(const RS_Vector& trimCoord,
                                    RS_AtomicEntity* trimEntity,
                                    const RS_Vector& limitCoord,
                                    RS_Entity* limitEntity,
                                    bool both,
                                    bool forPreview) {

    Q_ASSERT(trimEntity  != nullptr && limitEntity != nullptr);
    if (both && !limitEntity->isAtomic()) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::trim: limitEntity is not atomic");
    }

    LC_TrimResult result;

    if(trimEntity->isLocked()|| !trimEntity->isVisible()) {
        return result;
    }

    RS_VectorSolutions sol = findIntersection(*trimEntity, *limitEntity);

    //if intersection are in start or end point can't trim/extend in this point, remove from solution. sf.net #3537053
    if (trimEntity->rtti()==RS2::EntityLine){
        const auto *line = dynamic_cast<RS_Line *>(trimEntity);
        for (unsigned int i=0; i< sol.size(); i++) {
            RS_Vector v = sol.at(i);
            if (v == line->getStartpoint()) {
                sol.removeAt(i);
            }
            else if (v == line->getEndpoint()) {
                sol.removeAt(i);
            }
        }
    }

    if (!sol.hasValid()) {
        return both ? trim( limitCoord, static_cast<RS_AtomicEntity*>(limitEntity), trimCoord, trimEntity, false, forPreview) : result;
    }

    RS_AtomicEntity* trimmed1 = nullptr;
    RS_AtomicEntity* trimmed2 = nullptr;
    // nullpointer fix? - yet check is added later in action, and setting trimmed2 to limitEntity is misleading on preview. So restored to previous version here.
    // auto* trimmed2 = static_cast<RS_AtomicEntity*>(limitEntity);

    if (trimEntity->rtti()==RS2::EntityCircle) {
        // convert a circle into a trimmable arc, need to start from intersections
        trimmed1 = trimCircle(dynamic_cast<RS_Circle*>(trimEntity), trimCoord, sol);
    } else {
        trimmed1 = static_cast<RS_AtomicEntity*>(trimEntity->clone());
        trimmed1->setHighlighted(false);
    }

    // trim trim entity
    size_t ind = 0;
    RS_Vector is(false), is2(false);

    //RS2::Ending ending = trimmed1->getTrimPoint(trimCoord, is);
    if ( trimEntity->trimmable() ) {
        is = trimmed1->prepareTrim(trimCoord, sol);
    } else {
        is = sol.getClosest(limitCoord, nullptr, &ind);
//sol.getClosest(limitCoord, nullptr, &ind);
        RS_DEBUG->print("RS_Modification::trim: limitCoord: %f/%f", limitCoord.x, limitCoord.y);
        RS_DEBUG->print("RS_Modification::trim: sol.get(0): %f/%f", sol.get(0).x, sol.get(0).y);
        RS_DEBUG->print("RS_Modification::trim: sol.get(1): %f/%f", sol.get(1).x, sol.get(1).y);
        RS_DEBUG->print("RS_Modification::trim: ind: %lu", ind);
        is2 = sol.get(ind==0 ? 1 : 0);
        //RS_Vector is2 = sol.get(ind);
        RS_DEBUG->print("RS_Modification::trim: is2: %f/%f", is2.x, is2.y);

    }
    if (!forPreview) {
        // remove trim entity from view:
        /*if (graphicView) {
            graphicView->deleteEntity(trimEntity);
        }*/
    }

    // remove limit entity from view:
    const bool trimBoth= both && !limitEntity->isLocked() && limitEntity->isVisible();
    if (trimBoth) {
        trimmed2 = static_cast<RS_AtomicEntity*>(limitEntity->clone());
        if (!forPreview) {
            trimmed2->setHighlighted(false);
        }
    }

    trimEnding(trimCoord, trimmed1, is);

    // trim limit entity:
    if (trimBoth) {
        if ( trimmed2->trimmable()) {
            is2 = trimmed2->prepareTrim(limitCoord, sol);
        }
        else {
            is2 = sol.getClosest(trimCoord);
        }

        trimEnding(limitCoord, trimmed2, is2);
    }
    if (!forPreview) {
        if (m_handleUndo) {
            const LC_UndoSection undo(m_document,m_viewport);
            undo.undoableReplace(trimEntity,trimmed1);
            if (trimBoth) {
                undo.undoableReplace(limitEntity, trimmed2);
            }
        }
        else {
            // add new trimmed trim entity:
            m_document->addEntity(trimmed1);
            // add new trimmed limit entity:
            if (trimBoth) {
                m_document->addEntity(trimmed2);
            }
        }
    }
    result.result = true;
    result.trimmed1 = trimmed1;
    result.trimmed2 = trimmed2;
    result.intersection1 = is;
    result.intersection2 = is2;

    if (trimmed1->isArc()){
        result.intersection1 = trimmed1->getStartpoint();
        result.intersection2 = trimmed1->getEndpoint();
    }

    m_viewport->notifyChanged();
    return result;
}

void RS_Modification::trimEnding(const RS_Vector &trimCoord, RS_AtomicEntity *trimmed1, const RS_Vector &is) const {
    const RS2::Ending ending = trimmed1->getTrimPoint(trimCoord, is);
    switch (ending) {
        case RS2::EndingStart: {
            trimmed1->trimStartpoint(is);
            break;
        }
        case RS2::EndingEnd: {
            trimmed1->trimEndpoint(is);
            break;
        }
        default:
            break;
    }
}

/**
 * Trims or extends the given trimEntity by the given amount.
 *
 * @param trimCoord Coordinate which defines which endpoint of the
 *   trim entity to trim.
 * @param trimEntity Entity which will be trimmed.
 * @param dist Amount to trim by.
 */
RS_Entity* RS_Modification::trimAmount(const RS_Vector& trimCoord,
                                 RS_AtomicEntity* trimEntity,
                                 double dist,
                                 bool trimBoth,
                                 bool &trimStart, bool &trimEnd) const {

    if (!trimEntity){
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::trimAmount: Entity is nullptr");
        return nullptr;
    }

    if (trimEntity->isLocked() || !trimEntity->isVisible()) {
        return nullptr;
    }

    RS_AtomicEntity *trimmed = nullptr;

    // remove trim entity:
    trimmed = static_cast<RS_AtomicEntity*>(trimEntity->clone());

    // trim trim entity
    trimStart = false;
    trimEnd = false;
    if (trimBoth){
        const RS_Vector isStart = trimmed->getNearestDist(-dist, trimmed->getStartpoint());
        const RS_Vector isEnd = trimmed->getNearestDist(-dist, trimmed->getEndpoint());

        trimmed->trimStartpoint(isStart);
        trimmed->trimEndpoint(isEnd );
        trimStart = true;
        trimEnd = true;
    }
    else {
        const RS_Vector is = trimmed->getNearestDist(-dist, trimCoord);

        if (trimCoord.distanceTo(trimmed->getStartpoint()) <
            trimCoord.distanceTo(trimmed->getEndpoint())){
            trimmed->trimStartpoint(is);
            trimStart = true;
        } else {
            trimmed->trimEndpoint(is);
            trimEnd = true;
        }
    }

    // add new trimmed trim entity:
    if (m_handleUndo) {
        const LC_UndoSection undo(m_document, m_viewport);
        undo.undoableReplace(trimEntity,trimmed);
    }
    else {
        if (m_document != nullptr) {
            m_document->addEntity(trimmed);
        }
    }
    m_viewport->notifyChanged();
    return trimmed;
}

/**
 * Cuts the given entity at the given point.
 */
bool RS_Modification::cut(const RS_Vector& cutCoord, RS_AtomicEntity* cutEntity, LC_DocumentModificationBatch& ctx){

#ifndef EMU_C99
    using std::isnormal;
#endif

    Q_ASSERT(cutEntity != nullptr);

    if(cutEntity->isLocked() || ! cutEntity->isVisible()) {
        return false;
    }
    if (!cutCoord.valid) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::cut: Point invalid.");
        return false;
    }

    // cut point is at endpoint of entity:
    if (cutCoord.distanceTo(cutEntity->getStartpoint())<RS_TOLERANCE ||
        cutCoord.distanceTo(cutEntity->getEndpoint())<RS_TOLERANCE) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::cut: Cutting point on endpoint");
        return false;
    }

    RS_AtomicEntity* cut1 = nullptr;
    RS_AtomicEntity* cut2 = nullptr;
    double a;

    const RS_Pen &originalPen = cutEntity->getPen(false);
    RS_Layer *originalLayer = cutEntity->getLayer(false);
    switch (cutEntity->rtti()) {
        case RS2::EntityCircle: {
            // convert to a whole 2 pi range arc
            const auto *originalCircle = dynamic_cast<RS_Circle *>(cutEntity);
            a = originalCircle->getCenter().angleTo(cutCoord);
            cut1 = new RS_Arc(cutEntity->getParent(),
                RS_ArcData(originalCircle->getCenter(), originalCircle->getRadius(), a, a + 2. * M_PI, false));
            cut1->setPen(originalPen);
            cut1->setLayer(originalLayer);
            // cut2 is nullptr by default
            break;
        }
        case RS2::EntitySplinePoints: {
            // interpolation spline can be closed
            // so we cannot use the default implementation
            cut2 = static_cast<LC_SplinePoints*>(cutEntity)->cut(cutCoord);
            cut1 = static_cast<RS_AtomicEntity*>(cutEntity->clone());

            cut1->setPen(originalPen);
            cut1->setLayer(originalLayer);
            if (cut2) {
                cut2->setPen(originalPen);
                cut2->setLayer(originalLayer);
            }
            break;
        }
        case RS2::EntityEllipse:{
            // handle ellipse arc the using the default method
            // ToDo, to really handle Ellipse Arcs properly, we need to create a new class RS_EllipseArc, keep RS_Ellipse for whole range Ellipses
            const auto* const ellipse=static_cast<const RS_Ellipse*>(cutEntity);
            if(RS_Math::isSameDirection( ellipse ->getAngle1(), ellipse ->getAngle2(), RS_TOLERANCE_ANGLE)
               && ! /*std::*/isnormal(ellipse->getAngle1())
               && ! /*std::*/isnormal(ellipse->getAngle2())
                ) {
                // whole ellipse, convert to a whole range elliptic arc
                a=ellipse->getEllipseAngle(cutCoord);
                cut1 = new RS_Ellipse{cutEntity->getParent(),
                                      RS_EllipseData{ellipse ->getCenter(),
                                                     ellipse ->getMajorP(),
                                                     ellipse ->getRatio(),
                                                     a,a+2.*M_PI,
                                                     ellipse ->isReversed()
                                      }
                };
                cut1->setPen(originalPen);
                cut1->setLayer(originalLayer);
                break;
            }else{
                //elliptic arc
                //missing "break;" line is on purpose
                //elliptic arc should be handled by default:
                //do not insert between here and default:
            }
        }
            // fall-through
        default:
            cut1 = static_cast<RS_AtomicEntity*>(cutEntity->clone());
            cut2 = static_cast<RS_AtomicEntity*>(cutEntity->clone());

            cut1->trimEndpoint(cutCoord);
            cut2->trimStartpoint(cutCoord);
    }

    // add new cut entity:
    ctx += cut1;
    if (cut2 != nullptr) {
        ctx += cut2;
    }

    ctx -= cutEntity;

    return true;
}

/**
 * Stretching.
 */

bool RS_Modification::stretch(const RS_Vector& firstCorner,
                              const RS_Vector& secondCorner,
                              const RS_Vector& offset,
                              bool removeOriginals) const {

    if (!offset.valid) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Modification::stretch: Offset invalid");
        return false;
    }

    QList<RS_Entity*> clonesList;
    QList<RS_Entity*> originalEntities;

    // Create new entities
    for(auto e: *m_document){ // fixme - sand - iteration over all entities in container???
        if (e != nullptr && e->isVisible() && !e->isLocked()) {
            if ((e->isInWindow(firstCorner, secondCorner) ||
                e->hasEndpointsWithinWindow(firstCorner, secondCorner))) {
                RS_Entity* clone = e->clone(); // fixme - sand - should we use proxy there?
                clone->stretch(firstCorner, secondCorner, offset);
                clonesList.push_back(clone);
                originalEntities.push_back(e);
            }
        }
    }

    deleteOriginalAndAddNewEntities(clonesList, originalEntities, false, removeOriginals);
    return true;
}

/**
 * Bevels a corner.
 *
 * @param coord1 Mouse coordinate to specify direction from intersection.
 * @param entity1 First entity of the corner.
 * @param coord2 Mouse coordinate to specify direction from intersection.
 * @param entity2 Second entity of the corner.
 * @param data Lengths and trim flag.
 */
LC_BevelResult* RS_Modification::bevel(const RS_Vector& coord1, RS_AtomicEntity* entity1, const RS_Vector& coord2, RS_AtomicEntity* entity2, RS_BevelData& data,
                                       bool previewOnly) const {
std::unique_ptr<LC_BevelResult> RS_Modification::bevel(
    const RS_Vector &coord1, RS_AtomicEntity *entity1,
    const RS_Vector &coord2, RS_AtomicEntity *entity2,
    RS_BevelData &data,
    bool previewOnly){

    RS_DEBUG->print("RS_Modification::bevel");

    Q_ASSERT(entity1 != nullptr && entity2 != nullptr);

    if (entity1->isLocked() || !entity1->isVisible() || entity2->isLocked() || !entity2->isVisible()) {
        return nullptr;
    }

    RS_EntityContainer *baseContainer = m_document;
    bool isPolyline = false;

    const LC_UndoSection undo(m_document, m_viewport,m_handleUndo && !previewOnly);

    // find out whether we're bevelling within a polyline:

    auto result = std::make_unique<LC_BevelResult>();

    //fixme - that check should be in action too
    if (entity1->getParent() && entity1->getParent()->rtti() == RS2::EntityPolyline){
        RS_DEBUG->print("RS_Modification::bevel: trimming polyline segments");
        if (entity1->getParent() != entity2->getParent()){
            RS_DEBUG->print(RS_Debug::D_WARNING,
                            "RS_Modification::bevel: entities not in the same polyline");
            result->error = LC_BevelResult::ERR_NOT_THE_SAME_POLYLINE;
            return result;
        }
        //TODO: check if entity1 & entity2 are lines.
        //bevel only can be with lines.

        // clone polyline for undo
        auto *cl = dynamic_cast<RS_EntityContainer *>(entity1->getParent()->clone());
        baseContainer = cl;
        if (m_handleUndo && !previewOnly){
            undo.undoableReplace(entity1->getParent(), cl);
        }

        entity1 = static_cast<RS_AtomicEntity*>(baseContainer->entityAt(entity1->getParent()->findEntity(entity1)));
        entity2 = static_cast<RS_AtomicEntity*>(baseContainer->entityAt(entity2->getParent()->findEntity(entity2)));

        isPolyline = true;
    }

    RS_DEBUG->print("RS_Modification::bevel: getting intersection");

    const RS_VectorSolutions sol =
        RS_Information::getIntersection(entity1, entity2, false);

    if (sol.getNumber() == 0){
        result->error = LC_BevelResult::ERR_NO_INTERSECTION;
        return result;
    }

    RS_AtomicEntity *trimmed1 = nullptr;
    RS_AtomicEntity *trimmed2 = nullptr;

    result->polyline = isPolyline;

    //if (data.trim || isPolyline) {
    if (isPolyline){
        trimmed1 = entity1;
        trimmed2 = entity2;
        //Always trim if are working with a polyline, to work with trim==false
        //bevel can't be part of the polyline
        data.trim = true;
    } else {
        trimmed1 = static_cast<RS_AtomicEntity*>(entity1->clone());
        trimmed2 = static_cast<RS_AtomicEntity*>(entity2->clone());
    }

    // trim entities to intersection
    RS_DEBUG->print("RS_Modification::bevel: trim entities to intersection 01");
    bool start1 = false;
    RS_Vector is = sol.getClosest(coord2);
    result->intersectionPoint = is;

    const RS2::Ending ending1 = trimmed1->getTrimPoint(coord1, is);
    switch (ending1) {
        case RS2::EndingStart:
            trimmed1->trimStartpoint(is);
            start1 = true;
            break;
        case RS2::EndingEnd:
            trimmed1->trimEndpoint(is);
            start1 = false;
            break;
        default:
            break;
    }

    result->trimStart1 = start1;

    RS_DEBUG->print("RS_Modification::bevel: trim entities to intersection 02");
    bool start2 = false;
    is = sol.getClosest(coord1);
    const RS2::Ending ending2 = trimmed2->getTrimPoint(coord2, is);
    switch (ending2) {
        case RS2::EndingStart:
            trimmed2->trimStartpoint(is);
            start2 = true;
            break;
        case RS2::EndingEnd:
            trimmed2->trimEndpoint(is);
            start2 = false;
            break;
        default:
            break;
    }
    //}
    result->trimStart2 = start2;

    // find definitive bevel points
    RS_DEBUG->print("RS_Modification::bevel: find definitive bevel points");
    const RS_Vector bp1 = trimmed1->getNearestDist(data.length1, start1);
    const RS_Vector bp2 = trimmed2->getNearestDist(data.length2, start2);

    // final trim:
    RS_DEBUG->print("RS_Modification::bevel: final trim");
    if (data.trim){
        switch (ending1) {
            case RS2::EndingStart:
                trimmed1->trimStartpoint(bp1);
                break;
            case RS2::EndingEnd:
                trimmed1->trimEndpoint(bp1);
                break;
            default:
                break;
        }

        switch (ending2) {
            case RS2::EndingStart:
                trimmed2->trimStartpoint(bp2);
                break;
            case RS2::EndingEnd:
                trimmed2->trimEndpoint(bp2);
                break;
            default:
                break;
        }

        // add new trimmed entities:
        if (!isPolyline && !previewOnly){
            m_document->addEntity(trimmed1);
            m_document->addEntity(trimmed2);
        }
    }

    // add bevel line:
    RS_DEBUG->print("RS_Modification::bevel: add bevel line");
    RS_Line *bevel = nullptr;

    if (previewOnly){
        bevel = new RS_Line(nullptr, bp1, bp2);
    }
    else{
        bevel = new RS_Line(baseContainer, bp1, bp2);
    }

    result->bevel = bevel;

    if (!isPolyline){
        if (!previewOnly){
            baseContainer->addEntity(bevel);
        }
    } else {
        int idx1 = baseContainer->findEntity(trimmed1);
        int idx2 = baseContainer->findEntity(trimmed2);
        int idx = idx1;
        //Verify correct order segment in polylines
        if (idx1 > idx2){
            //inverted, reorder it (swap).
            idx1 = idx2;
            idx2 = idx;
            RS_AtomicEntity *trimmedTmp = trimmed1;
            trimmed1 = trimmed2;
            trimmed2 = trimmedTmp;
        }
        idx = idx1;

        if (!previewOnly){
            selectClone(baseContainer, bevel);
            bevel->setLayer(baseContainer->getLayer());
            bevel->setPen(baseContainer->getPen(false));
        }

        // insert bevel at the right position:
        if (trimmed1 == baseContainer->first() && trimmed2 == baseContainer->last() && baseContainer->count() > 2){
            //bevel are from last and first segments, add at the end
            if (trimmed2->getEndpoint().distanceTo(bevel->getStartpoint()) > 1.0e-4){
                bevel->reverse();
            }
            idx = idx2;
        } else {
            //consecutive segments
            if (trimmed1->getEndpoint().distanceTo(bevel->getStartpoint()) > 1.0e-4){
                bevel->reverse();
            }
        }
        baseContainer->insertEntity(idx + 1, bevel); // fixme - selection - probably it's better check selection flag on insert?
    }

    result->trimmed1 = trimmed1;
    result->trimmed2 = trimmed2;

    if (isPolyline){
        auto* polyline = dynamic_cast<RS_Polyline *>(baseContainer);
        polyline->updateEndpoints();
        result->polyline = polyline;
    }

    RS_DEBUG->print("RS_Modification::bevel: handling undo");

    if (m_handleUndo && !previewOnly){
        if (!isPolyline) {
            if (data.trim){
                undo.undoableReplace(entity2, trimmed1);
                undo.undoableReplace(entity1, trimmed2);
            }
            undo.undoableAdd(bevel);
        }
    }
    //Do not delete trimmed* if are part of a polyline
    if (!(data.trim || isPolyline)){
        RS_DEBUG->print("RS_Modification::bevel: delete trimmed elements");
        delete trimmed1;
        delete trimmed2;
        RS_DEBUG->print("RS_Modification::bevel: delete trimmed elements: ok");
    }

    m_viewport->notifyChanged();
    return result;
}

/**
 * Rounds a corner.
 *
 * @param coord Mouse coordinate to specify the rounding.
 * @param entity1 First entity of the corner.
 * @param entity2 Second entity of the corner.
 * @param data Radius and trim flag.
 */
std::unique_ptr<LC_RoundResult> RS_Modification::round(const RS_Vector& coord,
                            const RS_Vector& coord1,
                            RS_AtomicEntity* entity1,
                            const RS_Vector& coord2,
                            RS_AtomicEntity* entity2,
                            RS_RoundData& data) const {
    Q_ASSERT(entity1 != nullptr && entity2 != nullptr);

    if (entity1->isLocked() || !entity1->isVisible() ||
        entity2->isLocked() || !entity2->isVisible()) {
        return nullptr;
    }

    auto result = std::make_unique<LC_RoundResult>();

    RS_EntityContainer *baseContainer = m_document;
    bool isPolyline = false;
    const LC_UndoSection undo(m_document, m_viewport,m_handleUndo);

    const auto parent1 = entity1->getParent();
    const auto parent2 = entity2->getParent();
    // find out whether we're rounding within a polyline:
    if (parent1 != nullptr &&  parent1->rtti() == RS2::EntityPolyline){

        if (parent1 != parent2){
            RS_DEBUG->print(RS_Debug::D_WARNING,
                            "RS_Modification::round: entities not in "
                            "the same polyline");
            result->error = LC_RoundResult::ERR_NOT_THE_SAME_POLYLINE;
            return result;
        }

        // clone polyline for undo
        const auto cl = dynamic_cast<RS_EntityContainer *>(parent1->clone());
        baseContainer = cl;

        if (m_handleUndo){
            undo.undoableReplace(parent1, cl);
        }

        const int index1 = parent1->findEntity(entity1);
        entity1 = static_cast<RS_AtomicEntity*>(baseContainer->entityAt(index1));

        const int index2 = parent2->findEntity(entity2);
        entity2 = static_cast<RS_AtomicEntity*>(baseContainer->entityAt(index2));

        isPolyline = true;
        result->polyline = true;
    }

    // create 2 tmp parallels
    QList<RS_Entity*> parallels;
    RS_Creation::createParallel(coord, data.radius, 1, entity1, false, parallels);
    const RS_Entity *par1 =  parallels.empty()? nullptr: parallels.front();
    parallels.clear();

    RS_Creation::createParallel(coord, data.radius, 1, entity2, false, parallels);
    const RS_Entity *par2 = parallels.empty()? nullptr: parallels.front();

    // fixme - MERGE_ INCOMING
    //RS_Creation creation(nullptr, nullptr);
    //std::unique_ptr<RS_Entity> par1 { creation.createParallel(coord, data.radius, 1, entity1)};
    //std::unique_ptr<RS_Entity> par2 { creation.createParallel(coord, data.radius, 1, entity2)};

    if ((par1 == nullptr) || (par2 == nullptr)) {
        result->error = LC_RoundResult::NO_PARALLELS;
        return result;
    }

    RS_VectorSolutions sol2 =
        RS_Information::getIntersection(entity1, entity2, false);

    RS_VectorSolutions sol =
        RS_Information::getIntersection(par1.get(), par2.get(), false);

    if (sol.getNumber() == 0) {
        result->error = LC_RoundResult::ERR_NO_INTERSECTION;
        return result;
    }

    // there might be two intersections: choose the closest:
    const RS_Vector is  = sol.getClosest(coord);
    const RS_Vector p1  = entity1->getNearestPointOnEntity(is, false);
    const RS_Vector p2  = entity2->getNearestPointOnEntity(is, false);
    const double ang1   = is.angleTo(p1);
    const double ang2   = is.angleTo(p2);
    const bool reversed = (RS_Math::getAngleDifference(ang1, ang2) > M_PI);
    bool isTrimming = data.radius <= RS_TOLERANCE;
    auto arc     = std::make_unique<RS_Arc>(baseContainer, RS_ArcData(is, data.radius, ang1, ang2, reversed));

    result->round = isTrimming ? nullptr : arc.get();

    RS_AtomicEntity *trimmed1 = nullptr;
    RS_AtomicEntity *trimmed2 = nullptr;

    if (data.trim || isPolyline){
        if (isPolyline){
            trimmed1 = entity1;
            trimmed2 = entity2;
        } else {
            trimmed1 = static_cast<RS_AtomicEntity*>(entity1->clone());
            trimmed2 = static_cast<RS_AtomicEntity*>(entity2->clone());
        }

        // trim entities to intersection
        RS_Vector is2 = sol2.getClosest(coord2);
        const RS2::Ending ending1 = trimmed1->getTrimPoint(coord1, is2);
        switch (ending1) {
            case RS2::EndingStart:
                trimmed1->trimStartpoint(p1);
                result->trim1Mode = LC_RoundResult::TRIM_START;
                break;
            case RS2::EndingEnd:
                trimmed1->trimEndpoint(p1);
                result->trim1Mode = LC_RoundResult::TRIM_END;
                break;
            default:
                trimmed1 = trimCircleForRound(trimmed1, *arc);
                result->trim1Mode = LC_RoundResult::TRIM_CIRCLE;
                break;
        }

        is2 = sol2.getClosest(coord1);
        const RS2::Ending ending2 = trimmed2->getTrimPoint(coord2, is2);
        switch (ending2) {
            case RS2::EndingStart:
                trimmed2->trimStartpoint(p2);
                result -> trim2Mode = LC_RoundResult::TRIM_START;
                break;
            case RS2::EndingEnd:
                trimmed2->trimEndpoint(p2);
                result->trim2Mode = LC_RoundResult::TRIM_END;
                break;
            default:
                trimmed2 = trimCircleForRound(trimmed2, *arc);
                result->trim2Mode = LC_RoundResult::TRIM_CIRCLE;
                break;
        }
    }

    // add rounding:
    if (!isPolyline){
        if (!isTrimming)
            baseContainer->addEntity(arc.get());
    } else {
        // find out which base entity is before the rounding:
        const int idx1 = baseContainer->findEntity(trimmed1);
        const int idx2 = baseContainer->findEntity(trimmed2);

        selectClone(baseContainer, arc);
        arc->setLayer(baseContainer->getLayer());
        arc->setPen(baseContainer->getPen(false));

        RS_DEBUG->print("RS_Modification::round: idx1<idx2: %d", (int) (idx1 < idx2));
        RS_DEBUG->print("RS_Modification::round: idx1!=0: %d", (int) (idx1 != 0));
        RS_DEBUG->print("RS_Modification::round: idx2==0: %d", (int) (idx2 == 0));
        RS_DEBUG->print("RS_Modification::round: idx1==(int)baseContainer->count()-1: %d",
                        (int) (idx1 == (int) baseContainer->count() - 1));

        const bool insertAfter1 = ((idx1 < idx2 && idx1 != 0) || (idx1 == 0 && idx2 == 1) ||
                             (idx2 == 0 && idx1 == (int) baseContainer->count() - 1));

        if (insertAfter1){
            if (trimmed1->getEndpoint().distanceTo(arc->getStartpoint()) > 1.0e-4){
                arc->reverse();
            }
            if (!isTrimming)
                baseContainer->insertEntity(idx1 + 1, arc.get());
        } else {
            if (trimmed2->getEndpoint().distanceTo(arc->getStartpoint()) > 1.0e-4){
                arc->reverse();
            }
            if (!isTrimming)
                baseContainer->insertEntity(idx2 + 1, arc.get());
        }
    }

    result->trimmed1 = trimmed1;
    result->trimmed2 = trimmed2;

    if (isPolyline){
        static_cast<RS_Polyline*>(baseContainer)->updateEndpoints();
    }

    if (m_handleUndo){
        if (!isPolyline) {
            if (data.trim) {
                undo.undoableReplace(entity2, trimmed2);
                undo.undoableReplace(entity1, trimmed1);
            }
            undo.undoableAdd(arc.release());
        }
    }
    else {
        // add new trimmed entities:
         if (!isPolyline) {
             m_document->addEntity(arc);
         }
         m_document->addEntity(trimmed1);
         m_document->addEntity(trimmed2);
    }

    if (!isTrimming)
        arc.release();

    m_viewport->notifyChanged();
    return result;
}

/**
 * Repetitive recursive block of code for the explode() method.
 */
static void updateExplodedChildrenRecursively(RS_EntityContainer* ec, RS_Entity* e, RS_Entity* clone, RS2::ResolveLevel rl, bool resolveLayer,
                                              bool resolvePen) {

    if (ec == nullptr || e == nullptr || clone == nullptr) {
        return;
    }

    if (resolveLayer) {
        clone->setLayer(ec->getLayer());
    } else {
        clone->setLayer(e->getLayer());
    }

    if (resolvePen) {
        //clone->setPen(ec->getPen(true));
        clone->setPen(ec->getPen(false));
    } else {
        clone->setPen(e->getPen(false));
    }

    clone->update();

    if (clone->isContainer()) {
        // Note: reassigning ec and e here, so keep
        // that in mind when writing code below this block.
        ec = static_cast<RS_EntityContainer*>(clone);
        for(RS_Entity* en: lc::LC_ContainerTraverser{*ec, rl}.entities()) {
            if (en != nullptr) {
                // Run the same code for every children recursively
                updateExplodedChildrenRecursively(ec, clone, en, rl, resolveLayer, resolvePen);
            }
        }
    }
}

// fixme - sand - decide how to treat keepSelected flag. So far one is ignored.
bool RS_Modification::explode(const QList<RS_Entity*> &entitiesList, const bool remove, [[maybe_unused]]const bool keepSelected) const {
    QList<RS_Entity*> clonesList;// Issue #2296, only collect exploded containers to delete
    std::vector<RS_Entity*> toDelete;

    for(const auto e: entitiesList){
        if (e->isContainer()) {
            // add entities from container:
            auto* ec = static_cast<RS_EntityContainer*>(e);

            RS2::ResolveLevel rl;
            bool resolvePen;
            bool resolveLayer;

            const auto containerType = ec->rtti();
            switch (containerType) {
                case RS2::EntityMText:
                case RS2::EntityText:
                case RS2::EntityHatch:
                case RS2::EntityPolyline:
                    rl = RS2::ResolveAll;
                    resolveLayer = true;
                    resolvePen = true;
                    break;

                case RS2::EntityInsert:
                    resolvePen = false;
                    resolveLayer = false;
                    rl = RS2::ResolveNone;
                    break;

                default:
                    if (RS2::isDimensionalEntity(containerType)) {
                        rl = RS2::ResolveNone;
                        resolveLayer = true;
                        resolvePen = true;
                    }
                    else {
                        rl = RS2::ResolveAll;
                        resolveLayer = true;
                        resolvePen = false;
                    }
                    break;
            }

            auto entities = lc::LC_ContainerTraverser{*ec, rl}.entities();
            for(RS_Entity* e2: entities) {
                if (e2 != nullptr) {
                    RS_Entity* clone = e2->clone();
                    clone->clearSelectionFlag();
                    clone->reparent(m_document);

                    clonesList.push_back(clone);

                    // In order to fix bug #819 and escape similar issues,
                    // we have to update all children of exploded entity,
                    // even those (below the tree) which are not direct
                    // subjects to the current explode() call.
                    updateExplodedChildrenRecursively(ec, e2, clone,
                                                         rl, resolveLayer, resolvePen);

                }
            }
            toDelete.push_back(e);
        } else {
            m_document->unselect(e); // fixme- selection - should it be called ? Check whether later e is deleted
                }
            }

    }
    // fixme - sand - review why explode deletes original atomic entities
    deleteOriginalAndAddNewEntities(clonesList, toDelete, false, remove);
    clonesList.clear();
    m_document->updateInserts();
    return true;
}

bool RS_Modification::explodeTextIntoLetters(const QList<RS_Entity*> &selectedEntitiesList, LC_DocumentModificationBatch& ctx) {
    for (const auto e : selectedEntitiesList) { // fixme - create generic iterator function with lambda!
        if (e == nullptr || e->isDeleted() || !e->isVisible()) {
            continue;
        }
        const auto rtti = e->rtti();
        switch (rtti) {
            case (RS2::EntityMText): {
                // add letters of text:
                const auto text = dynamic_cast<RS_MText*>(e);
                doExplodeTextIntoLetters(text, ctx);
                ctx -= e; // originals are always deleted
                break;
            }
            case (RS2::EntityText): {
                // add letters of text:
                const auto text = dynamic_cast<RS_Text*>(e);
                doExplodeTextIntoLetters(text, ctx);
                ctx -= e; // originals are always deleted
                break;
            }
            default: {
            }
        }
    }
    return true;
}

bool RS_Modification::doExplodeTextIntoLetters(RS_MText* text, LC_DocumentModificationBatch& ctx) {
    if (text == nullptr) {
        return false;
    }
    if(text->isLocked() || ! text->isVisible()) {
        return false;
    }

    // iterate though lines:
    for(const auto e2: *text){
        if (e2 == nullptr) {
            break;
        }
        // text lines:
        if (e2->rtti()==RS2::EntityContainer) {
            const auto line = dynamic_cast<RS_EntityContainer *>(e2);

            // iterate though letters:
            for(const auto e3: *line){
                if (e3 == nullptr) {
                    break;
                }

                if (e3->rtti()==RS2::EntityMText) { // super / sub texts:
                    const auto e3MText = dynamic_cast<RS_MText *>(e3);
                    doExplodeTextIntoLetters(e3MText, ctx);
                }
                else if (e3->rtti()==RS2::EntityInsert) {    // normal letters:
                    const auto letter = dynamic_cast<RS_Insert *>(e3);

                    const auto letterAsText = new RS_MText(
                        nullptr,
                        RS_MTextData(letter->getInsertionPoint(),
                                     text->getHeight(),
                                     100.0,
                                     RS_MTextData::VABottom, RS_MTextData::HALeft,
                                     RS_MTextData::LeftToRight, RS_MTextData::Exact,
                                     1.0,
                                     letter->getName(),
                                     text->getStyle(),
                                     letter->getAngle(),
                                     RS2::Update));

                    letterAsText->setLayer(text->getLayer());
                    letterAsText->setPen(text->getPen(false));

                    ctx += letterAsText;
                    letterAsText->update();
                }
            }
        }
    }

    return true;
}

bool RS_Modification::doExplodeTextIntoLetters(RS_Text* text, LC_DocumentModificationBatch& ctx) {
    if(text->isLocked() || ! text->isVisible()) {
        return false;
    }

    if(text->isLocked() || ! text->isVisible()) return false;

    // iterate though letters:
    for(const auto e2: *text){
        if (e2 == nullptr) {
            break;
        }

        if (e2->rtti()==RS2::EntityInsert) {
            const auto letter = dynamic_cast<RS_Insert *>(e2);
            const auto letterAsText = new RS_Text(
                nullptr,
                RS_TextData(letter->getInsertionPoint(),
                            letter->getInsertionPoint(),
                            text->getHeight(),
                            text->getWidthRel(), RS_TextData::VABaseline,
                            RS_TextData::HALeft, RS_TextData::None, /*text->getTextGeneration(),*/
                            letter->getName(),
                            text->getStyle(),
                            letter->getAngle(),
                            RS2::Update));

            letterAsText->setLayer(text->getLayer());
            letterAsText->setPen(text->getPen(false));

            ctx += letterAsText;
            letterAsText->update();
        }
    }
    return true;
}
