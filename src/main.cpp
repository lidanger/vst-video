#include "app.h"
#include "mainwindow.h"
#include <QApplication>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    App a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
