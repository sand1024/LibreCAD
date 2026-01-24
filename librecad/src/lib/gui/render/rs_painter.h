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


#ifndef RS_PAINTER_H
#define RS_PAINTER_H

#include <QPainter>

#include "lc_coordinates_mapper.h"
#include "lc_rect.h"
#include "rs_pen.h"

class RS_Arc;
class RS_Circle;
class RS_Color;
class RS_Ellipse;
class RS_Entity;
class RS_EntityContainer;
class RS_Pen;
class RS_Polyline;
class RS_Spline;

class QPainterPath;
class QRect;
class QRectF;
class QPolygon;
class QPolygonF;
class QImage;
class QBrush;
class QString;

class LC_GraphicViewport;
class LC_GraphicViewportRenderer;

struct LC_SplinePointsData;

/**
 * This class is a common interface for a painter class. Such
 * a class will in it's implementation be responsible to paint
 * lines, arcs, ... in widgets. All angles in rad.
 *
 * Note that this is just an interface used as a slot to
 * communicate with the LibreCAD from a GUI level. This
 * does not contain any Qt or platform specific code.
 */
class RS_Painter: public QPainter, LC_CoordinatesMapper {
public:
    explicit RS_Painter(QPaintDevice* pd);
    ~RS_Painter() = default;

    enum ArcRenderHint{
        FULL_IN_VIEW,
        ARC_IN_VIEW,
        SEGMENT,
        ANY
    };

    // coordinates translations
    RS_Vector toGui(const RS_Vector& worldCoordinates) const;
    QPointF toGuiPointF(const RS_Vector& worldCoordinates) const;
    void toGui(const RS_Vector& wcsCoordinate, double &uiX, double &uiY) const;
    double toGuiDX(double ucsDX) const;
    double toGuiDY(double ucsDY) const;
    QTransform getToGuiTransform() const;

    bool isPrinting() const
    {
        return m_printinMode;
    } // fixme - temporary support, refactor further
    bool isPrintPreview() const
    {
        return m_printPreview;
    } // fixme - temporary support, refactor further

    LC_GraphicViewport* getViewPort() const
    {
        return m_viewport;
    }
    void setViewPort(LC_GraphicViewport* v);
    void setRenderer(LC_GraphicViewportRenderer *r) {m_renderer = r;}
    void updateDashOffset(const RS_Entity* e);
    void clearDashOffset() {m_currenPatternOffset = 0.0;}
    double currentDashOffset() const {return m_currenPatternOffset;}

    void drawEntityArc(RS_Arc* arc);
    void drawEntityPolyline(const RS_Polyline *polyline);
    void drawEntityCircle(const RS_Circle* circle);

    int determinePointScreenSize(double pdsize) const;

    // methods called form entities to render them. All methods with WCS suffix - obtains world coordinates!
    void drawEllipseWCS(const RS_Vector &wcsCenter, double wcsMajorRadius, double ratio, double wcsAngleDegrees);

    void drawEllipseArcWCS(
        const RS_Vector &wcsCenter, double wcsMajorRadius, double ratio, double wcsAngleDegrees,
        double angle1Degrees, double angle2Degrees, double angularLength, bool reversed);

    void drawSplinePointsWCS(const std::vector<RS_Vector> &wcsControlPoints, bool closed);
    void drawCircleWCS(const RS_Vector &wcsCenter, double wcsRadius);
    void drawPointEntityWCS(const RS_Vector &wcsPos);
    void drawRefPointEntityWCS(const RS_Vector &wcsPos, int pdMode, double pdSize);
    void drawSolidWCS(const RS_Vector &wcsP0, const RS_Vector &wcsP1, const RS_Vector &wcsP2, const RS_Vector &wcsP3);
    void drawSolidWCS(const RS_VectorSolutions& wcsVertices);
    void drawFilledPolygonWCS(const RS_Vector& wcsV1, const RS_Vector& wcsV2, const RS_Vector& wcsV3,
                             const RS_Vector& wcsV4,const RS_Vector& wcsV5);
    void drawFilledCircleWCS(const RS_Vector& wcsCenter, double radius);
    void drawPolygonWCS(const RS_Vector& wcsV1, const RS_Vector& wcsV2, const RS_Vector& wcsV3,
                        const RS_Vector& wcsV4, const RS_Vector& wcsV5);
    void drawPolygonWCS(const std::vector<RS_Vector>& wcsPoints);

    void drawArcWCS(const RS_Vector &wcsCenter, double wcsRadius, double wcsStartAngleDegrees, double angularLength);
    void drawSplineWCS(const RS_Spline &spline);
    void drawLineWCS(const RS_Vector &wcsP1, const RS_Vector &wcsP2);
    void drawLineUIScaled(const QPointF& from, const QPointF& to, double lineWidthFactor);
    void drawLineWCSScaled(const RS_Vector& wcsP1, const RS_Vector& wcsP2, double lineWidthFactor);
    void drawPolylineWCS(const RS_Polyline *polyline);
    void drawHandleWCS(const RS_Vector &wcsPos, const RS_Color &c, QPointF& previousUIPosition, int size = -1);
    void drawImgWCS(const QImage &img, const RS_Vector &wcsInsertionPoint, const RS_Vector &uVector, const RS_Vector &vVector);

    // drawing in screen coordinates
    void drawCircleUI(const RS_Vector& uiCenter, double uiRadius);
    // just draws circle without trying to use any interpolations, used by overlays etc...
    void drawCircleUIDirect(const RS_Vector& uiPos, double uiRadius);
    void drawCircleUI(double uiCenterX, double uiCenterY, double uiRadius);
    void drawLineUISimple(double x1, double y1, double x2, double y2);
    void drawLineUISimple(const RS_Vector &p1, const RS_Vector &p2);
    void drawText(const QRect &uiRect, int flags, const QString &text, QRect *uiBoundingBox);
    void drawText(const QRect &rect, const QString &text, QRect *boundingBox);
    void drawRectUI(double uiX1, double uiY1, double uiX2, double uiY2);
    void drawPointEntityUI(const RS_Vector& uiPos, int pdmode, int pdsize);

    // methods invoked from entity containers and printing
    void drawEntity(RS_Entity* entity);
    void drawAsChild(RS_Entity* entity);
    void drawInfiniteWCS(const RS_Vector& startpoint, const RS_Vector& endpoint);

    /**
     * Sets the drawing mode.
     */
    void setDrawingMode(const RS2::DrawingMode m) {m_drawingMode = m;}

    // When set to true, only entities that are selected will be drawn
    void setDrawSelectedOnly(const bool dso) {m_drawSelectedEntities=dso;}

    // When true, only selected items will be draw
    bool shouldDrawSelected() const {return m_drawSelectedEntities;}
    /**
     * @return Current drawing mode.
     */
    RS2::DrawingMode getDrawingMode() const {return m_drawingMode;}
    void setPointsMode(const int pdMode){m_pointsMode = pdMode;}

    void drawGridPoint(const RS_Vector& p);
    void drawGridPoint(double x, double y);


    void fillRect(int x1, int y1, int w, int h, const RS_Color& col);
    void fillRect ( const QRectF & rectangle, const RS_Color & color );
    void fillRect ( const QRectF & rectangle, const QBrush & brush );

    void fillPolygonUI(const QPolygonF& uiPolygon);
    void fillTriangleUI(const RS_Vector& uiP1,const RS_Vector& uiP2,const RS_Vector& uiP3);
    void fillTriangleUI(double uiX1, double uiY1, double uiX2, double uiY2, double uiX3, double uiY3);
    void fillEllipseUI(const QPointF& point_f, double radiusX, double radiusY);

    void drawPath ( const QPainterPath & path);
    void fillPath ( const QPainterPath & path, const QBrush& brush);
//    void drawHandle(const RS_Vector& p, const RS_Color& c, int size=-1);

    RS_Pen getPen() const;
    void setPen(const RS_Pen& pen);
    void setPen(const RS_Color& color);
    void setPen(int r, int g, int b);
    void disablePen();
    void setBrushColor(const RS_Color& color);
    void erase();
    int getWidth() const;  // todo - sand - ucs - check usage!!! Probably it's different width expected (from viewport, rather than from device)
    int getHeight() const; // todo - sand - ucs - check usage!!! Probably it's different width expected (from viewport, rather than from device)
    double getDpmm() const;
    void setClipRect(int x, int y, int w, int h);
    void resetClipping();
    /**
     * @brief createSolidFillPath create QPainterPath for solid fill. The contour is expected to contain
     *          RS_EntityContainer only, with each container contains a closed edges, ordered by contour order:
     *          i.e., each edge's end point is coincident with the start point of its next neighbor. The end point
     *          of the last edge is coincident of the start point of the first edge.
     *          For closed edges (circles/ellipses), each loop container contains a single edge.
     *          TODO: self-intersection support.
     * @param contour
     * @return
     */
    QPainterPath createSolidFillPath(const RS_EntityContainer& contour) const;
    void noCapStyle();
    RS_Pen& getRsPen();
    void setPenJoinStyle(Qt::PenJoinStyle style);
    void setPenCapStyle(Qt::PenCapStyle style);
    void setMinCircleDrawingRadius(double val);
    void setMinArcDrawingRadius(double val);
    void setMinEllipseMajorRadius(double val);
    void setMinEllipseMinorRadius(double val);
    void setMinLineDrawingLen(double val);
    void setMinRenderableTextHeightInPx(int i);
    void setDefaultWidthFactor(const double factor){ m_defaultWidthFactor = factor;}
    void updatePointsScreenSize(double pdSize);

    bool isTextLineNotRenderable(double wcsLineHeight) const;

    void setRenderArcsInterpolate(const bool value){ m_arcRenderInterpolate = value;}
    void setRenderArcsInterpolationAngleFixed(const bool value){m_arcRenderInterpolationAngleFixed = value;}
    void setRenderArcsInterpolationAngleValue(const double val) {m_arcRenderInterpolationAngleValue = val;}
    void setRenderArcsInterpolationMaxSagitta(const double val) {m_arcRenderInterpolationMaxSagitta = val;}
    void setRenderCirclesSameAsArcs(const bool val) {m_circleRenderSameAsArcs = val;}

    void disableUCS();

    void setWorldBoundingRect(const LC_Rect &worldBoundingRect) {m_wcsBoundingRect = worldBoundingRect;}
    bool isFullyWithinBoundingRect(const RS_Entity* e) const;
    bool isFullyWithinBoundingRect(const LC_Rect &rect) const;

    const LC_Rect &getWcsBoundingRect() const;
    /**
     * @brief getMaximumArcSplineError - the maximum rendering error due to QPainter arc rendering by cubic spline approximation,
     *                                   for an arc of raidus 1, the maximum rendering error from approximating the and arc of 0
     *                                   to 90 degrees by a cubic spline with control points:
     *                                   (1, 0), (1, 4/3 (\sqrt 2 - 1)), (4/3 (\sqrt 2 - 1), 1), (0, 1)
     * @return - the QPainter implementation has the maximum error at 3e-4 for r=1
     */
    static constexpr double getMaximumArcSplineError() {
        // Issue #2035 : arc render precision
        // QPainter::arcTo() approximates an arc or radius=1, with angle from 0 to 90 degrees by a cubic spline with
        // 4 control points: (1, 0), (1, 4/3 (\sqrt 2 - 1)), (4/3 (\sqrt 2 - 1), 1), (0, 1)
        // The maximum approximation error is 3e-4
        return 3e-4;
    }

    static constexpr int getMaximumArcNonErrorRadius() {
        // fixme - sand - move to the setting??
        return 3000;
    }

    void drawEllipseBySplinePointsUI(const RS_Ellipse& ellipse, QPainterPath &path) const;


protected:
    /**
     * Current drawing mode.
     */
    RS2::DrawingMode m_drawingMode = RS2::ModeFull;
    // When set to true, only selected entities should be drawn
    bool m_drawSelectedEntities = false;

    RS_Pen m_lpen;
    long m_rememberX = 0; // Used for the moment because QPainter doesn't support moveTo anymore, thus we need to remember ourselves the moveTo positions
    long m_rememberY = 0;

    Qt::PenJoinStyle m_penJoinStyle = Qt::RoundJoin;
    Qt::PenCapStyle m_penCapStyle = Qt::RoundCap;
    QPen m_lastUsedPen;
    double m_cachedDpmm = 0.;
    double m_minCircleDrawingRadius = 2.0;
    double m_minArcDrawingRadius = 0.8;
    double m_minEllipseMajorRadius = 2.;
    double m_minEllipseMinorRadius = 1.;
    double m_minLineDrawingLen = 2;
    bool m_arcRenderInterpolate = false;
    bool m_arcRenderInterpolationAngleFixed = false;
    double m_arcRenderInterpolationAngleValue = M_PI/36;
    double m_arcRenderInterpolationMaxSagitta = 0.9;
    bool m_circleRenderSameAsArcs = false;

    double m_minRenderableTextHeightInPx = 1;
    double m_defaultWidthFactor = 1.0;

    double m_currenPatternOffset = 0.0;

    int m_screenPointsSize = 0;
    int m_pointsMode = 0;

    // cached factor and offset from viewport - for efficiency of coordinates translations.
    RS_Vector m_viewPortFactor{1., 1.};
    double& m_viewPortFactorX = m_viewPortFactor.x;
    double& m_viewPortFactorY = m_viewPortFactor.y;
    int m_viewPortOffsetX = 0;
    int m_viewPortOffsetY = 0;
    RS_Vector m_viewPortOffset;
    double m_viewPortHeight = 0.0;

    LC_Rect m_wcsBoundingRect;

    LC_GraphicViewportRenderer* m_renderer = nullptr;
    LC_GraphicViewport* m_viewport = nullptr;

//    void drawPolygonF(const QPolygonF &a, Qt::FillRule rule);
    void debugOutPath(const QPainterPath &tmpPath) const;
    double getDpmmCached() const {return m_cachedDpmm;}

    void drawArcEntity(RS_Arc* arc, QPainterPath &path);

    // painting in UI coordinates
    void drawEllipseUI(double uiCenterX, double uiCenterY, double uiRadiusMajor, double uiRadiusMinor, double uiAngleDegrees);
    void drawEllipseArcUI(double uiCenterX, double uiCenterY, double uiMajorRadius, double uiMinorRadius, double uiMajorAngleDegrees,
                          double angle1Degrees, double angle2Degrees, double angleLength, bool reversed);
    void drawEllipseUI(const RS_Vector& uiCenter, const RS_Vector& uiRadii, double uiAngleDegrees);
    void drawEllipseArcUI(const RS_Vector& uiCenter, const RS_Vector& uiRadii, double uiMajorAngleDegrees,
                           double angle1Degrees, double angle2Degrees, double angularLength, bool reversed);
    void drawSplinePointsUI(const std::vector<RS_Vector> &uiControlPoints, bool closed);
    void drawArcSplinePointsUI(const std::vector<RS_Vector> &uiControlPoints, QPainterPath &path);

    void drawArcEntityUI( double uiCenterX,double uiCenterY,double uiRadiusX,double uiRadiusY,double uiStartAngleDegrees,double angularLength);
    void drawArc(double uiCenterX, double uiCenterY, double uiRadiusX, double uiRadiusY,
                 double uiStartAngleDegrees, double angularLength, QPainterPath &path) const;
    void drawLineUI(double x1, double y1, double x2, double y2);
    void drawLineUI(const QPointF& startPoint, const QPointF& endPoint);
    void drawImgUI(const QImage& img, const RS_Vector& uiInsert, const RS_Vector& uVector, const RS_Vector& vVector, const RS_Vector& factor);

    void drawRectUI(const RS_Vector& p1, const RS_Vector& p2);


    void drawTextH(int x1, int y1, int x2, int y2,
                   const QString& text);
    void drawTextV(int x1, int y1, int x2, int y2,
                   const QString& text);

// fixme - sand, ucs - temporary, remove
    bool m_printinMode = false;
    bool m_printPreview = false;

    void drawArcInterpolatedByLines(const RS_Vector& uiCenter, double uiRadiusX, double uiStartAngleDegrees,
                                    double angularLength, QPainterPath &path) const;

    void drawArcQT(const RS_Vector& uiCenter, const RS_Vector& uiRadii, double uiStartAngleDegrees,
                   double angularLength, QPainterPath &path);

    void drawArcSegmentBySplinePointsUI(const RS_Vector& uiCenter, double uiRadiusX, double startAngleRad,
                                        double angularLengthRad, QPainterPath &path);
private:
    void addEllipseArcToPath(QPainterPath& localPath, const RS_Vector& uiRadii, double startAngleDeg, double angularLengthDeg, bool useSpline) const;
    // helper method: approximate a centered ellipse with lc_splinepoints
    void drawEllipseSegmentBySplinePointsUI(const RS_Vector& uiRadii, double startRad, double lenRad, QPainterPath &path, bool closed) const;
    void addSplinePointsToPath(const std::vector<RS_Vector> &uiControlPoints, bool closed, QPainterPath &path) const;
};

#endif
