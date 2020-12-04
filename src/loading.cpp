#include "loading.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QTimer>

Loading* Loading::_inst = nullptr;

Loading::Loading(QWidget *parent)
    : QDialog (parent)
{
    _init();
}

void Loading::show()
{
    QTimer::singleShot(20000, this, SLOT(hide()));

    QDialog::show();
}

void Loading::setParent(QWidget *parent)
{
    if(_inst != nullptr)
    {
        delete _inst;
    }

    _inst = new Loading(parent);
}

Loading *Loading::instance()
{
    if(_inst == nullptr)
        _inst = new Loading();

    return _inst;
}

void Loading::_init()
{
    this->setLayout(new QVBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);

    auto movie = new QMovie(":/img/loading.gif");
    movie->start();

    auto load = new QLabel();
    load->setMovie(movie);
    this->layout()->addWidget(load);

    this->resize(32, 32);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);    // 背景透明
}
