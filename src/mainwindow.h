#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "detailpage.h"
#include "browsepage.h"
#include "playerpage.h"
#include "resourcelist.h"
#include "searchpage.h"
#include "titlebar.h"
#include "trayicon.h"

#include <QDockWidget>
#include <QLineEdit>
#include <QMainWindow>
#include <QStackedWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void _show_search_player();
    void _start_search();
    void _start_browse(const QString &class_name, int sub_index, bool live);
    void _dockLocationChanged(Qt::DockWidgetArea area);
    void _dock_topLevelChanged(bool topLevel);
    void _itemSelectToPlay(int index, const QString &name, const QString &api, const QString &id);
    void _show_detial(int index, const QString &name, const QString &api, const QString &id);
    void _show_detail_player();
    void _show_browse_player();
    void _show_fullscreen();
    void _aboutToPlay(const QString &name, const QStringList &part_names, const QStringList &part_urls, int index, qint64 time);
    void _records_aboutToShow();
    void _clear_play_records();
    void _show_big_video();
    void _show_resouceList();
    void _show_titleBar(bool show = true);
    void _record_selected();
    void _play_favorite();
    void _shotcut_setting();
    void _resource_setting();

private:
    void _init_window();
    void _init_control();
    void _init_menu();
    void _init_shotcut();
    // 检查命令行参数
    void _check_arguments();

private:
    TitleBar *_titlebar;
    QMainWindow *_real_window;
    QDockWidget *_dock;
    QWidget *_dock_titlebar;
    QStackedWidget *_stack;
    ResourceList *_resource;
    PlayerPage *_player_page;
    DetailPage *_detail_page;
    BrowsePage *_browse_page;
    SearchPage *_search_page;

    TrayIcon *_tray;

    QLineEdit *_search_key;
    QPushButton *_search_button;
    QPushButton *_resource_button;
    QPushButton *_fullscreen_button;

    QMenu *_records;

    QString _last_class_name;
    int _last_sub_index;
};

#endif // MAINWINDOW_H
