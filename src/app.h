#ifndef APP_H
#define APP_H

#include <QApplication>

class App : public QApplication
{
    Q_OBJECT

public:
    App(int &argc, char **argv);

public:
    static bool isStopped() { return _stop; }

private slots:
    void _aboutToQuit();

private:
    static bool _stop;

};

#endif // APP_H
