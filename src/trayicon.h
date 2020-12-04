#ifndef TRAYICON_H
#define TRAYICON_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    TrayIcon(QMainWindow *parent = nullptr);

private slots:
    void _activeTray(QSystemTrayIcon::ActivationReason reason);
    void _application_exit();
    void _show_mainwindow();

private:
    void _init();

private:
    QMainWindow *_window;

    qint64 _trayClickCounter;
};

#endif // TRAYICON_H
