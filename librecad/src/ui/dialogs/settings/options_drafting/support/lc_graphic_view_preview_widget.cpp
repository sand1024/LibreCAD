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
#include "lc_settings_manager_drafting.h"
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

namespace {
    class PreviewSnapConfigProvider : public LC_CurrentSnapConfigProviderInterface {
    public:
        PreviewSnapConfigProvider() {
            m_snapMode.snapCenter = true;
            m_snapMode.snapIntersection = true;
            m_snapMode.snapEndpoint = true;
            m_snapMode.snapDistance = true;
            m_snapMode.snapMiddle = true;
            m_snapMode.snapOnEntity = true;
            m_snapMode.snapGrid = true;
            m_snapMode.snapVisual = true;
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
            const double result = RS_Math::deg2rad(snapStepDegrees);
            return result;
        }

        RS_SnapMode* getSnapMode() override {
            return &m_snapMode;
        }

        double getSnapDistance() override {
            return 10.0;
        };

        int getSnapMiddlePoints() override {
            return 1;
        };

        RS_Vector getRelativeZero() override {
            return RS_Vector(20, 10);
        };

    private:
        RS_SnapMode m_snapMode;
    };

    class PreviewVisualSnapSolutionSolver : public LC_VisualSnapSolutionSolverBase {
    public:
        PreviewVisualSnapSolutionSolver(PreviewSnapConfigProvider* configProvider, LC_VisualSnapOptions* options) :
            LC_VisualSnapSolutionSolverBase(configProvider, options) {}
    };
}

LC_GraphicViewPreviewWidget::LC_GraphicViewPreviewWidget(QWidget* parent) : QWidget(parent),
                                                                            m_overlayBoxOptions{std::make_unique<LC_OverlayBoxOptions>()},
                                                                            m_visualSnapOptions{std::make_unique<LC_VisualSnapOptions>()},
                                                                            m_infoCursorOverlayData{std::make_unique<LC_InfoCursorData>()} {
    auto* layout = new QVBoxLayout(this);

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
    connect(m_previewView, &RS_GraphicView::onSwitchToDefaultAction,
            [this](bool actionIsDefault, [[maybe_unused]] RS2::ActionType actionRtti,
                   [[maybe_unused]] RS2::ActionType prevActionRtti) -> void { updateLivePreview(); });

    connect(m_previewView, &RS_GraphicView::currentActionChanged, [this](RS2::ActionType actionRtti)-> void {
        if (actionRtti == RS2::ActionNone || actionRtti == RS2::ActionZoomPan) {
            updateLivePreview();
        }
    });

    const auto previewContainer = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
    previewContainer->setOwner(false);
}

LC_GraphicViewPreviewWidget::~LC_GraphicViewPreviewWidget() {}

void LC_GraphicViewPreviewWidget::updateLivePreview() {
    doUpdatePreviewForContentCategory(m_currentPageTag, false);
}

void LC_GraphicViewPreviewWidget::drawPreview() {
    auto* viewport = m_previewView->getViewPort();
    RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
    container->clear();
    container->setOwner(false);
    m_preview->calculateBorders();
    container->addEntity(m_preview.get());
    m_preview->addReferenceEntitiesToContainer(container);
    m_previewView->redraw(RS2::RedrawOverlay);
    m_hasPreview = true;
}

void LC_GraphicViewPreviewWidget::clearPreview() {
    if (m_hasPreview) {
        //avoid deleting NULL or empty preview
        m_preview->clear();
        m_hasPreview = false;
    }
    if (!m_previewView->isCleanUp()) {
        auto* viewport = m_previewView->getViewPort();
        RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
        container->clear();

        LC_OverlayDrawablesContainer* drawablesContainer = viewport->getOverlaysDrawablesContainer(RS2::ActionPreviewEntity);
        drawablesContainer->clear();
    }
}

void LC_GraphicViewPreviewWidget::clearHighlights() const {
    m_highlight->clear();
    if (!m_previewView->isCleanUp()) {
        auto* viewport = m_previewView->getViewPort();
        RS_EntityContainer* overlayContainer = viewport->getOverlayEntitiesContainer(RS2::OverlayEffects);
        overlayContainer->clear();

        LC_OverlayDrawablesContainer* drawablesContainer = viewport->getOverlaysDrawablesContainer(RS2::OverlayEffects);
        drawablesContainer->clear();
    }
}

void LC_GraphicViewPreviewWidget::showOriginAndAxisLines(bool clear, bool enable) {
    auto* viewport = m_previewView->getViewPort();
    if (clear) {
        CFG_Appearance::o_AnglesBasisMarkEnabled = m_savedDrawAnglesBasis;
        CFG_Appearance::o_ShowWCSZeroMarker = m_savedShowWCSZeroMarker;
        CFG_Appearance::o_ShowUCSZeroMarker = m_savedShowUCSZeroMarker;
        CFG_Appearance::o_ShowAxisLines = m_savedShowAxisLines;

        viewport->applyUCS(&LC_WCS::instance);
   }
    else {
        m_savedDrawAnglesBasis = CFG_Appearance::o_AnglesBasisMarkEnabled;
        CFG_Appearance::o_AnglesBasisMarkEnabled = enable;

        m_savedShowUCSZeroMarker = CFG_Appearance::o_ShowUCSZeroMarker;
        CFG_Appearance::o_ShowUCSZeroMarker = enable;

        m_savedShowWCSZeroMarker = CFG_Appearance::o_ShowWCSZeroMarker;
        CFG_Appearance::o_ShowWCSZeroMarker = enable;

        m_savedShowAxisLines = CFG_Appearance::o_ShowAxisLines;
        CFG_Appearance::o_ShowAxisLines = enable;

        viewport->createUCS({50, 50}, RS_Math::deg2rad(30));
    }
    auto* renderer = m_previewView->getGraphicViewRenderer();
    renderer->loadSettings();
}

void LC_GraphicViewPreviewWidget::showDraftMode(bool enable) {
    auto* renderer = m_previewView->getGraphicViewRenderer();
    renderer->setDraftMode(enable);
}

void LC_GraphicViewPreviewWidget::doUpdatePreviewForContentCategory(const QString& tag, bool clear) {
    clearPreview();
    clearHighlights();

    m_overlayBoxOptions->loadSettings();
    m_previewView->loadSettings();

    auto* renderer = m_previewView->getGraphicViewRenderer();
    auto* document = m_previewView->getDocument();
    auto* viewport = m_previewView->getViewPort();

    m_previewView->setUpdatesEnabled(false);

    using namespace LC_SettingsPagesDrafting;

    if (tag == PAGE_DRAFTING_THEME_MAIN_COLORS) {
        if (clear) {
            m_previewView->setHighlightEntitiesOnHover(false);
            auto* e = document->getNearestEntity(RS_Vector(20, 68));
            if (e != nullptr) {
                e->setHighlighted(false);
            }
            showOriginAndAxisLines(true);
        }
        else {
            m_previewView->setHighlightEntitiesOnHover(true);
            auto* e = document->getNearestEntity(RS_Vector(20, 68));
            if (e != nullptr) {
                e->setHighlighted(true);
                e->setUserDefVar("_fixedHighlight", "fixed");
            }
            showOriginAndAxisLines(true);
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_DRAFT_MODE || tag == PAGE_DRAFTING_METRICS_DRAFT_MODE) {
        showDraftMode(!clear);
    }
    else if (tag == PAGE_DRAFTING_THEME_SELECTION) {
        m_previewView->clearOverlayDrawables(RS2::OverlayGraphics::OverlayEffects);
        m_previewView->clearOverlayDrawables(RS2::OverlayGraphics::Snapper);
        if (clear) {
            selectAll(document, viewport, false);
        }
        else {
            previewSelectionOverlayBoxes();
            selectAll(document, viewport);
            previewEntityPickBox();
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_PREVIEW) {
        if (clear) {
        }
        else {
            preparePreviewPreview();
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_GRID_POINTS) {
        if (clear) {
            updateGrid(-1);
        }
        else {
            updateGrid(0);
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_GRID_LINES) {
        if (clear) {
            updateGrid(-1);
        }
        else {
            updateGrid(1);
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_GRID) {
        if (clear) {
            updateGrid(-1);
        }
        else {
            updateGrid(CFG_Grid::o_GridType);
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_COORDINATE_SYSTEM) {
        if (clear) {
            showOriginAndAxisLines(clear);
        }
        else {
            showOriginAndAxisLines(false, true);
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_SNAP_INDICATOR) {
        if (clear) {
            m_previewView->clearOverlayDrawables(RS2::Snapper);
        }
        else {
            previewSnapIndicator(false, {50, 50});
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_SNAP_VISUAL) {
        if (clear) {
            m_highlight->clear();
        }
        else {
            previewSnapVisual();
        }
    }
    else if (tag == PAGE_DRAFTING_THEME_INFO_CURSOR) {
        if (clear) {
            m_previewView->clearOverlayDrawables(RS2::InfoCursor);
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->hideRelativeInputWidget();
        }
        else {
            // previewSnapIndicator(false, {30, 150});
            previewCompleteInfoCursor({30, 150});
            previewRelativePointInputAssistant();
        }
    }
    else if (tag == PAGE_DRAFTING_METRICS_MARKERS) {
        if (clear) {
            selectAll(document, viewport, false);
            auto* e = document->getNearestEntity(RS_Vector(20, 68));
            if (e != nullptr) {
                e->setHighlighted(false);
            }
            e->setSelectionFlag(false);
        }
        else {
            selectAll(document, viewport);
            preparePreviewPreview();
            auto* e = document->getNearestEntity(RS_Vector(20, 68));
            if (e != nullptr) {
                e->setHighlighted(true);
                e->setUserDefVar("_fixedHighlight", "fixed");
                e->setSelectionFlag(false);
            }
        }
    }
    else if (tag == PAGE_DRAFTING_METRICS_COORDINATE_SYSTEM) {
        if (clear) {
            showOriginAndAxisLines(clear);
        }
        else {
            showOriginAndAxisLines(false, true);
        }
    }
    else if (tag == PAGE_DRAFTING_METRICS_OVERLAYS) {
        if (clear) {
            m_previewView->clearOverlayDrawables(RS2::InfoCursor);
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->hideRelativeInputWidget();
        }
        else {
            auto* directOverlay = new RS_OverlayBox({0, 50}, {45, 100}, m_overlayBoxOptions.get());
            m_previewView->addOverlayDrawable(directOverlay, RS2::OverlayGraphics::OverlayEffects);
            // previewSnapIndicator(false, {30, 150});
            previewCompleteInfoCursor({30, 150});
            previewRelativePointInputAssistant();
        }
    }
    else if (tag == PAGE_DRAFTING_METRICS_SNAPPING) {
        if (clear) {
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->clearOverlayDrawables(RS2::OverlayGraphics::OverlayEffects);
        }
        else {
            previewSnapIndicator(true,{50, 50});
            previewEntityPickBox();
            previewSnapVisual();
        }
    }
    else if (tag == PAGE_DRAFTING_BEHAVIOR_INFO_CURSOR) {
        if (clear) {
            m_previewView->clearOverlayDrawables(RS2::InfoCursor);
            m_previewView->clearOverlayDrawables(RS2::Snapper);
            m_previewView->hideRelativeInputWidget();
        }
        else {
            previewActualInfoCursor({30, 100});
            previewSnapIndicator(true,{30, 100});
        }
    }

    if (clear) {
        m_currentPageTag = "";
    }
    else {
        m_currentPageTag = tag;
    }
    drawPreview();
    drawHighlights();
    m_previewView->setUpdatesEnabled(true);
    QTimer::singleShot(10, this, [this]{
        m_previewView->redraw(RS2::RedrawAll, true);
    });
}

void LC_GraphicViewPreviewWidget::updatePreviewForContentCategory(const QString& tag) {
    doUpdatePreviewForContentCategory(tag, false);
}

void LC_GraphicViewPreviewWidget::cleanupPreviewForContentCategory(const QString& tag) {
    doUpdatePreviewForContentCategory(tag, true);
}

void LC_GraphicViewPreviewWidget::previewSnapIndicator(bool showAngleSnap, const RS_Vector indicatorPosition) {
    m_previewView->clearOverlayDrawables(RS2::Snapper);
    m_preview->clear();
    SnapIndicatorOptions m_snapIndicator;
    m_snapIndicator.loadSettings();

    if (m_snapIndicator.drawLines || m_snapIndicator.drawShape) {
        auto* crosshair = new LC_Crosshair(indicatorPosition, m_snapIndicator, RS2::SnapType::INTERSECTION);
        crosshair->setShapesPen(m_snapIndicator.shape_Pen);
        m_previewView->addOverlayDrawable(crosshair, RS2::OverlayGraphics::Snapper);
    }

    const int m_refPointMode = CFG_Appearance::o_PreviewRefPointType;
    const int m_refPointSize = CFG_Appearance::o_PreviewRefPointSize;

    const RS_Vector basePoint(20, 10);
    const auto* point = new LC_RefPoint(this->m_preview.get(), basePoint, m_refPointSize, m_refPointMode);
    m_preview->addEntity(point);

    const auto* refLine = new LC_RefLine(this->m_preview.get(), basePoint, indicatorPosition);
    m_preview->addEntity(refLine);

    auto* pointSelected = new LC_RefPoint(this->m_preview.get(), indicatorPosition, m_refPointSize, m_refPointMode);
    pointSelected->setHighlighted(true);
    m_preview->addEntity(pointSelected);
    const auto viewport = m_previewView->getViewPort();


    // port from RS_PreviewActionInterface::previewSnapAngleMark - preview of relative Angle mark
    if (showAngleSnap) {
        const double angle = basePoint.angleTo(indicatorPosition);

        const double angleBase = m_previewView->getDocument()->getGraphic()->getAnglesBase();
        const int radiusInPixels = CFG_Appearance::o_AngleSnapMarkerSize;
        const int lineInPixels = radiusInPixels * 2; // todo - move to settings
        const double lineLength = viewport->toUcsDX(lineInPixels);
        const double angleZero = viewport->toWorldAngle(angleBase);
        const double correctedAngle = RS_Math::correctAnglePlusMinusPi(angle);
        if (LC_LineMath::isMeaningfulAngle(correctedAngle)) {
            const double radius = viewport->toUcsDX(radiusInPixels);
            const bool isAnglesCounterClockWise = CFG_Defaults::o_AnglesCounterClockwise;
            const auto* arc = new LC_RefArc(m_preview.get(),
                                            RS_ArcData(basePoint, radius, angleZero, correctedAngle, !isAnglesCounterClockWise));
            m_preview->addEntity(arc);
            const auto* line = new LC_RefLine(this->m_preview.get(), basePoint, basePoint + RS_Vector::polar(lineLength, correctedAngle));
            m_preview->addEntity(line);
        }
        const auto* line = new LC_RefLine(this->m_preview.get(), basePoint, basePoint.relative(lineLength, angleZero));
        m_preview->addEntity(line);
    }
}

void LC_GraphicViewPreviewWidget::selectAll(RS_Document* document, LC_GraphicViewport* viewport, bool enable) {
    const RS_Selection s(document, viewport);
    s.selectAll(enable);
}

void LC_GraphicViewPreviewWidget::previewSelectionOverlayBoxes() {
    m_overlayBoxOptions->loadSettings();

    auto* directOverlay = new RS_OverlayBox({0, 50}, {45, 100}, m_overlayBoxOptions.get());
    m_previewView->addOverlayDrawable(directOverlay, RS2::OverlayGraphics::OverlayEffects);

    auto* invertedSelectionOverlay = new RS_OverlayBox({100, 100}, {55, 50}, m_overlayBoxOptions.get());
    m_previewView->addOverlayDrawable(invertedSelectionOverlay, RS2::OverlayGraphics::OverlayEffects);
}

void LC_GraphicViewPreviewWidget::previewEntityPickBox() {
    SnapIndicatorOptions snapIndicatorOptions;
    snapIndicatorOptions.loadSettings();
    auto* crosshair = new LC_Crosshair({85, 35}, snapIndicatorOptions.lines_Pen, snapIndicatorOptions.pickBox_Pen,
                                       CFG_Snap::o_AdvSnapEntityCatchRange / 2, snapIndicatorOptions.pickBoxLinesSize,
                                       snapIndicatorOptions.lines_Type, snapIndicatorOptions.crossHairLinesPercent);

    m_previewView->addOverlayDrawable(crosshair, RS2::OverlayGraphics::Snapper);
}


void LC_GraphicViewPreviewWidget::updateGrid(int forcedType) const {
    const auto viewport = m_previewView->getViewPort();
    const auto rsGrid = viewport->getGrid();
    const auto gridOptions = rsGrid->getOptions();
    gridOptions->forcedGridType = forcedType;
    gridOptions->loadSettings();
    rsGrid->invalidate(true);
    m_previewView->redraw(RS2::RedrawGrid, true);
}

void LC_GraphicViewPreviewWidget::preparePreviewPreview() {
    m_preview->loadSettings();
    const int m_refPointMode = CFG_Appearance::o_PreviewRefPointType;
    const int m_refPointSize = CFG_Appearance::o_PreviewRefPointSize;

    m_preview->clear();

    QList<RS_Entity*> entitiesList;

    m_previewView->getDocument()->collect(entitiesList, [](RS_Entity* e) -> bool {
        const auto layerName = e->getLayerResolved()->getName();
        const bool isZero = layerName == "0";
        const bool result = isZero && e->isAtomic();
        return result;
    });

    m_preview->addClonesFromList(entitiesList);
    m_preview->move(RS_Vector(30, 20));

    if (CFG_Appearance::o_VisualizePreviewRefPoints) {
        auto* point = new LC_RefPoint(this->m_preview.get(), RS_Vector(50, 30), m_refPointSize, m_refPointMode);
        point->setHighlighted(true);
        m_preview->addEntity(point);

        const auto* refLine = new LC_RefLine(this->m_preview.get(), RS_Vector(20, 10), RS_Vector(50, 30));
        m_preview->addEntity(refLine);

        const auto* pointSelected = new LC_RefPoint(this->m_preview.get(), RS_Vector(20, 10), m_refPointSize, m_refPointMode);
        m_preview->addEntity(pointSelected);
    }
}

void LC_GraphicViewPreviewWidget::previewSnapVisual() {
    m_previewView->clearOverlayDrawables(RS2::PermanentHighlights);

    m_visualSnapOptions->load();

    m_visualSnapOptions->showNotSnappableGuides = false;
    // m_visualSnapOptions->createAngleStepRaysForEntitiesEndpoints = false;
    m_visualSnapOptions->createVertexVertexDistanceCirclesTangents = false;
    m_visualSnapOptions->createVertexVertexDistanceCirclesTangents = false;

    prepareVisualSnapSolution();
    LC_VisualSnapSolutionVisualizer visualizer(nullptr, m_visualSnapOptions.get());
    visualizer.setViewport(m_previewView->getViewPort());

    visualizer.visualizeSolution(m_preview.get(), m_highlight.get(), *m_solution);
}

void LC_GraphicViewPreviewWidget::drawHighlights() const {
    auto viewport = m_previewView->getViewPort();
    RS_EntityContainer* overlayContainer = viewport->getOverlayEntitiesContainer(RS2::OverlayEffects);
    overlayContainer->clear();
    overlayContainer->setOwner(false);
    m_highlight->addEntitiesToContainer(overlayContainer);
    m_previewView->redraw(RS2::RedrawOverlay);
}

void LC_GraphicViewPreviewWidget::addDocumentEntityToVisualSnap(RS_Entity* entity1) const {
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
    const auto vertexStart = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, start, entity1);
    m_solution->snapData->storeVertexRef(vertexStart);
    const auto vertexEnd = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, end, entity1);
    m_solution->snapData->storeVertexRef(vertexEnd);
}

void LC_GraphicViewPreviewWidget::addEntityPointToVisualSnap(RS_Entity* entity1) const {
    const auto vertexStart = new LC_VisualSnapVertex(RS2::SnapType::ENDPOINT, entity1->getStartpoint(), entity1);
    m_solution->snapData->storeVertexRef(vertexStart);
}

void LC_GraphicViewPreviewWidget::prepareVisualSnapSolution() {
    m_solution = std::make_unique<LC_VisualSnapSolution>();

    m_solution->snapData = m_previewView->getVisualSnapData();
    m_solution->snapData->clear();

    const auto entity1 = m_previewView->getNearestEntity({33, 43});
    addDocumentEntityToVisualSnap(entity1);

    const auto entity2 = m_previewView->getNearestEntity({45, 16.8});
    addEntityPointToVisualSnap(entity2);

    PreviewSnapConfigProvider snapConfigProvider;
    PreviewVisualSnapSolutionSolver solver(&snapConfigProvider, m_visualSnapOptions.get());
    const auto viewport = m_previewView->getViewPort();
    solver.setViewport(viewport);
    const double graphSnapRange = viewport->toUcsDX(m_visualSnapOptions->guidingEntitiesSnapDistance);
    const double snapRangeToUse = graphSnapRange;
    solver.setSnapRange(snapRangeToUse);

    // auto vertex = new LC_VisualSnapVertex(RS2::SnapType::FREE, {55.12, 43.36}, nullptr);
    // vertex->flagHighlighted = true;
    // m_solution->snapData->storeVertexRef(vertex);

    solver.solveVisualSnap({55.12, 43.36}, *m_solution);
}

void LC_GraphicViewPreviewWidget::previewCompleteInfoCursor(const RS_Vector& position) {
    const auto prefs = m_previewView->getInfoCursorOverlayPreferences();

    auto prefsCopy = prefs;
    prefsCopy->enabled = true;
    prefsCopy->showAbsolutePosition = true;
    prefsCopy->showAbsolutePositionWCS = true;
    prefsCopy->showRelativePositionDeltas = true;
    prefsCopy->showRelativePositionDistAngle = true;
    prefsCopy->showCommandPrompt = true;
    prefsCopy->showCurrentActionName = true;
    prefsCopy->showEntityInfoOnCatch = false;
    prefsCopy->showEntityInfoOnCreation = true;
    prefsCopy->showEntityInfoOnModification = false;
    prefsCopy->showSnapType = true;

    const auto m_viewport = m_previewView->getViewPort();
    const auto overlayContainer = m_viewport->getOverlaysDrawablesContainer(RS2::InfoCursor);

    const auto entity = overlayContainer->first();
    auto* infoCursor = dynamic_cast<LC_OverlayInfoCursor*>(entity);
    if (infoCursor == nullptr) {
        infoCursor = new LC_OverlayInfoCursor(position, &prefsCopy->options);
        overlayContainer->add(infoCursor);
    }
    else {
        infoCursor->setOptions(&prefsCopy->options);
        infoCursor->setPos(position);
    }
    prepareInfoCursor(prefsCopy, position);
    infoCursor->setZonesData(m_infoCursorOverlayData.get());
}

void LC_GraphicViewPreviewWidget::previewActualInfoCursor(const RS_Vector& position) {
    const auto prefs = m_previewView->getInfoCursorOverlayPreferences();
    const auto m_viewport = m_previewView->getViewPort();
    const auto overlayContainer = m_viewport->getOverlaysDrawablesContainer(RS2::InfoCursor);
    if (prefs->enabled) {
        // fixme - this is not absolutely safe if someone put another cursor to overlay container! Rework later!!
        const auto entity = overlayContainer->first();
        auto* infoCursor = dynamic_cast<LC_OverlayInfoCursor*>(entity);
        if (infoCursor == nullptr) {
            infoCursor = new LC_OverlayInfoCursor(position, &prefs->options);
            overlayContainer->add(infoCursor);
        }
        else {
            infoCursor->setOptions(&prefs->options);
            infoCursor->setPos(position);
        }
        prepareInfoCursor(prefs, position);
        infoCursor->setZonesData(m_infoCursorOverlayData.get());
    }
    else {
        overlayContainer->clear();
    }
}


void LC_GraphicViewPreviewWidget::prepareInfoCursor(LC_InfoCursorOverlayPrefs* prefs, const RS_Vector& position) const {
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
            const QString angleStr = QString(showLabels ? tr("Angle: ") : "< ") + "156.1" + QChar(0xB0);

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
            entityInfo.append("LINE\nLayer:0\nLength:75.2312\nAngle:0.00\n");
        }
        if (prefs->showEntityInfoOnCreation) {
            entityInfo.append("Entity to Create:\nLINE\nLayer:0\nLength:75.2312\nAngle:0.00\n");
        }
        if (prefs->showEntityInfoOnModification) {
            entityInfo.append("Entity to Modify:\nLINE\nLayer:0\nLength:75.2312\nAngle:0.00");
        }
    }
    m_infoCursorOverlayData->setZone2(entityInfo);

    m_previewView->clearOverlayDrawables(RS2::Snapper);
    m_preview->clear();
    SnapIndicatorOptions m_snapIndicator;
    m_snapIndicator.loadSettings();
    m_snapIndicator.drawLines = false;
    m_snapIndicator.drawShape = true;

    auto* crosshair = new LC_Crosshair(position, m_snapIndicator, RS2::SnapType::MIDDLE);
    crosshair->setShapesPen(m_snapIndicator.shape_Pen);
    m_previewView->addOverlayDrawable(crosshair, RS2::OverlayGraphics::Snapper);
}

void LC_GraphicViewPreviewWidget::previewRelativePointInputAssistant() {
    m_previewView->showRelativeInputWidget({50, 50}, {20, 10}, true, RS2::REL_POINT_LENGTH, true);
    const int m_refPointMode = CFG_Appearance::o_PreviewRefPointType;
    const int m_refPointSize = CFG_Appearance::o_PreviewRefPointSize;

    const auto viewport = m_previewView->getViewPort();
    RS_EntityContainer* container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);

    m_preview->clear();

    const auto* point = new LC_RefPoint(this->m_preview.get(), RS_Vector(20, 10), m_refPointSize, m_refPointMode);
    m_preview->addEntity(point);

    const auto* refLine = new LC_RefLine(this->m_preview.get(), RS_Vector(20, 10), RS_Vector(50, 50));
    m_preview->addEntity(refLine);

    auto* pointSelected = new LC_RefPoint(this->m_preview.get(), RS_Vector(50, 50), m_refPointSize, m_refPointMode);
    m_preview->addEntity(pointSelected);
    pointSelected->setHighlighted(true);
}
