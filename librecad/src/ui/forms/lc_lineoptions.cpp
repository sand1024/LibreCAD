#include "lc_lineoptions.h"


#include "rs_actiondrawline.h"
#include "ui_lc_lineoptions.h"
#include "rs_debug.h"
#include "rs_math.h"
#include "lc_abstract_action_draw_line.h"

/*
 *  Constructs a QG_LineOptions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
LC_LineOptions::LC_LineOptions(QWidget* parent, Qt::WindowFlags fl)
    : LC_ActionOptionsWidget(parent)
    , ui(new Ui::Ui_LineOptionsRel{})
{
    ui->setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
LC_LineOptions::~LC_LineOptions(){
//    saveSettings();
    delete ui;
};

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void LC_LineOptions::languageChange()
{
    ui->retranslateUi(this);
    connect(ui->rbAngle, SIGNAL(clicked(bool)), this, SLOT(onAngleClicked(bool)));
    connect(ui->cbRelAngle, SIGNAL(clicked(bool)), this, SLOT(onAngleRelativeClicked(bool)));
    connect(ui->leAngle, &QLineEdit::editingFinished, this, &LC_LineOptions::onSetAngle);
}

// fixme -  options
void LC_LineOptions::doSetAction(RS_ActionInterface* a, bool update) {
        action = static_cast<LC_ActionDrawLineRel*>(a);

        if (update){
            ui->bClose->setEnabled(action->mayClose());
            ui->bUndo ->setEnabled(action->mayUndo());
            ui->bRedo ->setEnabled(action->mayRedo());
            ui->bPolyline->setEnabled(action->mayClose());
            ui->bStart->setEnabled(action->mayStart());

            ui->rbPoint->setChecked(action->getDirection() == LC_ActionDrawLineRel::DIRECTION_POINT);
            ui->rbX->setChecked(action->getDirection() == LC_ActionDrawLineRel::DIRECTION_X);
            ui->rbY->setChecked(action->getDirection() == LC_ActionDrawLineRel::DIRECTION_Y);
            ui->rbAngle->setChecked(action->getDirection() == LC_ActionDrawLineRel::DIRECTION_ANGLE);

            ui->leAngle->setText( QString::number(action->getAngleValue(), 'g', 6));
            ui->cbRelAngle->setChecked(action->isAngleRelative());
        }
}

void LC_LineOptions::onAngleClicked(bool value){
    ui->leAngle->setEnabled(value);
    ui->cbRelAngle->setEnabled(value);
    if (action){
        if (value){
            action->setSetAngleDirectionState();
        }
    }
}

void LC_LineOptions::closeLine() {
    if (action) {
        action->close();
    }
}

void LC_LineOptions::undo() {
    if (action) {
        action->undo();
    }
}


void LC_LineOptions::redo() {
    if (action) {
        action->redo();
    }
}

void LC_LineOptions::polyline() {
    if (action) {
        action->polyline();
    }
}

void LC_LineOptions::setYState() {
    if (action) {
        action->setSetYDirectionState();
    }
}

void LC_LineOptions::setXState() {
    if (action) {
        action->setSetXDirectionState();
    }
}

void LC_LineOptions::setPointState() {
    if (action) {
        action->setSetPointDirectionState();
    }
}

void LC_LineOptions::onAngleRelativeClicked(bool value){
    if (action) {
        action->setAngleIsRelative(value);
    }
}

void LC_LineOptions::onSetAngle() {
    if (action) {
        bool ok = false;
        QString val = ui->leAngle->text();
        double angle =RS_Math::eval(val, &ok);
        if(!ok) return;
        if (std::abs(angle) < RS_TOLERANCE_ANGLE) angle=0.0;
        action->setAngleValue(angle);
        action->setSetAngleDirectionState();
        ui->rbAngle->setChecked(true);
        ui->leAngle->setText(QString::number(angle, 'g', 6));
    }
}

void LC_LineOptions::start() {
    if (action) {
        action->setNewStartPointState();
    }
}

void LC_LineOptions::clearAction(){
    action = nullptr;
}

bool LC_LineOptions::checkActionRttiValid(RS2::ActionType actionType){
    return actionType == RS2::ActionDrawLineRel;
}
