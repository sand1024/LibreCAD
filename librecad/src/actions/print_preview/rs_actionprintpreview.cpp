/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "rs_actionprintpreview.h"

#include <QMouseEvent>
#include<cmath>

#include "lc_graphicviewport.h"
#include "lc_printpreviewview.h"
#include "qg_printpreviewoptions.h"
#include "rs_coordinateevent.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_settings.h"
#include "rs_units.h"
#include "rs_vector.h"

class LC_PrintPreviewView;

struct RS_ActionPrintPreview::ActionData {
    RS_Vector v1{};
    RS_Vector v2{};
};

/**
 * Constructor.
 */
RS_ActionPrintPreview::RS_ActionPrintPreview(LC_ActionContext *actionContext)
    :RS_ActionInterface("Print Preview", actionContext, RS2::ActionFilePrintPreview)
    , m_actionData(std::make_unique<ActionData>()){

    const bool fixed = LC_GET_ONE_BOOL("PrintPreview", "PrintScaleFixed");

    if (!fixed) {
        fit();
        updateOptions();
    }
    setPaperScaleFixed(fixed);
}

RS_ActionPrintPreview::~RS_ActionPrintPreview()=default;

void RS_ActionPrintPreview::init(const int status) {
    RS_ActionInterface::init(status);
}

void RS_ActionPrintPreview::invokeSettingsDialog() const {
    if (m_graphic) {
        // fixme - sand - Actually, relevant settings there is just page setup and whole drawing options are ovekill.
        // fixme - sand - rework this with proper layouts support!!!
        // fixme - change to LC_AppWindowDialogsInvoker::requestOptionsDrawingDialog
        RS_DIALOGFACTORY->requestOptionsDrawingDialog(*m_graphic);
        updateCoordinateWidgetFormat();
        updateOptionsUI(QG_PrintPreviewOptions::MODE_UPDATE_ORIENTATION);
        zoomToPage();
    }
}

bool RS_ActionPrintPreview::isPortrait() const {
    bool landscape;
    m_graphic->getPaperFormat(&landscape);
    return !landscape;
}

void RS_ActionPrintPreview::setPaperOrientation(const bool portrait) const {
    bool landscape;
    const RS2::PaperFormat format = m_graphic->getPaperFormat(&landscape);
    if (landscape != !portrait) {
        m_graphic->setPaperFormat(format, !portrait);
        zoomToPage();
    }
}

void RS_ActionPrintPreview::mouseMoveEvent(QMouseEvent* e) {
    switch (getStatus()) {
        case Moving: {
            m_actionData->v2 = toGraph(e);
            // if Shift is pressed the paper moves only horizontally
            if (isShift(e)) {
                m_actionData->v2.y = m_actionData->v1.y;
            }
            // if Ctrl is pressed the paper moves only vertically
            if (isControl(e)) {
                m_actionData->v2.x = m_actionData->v1.x;
            }
            if (m_graphic) {
                const RS_Vector pinsbase = m_graphic->getPaperInsertionBase();
                const double scale = m_graphic->getPaperScale();
                m_graphic->setPaperInsertionBase(pinsbase - m_actionData->v2 * scale + m_actionData->v1 * scale);

#ifdef DEBUG_PAPER_INSERTION_BASE
                const RS_Vector &pib = graphic->getPaperInsertionBase();
                LC_ERR << "PIB:" <<  pib.x << " , " << pib.y;
#endif
            }
            m_actionData->v1 = m_actionData->v2;
            m_graphicView->redraw(RS2::RedrawGrid); // DRAW Grid also draws paper, background items
            break;
        }
        default:
            break;
    }
}

void RS_ActionPrintPreview::mousePressEvent(QMouseEvent* e) {
    if (e->button()==Qt::LeftButton) {
        switch (getStatus()) {
        case Neutral: {
            m_actionData->v1 = toGraph(e);
            setStatus(Moving);
            break;
        }
        default:
            break;
        }
    }
}

void RS_ActionPrintPreview::mouseReleaseEvent(QMouseEvent* e) {
    switch (getStatus()) {
        case Moving:
            setStatus(Neutral);
            break;

        default:
            e->accept();
            break;
    }
}

void RS_ActionPrintPreview::onCoordinateEvent( [[maybe_unused]]int status,  [[maybe_unused]]bool isZero, const RS_Vector &pos) {
    const RS_Vector pinsbase = m_graphic->getPaperInsertionBase();
    RS_Vector mouse = pos;
    //    qDebug()<<"coordinateEvent= ("<<mouse.x<<", "<<mouse.y<<")";

    if(m_bPaperOffset) {
        commandMessage(tr("Printout offset in paper coordinates by (%1, %2)").arg(mouse.x).arg(mouse.y));
        mouse *= m_graphic->getPaperScale();
    }else {
        commandMessage(tr("Printout offset in graph coordinates by (%1, %2)").arg(mouse.x).arg(mouse.y));
    }

    //    RS_DIALOGFACTORY->commandMessage(tr("old insertion base (%1, %2)").arg(pinsbase.x).arg(pinsbase.y));
    //    RS_DIALOGFACTORY->commandMessage(tr("new insertion base (%1, %2)").arg((pinsbase-mouse).x).arg((pinsbase-mouse).y));

    m_graphic->setPaperInsertionBase(pinsbase-mouse);
    m_graphicView->redraw(RS2::RedrawGrid); // DRAW Grid also draws paper, background items
}

bool RS_ActionPrintPreview::doProcessCommand( [[maybe_unused]]int status, const QString &command) {
    bool accept = true;
    //    qDebug()<<"cmd="<<c;
    if (checkCommand("blackwhite", command)) {
        setBlackWhite(true);
        commandMessage(tr("Printout in Black/White"));
        updateOptions();
    } else if (checkCommand("color", command)) {
        setBlackWhite(false);
        commandMessage(tr("Printout in color"));
        updateOptions();
    } else if (checkCommand("graphoffset", command)) {
        m_bPaperOffset=false;
        commandMessage(tr("Printout offset in graph coordinates"));
        updateOptions();
    } else if (checkCommand("paperoffset", command)) {
        m_bPaperOffset=true;
        commandMessage(tr("Printout offset in paper coordinates"));
        updateOptions();
    }
    else{
        //coordinate event
        if (command.contains(',')){
            QString coord = command;
            if(command.startsWith('@')) {
                commandMessage(tr("Printout offset ignores relative zero. Ignoring '@'"));
                coord.remove(0, 1);
            }
            //        qDebug()<<"offset by absolute coordinate: ";

            const int commaPos = coord.indexOf(',');
            bool ok1 = false, ok2 = false;
            const double x = RS_Math::eval(coord.left(commaPos), &ok1);
            const double y = RS_Math::eval(coord.mid(commaPos+1), &ok2);
            if (ok1 && ok2) {
                RS_CoordinateEvent ce(RS_Vector(x,y));
                coordinateEvent(&ce);
            }
            else{
                accept = false;
            }
        }
        else{
            accept = false;
        }
    }
    return accept;
}

QString RS_ActionPrintPreview::getAdditionalHelpMessage() {
    return tr(": select printout offset coordinates")+
           "\n"+tr("type in offset from command line to offset printout");
}

QStringList RS_ActionPrintPreview::getAvailableCommands() {
    QStringList cmd;
    cmd +=command("blackwhite");
    cmd +=command("color");
    cmd +=command("graphoffset");
    cmd +=command("paperoffset");
    cmd +=command("help");
    return cmd;
}

void RS_ActionPrintPreview::resume() {
    RS_ActionInterface::resume();
}

//printout warning in command widget
void RS_ActionPrintPreview::printWarning(const QString& s) const {
    commandMessage(s);
}

RS2::CursorType RS_ActionPrintPreview::doGetMouseCursor([[maybe_unused]] const int status){
    switch (status) {
        case Moving:
            return RS2::ClosedHandCursor;
        default:
            return RS2::OpenHandCursor;
    }
}

void RS_ActionPrintPreview::center() const {
    if (m_graphic) {
        m_graphic->centerToPage();
        m_viewport->zoomPage();
    }
}

void RS_ActionPrintPreview::zoomToPage() const {
    if (m_graphic) {
        m_viewport->zoomPageEx();
    }
}

void RS_ActionPrintPreview::fit() const {
    if (m_graphic) {
        const RS_Vector paperSize=RS_Units::convert(m_graphic->getPaperSize(), getUnit(), RS2::Millimeter);

        if(std::abs(paperSize.x) < 10.|| std::abs(paperSize.y) < 10.) {
            printWarning("Warning:: Paper size less than 10mm."
                         " Paper is too small for fitting to page\n"
                         "Please set paper size by Menu: Options->Current Drawing Preferences->Paper");
        }
        //        double f0=graphic->getPaperScale();
        if (!m_graphic->fitToPage()) {
            commandMessage(tr("RS_ActionPrintPreview::fit(): Invalid paper size"));
        }
        else{
            m_graphic->setPagesNum(1,1);
            updateOptionsUI(QG_PrintPreviewOptions::MODE_UPDATE_PAGE_NUMBERS);
        }
        m_graphic->centerToPage();
        m_viewport->zoomPage();
        redraw();
    }
}

bool RS_ActionPrintPreview::setScale(const double newScale, const bool autoZoom) const {
    if (m_graphic != nullptr) {
        if(std::abs(newScale - m_graphic->getPaperScale()) < RS_TOLERANCE ) {
            return false;
        }

        auto pinBase = m_graphic->getPaperInsertionBase();
        const double oldScale = m_graphic->getPaperScale();

        m_graphic->setPaperScale(newScale);

        // changing scale around the drawing center
        // insertion base = center - 0.5 * size * scale
        // To keep the center position, the difference in insertion base is
        //   0.5 * size * (oldScale - newScale)
        pinBase += m_graphic->getSize()*(oldScale - newScale)*0.5;
        m_graphic->setPaperInsertionBase(pinBase);

        if(autoZoom) {
            zoomPageExWithBorder(100);
        }
        redraw();
        return true;
    }
    return false;
}

void RS_ActionPrintPreview::zoomPageExWithBorder(const int borderSize) const {
    const int bBottom = m_viewport->getBorderBottom();
    const int bTop = m_viewport->getBorderTop();
    const int bLeft = m_viewport->getBorderLeft();
    const int bRight = m_viewport->getBorderRight();
    // just a small usability improvement - we set additional borders on zoom to let the user
   // see that there might be drawing elements around paper
    m_viewport->setBorders(borderSize, borderSize, borderSize, borderSize);
    m_viewport->zoomPageEx();
    m_viewport->setBorders(bLeft, bTop, bRight, bBottom);
}

double RS_ActionPrintPreview::getScale() const{
    double ret = 1.0;
    if (m_graphic) {
        ret = m_graphic->getPaperScale();
    }
    return ret;
}

bool RS_ActionPrintPreview::isLineWidthScaling() const {
    return m_graphicView->getLineWidthScaling();
}

void RS_ActionPrintPreview::setLineWidthScaling(const bool state) const {
    m_graphicView->setLineWidthScaling(state);
    redraw();
}

bool RS_ActionPrintPreview::isBlackWhite() const {
    const LC_PrintPreviewView* printPreview = dynamic_cast<LC_PrintPreviewView *>(m_graphicView);
    if (printPreview != nullptr) {
        return printPreview->getDrawingMode() == RS2::ModeBW;
    }
    return false;
}

void RS_ActionPrintPreview::setBlackWhite(const bool bw) const {
    const auto* printPreview = dynamic_cast<LC_PrintPreviewView *>(m_graphicView);
    if (printPreview != nullptr) {
        if (bw) {
            printPreview->setDrawingMode(RS2::ModeBW);
        } else {
            printPreview->setDrawingMode(RS2::ModeFull);
        }
    }
}

RS2::Unit RS_ActionPrintPreview::getUnit() const {
    if (m_graphic) {
        return m_graphic->getUnit();
    }
    return RS2::None;
}

/** set paperscale fixed */
void RS_ActionPrintPreview::setPaperScaleFixed(const bool fixed) const {
    m_graphic->setPaperScaleFixed(fixed);
}

/** get paperscale fixed */
bool RS_ActionPrintPreview::isPaperScaleFixed() const {
    return m_graphic->getPaperScaleFixed();
}

/** calculate number of pages needed to contain a drawing */
void RS_ActionPrintPreview::calcPagesNum(const bool multiplePages) {
    if (m_graphic) {
        if (multiplePages) {
            const RS_Vector printArea = m_graphic->getPrintAreaSize(false);
            const RS_Vector graphicSize = m_graphic->getSize() * m_graphic->getPaperScale();
            const int pX = ceil(graphicSize.x / printArea.x);
            const int pY = ceil(graphicSize.y / printArea.y);

            if (pX > 99 || pY > 99) { // fixme - why such limit? Why hardcoded?
                commandMessage(tr("Limit of pages has been exceeded."));
                return;
            }

            m_graphic->setPagesNum(pX, pY);
            m_graphic->centerToPage();
            m_viewport->zoomPage();
        }
        else {
            m_graphic->setPagesNum(1, 1);
        }
        updateOptionsUI(QG_PrintPreviewOptions::MODE_UPDATE_PAGE_NUMBERS);
        updateOptions();
    }
}
// fixme - sand -  review and check why subtle rounding issues occur on some pages values
void RS_ActionPrintPreview::setPagesNumHorizontal(const int pagesCount) {
    const RS_Vector printArea = m_graphic->getPrintAreaSize(false);
    const RS_Vector graphicSize = m_graphic->getSize();
    const double paperScale = pagesCount * printArea.x / (graphicSize.x + 5);
    const int vertPagesCount = ceil(graphicSize.y * paperScale / printArea.y);
    m_graphic->setPagesNum(pagesCount, vertPagesCount);
    m_graphic->setPaperScale(paperScale);

//    zoomPageExWithBorder(100);
    m_graphic->centerToPage();
    m_viewport->zoomPage();
    updateOptionsUI(QG_PrintPreviewOptions::MODE_UPDATE_PAGE_NUMBERS);
    updateOptions();
}

void RS_ActionPrintPreview::setPagesNumVertical(const int pagesCount) {
    const RS_Vector printArea = m_graphic->getPrintAreaSize(false);
    const RS_Vector graphicSize = m_graphic->getSize();
    double paperScale = pagesCount * printArea.y / (graphicSize.y + 5);

    const int horPagesCount = ceil(graphicSize.x * paperScale / printArea.x);

    const double paperScaleHor = horPagesCount * printArea.x / graphicSize.x;

    paperScale = std::min(paperScaleHor, paperScale);

    m_graphic->setPagesNum(horPagesCount, pagesCount);
    m_graphic->setPaperScale(paperScale);

//    zoomPageExWithBorder(100);
    m_graphic->centerToPage();
    m_viewport->zoomPage();
    updateOptionsUI(QG_PrintPreviewOptions::MODE_UPDATE_PAGE_NUMBERS);
    updateOptions();
}

int RS_ActionPrintPreview::getPagesNumHorizontal() const {
    return m_graphic->getPagesNumHoriz();
}

int RS_ActionPrintPreview::getPagesNumVertical() const {
    return m_graphic->getPagesNumVert();
}
    void RS_ActionPrintPreview::updateMouseButtonHints() {
        updateMouseWidget(tr("Drag with Left Button to Position Paper or with Middle Button to Pan" ), "", MOD_SHIFT_AND_CTRL(tr("Move Horizontally"), tr("Move Vertically")));
    }

LC_ActionOptionsWidget* RS_ActionPrintPreview::createOptionsWidget() {
    return new QG_PrintPreviewOptions();
}
