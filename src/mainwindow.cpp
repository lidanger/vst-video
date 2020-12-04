#include "favorites.h"
#include "framelesshelper.h"
#include "loading.h"
#include "mainwindow.h"
#include "playerconfig.h"
#include "resourcesettings.h"
#include "shortcutsettings.h"
#include "titlebar.h"
#include "webresource.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _init_window();
    _init_control();
    _init_menu();
    _init_shotcut();
    _check_arguments();
}

MainWindow::~MainWindow()
{

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == _dock)
        {
            _titlebar->setWidgetStart(_dock->width());
        }
    }
    else if(event->type() == QEvent::FocusOut)
    {
        if(watched == _search_key)
        {
            _search_key->hide();
            _search_button->setIcon(QIcon(":/img/search.svg"));
            _stack->currentWidget()->setFocus();
        }
    }
    else if(event->type() == QEvent::KeyPress)
    {
        auto ket = static_cast<QKeyEvent*>(event);

        if (ket->key() == Qt::Key_Escape)
        {
            if(this->isFullScreen())
            {
                _show_fullscreen();
            }
            else if(_dock->isHidden())
            {
                _dock->show();
            }
        }
    }
    else if(event->type() == QEvent::Show)
    {
        if(watched == _dock)
        {
            WebResource::instance()->app.hideResource = false;
        }
    }
    else if(event->type() == QEvent::Hide)
    {
        if(watched == _dock)
        {
            WebResource::instance()->app.hideResource = true;
        }
    }
    else if(event->type() == QEvent::WindowStateChange)
    {
        if(watched == this)
        {
            WebResource::instance()->app.isFullScreen = this->isFullScreen();
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::_show_search_player()
{
    if(_stack->currentWidget() == _search_page)
    {
        _search_button->setIcon(QIcon(":/img/search.svg"));
        _search_key->hide();
        _stack->setCurrentWidget(_player_page);
        _stack->currentWidget()->setFocus();
    }
    else
    {
        _search_button->setIcon(QIcon(":/img/icon.png"));
        _search_key->show();
        _search_key->setFocus();
        _stack->setCurrentWidget(_search_page);
    }
}

void MainWindow::_start_search()
{
    auto keyword = _search_key->text();
    if(keyword.isEmpty())
        return;

    _search_page->startSearch(keyword);
    _stack->setCurrentWidget(_search_page);
    _stack->currentWidget()->setFocus();
}

void MainWindow::_start_browse(const QString &class_name, int sub_index, bool live)
{
    if(live)
    {
        if(_last_class_name != class_name || _last_sub_index != sub_index)
            _player_page->startPlayLive(sub_index);

        _stack->setCurrentWidget(_player_page);
    }
    else
    {
        if(_last_class_name != class_name || _last_sub_index != sub_index)
            _browse_page->startBrowse(class_name, sub_index);

        _stack->setCurrentWidget(_browse_page);
    }

    _stack->currentWidget()->setFocus();

    _last_class_name = class_name;
    _last_sub_index = sub_index;
}

void MainWindow::_dockLocationChanged(Qt::DockWidgetArea area)
{
    if(area == Qt::DockWidgetArea::NoDockWidgetArea)
    {
        _dock->setTitleBarWidget(nullptr);
    }
    else
    {
        _dock->setTitleBarWidget(_dock_titlebar);
    }

    _stack->currentWidget()->setFocus();
}

void MainWindow::_dock_topLevelChanged(bool topLevel)
{
    if(topLevel)
    {
        _dock->setTitleBarWidget(nullptr);
    }
    else
    {
        _dock->setTitleBarWidget(_dock_titlebar);
    }
}

void MainWindow::_itemSelectToPlay(int index, const QString &name, const QString &api, const QString &id)
{
    _detail_page->loadInfo(index, name, api, id);
    _stack->setCurrentWidget(_detail_page);
}

void MainWindow::_show_detial(int index, const QString &name, const QString &api, const QString &id)
{
    _detail_page->loadInfo(index, name, api, id, false);
    _stack->setCurrentWidget(_detail_page);
}

void MainWindow::_show_detail_player()
{
    if(_stack->currentWidget() == _detail_page)
        _stack->setCurrentWidget(_player_page);
    else
    {
        //if(!WebResource::instance()->app.live)
        _stack->setCurrentWidget(_detail_page);
    }

    _stack->currentWidget()->setFocus();
}

void MainWindow::_show_browse_player()
{
    if(_stack->currentWidget() == _browse_page)
        _stack->setCurrentWidget(_player_page);
    else
        _stack->setCurrentWidget(_browse_page);

    _stack->currentWidget()->setFocus();
}

void MainWindow::_show_fullscreen()
{
    if(this->isFullScreen())
    {
        _stack->setStyleSheet(WebResource::instance()->app.showResourceBeforeFullScreen ? "" : "border:none;");
        _fullscreen_button->setIcon(QIcon(":/img/full_out.svg"));

        this->showNormal();

        _titlebar->show();

        if (WebResource::instance()->app.showResourceBeforeFullScreen)
        {
            _dock->show();
        }

        setWindowState(WebResource::instance()->app.statesBeforeFullScreen);
    }
    else
    {
        // 保存全屏前状态
        WebResource::instance()->app.showResourceBeforeFullScreen = _dock->isVisible();
        WebResource::instance()->app.statesBeforeFullScreen = this->windowState();

        _titlebar->hide();
        _dock->hide();

        _fullscreen_button->setIcon(QIcon(":/img/general_out.svg"));
        _stack->setStyleSheet("border:none;");

        this->showFullScreen();
    }

    _stack->currentWidget()->setFocus();
}

void MainWindow::_aboutToPlay(const QString &name, const QStringList &part_names, const QStringList &part_urls, int index, qint64 time)
{
    _player_page->startPlayVideo(name, part_names, part_urls, index, time);
    //_stack->setCurrentWidget(_player_page);
}

void MainWindow::_records_aboutToShow()
{
    _records->clear();

    auto keys = PlayerConfig::instance()->history();
    if (!keys.isEmpty())
    {
        // 格式：时间(20201203143935)=片名
        keys.sort();

        QString last;
        for (int i = keys.size() - 1; i >= 0 && i > keys.size() - 11; i--)
        {
            auto val = PlayerConfig::instance()->historyItem(keys[i]);
            if(last == val)
                continue;

            _records->addAction(val, this, &MainWindow::_record_selected);
            last = val;
        }
    }

    // 添加清除按钮
    _records->addSeparator();
    _records->addAction("清空", this, &MainWindow::_clear_play_records);
}

void MainWindow::_clear_play_records()
{
    PlayerConfig::instance()->clearHistory();
}

void MainWindow::_show_big_video()
{
    if(_stack->currentWidget() == _player_page)
        _show_fullscreen();
    else
        _stack->setCurrentWidget(_player_page);
}

void MainWindow::_show_resouceList()
{
    _dock->setVisible(!_dock->isVisible());
}

void MainWindow::_show_titleBar(bool show)
{
    _titlebar->setVisible(show);
}

void MainWindow::_init_window()
{
    this->setMinimumSize(800, 545);
    this->setWindowFlags(Qt::FramelessWindowHint | this->windowFlags());
    if(PlayerConfig::instance()->topHint())
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    this->installEventFilter(this);

    auto screenRect = QApplication::desktop()->screenGeometry(this);
    this->move((screenRect.width() - this->width()) / 2, (screenRect.height() - this->height()) / 2);

    this->setCentralWidget(new QWidget(this));
    this->centralWidget()->setLayout(new QVBoxLayout(this->centralWidget()));
    this->centralWidget()->layout()->setMargin(0);
    this->centralWidget()->layout()->setSpacing(0);

    _titlebar = new TitleBar(this);
    _titlebar->setWindowTitle("全聚合影视");
    _titlebar->setWindowIcon(QIcon(":/img/ico.svg"));
    this->centralWidget()->layout()->addWidget(_titlebar);

    auto fhelper = new FramelessHelper(this);
    fhelper->activateOn(this);           // 激活当前窗体
    fhelper->setTitleHeight(50);         // 设置窗体的标题栏高度，可拖动高度
    fhelper->setWidgetMovable(true);     // 设置窗体可移动
    fhelper->setWidgetResizable(true);   // 设置窗体可缩放
    fhelper->setOnlyTitleBarMove(false); // 设置是否只标题栏可拖动
    fhelper->setRubberBandOnMove(true);  // 设置橡皮筋效果-可移动
    fhelper->setRubberBandOnResize(true);  // 设置橡皮筋效果-可缩放

    _real_window = new QMainWindow(this);
    _real_window->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
    _real_window->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->centralWidget()->layout()->addWidget(_real_window);

    // 通知栏图标
    _tray = new TrayIcon(this);
    _tray->show();

    // 关于框
    auto about = new QMessageBox(QMessageBox::NoIcon, "关于", "<H3>全聚合影视 v2.55（2020.11.29 魔改版）</H3>一款基于 Qt5 的云播放器。<H4>作者：nohacks</H4><ul><li>E-mail：<a href='mailto:nohacks@vip.qq.com'>nohacks@vip.qq.com</a></li><li>主&nbsp;&nbsp;&nbsp;&nbsp;页：<a href='https://github.com/xymov'>https://github.com/xymov</a></li></ul><H4>致谢：</H4><ul><li>播放器：<a href='https://github.com/sonichy/HTYMediaPlayer'>https://github.com/sonichy/HTYMediaPlayer</a></li></ul>", QMessageBox::Ok, this);
    about->setIconPixmap(QPixmap(":/img/icon.png"));
    about->setStyleSheet("QLabel#qt_msgbox_label{"
                        "min-width: 360px;"
                        "min-height: 222px;"
                        //"font-size:14px;"
                        "}");
    _titlebar->setAboutDialog(about);

    // 资源加载图标
    Loading::setParent(this);

    auto action = new QAction("分类浏览", this);
    action->setShortcut(QKeySequence("F3"));
    connect(action, &QAction::triggered, this, &MainWindow::_show_browse_player);
    this->addAction(action);

    action = new QAction("视频详情", this);
    action->setShortcut(QKeySequence("F4"));
    connect(action, &QAction::triggered, this, &MainWindow::_show_detail_player);
    this->addAction(action);
}

void MainWindow::_init_control()
{
    // 标题栏
    auto frame = new QFrame();
    frame->setMinimumSize(40, 40);
    frame->setLayout(new QHBoxLayout(frame));
    frame->layout()->setMargin(0);
    frame->layout()->setSpacing(0);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _titlebar->addWidget(frame);

    _search_key = new QLineEdit(_titlebar);
    _search_key->setPlaceholderText("输入关键字进行搜索");
    _search_key->setMinimumSize(100, 32);
    _search_key->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(_search_key, &QLineEdit::returnPressed, this, &MainWindow::_start_search);
    _search_key->installEventFilter(this);
    _search_key->hide();
    frame->layout()->addWidget(_search_key);

    _search_button = new QPushButton(this);
    _search_button->setToolTip("资源搜索");
    _search_button->setFixedSize(60, 40);
    _search_button->setIconSize(QSize(20, 20));
    _search_button->setIcon(QIcon(":/img/search.svg"));
    _search_button->setShortcut(QKeySequence("F2"));
    connect(_search_button, &QPushButton::pressed, this, &MainWindow::_show_search_player);
    _titlebar->addWidget(_search_button);

    _resource_button = new QPushButton(this);
    _resource_button->setToolTip("资源列表");
    _resource_button->setFixedSize(60, 40);
    _resource_button->setIconSize(QSize(20, 20));
    _resource_button->setIcon(QIcon(":/img/list_out.svg"));
    _resource_button->setShortcut(QKeySequence("F10"));
    connect(_resource_button, &QPushButton::pressed, this, &MainWindow::_show_resouceList);
    _titlebar->addWidget(_resource_button);

    _fullscreen_button = new QPushButton(this);
    _fullscreen_button->setToolTip("切换全屏");
    _fullscreen_button->setFixedSize(60, 40);
    _fullscreen_button->setIconSize(QSize(20, 20));
    _fullscreen_button->setIcon(QIcon(":/img/full_out.svg"));
    _fullscreen_button->setShortcut(QKeySequence("F11"));
    connect(_fullscreen_button, &QPushButton::pressed, this, &MainWindow::_show_fullscreen);
    _titlebar->addWidget(_fullscreen_button);

    // 内容区
    _stack = new QStackedWidget(this);
    _real_window->setCentralWidget(_stack);

    _player_page = new PlayerPage(this);
    connect(_player_page, &PlayerPage::requestShowDetail, this, &MainWindow::_show_detail_player);
    connect(_player_page, &PlayerPage::requestShowPlayer, this, &MainWindow::_show_big_video);
    connect(_player_page, &PlayerPage::requestShowFullScreen, this, &MainWindow::_show_fullscreen);
    connect(_player_page, SIGNAL(requestShowTitleBar(bool)), this, SLOT(_show_titleBar(bool)));
    _stack->addWidget(_player_page);

    _detail_page = new DetailPage(this, _player_page->video(), _player_page->player());
    connect(_detail_page, SIGNAL(aboutToPlay(const QString &, const QStringList &, const QStringList &, int, qint64)), this, SLOT(_aboutToPlay(const QString &, const QStringList &, const QStringList &, int, qint64)));
    _stack->addWidget(_detail_page);

    _browse_page = new BrowsePage(this, _player_page->video(), _player_page->player());
    connect(_browse_page, SIGNAL(itemSelectToPlay(int, const QString &, const QString &, const QString &)), this, SLOT(_itemSelectToPlay(int, const QString &, const QString &, const QString &)));
    connect(_browse_page, SIGNAL(requestShowDetail(int, const QString &, const QString &, const QString &)), this, SLOT(_show_detial(int, const QString &, const QString &, const QString &)));
    _stack->addWidget(_browse_page);

    _search_page = new SearchPage(this, _player_page->video(), _player_page->player());
    connect(_search_page, SIGNAL(itemSelectToPlay(int, const QString &, const QString &, const QString &)), this, SLOT(_itemSelectToPlay(int, const QString &, const QString &, const QString &)));
    connect(_search_page, SIGNAL(requestShowDetail(int, const QString &, const QString &, const QString &)), this, SLOT(_show_detial(int, const QString &, const QString &, const QString &)));
    _stack->addWidget(_search_page);

    // 资源区
    _dock = new QDockWidget("资源列表", _real_window);
    _dock->setMinimumWidth(145);
    _dock->setMaximumWidth(400);
    _dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    //_dock->setStyleSheet("QDockWidget::float-button{background:transparent;}");
    _dock->setTitleBarWidget(_dock_titlebar = new QWidget(_dock));
    connect(_dock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(_dockLocationChanged(Qt::DockWidgetArea)));
    connect(_dock, SIGNAL(topLevelChanged(bool)), this, SLOT(_dock_topLevelChanged(bool)));
    _dock->installEventFilter(this);
    _real_window->addDockWidget(Qt::LeftDockWidgetArea, _dock);
    _real_window->resizeDocks({_dock}, {145}, Qt::Horizontal);
    _real_window->setStyleSheet("QMainWindow::separator{"
                                "width:1px;"
                                "height:1px;}");

    _resource = new ResourceList(this);
    connect(_resource, SIGNAL(resourceSelected(const QString &, int, bool)), this, SLOT(_start_browse(const QString &, int, bool)));
    _dock->setWidget(_resource);
}

void MainWindow::_init_menu()
{
    auto menu = new QMenu();
    _records = menu->addMenu("播放记录");
    connect(_records, &QMenu::aboutToShow, this, &MainWindow::_records_aboutToShow);
    _records->addSeparator();
    _records->addAction("清空", this, &MainWindow::_clear_play_records);
    menu->addAction("收藏夹", this, &MainWindow::_play_favorite);
    menu->addSeparator();
    menu->addAction("资源设置", this, &MainWindow::_resource_setting);
    menu->addAction("停靠位置", this, [=](){ _dock->setFloating(true); });
    menu->addAction("资源刷新", _resource, &ResourceList::reload);
    menu->addSeparator();
    menu->addAction("快捷键", this, &MainWindow::_shotcut_setting);
    menu->addSeparator();
    menu->addAction("查看更新", this, [=](){ QDesktopServices::openUrl(QUrl("https://github.com/lidanger/vst-video/releases")); });

    _titlebar->setMainMenu(menu);
}

void MainWindow::_init_shotcut()
{
    auto actions = this->findChildren<QAction*>(nullptr, Qt::FindDirectChildrenOnly);
    foreach(auto action, actions)
    {
        if(action->text().isEmpty())
            continue;

        auto key = PlayerConfig::instance()->shortcutKeys(action->text());
        if(key.isEmpty())
            continue;

        action->setShortcut(QKeySequence(key));
    }

    // button
    auto buttons = _titlebar->findChildren<QPushButton*>();
    foreach(auto button, buttons)
    {
        auto txt = button->text().isEmpty() ? button->toolTip() : button->text();
        if(txt.isEmpty())
            continue;

        auto key = PlayerConfig::instance()->shortcutKeys(txt);
        if(key.isEmpty())
            continue;

        button->setShortcut(QKeySequence(key));
    }
}

void MainWindow::_check_arguments()
{
    auto arguments = QCoreApplication::arguments();
    if(arguments.length() == 1)
        return;

    // 去掉应用程序名
    arguments.removeFirst();

    _dock->hide();
    WebResource::instance()->app.hideResource = true;
    _player_page->startPlayVideoFile(arguments);
    _stack->setCurrentWidget(_player_page);
}

// 播放记录被选择
void MainWindow::_record_selected()
{
    auto sub = static_cast<QAction*>(QObject::sender());
    if(sub == nullptr)
        return;

    auto name = sub->text();

    // 格式：片名=集数|位置|时间(20201203143935)|api|id
    auto data = PlayerConfig::instance()->videoCurrentPosition(name);

    auto v = data.split("|", QString::SkipEmptyParts);
    if(v.size() != 5)
        return;

    auto part = v.value(0);
    auto pos = v.value(1);
    auto api = v.value(3);
    auto id = v.value(4);

    _detail_page->loadInfo(part.toInt(), name, api, id, true, pos.toLongLong());
}

void MainWindow::_play_favorite()
{
    if(Favorites(this).exec() == QDialog::Accepted)
    {
        auto name = PlayerConfig::instance()->currentVideo();

        // 格式：片名=集数|位置|时间(20201203143935)|api|id
        auto data = PlayerConfig::instance()->videoCurrentPosition(name);

        auto v = data.split("|", QString::SkipEmptyParts);
        if(v.size() != 5)
            return;

        auto part = v.value(0);
        auto pos = v.value(1);
        auto api = v.value(3);
        auto id = v.value(4);

        _detail_page->loadInfo(part.toInt(), name, api, id, true, pos.toLongLong());
    }
}

void MainWindow::_shotcut_setting()
{
    if(ShortcutSettings(this).exec() == QDialog::Accepted)
    {
        _init_shotcut();
    }
}

void MainWindow::_resource_setting()
{
    if(ResourceSettings(this).exec() == QDialog::Accepted)
    {
        _resource->reload();
    }
}
