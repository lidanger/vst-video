#include "app.h"
#include "webresource.h"

bool App::_stop = false;

App::App(int &argc, char **argv)
    : QApplication (argc, argv)
{
    this->setApplicationName("vst-video");
    this->setOrganizationName("xymov");
    this->setAttribute(Qt::AA_EnableHighDpiScaling);

    connect(this, &QApplication::aboutToQuit, this, &App::_aboutToQuit);
}

void App::_aboutToQuit()
{
    _stop = true;
    WebResource::instance()->stopAll();
}
