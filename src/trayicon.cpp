#include "trayicon.h"

#include <QApplication>
#include <QDateTime>
#include <QMenu>

TrayIcon::TrayIcon(QMainWindow *parent)
    : QSystemTrayIcon (parent)
    , _window (parent)
{
    _init();
}

void TrayIcon::_init()
{
    this->setIcon(QIcon(":/img/icon.png"));
    this->setToolTip(_window->windowTitle());

    auto menu = new QMenu(_window);
    menu->addAction("退出", this, &TrayIcon::_application_exit);
    this->setContextMenu(menu);

    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::_activeTray);
}

void TrayIcon::_activeTray(QSystemTrayIcon::ActivationReason reason)
{
    qint64 dt = QDateTime::currentMSecsSinceEpoch();

    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
        _window->setVisible(!_window->isVisible());

        if (!_window->isVisible())
            this->showMessage(_window->windowTitle(), "主窗口已隐藏，可通过系统托盘区图标恢复", this->icon(), 3000);
        break;
    case QSystemTrayIcon::Trigger:
        // 由于双击不起作用，这里用计数器模拟双击。300 毫秒内的两次点击表示双击
        if (dt - _trayClickCounter < 300)
            this->activated(QSystemTrayIcon::DoubleClick);

        _trayClickCounter = dt;
        break;
    case QSystemTrayIcon::Context:
        if (this->contextMenu() != nullptr)
            this->contextMenu()->exec(QCursor::pos());
        break;
    case QSystemTrayIcon::Unknown:
    case QSystemTrayIcon::MiddleClick:
        break;
    }
}

void TrayIcon::_application_exit()
{
    QApplication::exit();
}

void TrayIcon::_show_mainwindow()
{
    _window->show();
}
