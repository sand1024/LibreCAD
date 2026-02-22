#include "lc_dlg_propertysheet_widget_options.h"
#include "ui_lc_dlg_propertysheet_widget_options.h"

LC_DlgPropertySheetWidgetOptions::LC_DlgPropertySheetWidgetOptions(QWidget* parent, LC_PropertySheetWidgetOptions* options)
    : LC_Dialog(parent, "PropertySheetOptions"), ui(new Ui::LC_DlgPropertySheetWidgetOptions), m_options{options} {
    ui->setupUi(this);
    ui->cbActivePen->setChecked(options->noSelectionActivePen);
    ui->cbActiveLayer->setChecked(options->noSelectionActiveLayer);
    ui->cbDrawingUnits->setChecked(options->noSelectionDrawingUnits);
    ui->cbGrid->setChecked(options->noSelectionGrid);
    ui->cbNamedViews->setChecked(options->noSelectionNamedView);
    ui->cbUCS->setChecked(options->noSelectionUCS);
    ui->cbPrintPaper->setChecked(options->noSelectionPrintPaper);
    ui->cbWorkspace->setChecked(options->noSelectionWorkspace);
    ui->cbGraphicView->setChecked(options->noSelectionGraphicView);
    ui->cbShowLinks->setChecked(options->showLinks);
    ui->cbShowSingelEntityCommands->setChecked(options->showSingleEntityCommands);
    ui->cbShowMultipleEntitiesCommands->setChecked(options->showMultiEntityCommands);
    ui->cbShowComputed->setChecked(options->showComputed);
    ui->cbDuplicateSelectionAction->setChecked(options->duplicateSelectionAction);
}

LC_DlgPropertySheetWidgetOptions::~LC_DlgPropertySheetWidgetOptions() {
    delete ui;
}

void LC_DlgPropertySheetWidgetOptions::accept() {
    LC_Dialog::accept();
    m_options->noSelectionActivePen = ui->cbActivePen->isChecked();
    m_options->noSelectionActiveLayer = ui->cbActiveLayer->isChecked();
    m_options->noSelectionDrawingUnits =  ui->cbDrawingUnits->isChecked();
    m_options->noSelectionGrid = ui->cbGrid->isChecked();
    m_options->noSelectionNamedView = ui->cbNamedViews->isChecked();
    m_options->noSelectionUCS = ui->cbUCS->isChecked();
    m_options->noSelectionPrintPaper = ui->cbPrintPaper->isChecked();
    m_options->noSelectionWorkspace = ui->cbWorkspace->isChecked();
    m_options->noSelectionGraphicView = ui->cbGraphicView->isChecked();
    m_options->showLinks = ui->cbShowLinks->isChecked();
    m_options->showSingleEntityCommands = ui->cbShowSingelEntityCommands->isChecked();
    m_options->showMultiEntityCommands = ui->cbShowMultipleEntitiesCommands->isChecked();
    m_options->showComputed = ui->cbShowComputed->isChecked();
    m_options->duplicateSelectionAction = ui->cbDuplicateSelectionAction->isChecked();
    m_options->save();
}
