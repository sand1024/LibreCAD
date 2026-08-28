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

#include "lc_settings_page_drawing_dimensions.h"
#include "ui_lc_settings_page_drawing_dimensions.h"
#include "lc_document_settings_backend.h"
#include "lc_dimstyleitem.h"
#include "lc_dimstylestreemodel.h"
#include "lc_dimstylepreviewgraphicview.h"
#include "lc_dimstylepreviewpanel.h"
#include "lc_dimstylesexporter.h"
#include "lc_dlgdimstylemanager.h"
#include "lc_dlgnewdimstyle.h"
#include "lc_inputtextdialog.h"
#include "rs_graphic.h"
#include "rs_dimension.h"
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>

#include "lc_settings_widget.h"

LC_SettingsPageDrawingDimensions::LC_SettingsPageDrawingDimensions(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Dimension Styles"), nullptr, parent) // Bypassing binder safely
    , m_graphic(graphic) {
}

LC_SettingsPageDrawingDimensions::~LC_SettingsPageDrawingDimensions() = default;

void LC_SettingsPageDrawingDimensions::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingDimensions>();
    ui->setupUi(m_widget);

    // 1. Setup the CAD Preview inside the page
    m_previewView = LC_DimStylePreviewGraphicView::init(m_widget, m_graphic, RS2::EntityUnknown);
    auto* previewLayout = new QVBoxLayout(ui->gbPreview);
    previewLayout->setContentsMargins(0, 0, 0, 0);

    auto* toolbar = new LC_DimStylePreviewPanel(ui->gbPreview);
    toolbar->setGraphicView(m_previewView);

    previewLayout->addWidget(toolbar);
    previewLayout->addWidget(m_previewView, 10);

    // 2. Symmetrical Visibility & Styling configurations
    // ui->dbDimEmbeddArrowBlocks->setVisible(false);
    ui->lvDimStyles->setContextMenuPolicy(Qt::CustomContextMenu);

    const bool autoRaiseButtons = CFG_Widgets::o_DockWidgetsFlatIcons;
    ui->tbDimNew->setAutoRaise(autoRaiseButtons);
    ui->tbDimRemove->setAutoRaise(autoRaiseButtons);
    ui->tbDimEdit->setAutoRaise(autoRaiseButtons);
    ui->tbDimRename->setAutoRaise(autoRaiseButtons);
    ui->tbDimDefault->setAutoRaise(autoRaiseButtons);
    ui->tbDimExport->setAutoRaise(autoRaiseButtons);
    ui->tbDimImport->setAutoRaise(autoRaiseButtons);
}

void LC_SettingsPageDrawingDimensions::loadSettings() {
    if (!m_graphic) return;

    // 1. Symmetrical usage resolution including substyles
    QMap<QString, int> usages;
    collectStylesUsage(usages);

    // 2. Build the Tree Model
    const QString defaultStyle = m_graphic->getDefaultDimStyleName();
    const auto dimStyles = m_graphic->getDimStyleList()->getStylesList();

    QList<LC_DimStyleItem*> items;
    for (const auto style : *dimStyles) {
        items << new LC_DimStyleItem(style->getCopy(), usages.value(style->getName(), 0), style->getName() == defaultStyle);
    }

    auto* model = new LC_DimStyleTreeModel(this, items, true);
    ui->lvDimStyles->setModel(model);
    ui->lvDimStyles->expandAll();

    // 3. Connect Selection & Double Click
    connect(ui->lvDimStyles->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &LC_SettingsPageDrawingDimensions::onCurrentStyleChanged);
    connect(ui->lvDimStyles, &QTreeView::doubleClicked, this, &LC_SettingsPageDrawingDimensions::onStyleDoubleClick);

    // 4. Select first style and refresh preview safely
    QModelIndex first = model->index(0, 0, QModelIndex());
    ui->lvDimStyles->setCurrentIndex(first);
    if (auto* item = model->getItemForIndex(first)) {
        updatePreview(item->dimStyle());
        updateActiveStyleLabel(model);
    }

    m_manualModified = false;
}

bool LC_SettingsPageDrawingDimensions::saveSettings() {
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    if (!model) return false;

    QList<LC_DimStyleItem*> items;
    model->collectAllStyleItems(items);

    QList<LC_DimStyle*> newStyles;
    QString activeName;
    for (auto* item : items) {
        newStyles.push_back(item->dimStyle()->getCopy());
        if (item->isActive()) activeName = item->dimStyle()->getName();
    }

    m_graphic->replaceDimStylesList(activeName, newStyles);

    // CRITICAL: Memory cleanup from legacy code
    model->cleanup(false);

    m_graphic->updateDimensions(false);
    m_manualModified = false;
    return true;
}

void LC_SettingsPageDrawingDimensions::setupBehavior() {
    connect(ui->tbDimNew, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleNew);
    connect(ui->tbDimRemove, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleRemove);
    connect(ui->tbDimEdit, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleEdit);
    connect(ui->tbDimRename, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleRename);
    connect(ui->tbDimDefault, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleSetDefault);
    connect(ui->tbDimExport, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleExport);
    connect(ui->tbDimImport, &QToolButton::clicked, this, &LC_SettingsPageDrawingDimensions::onStyleImport);
    connect(ui->lvDimStyles, &QTreeView::customContextMenuRequested, this, &LC_SettingsPageDrawingDimensions::onContextMenu);
}

void LC_SettingsPageDrawingDimensions::updateActiveStyleLabel(LC_DimStyleTreeModel* model) {
    if (!model) return;
    const auto* activeStyleItem = model->getActiveStyleItem();
    ui->lblActiveStyle->setText(activeStyleItem ? activeStyleItem->baseName() : QString());
}

void LC_SettingsPageDrawingDimensions::collectStylesUsage(QMap<QString, int>& map) const {
    for (RS_Entity* e : m_graphic->getEntityList()) {
        const auto type = e->rtti();
        if (e->isDeleted() || !RS2::isDimensionalEntity(type)) continue;

        const auto* dim = static_cast<RS_Dimension*>(e);
        QString rawStyleName = dim->getStyle();

        // Symmetrical Substyle Resolution: Finds the correct parent style
        const auto* resolvedStyle = m_graphic->getDimStyleByName(rawStyleName, type);
        if (resolvedStyle != nullptr) {
            map[resolvedStyle->getName()] = map.value(resolvedStyle->getName(), 0) + 1;
        }
    }
}

void LC_SettingsPageDrawingDimensions::onCurrentStyleChanged(const QModelIndex& current, const QModelIndex&) {
    if (!current.isValid()) return;
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    auto* item = model->getItemForIndex(current);
    refreshActionButtons(item);
    updatePreview(item->dimStyle());
}

void LC_SettingsPageDrawingDimensions::updatePreview(LC_DimStyle* style) {
    m_previewView->setDimStyle(style);
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    QList<LC_DimStyleItem*> family;
    model->collectAllItemsForStyle(style, &family);
    for (auto* f : family) {
        if (f->dimStyle()->getName() != style->getName())
            m_previewView->addDimStyle(f->dimStyle());
    }
    m_previewView->updateDims();
    m_previewView->refresh();
}

void LC_SettingsPageDrawingDimensions::refreshActionButtons(const LC_DimStyleItem* item) {
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    if (!item || !model) return;

    const int itemsCount = model->itemsCount();
    ui->tbDimDefault->setEnabled(!item->isActive() && item->isBaseStyle());

    const bool notUsed = item->isNotUsedInDrawing();
    ui->tbDimRemove->setEnabled(itemsCount > 1 && notUsed);
    ui->tbDimRename->setEnabled(item->isBaseStyle() && notUsed);
}

void LC_SettingsPageDrawingDimensions::onStyleNew() {
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    const QModelIndex selectedIdx = ui->lvDimStyles->selectionModel()->currentIndex();

    const LC_DimStyleItem* defaultItem = selectedIdx.isValid() ?
        model->getItemForIndex(selectedIdx) : model->getActiveStyleItem();

    LC_DlgNewDimStyle dlg(m_widget);
    QList<LC_DimStyleItem*> allItems;
    model->collectAllStyleItems(allItems);
    dlg.setup(defaultItem, allItems);

    if (dlg.exec() == QDialog::Accepted) {
        QString newName = dlg.getStyleName();
        const auto* baseItem = dlg.getBaseDimStyle();
        const RS2::EntityType dimType = dlg.getDimensionType();

        LC_DimStyle* styleToEdit = baseItem->dimStyle()->getCopy();
        styleToEdit->setName(newName);
        styleToEdit->setFromVars(false);
        styleToEdit->resetFlags(true);

        LC_DlgDimStyleManager manager(m_widget, styleToEdit, m_graphic, dimType);
        if (manager.exec() == QDialog::Accepted) {
            model->addItem(new LC_DimStyleItem(styleToEdit, 0, false));
            ui->lvDimStyles->expandAll();
            m_manualModified = true;
        } else {
            delete styleToEdit;
        }
    }
}

void LC_SettingsPageDrawingDimensions::onStyleEdit() {
    const QModelIndex idx = ui->lvDimStyles->selectionModel()->currentIndex();
    if (!idx.isValid()) return;

    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    LC_DimStyleItem* item = model->getItemForIndex(idx);
    LC_DimStyle* styleToEdit = item->dimStyle()->getCopy();

    LC_DlgDimStyleManager manager(m_widget, styleToEdit, m_graphic, item->forDimensionType());
    if (manager.exec() == QDialog::Accepted) {
        styleToEdit->copyTo(item->dimStyle());
        m_manualModified = true;
        updatePreview(item->dimStyle());
    }
    delete styleToEdit;
}

void LC_SettingsPageDrawingDimensions::onStyleRename() {
    const QModelIndex idx = ui->lvDimStyles->selectionModel()->currentIndex();
    if (!idx.isValid()) return;

    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    LC_DimStyleItem* item = model->getItemForIndex(idx);
    if (!item->isBaseStyle() || !item->isNotUsedInDrawing()) return;

    QString oldName = item->dimStyle()->getName();
    QString newName = getUniqueName(tr("Rename Style"), tr("Enter new unique name:"), oldName);

    if (!newName.isEmpty()) {
        item->setNewBaseName(newName);
        model->emitDataChanged();
        m_manualModified = true;
    }
}

void LC_SettingsPageDrawingDimensions::onStyleRemove() {
    const QModelIndex idx = ui->lvDimStyles->selectionModel()->currentIndex();
    if (!idx.isValid()) return;

    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    LC_DimStyleItem* item = model->getItemForIndex(idx);
    const int itemsCount = model->itemsCount();

    if (itemsCount == 1) {
        QMessageBox::critical(m_widget, tr("Removing Style"), tr("Cannot delete the last style."));
        return;
    }

    if (item->usageCount() == 0) {
        if (QMessageBox::warning(m_widget, tr("Delete Style"),
            tr("Are you sure you want to delete style '%1'?").arg(item->baseName()),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            model->removeItem(item);
            m_manualModified = true;
        }
    }
}

void LC_SettingsPageDrawingDimensions::onStyleSetDefault() {
    const QModelIndex idx = ui->lvDimStyles->selectionModel()->currentIndex();
    if (!idx.isValid()) return;

    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    model->setActiveStyleItem(idx);
    ui->lvDimStyles->expandAll();

    updateActiveStyleLabel(model);
    m_manualModified = true;
}

void LC_SettingsPageDrawingDimensions::onStyleExport() {
    LC_DimStylesExporter exporter;
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    QList<LC_DimStyleItem*> items;
    model->collectAllStyleItems(items);
    exporter.exportStyles(m_widget, items, m_graphic->getFilename());
}

void LC_SettingsPageDrawingDimensions::onStyleImport() {
    LC_DimStylesExporter exporter;
    QList<LC_DimStyle*> imported;
    if (exporter.importStyles(m_widget, imported)) {
        auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
        model->mergeWith(imported);
        m_manualModified = true;
    }
}

void LC_SettingsPageDrawingDimensions::onStyleDoubleClick(const QModelIndex&) {
    onStyleEdit();
}

void LC_SettingsPageDrawingDimensions::onContextMenu(const QPoint& pos) {
    QModelIndex idx = ui->lvDimStyles->indexAt(pos);
    QMenu menu(m_widget);
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());

    using ActionMemberFunc = void (LC_SettingsPageDrawingDimensions::*)();
    auto addAction = [this, &menu](const QString& icon, const QString& text, ActionMemberFunc func, bool enabled = true) {
        auto* action = menu.addAction(QIcon(":/icons/" + icon + ".lci"), text, this, func);
        action->setEnabled(enabled);
    };

    if (idx.isValid()) {
        const LC_DimStyleItem* item = model->getItemForIndex(idx);

        if (!item->isActive() && item->isBaseStyle()) {
            addAction("dim_default", tr("Set as Active"), &LC_SettingsPageDrawingDimensions::onStyleSetDefault);
        }
        addAction("add", tr("Create Style"), &LC_SettingsPageDrawingDimensions::onStyleNew);
        menu.addSeparator();
        addAction("attributes", tr("Edit Style"), &LC_SettingsPageDrawingDimensions::onStyleEdit);

        if (item->isBaseStyle() && item->isNotUsedInDrawing()) {
            addAction("rename_active_block", tr("Rename Style"), &LC_SettingsPageDrawingDimensions::onStyleRename);
        }
        if (item->isNotUsedInDrawing()) {
            addAction("remove", tr("Delete Style"), &LC_SettingsPageDrawingDimensions::onStyleRemove);
        }
        menu.addSeparator();
        addAction("export", tr("Export Styles"), &LC_SettingsPageDrawingDimensions::onStyleExport);
        addAction("import", tr("Import Styles"), &LC_SettingsPageDrawingDimensions::onStyleImport);
    } else {
        addAction("add", tr("Create Style"), &LC_SettingsPageDrawingDimensions::onStyleNew);
        addAction("import", tr("Import Styles"), &LC_SettingsPageDrawingDimensions::onStyleImport);
    }

    menu.exec(ui->lvDimStyles->viewport()->mapToGlobal(pos));
}

QString LC_SettingsPageDrawingDimensions::getUniqueName(const QString& title, const QString& label, const QString& base) {
    auto* model = static_cast<LC_DimStyleTreeModel*>(ui->lvDimStyles->model());
    bool ok;
    QString name = base;
    int i = 1;

    while (true) {
        name = LC_InputTextDialog::getText(m_widget, title, label, {}, true, name, &ok);
        if (!ok) return "";
        name = name.trimmed();
        if (model->findByName(name) == nullptr) return name;
        name = base + "_" + QString::number(i++);
    }
}

void LC_SettingsPageDrawingDimensions::onDialogShown() {
    if (m_previewView) {
        m_previewView->zoomAuto();
    }
}

void LC_SettingsPageDrawingDimensions::onPageResized() {
    if (m_previewView) {
        m_previewView->zoomAuto();
    }
}
bool LC_SettingsPageDrawingDimensions::isModified() const {
    return m_manualModified;
}


namespace CFG_PageDimensionStyles{
    inline const LC_SettingsGroupBase Group("SettingsPage.DimensionStyles");
    inline const LC_Setting<double> o_ContentRatio(&Group, "ContentRatio", 0.3);
}

void LC_SettingsPageDrawingDimensions::onAboutToShow() {
    if (isSettingsDialogVisible()) {
        m_widget->adjustSize();
        m_pageWasVisible = true;
        const int splitterWidth = ui->splitter->width();
        const double ratio = CFG_PageDimensionStyles::o_ContentRatio;
        int first = splitterWidth*ratio;
        int second = splitterWidth - first;
        ui->splitter->setSizes({first,second});
    }
}

void LC_SettingsPageDrawingDimensions::onAboutToHide() {
    if (m_pageWasVisible) {
        QList<int> currentMainSplitterSizes = ui->splitter->sizes();
        if (!currentMainSplitterSizes.isEmpty()) {
            const int splitterWidth = ui->splitter->width();
            const double width = currentMainSplitterSizes[0];
            const double ratio = width / splitterWidth;
            CFG_PageDimensionStyles::o_ContentRatio = ratio;
        }
    }
}
