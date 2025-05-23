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
#ifndef RS_ACTIONBLOCKSINSERT_H
#define RS_ACTIONBLOCKSINSERT_H

#include "rs_previewactioninterface.h"

class RS_Block;
struct RS_InsertData;

/**
 * This action class can handle user events for inserting blocks into the
 * current drawing.
 *
 * @author Andrew Mustun
 */
class RS_ActionBlocksInsert:public RS_PreviewActionInterface {
    Q_OBJECT
public:
    RS_ActionBlocksInsert(LC_ActionContext *actionContext);
    ~RS_ActionBlocksInsert() override;
    void init(int status) override;
    void reset();
    void trigger() override;
    QStringList getAvailableCommands() override;
    double getAngle() const;
    void setAngle(double a);
    double getFactor() const;
    void setFactor(double f);
    int getColumns() const;
    void setColumns(int c);
    int getRows() const;
    void setRows(int r);
    double getColumnSpacing() const;
    void setColumnSpacing(double cs);
    double getRowSpacing() const;
    void setRowSpacing(double rs);
protected:
    /**
 * Action States.
 */
    enum Status {
        SetUndefined = -1, /**< Setting undefined for initialisation. */
        SetTargetPoint = 0, /**< Setting the reference point. */
        SetAngle, /**< Setting angle in the command line. */
        SetFactor, /**< Setting factor in the command line. */
        SetColumns, /**< Setting columns in the command line. */
        SetRows, /**< Setting rows in the command line. */
        SetColumnSpacing, /**< Setting column spacing in the command line. */
        SetRowSpacing /**< Setting row spacing in the command line. */
    };

    RS_Block *m_block = nullptr;
    std::unique_ptr<RS_InsertData> m_data;
    /** Last status before entering option. */
    Status m_lastStatus = SetUndefined;
    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    bool doProcessCommand(int status, const QString &command) override;
    void onCoordinateEvent(int status, bool isZero, const RS_Vector &pos) override;
    LC_ActionOptionsWidget* createOptionsWidget() override;
    void updateMouseButtonHints() override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
};
#endif
