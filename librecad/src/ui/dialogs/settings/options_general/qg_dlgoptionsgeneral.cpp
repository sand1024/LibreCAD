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
#include "qg_dlgoptionsgeneral.h"

#include <QMessageBox>
#include "qc_applicationwindow.h"
#include <QColorDialog>

#include "qg_filedialog.h"

#include "rs_debug.h"
#include "rs_system.h"
#include "rs_settings.h"
#include "rs_units.h"
#include "lc_defaults.h"
#include "rs_math.h"

/*
 *  Constructs a QG_DlgOptionsGeneral as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */

int QG_DlgOptionsGeneral::current_tab = 0;

QG_DlgOptionsGeneral::QG_DlgOptionsGeneral(QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl){
    setModal(modal);
    setupUi(this);
    tabWidget->setCurrentIndex(current_tab);
    init();
    connect(variablefile_button, &QToolButton::clicked,
            this, &QG_DlgOptionsGeneral::setVariableFile);
    connect(fonts_button, &QToolButton::clicked,
            this, &QG_DlgOptionsGeneral::setFontsFolder);

    connect(translation_button, &QToolButton::clicked,
            this, &QG_DlgOptionsGeneral::setTranslationsFolder);

    connect(hatchpatterns_button, &QToolButton::clicked,
            this, &QG_DlgOptionsGeneral::setHatchPatternsFolder);

    connect(cbAutoBackup, &QCheckBox::stateChanged,
            this, &QG_DlgOptionsGeneral::onAutoBackupChanged);

    connect(cbVisualizeHovering, &QCheckBox::stateChanged,
            this, &QG_DlgOptionsGeneral::on_cbVisualizeHoveringClicked);
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_DlgOptionsGeneral::languageChange() {
    retranslateUi(this);
}

void QG_DlgOptionsGeneral::init() {
    // Fill combobox with languages:
    QStringList languageList = RS_SYSTEM->getLanguageList();
    languageList.sort();
    languageList.prepend("en");
    for (auto const &lang: languageList) {

        RS_DEBUG->print("QG_DlgOptionsGeneral::init: adding %s to combobox",
                        lang.toLatin1().data());

        QString l = RS_SYSTEM->symbolToLanguage(lang);
        if (!l.isEmpty() && cbLanguage->findData(lang) == -1) {
            RS_DEBUG->print("QG_DlgOptionsGeneral::init: %s", l.toLatin1().data());
            cbLanguage->addItem(l, lang);
            cbLanguageCmd->addItem(l, lang);
        }
    }

    RS_SETTINGS->beginGroup("/Appearance");

    // set current language:
    QString def_lang = "en";
    QString lang = RS_SETTINGS->readEntry("/Language", def_lang);
    cbLanguage->setCurrentIndex(cbLanguage->findText(RS_SYSTEM->symbolToLanguage(lang)));

    QString langCmd = RS_SETTINGS->readEntry("/LanguageCmd", def_lang);
    cbLanguageCmd->setCurrentIndex(cbLanguageCmd->findText(RS_SYSTEM->symbolToLanguage(langCmd)));

    // graphic view:

    // Snap Indicators
    bool indicator_lines_state = RS_SETTINGS->readNumEntry("/indicator_lines_state", 1);
    indicator_lines_checkbox->setChecked(indicator_lines_state);

    QString indicator_lines_type = RS_SETTINGS->readEntry("/indicator_lines_type", "Crosshair");
    int index = indicator_lines_combobox->findText(indicator_lines_type);
    indicator_lines_combobox->setCurrentIndex(index);

    bool indicator_shape_state = RS_SETTINGS->readNumEntry("/indicator_shape_state", 1);
    indicator_shape_checkbox->setChecked(indicator_shape_state);

    QString indicator_shape_type = RS_SETTINGS->readEntry("/indicator_shape_type", "Circle");
    index = indicator_shape_combobox->findText(indicator_shape_type);
    indicator_shape_combobox->setCurrentIndex(index);

    bool cursor_hiding = RS_SETTINGS->readNumEntry("/cursor_hiding", 0);
    cursor_hiding_checkbox->setChecked(cursor_hiding);

    bool showSnapOptionsInSnapToolbar = RS_SETTINGS->readNumEntry("/showSnapOptionsInSnapToolbar", 0);
    cbShowSnapOptionsInSnapBar->setChecked(showSnapOptionsInSnapToolbar);

    bool hideRelativeZero = RS_SETTINGS->readNumEntry("/hideRelativeZero", 0);
    cbHideRelativeZero->setChecked(hideRelativeZero);
    bool visualizeHovering = RS_SETTINGS->readNumEntry("/VisualizeHovering", 1);
    cbVisualizeHovering->setChecked(visualizeHovering);

    bool visualizeHoveringRefPoints = RS_SETTINGS->readNumEntry("/VisualizeHoveringRefPoints", 1);
    cbShowRefPointsOnHovering->setChecked(visualizeHoveringRefPoints);
    cbShowRefPointsOnHovering->setEnabled(visualizeHovering);

    bool visualizePreviewRefPoints = RS_SETTINGS->readNumEntry("/VisualizePreviewRefPoints", 1);
    cbDisplayRefPoints->setChecked(visualizePreviewRefPoints);

    // scale grid:
    QString scaleGrid = RS_SETTINGS->readEntry("/ScaleGrid", "1");
    cbScaleGrid->setChecked(scaleGrid == "1");
    QString minGridSpacing = RS_SETTINGS->readEntry("/MinGridSpacing", "10");
    cbMinGridSpacing->setCurrentIndex(cbMinGridSpacing->findText(minGridSpacing));

    int checked = RS_SETTINGS->readNumEntry("/Antialiasing");
    cb_antialiasing->setChecked(checked ? true : false);

    checked = RS_SETTINGS->readNumEntry("/UnitlessGrid", 0);
    cb_unitless_grid->setChecked(checked ? true : false);
    checked = RS_SETTINGS->readNumEntry("/Autopanning");
    cb_autopanning->setChecked(checked ? true : false);

    checked = RS_SETTINGS->readNumEntry("/ScrollBars");
    scrollbars_check_box->setChecked(checked ? true : false);

    // preview:
    initComboBox(cbMaxPreview, RS_SETTINGS->readEntry("/MaxPreview", "100"));

    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("Colors");
    initComboBox(cbBackgroundColor, RS_SETTINGS->readEntry("/background", RS_Settings::background));
    initComboBox(cbGridColor, RS_SETTINGS->readEntry("/grid", RS_Settings::grid));
    initComboBox(cbMetaGridColor, RS_SETTINGS->readEntry("/meta_grid", RS_Settings::meta_grid));
    initComboBox(cbSelectedColor, RS_SETTINGS->readEntry("/select", RS_Settings::select));
    initComboBox(cbHighlightedColor, RS_SETTINGS->readEntry("/highlight", RS_Settings::highlight));
    initComboBox(cbStartHandleColor, RS_SETTINGS->readEntry("/start_handle", RS_Settings::start_handle));
    initComboBox(cbHandleColor, RS_SETTINGS->readEntry("/handle", RS_Settings::handle));
    initComboBox(cbEndHandleColor, RS_SETTINGS->readEntry("/end_handle", RS_Settings::end_handle));
    initComboBox(cbRelativeZeroColor, RS_SETTINGS->readEntry("/relativeZeroColor", RS_Settings::relativeZeroColor));
    initComboBox(cbPreviewRefColor, RS_SETTINGS->readEntry("/previewReferencesColor", RS_Settings::previewRefColor));
    initComboBox(cbPreviewRefHighlightColor,
                 RS_SETTINGS->readEntry("/previewReferencesHighlightColor", RS_Settings::previewRefHighlightColor));
    initComboBox(cb_snap_color, RS_SETTINGS->readEntry("/snap_indicator", RS_Settings::snap_indicator));
    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("/Paths");

    lePathTranslations->setText(RS_SETTINGS->readEntry("/Translations", ""));
    lePathHatch->setText(RS_SETTINGS->readEntry("/Patterns", ""));
    lePathFonts->setText(RS_SETTINGS->readEntry("/Fonts", ""));
    lePathLibrary->setText(RS_SETTINGS->readEntry("/Library", "").trimmed());
    leTemplate->setText(RS_SETTINGS->readEntry("/Template", "").trimmed());
    variablefile_field->setText(RS_SETTINGS->readEntry("/VariableFile", "").trimmed());
    RS_SETTINGS->endGroup();

    // units:
    for (int i = RS2::None; i < RS2::LastUnit; i++) {
        if (i != (int) RS2::None)
            cbUnit->addItem(RS_Units::unitToString((RS2::Unit) i));
    }
    // RVT_PORT cbUnit->listBox()->sort();
    cbUnit->insertItem(0, RS_Units::unitToString(RS2::None));

    QString def_unit = "Millimeter";

    RS_SETTINGS->beginGroup("/Defaults");
//    cbUnit->setCurrentIndex( cbUnit->findText(QObject::tr( RS_SETTINGS->readEntry("/Unit", def_unit) )) );
    cbUnit->setCurrentIndex(cbUnit->findText(QObject::tr(RS_SETTINGS->readEntry("/Unit", def_unit).toUtf8().data())));
    // Auto save timer
    cbAutoSaveTime->setValue(RS_SETTINGS->readNumEntry("/AutoSaveTime", 5));
    cbAutoBackup->setChecked(RS_SETTINGS->readNumEntry("/AutoBackupDocument", 1));
    cbUseQtFileOpenDialog->setChecked(RS_SETTINGS->readNumEntry("/UseQtFileOpenDialog", 1));
    cbWheelScrollInvertH->setChecked(RS_SETTINGS->readNumEntry("/WheelScrollInvertH", 0));
    cbWheelScrollInvertV->setChecked(RS_SETTINGS->readNumEntry("/WheelScrollInvertV", 0));
    cbInvertZoomDirection->setChecked(RS_SETTINGS->readNumEntry("/InvertZoomDirection", 0));
    cbAngleSnap->setCurrentIndex(RS_SETTINGS->readNumEntry("/AngleSnapStep", 3));

    RS_SETTINGS->endGroup();

//update entities to selected entities to the current active layer
    RS_SETTINGS->beginGroup("/Modify");
    auto toActive = RS_SETTINGS->readNumEntry("/ModifyEntitiesToActiveLayer", 0);
    cbToActiveLayer->setChecked(toActive == 1);
    RS_SETTINGS->writeEntry("/ModifyEntitiesToActiveLayer", cbToActiveLayer->isChecked() ? 1 : 0);
    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("/CADPreferences");
    cbAutoZoomDrawing->setChecked(RS_SETTINGS->readNumEntry("/AutoZoomDrawing"));
    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("Startup");
    cbSplash->setChecked(RS_SETTINGS->readNumEntry("/ShowSplash", 1) == 1);
    tab_mode_check_box->setChecked(RS_SETTINGS->readNumEntry("/TabMode", 0));
    maximize_checkbox->setChecked(RS_SETTINGS->readNumEntry("/Maximize", 0));
    left_sidebar_checkbox->setChecked(RS_SETTINGS->readNumEntry("/EnableLeftSidebar", 1));
    cad_toolbars_checkbox->setChecked(RS_SETTINGS->readNumEntry("/EnableCADToolbars", 1));
    cbOpenLastFiles->setChecked(RS_SETTINGS->readNumEntry("/OpenLastOpenedFiles", 1));
    RS_SETTINGS->endGroup();

    cbEvaluateOnSpace->setChecked(RS_SETTINGS->readNumEntry("/Keyboard/EvaluateCommandOnSpace", false));
    cbToggleFreeSnapOnSpace->setChecked(RS_SETTINGS->readNumEntry("/Keyboard/ToggleFreeSnapOnSpace", false));

    initReferencePoints();

    restartNeeded = false;
}

void QG_DlgOptionsGeneral::initComboBox(QComboBox *cb, const QString &text) {
    int idx = cb->findText(text);
    if (idx < 0) {
        idx = 0;
        cb->insertItem(idx, text);
    }
    cb->setCurrentIndex(idx);
}

void QG_DlgOptionsGeneral::setRestartNeeded() {
    restartNeeded = true;
}

void QG_DlgOptionsGeneral::setTemplateFile() {
    RS2::FormatType type = RS2::FormatDXFRW;
    QG_FileDialog dlg(this);
    QString fileName = dlg.getOpenFile(&type);
    leTemplate->setText(fileName);
}

void QG_DlgOptionsGeneral::ok(){
    if (RS_Settings::save_is_allowed){
        //RS_SYSTEM->loadTranslation(cbLanguage->currentText());
        RS_SETTINGS->beginGroup("/Appearance");
        RS_SETTINGS->writeEntry("/ScaleGrid", QString("%1").arg((int) cbScaleGrid->isChecked()));
        RS_SETTINGS->writeEntry("/hideRelativeZero", QString("%1").arg((int) cbHideRelativeZero->isChecked()));
        RS_SETTINGS->writeEntry("/VisualizeHovering", QString{cbVisualizeHovering->isChecked() ? "1" : "0"});
        RS_SETTINGS->writeEntry("/VisualizeHoveringRefPoints", QString{cbShowRefPointsOnHovering->isChecked() ? "1" : "0"});
        RS_SETTINGS->writeEntry("/VisualizePreviewRefPoints", QString{cbDisplayRefPoints->isChecked() ? "1" : "0"});
        RS_SETTINGS->writeEntry("/MinGridSpacing", cbMinGridSpacing->currentText());
        RS_SETTINGS->writeEntry("/MaxPreview", cbMaxPreview->currentText());
        RS_SETTINGS->writeEntry("/Language", cbLanguage->itemData(cbLanguage->currentIndex()));
        RS_SETTINGS->writeEntry("/LanguageCmd", cbLanguageCmd->itemData(cbLanguageCmd->currentIndex()));
        RS_SETTINGS->writeEntry("/indicator_lines_state", indicator_lines_checkbox->isChecked());
        RS_SETTINGS->writeEntry("/indicator_lines_type", indicator_lines_combobox->currentText());
        RS_SETTINGS->writeEntry("/indicator_shape_state", indicator_shape_checkbox->isChecked());
        RS_SETTINGS->writeEntry("/indicator_shape_type", indicator_shape_combobox->currentText());
        RS_SETTINGS->writeEntry("/cursor_hiding", cursor_hiding_checkbox->isChecked());
        RS_SETTINGS->writeEntry("/showSnapOptionsInSnapToolbar", cbShowSnapOptionsInSnapBar->isChecked());
        RS_SETTINGS->writeEntry("/UnitlessGrid", cb_unitless_grid->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/Antialiasing", cb_antialiasing->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/Autopanning", cb_autopanning->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/ScrollBars", scrollbars_check_box->isChecked() ? 1 : 0);
        RS_SETTINGS->endGroup();

        RS_SETTINGS->beginGroup("Colors");
        RS_SETTINGS->writeEntry("/background", cbBackgroundColor->currentText());
        RS_SETTINGS->writeEntry("/grid", cbGridColor->currentText());
        RS_SETTINGS->writeEntry("/meta_grid", cbMetaGridColor->currentText());
        RS_SETTINGS->writeEntry("/select", cbSelectedColor->currentText());
        RS_SETTINGS->writeEntry("/highlight", cbHighlightedColor->currentText());
        RS_SETTINGS->writeEntry("/start_handle", cbStartHandleColor->currentText());
        RS_SETTINGS->writeEntry("/handle", cbHandleColor->currentText());
        RS_SETTINGS->writeEntry("/end_handle", cbEndHandleColor->currentText());
        RS_SETTINGS->writeEntry("/relativeZeroColor", cbRelativeZeroColor->currentText());
        RS_SETTINGS->writeEntry("/previewReferencesColor", cbPreviewRefColor->currentText());
        RS_SETTINGS->writeEntry("/previewReferencesHighlightColor", cbPreviewRefHighlightColor->currentText());
        RS_SETTINGS->writeEntry("/snap_indicator", cb_snap_color->currentText());
        RS_SETTINGS->endGroup();

        RS_SETTINGS->beginGroup("/Paths");
        RS_SETTINGS->writeEntry("/Translations", lePathTranslations->text());
        RS_SETTINGS->writeEntry("/Patterns", lePathHatch->text());
        RS_SETTINGS->writeEntry("/Fonts", lePathFonts->text());
        RS_SETTINGS->writeEntry("/Library", lePathLibrary->text());
        RS_SETTINGS->writeEntry("/Template", leTemplate->text());
        RS_SETTINGS->writeEntry("/VariableFile", variablefile_field->text());
        RS_SETTINGS->endGroup();

        RS_SETTINGS->beginGroup("/Defaults");
        RS_SETTINGS->writeEntry("/Unit",
                                RS_Units::unitToString(RS_Units::stringToUnit(cbUnit->currentText()), false/*untr.*/));
        RS_SETTINGS->writeEntry("/AutoSaveTime", cbAutoSaveTime->value());
        RS_SETTINGS->writeEntry("/AutoBackupDocument", cbAutoBackup->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/UseQtFileOpenDialog", cbUseQtFileOpenDialog->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/WheelScrollInvertH", cbWheelScrollInvertH->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/WheelScrollInvertV", cbWheelScrollInvertV->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/InvertZoomDirection", cbInvertZoomDirection->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/AngleSnapStep", cbAngleSnap ->currentIndex());
        RS_SETTINGS->endGroup();

        //update entities to selected entities to the current active layer
        RS_SETTINGS->beginGroup("/Modify");
        RS_SETTINGS->writeEntry("/ModifyEntitiesToActiveLayer", cbToActiveLayer->isChecked() ? 1 : 0);
        RS_SETTINGS->endGroup();

        RS_SETTINGS->beginGroup("/CADPreferences");
        RS_SETTINGS->writeEntry("/AutoZoomDrawing", cbAutoZoomDrawing->isChecked() ? 1 : 0);
        RS_SETTINGS->endGroup();

        RS_SETTINGS->beginGroup("Startup");
        RS_SETTINGS->writeEntry("/ShowSplash", cbSplash->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/TabMode", tab_mode_check_box->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/Maximize", maximize_checkbox->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/EnableLeftSidebar", left_sidebar_checkbox->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/EnableCADToolbars", cad_toolbars_checkbox->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/OpenLastOpenedFiles", cbOpenLastFiles->isChecked() ? 1 : 0);
        RS_SETTINGS->endGroup();

        RS_SETTINGS->writeEntry("/Keyboard/EvaluateCommandOnSpace", cbEvaluateOnSpace->isChecked() ? 1 : 0);
        RS_SETTINGS->writeEntry("/Keyboard/ToggleFreeSnapOnSpace", cbToggleFreeSnapOnSpace->isChecked() ? 1 : 0);

        saveReferencePoints();
    }

    if (restartNeeded == true) {
        QMessageBox::warning(this, tr("Preferences"),
                             tr("Please restart the application to apply all changes."));
    }
    accept();
}

void QG_DlgOptionsGeneral::on_tabWidget_currentChanged(int index){
    current_tab = index;
}

void QG_DlgOptionsGeneral::set_color(QComboBox *combo, QColor custom) {
    QColor current = QColor::fromString(combo->lineEdit()->text());

    QColorDialog dlg;
    dlg.setCustomColor(0, custom.rgb());

    QColor color = dlg.getColor(current, this, "Select Color", QColorDialog::DontUseNativeDialog);
    if (color.isValid()) {
        combo->lineEdit()->setText(color.name());
    }
}

void QG_DlgOptionsGeneral::on_pb_background_clicked() {
    set_color(cbBackgroundColor, QColor(RS_Settings::background));
}

void QG_DlgOptionsGeneral::on_pb_grid_clicked() {
    set_color(cbGridColor, QColor(RS_Settings::grid));
}

void QG_DlgOptionsGeneral::on_pb_meta_clicked() {
    set_color(cbMetaGridColor, QColor(RS_Settings::meta_grid));
}

void QG_DlgOptionsGeneral::on_pb_selected_clicked() {
    set_color(cbSelectedColor, QColor(RS_Settings::select));
}

void QG_DlgOptionsGeneral::on_pb_highlighted_clicked() {
    set_color(cbHighlightedColor, QColor(RS_Settings::highlight));
}

void QG_DlgOptionsGeneral::on_pb_start_clicked() {
    set_color(cbStartHandleColor, QColor(RS_Settings::start_handle));
}

void QG_DlgOptionsGeneral::on_pb_handle_clicked() {
    set_color(cbHandleColor, QColor(RS_Settings::handle));
}

void QG_DlgOptionsGeneral::on_pb_end_clicked() {
    set_color(cbEndHandleColor, QColor(RS_Settings::end_handle));
}

void QG_DlgOptionsGeneral::on_pb_snap_color_clicked() {
    set_color(cb_snap_color, QColor(RS_Settings::snap_indicator));
}

void QG_DlgOptionsGeneral::on_pb_relativeZeroColor_clicked() {
    set_color(cbRelativeZeroColor, QColor(RS_Settings::relativeZeroColor));
}

void QG_DlgOptionsGeneral::on_pb_previewRefColor_clicked() {
    set_color(cbPreviewRefColor, QColor(RS_Settings::previewRefColor));
}

void QG_DlgOptionsGeneral::on_pb_previewRefHighlightColor_clicked() {
    set_color(cbPreviewRefHighlightColor, QColor(RS_Settings::previewRefHighlightColor));
}

void QG_DlgOptionsGeneral::on_pb_clear_all_clicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Clear settings"),
                                  tr("This will also include custom menus and toolbars. Continue?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        RS_SETTINGS->clear_all();
        QMessageBox::information(this, "info", "You must restart LibreCAD to see the changes.");
    }
}

void QG_DlgOptionsGeneral::on_pb_clear_geometry_clicked() {
    RS_SETTINGS->clear_geometry();
    QMessageBox::information(this, "info", "You must restart LibreCAD to see the changes.");
}

void QG_DlgOptionsGeneral::setVariableFile() {
    QString path = QFileDialog::getOpenFileName(this);
    if (!path.isEmpty()) {
        variablefile_field->setText(QDir::toNativeSeparators(path));
    }
}

/*!
 * \brief slot for the font folder selection icon
 * \author ravas
 * \date 2016-286
 */
void QG_DlgOptionsGeneral::setFontsFolder() {
    QString folder = selectFolder("Select Fonts Folder");
    if (folder != nullptr) {
        lePathFonts->setText(QDir::toNativeSeparators(folder));
    }
}

void QG_DlgOptionsGeneral::setTranslationsFolder() {
    QString folder = selectFolder("Select Translations Folder");
    if (folder != nullptr) {
        lePathTranslations->setText(QDir::toNativeSeparators(folder));
    }
}

void QG_DlgOptionsGeneral::setHatchPatternsFolder() {
    QString folder = selectFolder("Select Hatch Patterns Folder");
    if (folder != nullptr) {
        lePathHatch->setText(QDir::toNativeSeparators(folder));
    }
}

QString QG_DlgOptionsGeneral::selectFolder(const char* title) {
    QString folder = nullptr;
    QFileDialog dlg(this);
    if (title != nullptr) {
        QString dlgTitle = tr(title);
        dlg.setWindowTitle(dlgTitle);
    }
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly);

    if (dlg.exec()) {
        folder = dlg.selectedFiles()[0];
    }
    return folder;
}

void QG_DlgOptionsGeneral::setLibraryPath() {
    QG_FileDialog dlg(this);
    dlg.setFileMode(QFileDialog::Directory);

    if (dlg.exec()) {
        auto dir = dlg.selectedFiles()[0];
        lePathLibrary->setText(QDir::toNativeSeparators(dir));
        setRestartNeeded();
    }
}

void QG_DlgOptionsGeneral::on_cbVisualizeHoveringClicked() {
    cbShowRefPointsOnHovering->setEnabled(cbVisualizeHovering->isChecked());
}

void QG_DlgOptionsGeneral::onAutoBackupChanged([[maybe_unused]] int state) {
    bool allowBackup = cbAutoBackup->checkState() == Qt::Checked;
    auto &appWindow = QC_ApplicationWindow::getAppWindow();
    appWindow->startAutoSave(allowBackup);
}

void QG_DlgOptionsGeneral::initReferencePoints() {

    RS_SETTINGS->beginGroup("/Appearance");
    // Points drawing style:
    int pdmode = RS_SETTINGS->readNumEntry("/RefPointType",
                                           DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
    QString pdsizeStr = RS_SETTINGS->readEntry("/RefPointSize", "2.0");
    RS_SETTINGS->endGroup();

// Set button checked for the currently selected point style
    switch (pdmode) {
        case DXF_FORMAT_PDMode_CentreDot:
        default:
            bDot->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_CentreBlank:
            bBlank->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_CentrePlus:
            bPlus->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_CentreCross:
            bCross->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_CentreTick:
            bTick->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreDot):
            bDotCircle->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreBlank):
            bBlankCircle->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentrePlus):
            bPlusCircle->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreCross):
            bCrossCircle->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreTick):
            bTickCircle->setChecked(true);
            break;

        case DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot):
            bDotSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreBlank):
            bBlankSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentrePlus):
            bPlusSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreCross):
            bCrossSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreTick):
            bTickSquare->setChecked(true);
            break;

        case DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreDot):
            bDotCircleSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreBlank):
            bBlankCircleSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentrePlus):
            bPlusCircleSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreCross):
            bCrossCircleSquare->setChecked(true);
            break;
        case DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreTick):
            bTickCircleSquare->setChecked(true);
            break;
    }

// Fill points display size value string, and set button checked for screen-size
// relative vs. absolute drawing units radio buttons. Negative pdsize => value
// gives points size as percent of screen size; positive pdsize => value gives
// points size in absolute drawing units; pdsize == 0 implies points size to be
// 5% relative to screen size.

    bool ok;
    double pdsize = RS_Math::eval(pdsizeStr, &ok);
    if (!ok) {
        pdsize = LC_DEFAULTS_PDSize;
    }
    if (pdsize <= 0.0)
        rbRelSize->setChecked(true);
    else
        rbAbsSize->setChecked(true);

    lePointSize->setText(QString::number(std::abs(pdsize), 'g', 6));

// Set the appropriate text for the display size value label
    updateLPtSzUnits();
}

void QG_DlgOptionsGeneral::updateLPtSzUnits() {
//	RS_DEBUG->print(RS_Debug::D_ERROR,"QG_DlgOptionsDrawing::updateLPtSzUnits, rbRelSize->isChecked() = %d",rbRelSize->isChecked());
    if (rbRelSize->isChecked())
        lPtSzUnits->setText(QApplication::translate("QG_DlgOptionsDrawing", "Screen %", nullptr));
    else
        lPtSzUnits->setText(QApplication::translate("QG_DlgOptionsDrawing", "Dwg Units", nullptr));
}

void QG_DlgOptionsGeneral::saveReferencePoints() {
// Points drawing style:
// Get currently selected point style from which button is checked
    int pdmode = LC_DEFAULTS_PDMode;

    if (bDot->isChecked())
        pdmode = DXF_FORMAT_PDMode_CentreDot;
    else if (bBlank->isChecked())
        pdmode = DXF_FORMAT_PDMode_CentreBlank;
    else if (bPlus->isChecked())
        pdmode = DXF_FORMAT_PDMode_CentrePlus;
    else if (bCross->isChecked())
        pdmode = DXF_FORMAT_PDMode_CentreCross;
    else if (bTick->isChecked())
        pdmode = DXF_FORMAT_PDMode_CentreTick;

    else if (bDotCircle->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreDot);
    else if (bBlankCircle->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreBlank);
    else if (bPlusCircle->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentrePlus);
    else if (bCrossCircle->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreCross);
    else if (bTickCircle->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreTick);

    else if (bDotSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot);
    else if (bBlankSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreBlank);
    else if (bPlusSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentrePlus);
    else if (bCrossSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreCross);
    else if (bTickSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreTick);

    else if (bDotCircleSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreDot);
    else if (bBlankCircleSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreBlank);
    else if (bPlusCircleSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentrePlus);
    else if (bCrossCircleSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreCross);
    else if (bTickCircleSquare->isChecked())
        pdmode = DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreTick);

// Get points display size from the value string and the relative vs. absolute
// size radio buttons state
    bool ok;
    double pdsize = RS_Math::eval(lePointSize->text(), &ok);
    if (!ok)
        pdsize = LC_DEFAULTS_PDSize;

    if (pdsize > 0.0 && rbRelSize->isChecked())
        pdsize = -pdsize;

    QString pdsizeStr = QString::number(pdsize);

    RS_SETTINGS->beginGroup("/Appearance");
    // Points drawing style:
    RS_SETTINGS->writeEntry("/RefPointType", pdmode);
    RS_SETTINGS->writeEntry("/RefPointSize", pdsizeStr);
    RS_SETTINGS->endGroup();

}

void QG_DlgOptionsGeneral::on_rbRelSize_toggled([[maybe_unused]] bool checked) {
//	RS_DEBUG->print(RS_Debug::D_ERROR,"QG_DlgOptionsDrawing::on_rbRelSize_toggled, checked = %d",checked);
    updateLPtSzUnits();
}