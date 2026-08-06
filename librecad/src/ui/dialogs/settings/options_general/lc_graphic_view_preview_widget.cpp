/*******************************************************************************
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


#include "lc_graphic_view_preview_widget.h"

#include <QVBoxLayout>
#include "lc_crosshair.h"
#include "lc_cursoroverlayinfo.h"
#include "lc_dimstylepreviewgraphicview.h"
#include "lc_dimstylepreviewpanel.h"
#include "lc_grid_options.h"
#include "lc_highlight.h"
#include "lc_overlayentitiescontainer.h"
#include "lc_refarc.h"
#include "lc_refline.h"
#include "lc_refpoint.h"
#include "lc_settings_defaults.h"
#include "lc_settings_grid.h"
#include "lc_visual_snap_data.h"
#include "lc_visual_snap_manager.h"
#include "lc_visual_snap_options.h"
#include "lc_visual_snap_solution_visualizer.h"
#include "qc_applicationwindow.h"
#include "rs_grid.h"
#include "rs_layer.h"
#include "rs_overlaybox.h"
#include "rs_preview.h"
#include "rs_selection.h"

LC_GraphicViewPreviewWidget::LC_GraphicViewPreviewWidget(QWidget* parent) : QWidget(parent), m_overlayBoxOptions{std::make_unique<LC_OverlayBoxOptions>()},
                                                                            m_visualSnapOptions{std::make_unique<LC_VisualSnapOptions>()},
        m_infoCursorOverlayData{std::make_unique<LC_InfoCursorData>()} {
        auto* layout = new QVBoxLayout(this);
        const auto graphic = new RS_Graphic();
        graphic->initForNewDocument();

        const auto& appWindow = QC_ApplicationWindow::getAppWindow();
        auto actionContext = appWindow->getActionContext();

        const auto graphicTmp = new RS_Graphic();
        graphicTmp->initForNewDocument();

        m_previewView = LC_DimStylePreviewGraphicView::init(this, graphicTmp, RS2::EntityUnknown);
        // how it could be???
        m_previewView->setFocusPolicy(Qt::ClickFocus);
        m_previewView->hideNonZeroLayers();

        layout->setContentsMargins(0, 0, 0, 0);

        const auto previewToolbar = new LC_DimStylePreviewPanel(this);
        previewToolbar->setGraphicView(m_previewView);

        layout->addWidget(previewToolbar);
        layout->addWidget(m_previewView, 10);

        const auto viewport = m_previewView->getViewPort();
        viewport->moveRelativeZero(RS_Vector(20, 10));

        m_previewView->zoomAuto();

        m_preview = std::make_unique<RS_Preview>(m_previewView->getDocument(), m_previewView->getViewPort());
        m_highlight = std::make_unique<LC_Highlight>();

        m_previewView->getGraphic(true)->setAnglesBase(RS_Math::deg2rad(-30));
        connect(m_previewView, &RS_GraphicView::onSwitchToDefaultAction, [this](bool actionIsDefault, RS2::ActionType actionRtti, RS2::ActionType prevActionRtti)->void {
                updateLivePreview();
        });

        connect(m_previewView, &RS_GraphicView::currentActionChanged, [this](RS2::ActionType actionRtti)->void {
            if (actionRtti == RS2::ActionNone || actionRtti == RS2::ActionZoomPan) {
                updateLivePreview();
            }
        });
    }

LC_GraphicViewPreviewWidget::~LC_GraphicViewPreviewWidget() {
}

void LC_GraphicViewPreviewWidget::updateLivePreview() {
        m_overlayBoxOptions->loadSettings();
        m_previewView->loadSettings();
        if (m_currentPageTag== "draw.view.draft_marker") {
           m_previewView->getGraphicViewRenderer()->setDraftMode(true);
        }
        else if (m_currentPageTag == "draw.preview") {
            preparePreviewPreview();
        }
        else if (m_currentPageTag == "draw.snap.general") {
            previewSnapIndicator(false);
        }
         else if (m_currentPageTag == "draw.snap.angle") {
             previewSnapIndicator(true);
         }
        else if (m_currentPageTag.startsWith("draw.snap.visual")) {
            previewSnapVisual();
        }
        else if (m_currentPageTag == "draw.info_cursor") {
            previewInfoCursor();
        }
        else if (m_currentPageTag == "draw.input_assistant") {
            previewRelativePointInputAssistant();
        }
        else if (m_currentPageTag == "draw.grid.points") {

        }
        else if (m_currentPageTag == "draw.grid.lines") {

        }
        m_previewView->redraw(RS2::RedrawAll, true);
    }

    void LC_GraphicViewPreviewWidget::cleanupPreviewForContentCategory(const QString& tag) {
        m_previewView->setUpdatesEnabled(false);
        auto renderer = m_previewView->getGraphicViewRenderer();
        auto document = m_previewView->getDocument();
        auto viewport = m_previewView->getViewPort();
        if (tag == "draw.view.draft_marker") {
            m_previewView->getGraphicViewRenderer()->setDraftMode(false);
        }
        else if (tag == "draw.view.handles") {
            const RS_Selection s(document, m_previewView->getViewPort());
            s.selectAll(false);
        }
        else if (tag == "draw.view.selection") {
            m_previewView->clearOverlayDrawables(RS2::OverlayGraphics::OverlayEffects);
            const RS_Selection s(document, m_previewView->getViewPort());
            s.selectAll(false);
        }
        else if (tag == "draw.view.draft_marker") {
            renderer->setDraftMode(CFG_Appearance::o_DraftMode);
        }
        else if (tag == "draw.preview") {
            m_previewView->clearOverlayEntities(RS2::ActionPreviewEntity);
            m_previewView->clearOverlayDrawables(RS2::ActionPreviewEntity);
            m_preview->clear();
        }
        else if (tag == "draw.highlight") {
            m_previewView->setHighlightEntitiesOnHover(false);
            auto e = document->getNearestEntity(RS_Vector(20, 68));
            if (e != nullptr) {
                e->setHighlighted(false);
            }
        }
        else if (tag == "draw.snap.general") {
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->clearOverlayEntities(RS2::ActionPreviewEntity);
        }
        else if (tag == "draw.snap.angle") {
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->clearOverlayEntities(RS2::ActionPreviewEntity);
        }
        else if (tag.startsWith("draw.snap.visual")) {
            m_previewView->clearOverlayDrawables(RS2::ActionPreviewEntity);
            m_previewView->clearOverlayDrawables(RS2::PermanentHighlights);
        }
        if (tag == "draw.info_cursor") {
            m_previewView->clearOverlayEntities(RS2::InfoCursor);
            m_previewView->clearOverlayEntities(RS2::ActionPreviewEntity);
            m_previewView->clearOverlayDrawables(RS2::Snapper);
        }
        if (tag == "draw.input_assistant") {
            m_previewView->clearOverlayEntities(RS2::ActionPreviewEntity);
            m_previewView->hideRelativeInputWidget();
            m_preview->clear();
        }
        else if (tag == "draw.grid.points") {
            updateGrid(-1);
        }
        else if (tag == "draw.grid.lines") {
            updateGrid(-1);
        }
        m_currentPageTag = "";
        m_previewView->setUpdatesEnabled(true);
    }

void LC_GraphicViewPreviewWidget::updatePreviewForContentCategory(const QString& tag) {
        auto renderer = m_previewView->getGraphicViewRenderer();
        auto document = m_previewView->getDocument();
        auto viewport = m_previewView->getViewPort();
        if (tag == "draw.view.draft_marker") {
            renderer->setDraftMode(true);
        }
        else {
            if (tag == "draw.view.handles") {
                const RS_Selection s(document, viewport);
                s.selectAll(true);
            }
            else if (tag == "draw.view.selection") {
                m_overlayBoxOptions->loadSettings();

                auto* directOverlay = new RS_OverlayBox({0,50},{45,100}, m_overlayBoxOptions.get());
                m_previewView->addOverlayDrawable(directOverlay, RS2::OverlayGraphics::OverlayEffects);

                auto* invertedSelectionOverlay = new RS_OverlayBox({100,100},{55,50}, m_overlayBoxOptions.get());
                m_previewView->addOverlayDrawable(invertedSelectionOverlay, RS2::OverlayGraphics::OverlayEffects);
                const RS_Selection s(document, viewport);
                s.selectAll(true);
            }
            else if (tag == "draw.preview") {
                preparePreviewPreview();
            }
            else if (tag == "draw.highlight") {
                m_previewView->setHighlightEntitiesOnHover(true);
                auto e = document->getNearestEntity(RS_Vector(20, 68));
                if (e != nullptr) {
                    e->setHighlighted(true);
                    e->setUserDefVar("_fixedHighlight", "fixed");
                }
            }
            else if (tag == "draw.snap.general") {
                previewSnapIndicator(false);
            }
            else if (tag == "draw.snap.angle") {
                previewSnapIndicator(true);
            }
            else if (tag.startsWith("draw.snap.visual")) {
                previewSnapVisual();
            }
            else if (tag == "draw.info_cursor") {
                previewInfoCursor();
            }
            else if (tag == "draw.input_assistant") {
                previewRelativePointInputAssistant();
            }
            else if (tag == "draw.grid.points") {
                updateGrid(0);
            }
            else if (tag == "draw.grid.lines") {
                updateGrid(1);
            }
        }
        m_currentPageTag = tag;
        m_previewView->redraw(RS2::RedrawAll, true);
        QCoreApplication::processEvents();
    }

void LC_GraphicViewPreviewWidget::previewSnapIndicator(bool showAngleSnap) const {
        m_previewView->clearOverlayDrawables(RS2::Snapper);
        m_preview->clear();
        SnapIndicatorOptions m_snapIndicator;
        m_snapIndicator.loadSettings();

        if (m_snapIndicator.drawLines || m_snapIndicator.drawShape) {
            auto* crosshair = new LC_Crosshair({50, 50}, m_snapIndicator);
            crosshair->setShapesPen(m_snapIndicator.shape_Pen);

            m_previewView->addOverlayDrawable(crosshair, RS2::OverlayGraphics::Snapper);
        }

        int m_refPointMode = CFG_Appearance::o_RefPointType;
        const QString pdsizeStr = CFG_Appearance::o_RefPointSize;
        bool ok = false;
        double m_refPointSize = RS_Math::eval(pdsizeStr, &ok);
        if (!ok) {
            m_refPointSize = LC_DEFAULTS_PDSize;
        }

        RS_Vector basePoint(20,10);
        auto* point = new LC_RefPoint(this->m_preview.get(), basePoint, m_refPointSize, m_refPointMode);
        m_preview->addEntity(point);

        auto* refLine = new LC_RefLine(this->m_preview.get(), basePoint, RS_Vector(50,50));
        m_preview->addEntity(refLine);

        auto* pointSelected = new LC_RefPoint(this->m_preview.get(), RS_Vector(50,50), m_refPointSize, m_refPointMode);
        pointSelected->setHighlighted(true);
        m_preview->addEntity(pointSelected);
        auto viewport = m_previewView->getViewPort();
        RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
        container->clear();
        container->setOwner(false);

        // port from RS_PreviewActionInterface::previewSnapAngleMark - preview of relative Angle mark
        if (showAngleSnap) {
            double angle = basePoint.angleTo({50,50});

            double angleBase = m_previewView->getDocument()->getGraphic()->getAnglesBase();
            const int radiusInPixels = CFG_Appearance::o_AngleSnapMarkerSize;
            const int lineInPixels = radiusInPixels * 2; // todo - move to settings
            const double lineLength = viewport->toUcsDX(lineInPixels);
            const double angleZero = viewport->toWorldAngle(angleBase);
            const double correctedAngle = RS_Math::correctAnglePlusMinusPi(angle);
            if (LC_LineMath::isMeaningfulAngle(correctedAngle)) {
                const double radius = viewport->toUcsDX(radiusInPixels);
                bool isAnglesCounterClockWise = CFG_Defaults::o_AnglesCounterClockwise;
                const auto* arc = new LC_RefArc(m_preview.get(), RS_ArcData(basePoint, radius, angleZero, correctedAngle, !isAnglesCounterClockWise));
                m_preview->addEntity(arc);
                auto* line = new LC_RefLine(this->m_preview.get(), basePoint, basePoint + RS_Vector::polar(lineLength, correctedAngle));
                m_preview->addEntity(line);
            }
            auto* line = new LC_RefLine(this->m_preview.get(), basePoint, basePoint.relative(lineLength, angleZero));
            m_preview->addEntity(line);
        }

        m_preview->calculateBorders();
        container->addEntity(m_preview.get());
        m_preview->addReferenceEntitiesToContainer(container);
    }

void LC_GraphicViewPreviewWidget::updateGrid(int forcedType) {
    auto viewport = m_previewView->getViewPort();
    auto rsGrid = viewport->getGrid();
    auto gridOptions = rsGrid->getOptions();
    gridOptions->forcedGridType = forcedType;
    gridOptions->loadSettings();
    rsGrid->invalidate(true);
    m_previewView->redraw(RS2::RedrawGrid, true);
}

void LC_GraphicViewPreviewWidget::preparePreviewPreview() {
        m_preview->loadSettings();
        int m_refPointMode = CFG_Appearance::o_RefPointType;
        const QString pdsizeStr = CFG_Appearance::o_RefPointSize;

        bool ok = false;
        double m_refPointSize = RS_Math::eval(pdsizeStr, &ok);
        if (!ok) {
            m_refPointSize = LC_DEFAULTS_PDSize;
        }

        m_preview->clear();

        auto viewport = m_previewView->getViewPort();
        RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
        container->clear();
        container->setOwner(false);

        QList<RS_Entity*> entitiesList;

        m_previewView->getDocument()->collect(entitiesList, [](RS_Entity* e) -> bool{
            const auto layerName = e->getLayerResolved()->getName();
            const bool isZero = layerName == "0";
            bool result = isZero && e->isAtomic();
            return result;
        });

        m_preview->addClonesFromList(entitiesList);
        m_preview->move(RS_Vector(30,20));

        if (CFG_Appearance::o_VisualizePreviewRefPoints) {
            auto* point = new LC_RefPoint(this->m_preview.get(), RS_Vector(50,30), m_refPointSize, m_refPointMode);
            m_preview->addEntity(point);

            auto* refLine = new LC_RefLine(this->m_preview.get(), RS_Vector(20,10), RS_Vector(50,30));
            m_preview->addEntity(refLine);

            auto* pointSelected = new LC_RefPoint(this->m_preview.get(), RS_Vector(20,10), m_refPointSize, m_refPointMode);
            pointSelected->setHighlighted(true);
            m_preview->addEntity(pointSelected);
        }

        m_preview->calculateBorders();
        container->addEntity(m_preview.get());
        m_preview->addReferenceEntitiesToContainer(container);
    }

    void LC_GraphicViewPreviewWidget::previewSnapVisual() {
        m_preview->clear();
        m_highlight->clear();

        m_previewView->clearOverlayDrawables(RS2::ActionPreviewEntity);
        m_previewView->clearOverlayDrawables(RS2::PermanentHighlights);

        RS_EntityContainer* container = m_previewView->getViewPort()->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
        container->setOwner(false);
        container->clear();
        m_visualSnapOptions->load();
        prepareVisualSnapSolution();
        LC_VisualSnapSolutionVisualizer visualizer(nullptr, m_visualSnapOptions.get());
        visualizer.setViewport(m_previewView->getViewPort());

        visualizer.visualizeSolution(m_preview.get(), m_highlight.get(), *m_solution);

        m_preview->calculateBorders();
        container->addEntity(m_preview.get());
        m_preview->addReferenceEntitiesToContainer(container);
    }

    namespace {
        class PreviewSnapConfigProvider : public LC_CurrentSnapConfigProviderInterface {
        public:
            PreviewSnapConfigProvider() {
                m_snapMode.snapCenter=true;
                m_snapMode.snapIntersection=true;
                m_snapMode.snapEndpoint=true;
                m_snapMode.snapDistance=true;
                m_snapMode.snapMiddle=true;
                m_snapMode.snapOnEntity=true;
                m_snapMode.snapGrid=true;
                m_snapMode.snapVisual=true;
            };

            ~PreviewSnapConfigProvider() override = default;

            double getAngleStep() override {
                const int stepType = CFG_Defaults::o_AngleSnapStep;
                double snapStepDegrees;
                switch (stepType) {
                    case 0:
                        snapStepDegrees = 1.0;
                        break;
                    case 1:
                        snapStepDegrees = 3.0;
                        break;
                    case 2:
                        snapStepDegrees = 5.0;
                        break;
                    case 3:
                        snapStepDegrees = 10.0;
                        break;
                    case 4:
                        snapStepDegrees = 15.0;
                        break;
                    case 5:
                        snapStepDegrees = 18.0;
                        break;
                    case 6:
                        snapStepDegrees = 22.5;
                        break;
                    case 7:
                        snapStepDegrees = 30.0;
                        break;
                    case 8:
                        snapStepDegrees = 45.0;
                        break;
                    case 9:
                        snapStepDegrees = 90.0;
                        break;
                    default:
                        snapStepDegrees = 15.0;
                        break;
                }
                double result = RS_Math::deg2rad(snapStepDegrees);
                return result;
            }

            RS_SnapMode* getSnapMode() override {return &m_snapMode;}
            double getSnapDistance() override {return 10.0;};
            int getSnapMiddlePoints() override {return 1;};
            RS_Vector getRelativeZero() override {return RS_Vector(20, 10);};
        private:
            RS_SnapMode m_snapMode;
        };

        class PreviewVisualSnapSolutionSolver : public LC_VisualSnapSolutionSolverBase {
        public:
            PreviewVisualSnapSolutionSolver(PreviewSnapConfigProvider* configProvider,
                                            LC_VisualSnapOptions* options) : LC_VisualSnapSolutionSolverBase(configProvider, options) {
            }
        };
    }

void LC_GraphicViewPreviewWidget::addDocumentEntityToVisualSnap(RS_Entity* entity1) {
    const auto docEntity = static_cast<RS_Line*>(entity1);
    const auto start = docEntity->getStartpoint();
    const auto end = docEntity->getEndpoint();
    const unsigned long long entityId = docEntity->getId();
    const auto snapEntity = new LC_RefSnapLine(start, end);
    snapEntity->setOriginalId(entityId);
    const auto docViewEntity = new LC_RefSnapLine(start, end);
    docViewEntity->setPen(docEntity->getPen(true));
    docViewEntity->setOriginalId(entityId);
    docViewEntity->setFlag(RS2::FlagInVisualSnap);
    m_solution->snapData->storeEntityRef(snapEntity, docViewEntity, entity1->getId());
    auto vertexStart = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, start, entity1);
    m_solution->snapData->storeVertexRef(vertexStart);
    auto vertexEnd = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, end, entity1);
    m_solution->snapData->storeVertexRef(vertexEnd);
}

void LC_GraphicViewPreviewWidget::addEntityPointToVisualSnap(RS_Entity* entity1) {
    auto vertexStart = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, entity1->getStartpoint(), entity1);
    m_solution->snapData->storeVertexRef(vertexStart);
}

    void LC_GraphicViewPreviewWidget::prepareVisualSnapSolution() {
        m_solution = std::make_unique<LC_VisualSnapSolution>();

        m_solution->snapData = m_previewView->getVisualSnapData();
        m_solution->snapData->clear();

        auto entity1 = m_previewView->getNearestEntity({33,43});
        addDocumentEntityToVisualSnap(entity1);

        auto entity2 = m_previewView->getNearestEntity({45,16.8});
        addDocumentEntityToVisualSnap(entity2);

        PreviewSnapConfigProvider snapConfigProvider;
        PreviewVisualSnapSolutionSolver solver(&snapConfigProvider, m_visualSnapOptions.get());
        auto viewport = m_previewView->getViewPort();
        solver.setViewport(viewport);
        const double graphSnapRange = viewport->toUcsDX(m_visualSnapOptions->guidingEntitiesSnapDistance);
        const double snapRangeToUse = graphSnapRange;
        solver.setSnapRange(snapRangeToUse);

        solver.solveVisualSnap({55.12, 43.36},*m_solution);
        auto vertex = new LC_VisualSnapVertex(RS2::SnapType::FREE, {55.12, 43.36}, nullptr);
        vertex->flagHighlighted = true;
        m_solution->snapData->storeVertexRef(vertex);

    }

void LC_GraphicViewPreviewWidget::previewInfoCursor() {
    auto prefs = m_previewView->getInfoCursorOverlayPreferences();
    auto m_viewport = m_previewView->getViewPort();
    const auto overlayContainer = m_viewport->getOverlaysDrawablesContainer(RS2::InfoCursor);
    if ( prefs->enabled) {
        // fixme - this is not absolutely safe if someone put another cursor to overlay container! Rework later!!
        const auto entity = overlayContainer->first();
        auto* infoCursor = dynamic_cast<LC_OverlayInfoCursor*>(entity);
        if (infoCursor == nullptr) {
            infoCursor = new LC_OverlayInfoCursor({30, 100}, &prefs->options);
            overlayContainer->add(infoCursor);
        }
        else {
            infoCursor->setOptions(&prefs->options);
            infoCursor->setPos({30, 100});
        }
        prepareInfoCursor(prefs);
        infoCursor->setZonesData(m_infoCursorOverlayData.get());
    }
    else {
        overlayContainer->clear();
    }
}

void LC_GraphicViewPreviewWidget::prepareInfoCursor(LC_InfoCursorOverlayPrefs* prefs) {
    QString coordAbs = "";
    QString coordPolar = "";
    if (prefs != nullptr && (prefs->showAbsolutePosition || prefs->showRelativePositionDistAngle || prefs->showRelativePositionDeltas)) {

            const bool showLabels = prefs->showLabels;
            if (prefs->showAbsolutePosition) {
                const QString absX = QString(showLabels ? "X: " : "") + "49.6870";
                const QString absY = QString(showLabels ? "Y: " : "") + "99.7434";
                coordAbs = absX + (prefs->multiLine ? "\n" : showLabels ? " " : " , ") + absY;
            }

            const bool hasUCS = true;
            if (prefs->showAbsolutePositionWCS && hasUCS) {
                const QString absX = QString(showLabels ? "WX: " : "W") + "23.3493";
                const QString absY = QString(showLabels ? "WY: " : "") + "234.2390";

                const QString coordAbsWCS = absX + (prefs->multiLine ? "\n" : showLabels ? " " : " , ") + absY;

                if (coordAbs.isEmpty()) {
                    coordAbs = coordAbsWCS;
                }
                else {
                    coordAbs = coordAbs + "\n" + coordAbsWCS;
                }
            }

            RS_Vector relativeToUse;
            if (prefs->showRelativePositionDistAngle) {
                const QString lenStr = QString(showLabels ? tr("Dist: ") : "@ ") + "20.1046";
                const QString angleStr = QString(showLabels ? tr("Angle: ") : "< ") + "156.1"+ QChar(0xB0);

                coordPolar = lenStr + (prefs->multiLine ? "\n" : showLabels ? " " : " ") + angleStr;
            }
            if (prefs->showRelativePositionDeltas) {
                const QString lenStr = QString(showLabels ? tr("dX: ") : "@ ") + "-19.0509";
                const QString angleStr = QString(showLabels ? tr("dY: ") : "") + "-9.00812";

                const QString coordDeltas = lenStr + (prefs->multiLine ? "\n" : showLabels ? " " : " , ") + angleStr;
                if (coordPolar.isEmpty()) {
                    coordPolar = coordDeltas;
                }
                else {
                    coordPolar = coordPolar + "\n" + coordDeltas;
                }
            }

    }
    m_infoCursorOverlayData->setZone1(coordAbs);
    m_infoCursorOverlayData->setZone3(coordPolar);

    QString commandPrompt = "";
    if (prefs->showCommandPrompt) {
        QString prompt = "Zoom, pan or select entity";
        if (prefs->showCurrentActionName) {
           commandPrompt = "Default: " + prompt;
        }
        else {
            commandPrompt = prompt;
        }

        QString modifiersStr = "SHIFT:Scroll Horizontally / Select Contour | CTRL:Pan / Scroll Vertically / Select Child entities";

        if (!modifiersStr.isEmpty()) {
            commandPrompt = commandPrompt + "\n" + modifiersStr;
        }
    }
    m_infoCursorOverlayData->setZone4(commandPrompt);

    QString entityInfo = "";
    if (prefs->showSnapType) {
        entityInfo.append("Grid\n");

        if (prefs->showEntityInfoOnCatch) {
            entityInfo.append("LINE\nLayer:0\nLengh:75.2312\nAngle:0.00\n");
        }
        if (prefs->showEntityInfoOnCreation) {
            entityInfo.append("Entity to Create:\nLINE\nLayer:0\nLengh:75.2312\nAngle:0.00\n");
        }
        if (prefs->showEntityInfoOnModification) {
            entityInfo.append("Entity to Modify:\nLINE\nLayer:0\nLengh:75.2312\nAngle:0.00");
        }
    }
    m_infoCursorOverlayData->setZone2(entityInfo);


    m_previewView->clearOverlayDrawables(RS2::Snapper);
    m_preview->clear();
    SnapIndicatorOptions m_snapIndicator;
    m_snapIndicator.loadSettings();
    m_snapIndicator.drawLines = false;
    m_snapIndicator.drawShape = true;

    auto* crosshair = new LC_Crosshair({30, 100}, m_snapIndicator);
    crosshair->setShapesPen(m_snapIndicator.shape_Pen);
    m_previewView->addOverlayDrawable(crosshair, RS2::OverlayGraphics::Snapper);
}

void LC_GraphicViewPreviewWidget::previewRelativePointInputAssistant() {
    m_previewView->showRelativeInputWidget({50,50}, {20,10}, true, RS2::REL_POINT_LENGTH, true);
    int m_refPointMode = CFG_Appearance::o_RefPointType;
    const QString pdsizeStr = CFG_Appearance::o_RefPointSize;

    bool ok = false;
    double m_refPointSize = RS_Math::eval(pdsizeStr, &ok);
    if (!ok) {
        m_refPointSize = LC_DEFAULTS_PDSize;
    }

    auto viewport = m_previewView->getViewPort();
    RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
    container->clear();
    container->setOwner(false);

    m_preview->clear();

    auto* point = new LC_RefPoint(this->m_preview.get(), RS_Vector(20,10), m_refPointSize, m_refPointMode);
    m_preview->addEntity(point);

    auto* refLine = new LC_RefLine(this->m_preview.get(), RS_Vector(20,10), RS_Vector(50,50));
    m_preview->addEntity(refLine);

    auto* pointSelected = new LC_RefPoint(this->m_preview.get(), RS_Vector(50,50), m_refPointSize, m_refPointMode);
    m_preview->addEntity(pointSelected);
    pointSelected->setHighlighted(true);


    m_preview->calculateBorders();
    container->addEntity(m_preview.get());
    m_preview->addReferenceEntitiesToContainer(container);
}
