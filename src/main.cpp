#include "app.h"
#include "mainwindow.h"
#include <QApplication>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    App a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
