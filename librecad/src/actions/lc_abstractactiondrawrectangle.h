#ifndef LC_ABSTRACTACTIONDRAWRECTANGLE_H
#define LC_ABSTRACTACTIONDRAWRECTANGLE_H

#include "rs_previewactioninterface.h"
#include "rs_vector.h"
#include "rs_polyline.h"

class LC_AbstractActionDrawRectangle:public RS_PreviewActionInterface {
Q_OBJECT

public:
    LC_AbstractActionDrawRectangle(const char *name, RS_EntityContainer &container, RS_GraphicView &graphicView);
    ~LC_AbstractActionDrawRectangle() override;

    /**
     * defines modes for drawing rect corners
     */
    enum {
        DRAW_STRAIGHT, // plain rect
        DRAW_RADIUS,  // rounded corners
        DRAW_BEVEL // bevels
    };

    /**
      * Action States.
      */
    enum Status {
        SetPoint1,
        SetPoint2,
        SetPoint3,
        SetHeight,
        SetWidth,
        SetPoint1Snap,
        SetPoint2Snap,
        SetSize,
        SetAngle,
        SetCorners,
        SetBevels,
        SetRadius,
        LAST_BASE_STATUS
    };

    void trigger() override;

    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void commandEvent(RS_CommandEvent* e) override;
    void coordinateEvent(RS_CoordinateEvent* e) override;
    void updateMouseButtonHints() override;

    bool isUsePolyline(){return usePolyline;};

    void setUsePolyline(bool value){usePolyline = value;};
    void setRadius(double radius);

    double getRadius(){return radius;};

    void setLengthX(double value);

    double getLengthX(){return bevelX;};

    void setLengthY(double value);

    double getLengthY(){return bevelY;};
    void setAngle(double angle);

    double getAngle(){return angle;}

    void setCornersMode(int value);

    int getCornersMode(){return cornersDrawMode;};

    void setInsertionPointSnapMode(int value);
    int getInsertionPointSnapMode(){return insertionPointSnapMode;};

    void setSnapToCornerArcCenter(bool b);
    bool isSnapToCornerArcCenter() {return snapToCornerArcCenter;};

protected:
    /**
     * should resulting rect be polyline or not
     */
    bool usePolyline;
    /**
     * flag that controls how to draw corners
     */
    int cornersDrawMode;
    /**
     * radius for rounded corners
     */
    double radius;
    /**
     * x value of bevel
     */
    double bevelX;
    /**
     * y value of bevel
     */
    double bevelY;
    /**
     * angle of rect's rotation
     */
    double angle;

    /**
     * flag that controls how to position rect relative to insertion point - may have different meanings in different actions
     */
    int insertionPointSnapMode;

    /**
     * flag that indicates that snap should be performed taking into consideration rounded corners (if true, snap is not for, say, corner,
     * but to center of nearest rounded corner arc
     */
    bool snapToCornerArcCenter;

    /**
     * Last point of snap during mouse move
     */
    RS_Vector lastSnapPoint;

    /**
     * built polyline for shape
     */
    RS_Polyline *resultingPolyline;

    void drawPreviewForPoint(RS_Vector &lastSnapPoint);
    void drawPreviewForLastPoint();
    void prepareCornersDrawMode(double &radiusX, double &radiusY, bool &drawComplex, bool &drawBulge) const;
    RS_Polyline* createPolylineByVertexes( RS_Vector bottomLeftCorner, RS_Vector bottomRightCorner,
                                           RS_Vector topRightCorner, RS_Vector topLeftCorner,
                                           bool drawBulge, bool drawComplex,
                                           double radiusX, double radiusY) const;
    virtual RS_Polyline *createPolyline(RS_Vector &snapPoint) const = 0;
    virtual void proceedMouseLeftButtonReleasedEvent(QMouseEvent *e) = 0;
    virtual void setMainStatus() = 0;
    virtual void processCommandValue(double value) = 0;
    virtual bool processCustomCommand(RS_CommandEvent *e, const QString &command, bool &toMainStatus) = 0;
    virtual void processCoordinateEvent(RS_CoordinateEvent *pEvent, RS_Vector vector, bool zero) = 0;
    virtual bool mayDrawPreview(QMouseEvent *pEvent);
    virtual void doAfterTrigger();
    virtual void processMouseEvent(QMouseEvent *e);
    virtual void doUpdateMouseButtonHints();
    void normalizeCorners(RS_Vector &bottomLeftCorner, RS_Vector &bottomRightCorner, RS_Vector &topRightCorner, RS_Vector &topLeftCorner) const;
    void stateUpdated(bool toMainStatus);
};

#endif // LC_ABSTRACTACTIONDRAWRECTANGLE_H