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


#ifndef RS_GRAPHIC_H
#define RS_GRAPHIC_H

#include <QDateTime>
#include <memory>

#include "lc_dimstyle.h"
#include "lc_dimstyleslist.h"
#include "lc_textstylelist.h"
#include "lc_ucslist.h"
#include "lc_viewslist.h"
#include "rs_blocklist.h"
#include "rs_document.h"
#include "rs_layerlist.h"
#include "rs_variabledict.h"

class RS_Dimension;
class LC_DimStyleToVariablesMapper;
class LC_DimStylesList;
class QString;

class LC_View;
class QG_LayerWidget;

class LC_GraphicModificationListener {
public:
    virtual ~LC_GraphicModificationListener() = default;
    virtual void graphicModified(const RS_Graphic* g, bool modified) = 0;
    virtual void undoStateChanged(const RS_Graphic* g, bool undoAvailable, bool redoAvailable) = 0;
};

struct LC_MarginsRect {
    double left {0.0};
    double top {0.0};
    double bottom {0.0};
    double right {0.0};

    bool operator == (const LC_MarginsRect& v) const {
        return left == v.left && top == v.top && bottom == v.bottom;
    }

    [[nodiscard]] double getLeft() const {
        return left;
    }

    void setLeft(const double left) {
        this->left = left;
    }

    [[nodiscard]] double getTop() const {
        return top;
    }

    void setTop(const double top) {
        this->top = top;
    }

    [[nodiscard]] double getBottom() const {
        return bottom;
    }

    void setBottom(const double bottom) {
        this->bottom = bottom;
    }

    [[nodiscard]] double getRight() const {
        return right;
    }

    void setRight(const double right) {
        this->right = right;
    }
};


/**
 * A graphic document which can contain entities layers and blocks.
 *
 * @author Andrew Mustun
 */
// fixme - sand - refactor and extract entities, so it should be more natual to DXF.
// Paper-related things should be layouts, ui viewports should be also exposed explicitly ...
// At least "active" one, so viewport will be accessible via document, not via graphic view.
class RS_Graphic : public RS_Document {
public:
    explicit RS_Graphic(RS_EntityContainer* parent=nullptr);
    ~RS_Graphic() override;

    virtual void onLoadingCompleted();
    /** @return RS2::EntityGraphic */
    RS2::EntityType rtti() const override {return RS2::EntityGraphic;}

    virtual unsigned countLayerEntities(RS_Layer* layer) const;

    RS_LayerList* getLayerList() override {return &m_layerList;}
    RS_BlockList* getBlockList() override {return &m_blockList;}
    LC_ViewList* getViewList() override {return &m_namedViewsList;}
    LC_UCSList* getUCSList() override {return &m_ucsList;}
    LC_DimStylesList* getDimStyleList() override {return &m_dimstyleList;}
    LC_TextStyleList* getTextStyleList() override {return &m_textStyleList;}
    void addDimStyle(LC_DimStyle* style) {m_dimstyleList.addDimStyle(style);}
    void newDoc() override;
    // Wrappers for Layer functions:
    void clearLayers() {
        m_layerList.clear();
        validateSelection();
    }
    unsigned countLayers() const {return m_layerList.count();}
    RS_Layer* layerAt(const unsigned i) const {return m_layerList.at(i);}
    void activateLayer(const QString& name, const bool notify = false) {m_layerList.activate(name, notify);}
    void activateLayer(RS_Layer* layer, const bool notify = false) {m_layerList.activate(layer, notify);}
    RS_Layer* getActiveLayer() const {return m_layerList.getActive();}
    virtual void addLayer(RS_Layer* layer) {m_layerList.add(layer);}
    void addEntity(const RS_Entity* entity) override;
    void removeLayer(RS_Layer* layer);
    void editLayer(RS_Layer* layer, const RS_Layer& source) {m_layerList.edit(layer, source);}
    RS_Layer* findLayer(const QString& name) {return m_layerList.find(name);}
    void toggleLayer(const QString& name) {m_layerList.toggle(name);validateSelection();}
    void toggleLayer(RS_Layer* layer) {m_layerList.toggle(layer); validateSelection();}
    void toggleLayerLock(RS_Layer* layer) {m_layerList.toggleLock(layer); validateSelection();}
    void toggleLayerPrint(RS_Layer* layer) {m_layerList.togglePrint(layer);}
    void toggleLayerConstruction(RS_Layer* layer) {m_layerList.toggleConstruction(layer);}
    void freezeAllLayers(const bool freeze) {m_layerList.freezeAll(freeze);validateSelection();}
    void lockAllLayers(const bool lock) {m_layerList.lockAll(lock);validateSelection();}
    void toggleLockLayers(const QList<RS_Layer*>& layers){m_layerList.toggleLockMulti(layers);validateSelection();}
    void togglePrintLayers(const QList<RS_Layer*>& layers){m_layerList.togglePrintMulti(layers);validateSelection();}
    void toggleConstructionLayers(const QList<RS_Layer*>& layers){m_layerList.toggleConstructionMulti(layers);validateSelection();}
    void toggleFreezeLayers(const QList<RS_Layer*>& layers){m_layerList.toggleFreezeMulti(layers);validateSelection();}
    void setFreezeLayers(const QList<RS_Layer*>& layersEnable, const QList<RS_Layer*>& layersDisable){m_layerList.setFreezeMulti(layersEnable, layersDisable);validateSelection();}
    void setLockLayers(const QList<RS_Layer*>& layersToUnlock, const QList<RS_Layer*>& layersToLock){m_layerList.setLockMulti(layersToUnlock, layersToLock);validateSelection();}
    void setPrintLayers(const QList<RS_Layer*>& layersNoPrint, const QList<RS_Layer*>& layersPrint){m_layerList.setPrintMulti(layersNoPrint, layersPrint);validateSelection();}
    void setConstructionLayers(const QList<RS_Layer*>& layersNoConstruction, const QList<RS_Layer*>& layersConstruction){m_layerList.setConstructionMulti(layersNoConstruction, layersConstruction);validateSelection();}

    void addLayerListListener(RS_LayerListListener* listener) {m_layerList.addListener(listener);}
    void removeLayerListListener(RS_LayerListListener* listener) {m_layerList.removeListener(listener);}
    void addViewListListener(LC_ViewListListener* listener) { m_namedViewsList.addListener(listener);}
    void removeViewListListener(LC_ViewListListener* listener) { m_namedViewsList.removeListener(listener);}

    // Wrapper for block functions:
    void clearBlocks() {m_blockList.clear();}
    unsigned countBlocks() const {return m_blockList.count();}
    RS_Block* blockAt(const unsigned i) {return m_blockList.at(i);}
    void activateBlock(const QString& name) {m_blockList.activate(name);}
    void activateBlock(RS_Block* block) {m_blockList.activate(block);}
    RS_Block* getActiveBlock() const {return m_blockList.getActive();}
    bool addBlock(RS_Block* block, const bool notify=true) {return m_blockList.add(block, notify);}
    void addBlockNotification() {m_blockList.addNotification();}
    void removeBlock(RS_Block* block) {m_blockList.remove(block);}
    RS_Block* findBlock(const QString& name) {return m_blockList.find(name);}
    QString newBlockName() {return m_blockList.newName();}
    void toggleBlock(const QString& name) {m_blockList.toggle(name);}
    void toggleBlock(RS_Block* block) {m_blockList.toggle(block);}
    void freezeAllBlocks(const bool freeze) {m_blockList.freezeAll(freeze);}
    void addBlockListListener(RS_BlockListListener* listener) {m_blockList.addListener(listener);}
    void removeBlockListListener(RS_BlockListListener* listener) {m_blockList.removeListener(listener);}

        // Wrappers for variable functions:
    void clearVariables();
    QString getCustomProperty(const QString& key, const QString& defaultValue) const;
    void addCustomProperty(const QString& key, const QString& value);
    void removeCustomProperty(const QString& key);
    bool hasCustomProperty(const QString& key) const;
    const QHash<QString, RS_Variable>& getCustomProperties() const;
    int countVariables() const;

    void addVariable(const QString& key, const RS_Vector& value, int code);
    void addVariable(const QString& key, const QString& value, int code);
    void addVariable(const QString& key, int value, int code);
    void addVariable(const QString& key, bool value, int code);
    void addVariable(const QString& key, double value, int code);
    void removeVariable(const QString& key);

    QHash<QString, RS_Variable>& getVariableDict();
    RS_Vector getVariableVector(const QString& key, const RS_Vector& def) const;
    QString getVariableString(const QString& key, const QString& def) const;
    int getVariableInt(const QString& key, int def) const;
    bool getVariableBool(const QString& key, bool def) const;
    double getVariableDouble(const QString& key, double def) const;

    void setVariableDictObject(const RS_VariableDict& inputVariableDict) {m_variableDict = inputVariableDict;}

    RS_VariableDict getVariableDictObject() const{
        return m_variableDict;
    }

    RS_VariableDict* getVariableDictObjectRef() {
        return &m_variableDict;
    }

    RS2::LinearFormat getLinearFormat() const;
    void setLinearFormat(RS2::LinearFormat linearFormat);
    void replaceCustomVars(const QHash<QString, QString>& vars);
    virtual void prepareForSave();

    static RS2::LinearFormat convertLinearFormatDXF2LC(int f);
    int getLinearPrecision() const;
    void setLinearPrecision(int value);
    RS2::AngleFormat getAngleFormat() const;
    void setAngleFormat(RS2::AngleFormat angleFormat);
    int getAnglePrecision() const;
    void addAnglePrecision(int value);

    RS_Vector getPaperSize() const;
    void setPaperSize(const RS_Vector& s);
    RS_Vector getPrintAreaSize(bool total=true) const;

    RS_Vector getPaperInsertionBase() const;
    void setPaperInsertionBase(const RS_Vector& p);

    RS2::PaperFormat getPaperFormat(bool* landscape) const;
    void setPaperFormat(RS2::PaperFormat f, bool landscape);

    double getPaperScale() const;
    void setPaperScale(double s);

    void setUnit(RS2::Unit u);
    RS2::Unit getUnit() const;

    bool isGridOn() const;
    void setGridOn(bool on);
    bool isIsometricGrid() const;
    void setIsometricGrid(bool on);
    void setCurrentUCS(const LC_UCS* ucs);
    LC_UCS* getCurrentUCS() const;
    RS2::IsoGridViewType getIsoView() const;
    void setIsoView(RS2::IsoGridViewType viewType);
    void centerToPage();
    bool fitToPage();
    bool isBiggerThanPaper() const;
    /**
     * @retval true The document has been modified since it was last saved.
     * @retval false The document has not been modified since it was last saved.
     */
    bool isModified() const override;
    /**
     * Sets the documents modified status to 'm'.
     */
    void setModified(bool m) override;
    void markSaved(const QDateTime &lastSaveTime);

    QDateTime getLastSaveTime(){return m_lastSaveTime;}
    void setLastSaveTime(const QDateTime &time) { m_lastSaveTime = time;}

    //if set to true, will refuse to modify paper scale
    void setPaperScaleFixed(const bool fixed){m_paperScaleFixed=fixed;}
    bool getPaperScaleFixed() const{return m_paperScaleFixed;}

    /**
     * Paper margins in millimeters
     */
    void setMargins(const double left, const double top, const double right, const double bottom){
        if (left >= 0.0) {
            m_marginLeft = left;
        }
        if (top >= 0.0) {
            m_marginTop = top;
        }
        if (right >= 0.0) {
            m_marginRight = right;
        }
        if (bottom >= 0.0) {
            m_marginBottom = bottom;
        }
    }

    void setMargins(const LC_MarginsRect& margins) {
        setMargins(margins.left, margins.top, margins.right, margins.bottom);
    }

    double getMarginLeft() const{return m_marginLeft;}
    double getMarginTop() const{return m_marginTop;}
    double getMarginRight() const{ return m_marginRight;}
    double getMarginBottom() const{return m_marginBottom;}

    /**
     * Paper margins in graphic units
     */
    void setMarginsInUnits(double left, double top, double right, double bottom);
    void setMarginsInUnits(const LC_MarginsRect& margins);
    LC_MarginsRect getMarginsInUnits() const;
    double getMarginLeftInUnits() const;
    double getMarginTopInUnits() const;
    double getMarginRightInUnits() const;
    double getMarginBottomInUnits() const;
    /**
     * Number of pages drawing occupies
     */
    void setPagesNum(int horiz, int vert);
    void setPagesNum(const QString &horizXvert);
    int getPagesNumHoriz() const {return m_pagesNumH;}
    int getPagesNumVert() const {return m_pagesNumV;}
    friend std::ostream& operator << (std::ostream& os, RS_Graphic& g);
    int clean();
    LC_View *findNamedView(const QString& viewName) const {return m_namedViewsList.find(viewName);}
    LC_UCS *findNamedUCS(const QString& ucsName) const {return m_ucsList.find(ucsName);}
    void addNamedView(LC_View *view) {m_namedViewsList.add(view);}
    void addUCS(LC_UCS *ucs) {m_ucsList.add(ucs);}

    double getAnglesBase() const;
    void setAnglesBase(double baseAngle);
    bool areAnglesCounterClockWise() const;
    void setAnglesCounterClockwise(bool on);
    RS_Vector getUserGridSpacing() const;
    void setUserGridSpacing(const RS_Vector& spacing);
    QString formatAngle(double angle) const;
    QString formatLinear(double linear) const;

    RS2::FormatType getFormatType() const;

    void setFormatType(RS2::FormatType formatType);

    /**
    * @return File name of the document currently loaded.
    * Note, that the default file name is empty.
    */
    QString getFilename() const {return m_filename;}

    /**
     * @return Auto-save file name of the document currently loaded.
     */
    QString getAutoSaveFileName() const {return m_autosaveFilename;}

    /**
     * Sets file name for the document currently loaded.
     */
    void setFilename(QString fn) {m_filename = std::move(fn);}

    const QString &getAutosaveFilename() const;

    void setAutosaveFileName(const QString &fileName);

    void setModificationListener(LC_GraphicModificationListener * listener) {m_modificationListener = listener;}

    LC_DimStyle* getFallBackDimStyleFromVars() const;
    LC_DimStyle* getDimStyleByName(const QString &name, RS2::EntityType dimType = RS2::EntityUnknown) const;
    QString getDefaultDimStyleName() const;
    void setDefaultDimStyleName(const QString& name);
    LC_DimStyle* getEffectiveDimStyle(const QString &styleName, RS2::EntityType dimType, const LC_DimStyle* styleOverride) const;
    LC_DimStyle* getEffectiveDimStyleForEdit(const QString& styleName, RS2::EntityType dimType, const LC_DimStyle* styleOverride) const;
    virtual LC_DimStyle* getResolvedDimStyle(const QString &dimStyleName, RS2::EntityType dimType = RS2::EntityUnknown) const;
    void updateFallbackDimStyle(const LC_DimStyle* fromStyle);
    void replaceDimStylesList(const QString& defaultStyleName, const QList<LC_DimStyle*>& styles);
    void validateSelection() const {m_selectedSet->cleanup();}
protected:
    void fireUndoStateChanged(bool undoAvailable, bool redoAvailable) const override;
private:
    QDateTime m_lastSaveTime;
    QString m_currentFileName; //keep a copy of filename for the modifiedTime

    // fixme - sand - files - change to unique_ptrs?
    RS_LayerList m_layerList{};
    RS_BlockList m_blockList{true};
    RS_VariableDict m_variableDict;
    RS_VariableDict m_customVariablesDict;
    LC_ViewList m_namedViewsList;
    LC_UCSList m_ucsList;
    LC_DimStylesList m_dimstyleList;
    LC_TextStyleList m_textStyleList;

    //if set to true, will refuse to modify paper scale
    bool m_paperScaleFixed = false;

    /** Format type */
    RS2::FormatType m_formatType = RS2::FormatUnknown;

    // Paper margins in millimeters
    double m_marginLeft = 0.;
    double m_marginTop = 0.;
    double m_marginRight = 0.;
    double m_marginBottom = 0.;

    // Number of pages drawing occupies
    int m_pagesNumH = 1;
    int m_pagesNumV = 1;

    /** File name of the document or empty for a new document. */
    QString m_filename;
    /** Auto-save file name of document. */
    QString m_autosaveFilename;

    LC_GraphicModificationListener* m_modificationListener = nullptr;
    bool m_anglesCounterClockWize;
};
#endif
