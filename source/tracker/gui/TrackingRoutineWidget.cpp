#include "TrackingRoutineWidget.hpp"
#include "ui_TrackingRoutineWidget.h"

/*!
 * Constructor.
 */
TrackingRoutineWidget::TrackingRoutineWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrackingRoutineWidget)
{
    ui->setupUi(this);
}

/*!
 * Destructor.
 */
TrackingRoutineWidget::~TrackingRoutineWidget()
{
    delete ui;
}
