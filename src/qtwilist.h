#ifndef QTWILIST_H
#define QTWILIST_H

#include <QMainWindow>

namespace Ui {
class qtwilist;
}

class qtwilist : public QMainWindow
{
    Q_OBJECT

public:
    explicit qtwilist(QWidget *parent = 0);
    ~qtwilist();

private:
    Ui::qtwilist *ui;
};

#endif // QTWILIST_H
