#include "qtwilist.h"
#include "ui_qtwilist.h"

qtwilist::qtwilist(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qtwilist)
{
    ui->setupUi(this);
}

qtwilist::~qtwilist()
{
    delete ui;
}
