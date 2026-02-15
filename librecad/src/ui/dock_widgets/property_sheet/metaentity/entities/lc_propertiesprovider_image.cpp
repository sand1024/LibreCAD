/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * ********************************************************************************
 */

#include "lc_propertiesprovider_image.h"

#include <QFileDialog>

#include "lc_property_qstring_file_view.h"
#include "rs_image.h"
#include "rs_units.h"

void LC_PropertiesProviderImage::doFillEntitySpecificProperties(LC_PropertyContainer* cont, const QList<RS_Entity*>& list) {
    const auto contGeometry = createGeometrySection(cont);

    // fixme - prop -  add errors processing!

    // fixme - add to method???
    add<RS_Image>({"file", tr("File"), tr("Name of image file")},
                  [this](const LC_Property::Names& names, RS_Image* entity, LC_PropertyContainer* container,
                         QList<LC_PropertyAtomic*>* props) -> void {
                      auto* property = new LC_PropertyQString(container, false);
                      property->setNames(names);
                      props->push_back(property);

                      LC_PropertyViewDescriptor descriptor(LC_PropertyQStringFileView::VIEW_NAME);
                      descriptor.attributes[LC_PropertyQStringFileView::ATTR_FILE_MODE] = QFileDialog::FileMode::ExistingFile;

                      const QStringList filters({"Image files (*.png *.gif *.jpg *.jpeg *.svg *.bmp)"});
                      descriptor.attributes[LC_PropertyQStringFileView::ATTR_FILENAME_FILTERS] = filters;
                      // fixme - prop - image file - add file filters for image
                      property->setViewDescriptor(descriptor);

                      const auto valueStorage = new LC_EntityPropertyValueDelegate<QString, RS_Image>();
                      valueStorage->setup(entity, m_widget, [](const RS_Image* e) -> QString {
                                              QString value = e->getFile();
                                              return value;
                                          }, [](const QString& value, [[maybe_unused]] LC_PropertyChangeReason reason,
                                                RS_Image* e) -> void {
                                              e->setFile(value);
                                          }, [](const QString& v, const RS_Image* e) -> bool {
                                              return v == e->getFile();
                                          });
                      property->setValueStorage(valueStorage, true);
                  }, list, contGeometry);

    addVector<RS_Image>({"insert", tr("Insertion Point"), tr("Point of image insertion")}, [](const RS_Image* e) -> RS_Vector {
                            return e->getInsertionPoint();
                        }, [](const RS_Vector& v, RS_Image* e) -> void {
                            e->setInsertionPoint(v);
                        }, list, contGeometry);

    addLinearDistance<RS_Image>({"scale", tr("Scale"), tr("Scale factor for image")}, [](const RS_Image* e) -> double {
                                    return e->getUVector().magnitude();
                                }, [](double& v, RS_Image* l) -> void {
                                    // fixme - prop- update geometry
                                    // l->setRadius(v);
                                }, list, contGeometry);

    addWCSAngle<RS_Image>({"angle", tr("Angle"), tr("Image rotation angle")}, [](const RS_Image* e) -> double {
                              return e->getUVector().angle();
                          }, [](double& v, RS_Image* l) -> void {
                              // fixme - update geometry
                              // l->setAngle1(v);
                          }, list, contGeometry);

    addReadOnlyString<RS_Image>({"sizeX", tr("Width pixels"), tr("Width of image in pixels")}, [this](const RS_Image* e)-> QString {
        const double len = e->getData().size.getX();
        QString value = formatInt(len);
        return value;
    }, list, contGeometry);

    addReadOnlyString<RS_Image>({"sizeY", tr("Height pixels"), tr("Height of image in pixels")}, [this](const RS_Image* e)-> QString {
        const double len = e->getData().size.getY();
        QString value = formatInt(len);
        return value;
    }, list, contGeometry);

    addLinearDistance<RS_Image>({"width", tr("Width"), tr("Width of image")}, [](const RS_Image* e) -> double {
                                    return e->getImageWidth();
                                }, [](double& v, RS_Image* l) -> void {
                                    // fixme - prop- update geometry
                                    // l->setRadius(v);
                                }, list, contGeometry);

    addLinearDistance<RS_Image>({"height", tr("Height"), tr("Height of image")}, [](const RS_Image* e) -> double {
                                    return e->getImageHeight();
                                }, [](double& v, RS_Image* l) -> void {
                                    // fixme - prop- update geometry
                                    // l->setRadius(v);
                                }, list, contGeometry);

    addReadOnlyString<RS_Image>({"dpi", tr("DPI"), tr("Dots per inch for the image")}, [this](const RS_Image* e)-> QString {
        const double scale = e->getUVector().magnitude();
        const double dpi = RS_Units::scaleToDpi(scale, getFormatter()->getUnit());
        QString value = formatDouble(dpi);
        return value;
    }, list, contGeometry);
}
