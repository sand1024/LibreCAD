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

#ifndef RS_ACTIONDRAWLINEPARALLEL_H
#define RS_ACTIONDRAWLINEPARALLEL_H

#include "rs_previewactioninterface.h"

class RS_Vector;

/**
 * This action class can handle user events to draw parallel 
 * lines, arcs and circles.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawLineParallel : public RS_PreviewActionInterface {
	Q_OBJECT
private:
    enum Status {
        SetEntity,    /**< Choose original entity. */
		//SetDistance,  /**< Setting distance in the command line. */
		SetNumber     /**< Setting number in the command line. */
		//SetThrough     /**< Setting a point for the parallel to go through. */
	};
	
public:
    RS_ActionDrawLineParallel(RS_EntityContainer& container,
                              RS_GraphicView& graphicView);
	~RS_ActionDrawLineParallel() override;
	
	void trigger() override;
	
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void updateMouseButtonHints() override;
	
	void commandEvent(RS_CommandEvent* e) override;
		QStringList getAvailableCommands() override;
	
	void hideOptions() override;
	void showOptions() override;

	void updateMouseCursor() override;
//    void updateToolBar() override;

	double getDistance() const;

	void setDistance(double d);

	int getNumber() const;

	void setNumber(int n);

private:
    /** Closest parallel. */
    RS_Entity* parallel = nullptr;
    /** Distance of the parallel. */
    double distance = 0.;
	/** Number of parallels. */
    int number = 0;
	/** Coordinate of the mouse. */
	std::unique_ptr<RS_Vector> coord;
	/** Original entity. */
    RS_Entity* entity = nullptr;
	/**
	 * Commands
	 */
	/*
        QString cmdDistance;
        QString cmdDistance2;
        QString cmdDistance3;
	
        QString cmdNumber;
        QString cmdNumber2;
        QString cmdNumber3;
	*/
};

#endif
