#include "qtwilist.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qtwilist w;
    w.show();

    return app.exec();
}

