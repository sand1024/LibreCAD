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

#ifndef RS_MODIFICATION_H
#define RS_MODIFICATION_H
#include <memory>
#include <QString>

#include "lc_copyutils.h"
#include "rs_pen.h"
#include "rs_vector.h"

class RS_Arc;
class RS_AtomicEntity;
class RS_Entity;
class RS_EntityContainer;
class RS_MText;
class RS_Text;
class RS_Line;
class RS_Insert;
class RS_Block;
class RS_Polyline;
class RS_Document;
class RS_Graphic;
class RS_GraphicView;
class LC_GraphicViewport;
class LC_SelectedSet;
class LC_UndoSection;
struct LC_DocumentModificationBatch;

struct LC_ModifyOperationFlags{
    bool useCurrentAttributes = false;
    bool useCurrentLayer = false;
    bool keepOriginals = false;
    int number = 0;
    bool multipleCopies = false;

    int obtainNumberOfCopies() const {
        int numberOfCopies = number;
        if (!multipleCopies){
            numberOfCopies = 1;
        }
        else{
            if (numberOfCopies < 1){
                numberOfCopies = 1;
            }
        }
        return numberOfCopies;
    }
};

/**
 * Holds the data needed for move modifications.
 */
struct RS_MoveData : LC_ModifyOperationFlags{
    RS_Vector offset;
};

struct LC_AlignRefData: LC_ModifyOperationFlags{
    bool scale;
    RS_Vector rotationCenter;
    RS_Vector offset;
    double rotationAngle = 0.0;
    double scaleFactor = 0.0;
};

struct RS_BoundData{
    RS_BoundData(const RS_Vector &left, const RS_Vector &top) {
        min = left;
        max = top;
    }

    RS_Vector min;
    RS_Vector max;

    RS_Vector getCenter() const {
        return (min+max)/2;
    }
};

/**
 * Holds the data needed for offset modifications.
 */
struct RS_OffsetData : LC_ModifyOperationFlags{
    RS_Vector coord;
    double distance = 0.;
};

/**
 * Holds the data needed for rotation modifications.
 */
struct RS_RotateData : LC_ModifyOperationFlags{
    RS_Vector center;
    RS_Vector refPoint;
    double angle = 0.;
    double secondAngle = 0.0;
    bool secondAngleIsAbsolute = false;
    bool twoRotations = false;
};

/**
 * Holds the data needed for scale modifications.
 */
struct RS_ScaleData : LC_ModifyOperationFlags {
    RS_Vector referencePoint;
    RS_Vector factor = RS_Vector(1.1,1.0,0.0);
    // Find the factor by a source and a target point
    bool isotropicScaling = true;
    bool toFindFactor = false;
};

/**
 * Holds the data needed for mirror modifications.
 */
struct RS_MirrorData : LC_ModifyOperationFlags{
    RS_Vector axisPoint1;
    RS_Vector axisPoint2;
};

/**
 * Holds the data needed for move/rotate modifications.
 */
struct RS_MoveRotateData : LC_ModifyOperationFlags{
    RS_Vector referencePoint;
    RS_Vector offset;
    double angle = 0.;
    bool sameAngleForCopies = false;
};

/**
 * Holds the data needed for rotation around two centers modifications.
 */
struct RS_Rotate2Data : LC_ModifyOperationFlags{
    RS_Vector center1;
    RS_Vector center2;

    double angle1 = 0.;
    double angle2 = 0.;
    bool mirrorAngles = false;
    bool sameAngle2ForCopies = false;
};

/**
 * Holds the data needed for beveling modifications.
 */
struct RS_BevelData{
    double length1 = 0.;
    double length2 = 0.;
    bool trim = false;
};

struct LC_TrimResult{
    bool result = false;
    RS_Entity* trimmed1 = nullptr;
    RS_Entity* trimmed2 = nullptr;
    RS_Vector intersection1;
    RS_Vector intersection2;
};

struct LC_BevelResult{
    RS_Line* bevel = nullptr;
    bool isPolyline = false;
    RS_Polyline* polyline = nullptr;
    RS_Entity* trimmed1 = nullptr;
    RS_Entity* trimmed2 = nullptr;
    bool trimStart1 = false;
    bool trimStart2 = false;
    RS_Vector intersectionPoint = RS_Vector(false);
    int error = OK;

    enum{
        OK,
        ERR_NO_INTERSECTION,
        ERR_NOT_THE_SAME_POLYLINE,
        ERR_VISIBILITY,
        ERR_NOT_LINES
    };
};

struct LC_ModificationContext {
    QList<RS_Entity*> *entitiesOriginal {nullptr};
    QList<RS_Entity*> entitiesToCreate;
    QList<RS_Entity*> entitiesToDelete;

    explicit LC_ModificationContext(QList<RS_Entity*> &originals): entitiesOriginal{&originals}{}
};

struct LC_RoundResult{
    bool isPolyline             = false;
    RS_Arc* round               = nullptr;
    RS_Entity* trimmed1         = nullptr;
    RS_Entity* trimmed2         = nullptr;
    RS_Polyline* polyline       = nullptr;
    int trim1Mode               = false;
    int trim2Mode               = false;
    RS_Vector trimmingPoint1    = RS_Vector(false);
    RS_Vector trimmingPoint2    = RS_Vector(false);
    RS_Vector intersectionPoint = RS_Vector(false);
    int error                   = OK;

    enum TrimMode {
        TRIM_START,
        TRIM_END,
        TRIM_CIRCLE
    };

    enum{
        OK,
        ERR_NO_INTERSECTION,
        NO_PARALLELS,
        ERR_INPUT
    };
};

/**
 * Holds the data needed for rounding modifications.
 */
struct RS_RoundData {
    double radius = 0.;
    bool trim = false;
};

/**
 * Holds the data needed for moving reference points.
 */
struct RS_MoveRefData {
    RS_Vector ref;
    RS_Vector offset;
};

/**
 * Holds the data needed for changing attributes.
 */
struct RS_AttributesData {
    QString layer;
    RS_Pen pen;
    bool changeLayer = false;
    bool changeColor = false;
    bool changeLineType = false;
    bool changeWidth = false;
    bool applyBlockDeep = false;
};



struct LC_LibraryInsertData: LC_CopyUtils::RS_PasteData {
    LC_LibraryInsertData(RS_Vector insertionPoint, double factor, double angle, const QString& blockName, RS_Graphic* source);
    //! Name of the block to create or an empty string to assign a new auto name.
    QString blockName;
    RS_Graphic* source;
};

/**
 * API Class for manipulating entities.
 * There's no interaction handled in this class.
 *
 * All modifications can be undone / redone if the container
 * is a RS_Graphic.
 *
 * This class is connected to an entity container and
 * can be connected to a graphic view.
 *
 * @author Andrew Mustun
 */
class RS_Modification {
public:
    RS_Modification(RS_EntityContainer* entityContainer, LC_GraphicViewport* viewport, bool handleUndo = true);

    static void revertDirection(QList<RS_Entity*>& originalEntities, LC_DocumentModificationBatch& ctx);
    static void doChangeEntityAttributes(RS_Entity* en, RS_Entity*& clone, const RS_AttributesData& data, QSet<RS_Block*> &blocks);
    static bool changeAttributes(const QList<RS_Entity*>& originalEntities, RS_AttributesData& data, LC_DocumentModificationBatch& ctx);
    static void doChangeBlockAttributes(RS_Block* block, RS_AttributesData& data, QSet<QString>& processedBlockNames);

    static void libraryInsert(const LC_LibraryInsertData& data,  RS_Graphic* destination, LC_DocumentModificationBatch& ctx);

    static void move(RS_MoveData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static bool rotate(RS_RotateData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static bool scale(RS_ScaleData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static bool mirror(RS_MirrorData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static bool moveRotate(RS_MoveRotateData& data, const QList<RS_Entity*>& entitiesList, bool previewOnly, LC_DocumentModificationBatch& ctx);

    static bool rotate2(RS_Rotate2Data& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static LC_TrimResult trim(const RS_Vector& trimCoord, RS_AtomicEntity* trimEntity, const RS_Vector& limitCoord, RS_Entity* limitEntity, bool both,
                              LC_DocumentModificationBatch& ctx);

    static RS_Entity* trimAmount(const RS_Vector& trimCoord, RS_AtomicEntity* entityToTrim, double dist, bool trimBoth, bool& trimStart, bool& trimEnd,
                          LC_DocumentModificationBatch& ctx);

    static bool offset(const RS_OffsetData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

    static bool cut(const RS_Vector& cutCoord, RS_AtomicEntity* cutEntity, LC_DocumentModificationBatch& ctx);

    // FIXME - sand - review & complete
    static bool stretch(const RS_Vector& firstCorner, const RS_Vector& secondCorner, const RS_Vector& offset, const QList<RS_Entity*>& entitiesList, bool removeOriginals,
                        LC_DocumentModificationBatch& ctx);

    static LC_BevelResult bevel(const RS_Vector& coord1, RS_AtomicEntity* entity1, const RS_Vector& coord2, RS_AtomicEntity* entity2, RS_BevelData& data,
                                bool previewOnly, LC_DocumentModificationBatch& ctx);

    static LC_RoundResult round(const RS_Vector& coord, const RS_Vector& coord1, RS_AtomicEntity* entity1, const RS_Vector& coord2, RS_AtomicEntity* entity2,
                                RS_RoundData& data, LC_DocumentModificationBatch& ctx);

    static bool explode(const QList<RS_Entity*>& entitiesList, bool remove, LC_DocumentModificationBatch& ctx);

    static bool explodeTextIntoLetters(const QList<RS_Entity*>& selectedEntitiesList, LC_DocumentModificationBatch& ctx);

    // fixme - review what for this method is used? action is not enabled
    static bool splitPolyline(RS_Polyline* polyline, RS_Entity& e1, RS_Vector v1, RS_Entity& e2, RS_Vector v2, RS_Polyline** polyline1, RS_Polyline** polyline2, LC_DocumentModificationBatch& ctx);

    static RS_Polyline* addPolylineNode(RS_Polyline* polyline, const RS_AtomicEntity& segment, const RS_Vector& node, LC_DocumentModificationBatch& ctx);
    static RS_Polyline* deletePolylineNode(RS_Polyline* polyline, const RS_Vector& node, LC_DocumentModificationBatch& ctx);
    // fixme - complete!!!
    static RS_Polyline* deletePolylineNodesBetween(RS_Polyline* polyline, const RS_Vector& node1, const RS_Vector& node2, LC_DocumentModificationBatch& ctx);
    static RS_Polyline* polylineTrim(RS_Polyline* polyline, RS_AtomicEntity& segment1, RS_AtomicEntity& segment2, LC_DocumentModificationBatch& ctx);

    static RS_BoundData getBoundingRect(QList<RS_Entity*>& selected);
    static bool alignRef(LC_AlignRefData& data, const QList<RS_Entity*>& entitiesList, bool forPreviewOnly, LC_DocumentModificationBatch& ctx);

private:
    static bool trimAtomicByEnding(RS_AtomicEntity* atomicToTrim, RS_Vector trimPoint, RS2::Ending ending);
    static bool doExplodeTextIntoLetters(RS_MText* text, LC_DocumentModificationBatch& ctx);
    static bool doExplodeTextIntoLetters(RS_Text* text,  LC_DocumentModificationBatch& ctx);
protected:

    static void trimEnding(const RS_Vector& trimCoord, RS_AtomicEntity* trimmed1, const RS_Vector& is);
    static LC_RoundResult::TrimMode roundingTrimEntity(RS_VectorSolutions entitiesIntersection, RS_AtomicEntity*& entityToTrim,
                               RS_Arc* arc, RS_Vector trimPoint, RS_Vector selectionPoint1, RS_Vector selectionPoint2);


    static RS_Entity* getClone(bool forPreviewOnly, const RS_Entity* e);
    static void selectClone(RS_Entity* original, RS_Entity* clone);
};

#endif
