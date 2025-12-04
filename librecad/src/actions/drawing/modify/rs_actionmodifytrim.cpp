/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
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

#include "rs_actionmodifytrim.h"

#include "lc_actioninfomessagebuilder.h"
#include "rs_arc.h"
#include "rs_atomicentity.h"
#include "rs_debug.h"
#include "rs_document.h"
#include "rs_ellipse.h"
#include "rs_modification.h"

struct RS_ActionModifyTrim::TrimActionData {
    RS_Vector limitCoord;
    RS_Vector trimCoord;
};

/**
 * @param both Trim both entities.
 */
RS_ActionModifyTrim::RS_ActionModifyTrim(LC_ActionContext *actionContext, bool both)
    : LC_UndoableDocumentModificationAction("Trim Entity",actionContext, both ? RS2::ActionModifyTrim2 : RS2::ActionModifyTrim)
    , m_trimEntity{nullptr}, m_limitEntity{nullptr}
    , m_actionData(std::make_unique<TrimActionData>()), m_both{both} {
}

RS_ActionModifyTrim::~RS_ActionModifyTrim() = default;

void RS_ActionModifyTrim::init(int status) {
    m_snapMode.clear();
    m_snapMode.restriction = RS2::RestrictNothing;
    RS_PreviewActionInterface::init(status);
}

void RS_ActionModifyTrim::doInitWithContextEntity(RS_Entity* contextEntity, const RS_Vector& clickPos) {
    if (isAtomic(contextEntity)) {
        m_trimEntity = static_cast<RS_AtomicEntity*>(contextEntity);
        m_actionData->trimCoord = clickPos;
    }
}

void RS_ActionModifyTrim::finish(bool updateTB) {
    RS_PreviewActionInterface::finish(updateTB);
}

bool RS_ActionModifyTrim::doTriggerModifications(LC_DocumentModificationBatch& ctx) {
    if (isAtomic(m_trimEntity) && m_limitEntity != nullptr /* && limitEntity->isAtomic()*/) {
        const LC_TrimResult trimResult =  RS_Modification::trim(m_actionData->trimCoord,  m_trimEntity,
               m_actionData->limitCoord, m_limitEntity,
               m_both, ctx);

        trimResult.trimmed1->setPen(m_trimEntity->getPen(false));
        trimResult.trimmed1->setLayer(m_trimEntity->getLayer(false));

        if (m_both) {
            if (trimResult.trimmed2 != nullptr) {
                trimResult.trimmed2->setPen(m_limitEntity->getPen(false));
                trimResult.trimmed2->setLayer(m_limitEntity->getLayer(false));
            }
        }
        ctx.dontSetActiveLayerAndPen();
        return trimResult.result;
    }
    return false;
}

void RS_ActionModifyTrim::doTriggerCompletion(bool success) {
    if (success) {
        m_trimEntity = nullptr;
        if (m_both) {
            m_limitEntity = nullptr;
            setStatus(ChooseLimitEntity);
        } else {
            setStatus(ChooseTrimEntity);
        }
    }
}

void RS_ActionModifyTrim::previewTrim(RS_Entity* entityToTrimCandidate, RS_Entity* limitingEntity, RS_Vector trimCoordinates, RS_Vector limitCoordinates, bool& trimInvalid) {
    if (entityToTrimCandidate != nullptr && entityToTrimCandidate != limitingEntity) {
        if (entityToTrimCandidate->isAtomic()) {
            auto *atomicTrimCandidate = dynamic_cast<RS_AtomicEntity *>(entityToTrimCandidate);

            LC_DocumentModificationBatch ctx;
            const LC_TrimResult trimResult = RS_Modification::trim(trimCoordinates, atomicTrimCandidate,
                                              limitCoordinates, limitingEntity,
                                              m_both, ctx);
            if (trimResult.result) {
                trimInvalid = false;
                highlightHover(entityToTrimCandidate);
                if (m_showRefEntitiesOnPreview) {
                    previewRefPoint(trimResult.intersection1);
                    previewRefTrimmedEntity(trimResult.trimmed1, entityToTrimCandidate);
                    if (trimResult.intersection2.valid) {
                        previewRefPoint(trimResult.intersection2);
                    }
                    if (m_both) {
                        if (trimResult.trimmed2 != nullptr) {
                            previewRefTrimmedEntity(trimResult.trimmed2, limitingEntity);
                        }
                    }
                    if (isInfoCursorForModificationEnabled()){
                        const auto msg = rtti() == RS2::ActionModifyTrim2 ? tr("Trim Two") : tr("Trim");
                        auto builder = msgStart().string(msg).vector(
                            tr("Intersection:"), trimResult.intersection1);
                        if (trimResult.intersection2.valid) {
                            builder.vector(tr("Intersection 2:"), trimResult.intersection2);
                        }
                        builder.toInfoCursorZone2(false);
                    }
                }
            }
        }
    }
}

// todo - check trim both mode - it seems that limiting entity should be atomic too...
void RS_ActionModifyTrim::onMouseMoveEvent(int status, LC_MouseEvent *e) {
    const RS_Vector mouse = e->graphPoint;
    bool trimInvalid = true;
    switch (status) {
        case ChooseLimitEntity: {
            RS_Entity *se = catchAndDescribe(e, RS2::ResolveAllButTextImage);
            if (m_trimEntity != nullptr) {
                previewTrim(m_trimEntity, se, m_actionData->trimCoord, mouse, trimInvalid);
                if (trimInvalid) {
                    highlightSelected(m_trimEntity);
                }
            }
            else  if (se != nullptr) {
                highlightHover(se);
            }

            break;
        }
        case ChooseTrimEntity: {
            RS_Entity *entityToTrimCandidate = catchAndDescribe(e, RS2::ResolveNone);
            previewTrim(entityToTrimCandidate, m_limitEntity, mouse, m_actionData->limitCoord, trimInvalid);
            if (trimInvalid) {
                highlightSelected(m_limitEntity);
            }
            break;
        }
        default:
            break;
    }
}

void RS_ActionModifyTrim::onMouseLeftButtonRelease(int status, LC_MouseEvent *e) {
    const RS_Vector mouse = e->graphPoint;
    switch (status) {
        case ChooseLimitEntity: {
            RS_Entity *se = catchEntityByEvent(e, RS2::ResolveAllButTextImage);
            if (se != nullptr) {
                m_limitEntity = se;
                if (m_limitEntity->rtti() != RS2::EntityPolyline/*&& limitEntity->isAtomic()*/) {
                    m_actionData->limitCoord = mouse;
                    if (m_trimEntity != nullptr) {
                        trigger();
                    }
                    else {
                        setStatus(ChooseTrimEntity);
                    }
                }
            }
            break;
        }
        case ChooseTrimEntity: {
            RS_Entity *se = catchEntityByEvent(e, RS2::ResolveNone);
            if (isAtomic(se) && se != m_limitEntity) {
                m_actionData->trimCoord = mouse;
                m_trimEntity = dynamic_cast<RS_AtomicEntity*>(se);
                trigger();
            }
            break;
        }
        default:
            break;
    }
}

void RS_ActionModifyTrim::onMouseRightButtonRelease(int status, [[maybe_unused]] LC_MouseEvent *e) {
    deletePreview();
    initPrevious(status);
}

void RS_ActionModifyTrim::updateMouseButtonHints() {
    switch (getStatus()) {
        case ChooseLimitEntity:
            if (m_both) {
                updateMouseWidgetTRCancel(tr("Select first trim entity"));
            } else {
                updateMouseWidgetTRBack(tr("Select limiting entity"));
            }
            break;
        case ChooseTrimEntity:
            if (m_both) {
                updateMouseWidgetTRCancel(tr("Select second trim entity"));
            } else {
                updateMouseWidgetTRBack(tr("Select entity to trim"));
            }
            break;
        default:
            updateMouseWidget();
            break;
    }
}
RS2::CursorType RS_ActionModifyTrim::doGetMouseCursor([[maybe_unused]] int status){
    return RS2::SelectCursor;
}

void RS_ActionModifyTrim::previewRefTrimmedEntity(RS_Entity *trimmed, RS_Entity* original) const {
    const int rtti = trimmed->rtti();
    switch (rtti){
        case RS2::EntityLine:{
            RS_Vector start = original->getStartpoint();
            const RS_Vector startTrimmed = trimmed->getStartpoint();
            RS_Vector end = original->getEndpoint();
            const RS_Vector endTrimmed = trimmed->getEndpoint();
            const bool sameStart = start == startTrimmed;
            const bool sameEnd = end == endTrimmed;

            if (!sameStart) {
                end = startTrimmed;
            }
            if (!sameEnd) {
                start = endTrimmed;
            }
            previewRefLine(start, end);
            break;
        }
        case RS2::EntityArc:{
            auto* arc = dynamic_cast<RS_Arc *>(trimmed);
            RS_ArcData data = arc->getData();
            data.reversed = !data.reversed;
            previewRefArc(data);
            break;
        }
        case RS2::EntityCircle:{
            // that's really strange case - not trimmed circle??? 
//            auto* circle = dynamic_cast<RS_Circle*>(trimmed);
//            previewRefCircle(circle->getCenter(), circle->getRadius());
            break;
        }
        case RS2::EntityEllipse:{
            const auto* ellipse = dynamic_cast<RS_Ellipse *>(trimmed);
            auto data = ellipse->getData();
            data.reversed = !data.reversed;
            previewRefEllipse(data);
            break;
        }
        case RS2::EntityParabola:{
            // fixme - check trimming of parabola and drawing part that will be trimmed
        }
        default:{
            previewEntity(trimmed);
            RS_DEBUG->print("RS_ActionModifyTrim::unhandled trimmed entity type");
        }
    }
}
