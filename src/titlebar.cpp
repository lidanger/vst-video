#include "playerconfig.h"
#include "titlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStyle>
#include <QDebug>

TitleBar::TitleBar(QMainWindow* parent)
    : QFrame (parent)
    , _window(parent)
{
    _init_control();
    _init_layout();
}

void TitleBar::setWindowTitle(const QString &title)
{
    _title->setText(title);
    _window->setWindowTitle(title);
}

void TitleBar::setWindowIcon(const QIcon &icon)
{
    _icon->setIcon(icon);
    _window->setWindowIcon(icon);
}

void TitleBar::setMainMenu(QMenu *menu)
{
    _createMainMenu(menu);
}

void TitleBar::addWidget(QWidget *widget)
{
    widget->setParent(_window);
    _content->layout()->addWidget(widget);
}

void TitleBar::setAboutDialog(QDialog *dialog)
{
    _about = dialog;
}

void TitleBar::setWidgetStart(int pos)
{
    _title->setFixedWidth(pos - _icon->width());
}

void TitleBar::_show_main_menu()
{
    _mainMenu->exec(this->mapToGlobal(QPoint(_menu->pos().x(), _menu->height())));
}

void TitleBar::_show_context_menu(const QPoint &pos)
{
    Q_UNUSED(pos)

    if (_window->isMaximized())
    {
        _contextMenu->findChild<QAction*>("title_context_menu_max")->setVisible(false);
        _contextMenu->findChild<QAction*>("title_context_menu_restore")->setVisible(true);
    }
    else
    {
        _contextMenu->findChild<QAction*>("title_context_menu_max")->setVisible(true);
        _contextMenu->findChild<QAction*>("title_context_menu_restore")->setVisible(false);
    }

    if(_window->windowFlags().testFlag(Qt::WindowStaysOnTopHint))
    {
        _contextMenu->findChild<QAction*>("title_context_menu_top")->setChecked(true);
    }
    else
    {
        _contextMenu->findChild<QAction*>("title_context_menu_top")->setChecked(false);
    }

    _contextMenu->exec(QCursor::pos());
}

void TitleBar::_bring_window_to_top()
{
    _window->hide();

    auto flags = _window->windowFlags();

    if(flags.testFlag(Qt::WindowStaysOnTopHint))
    {
        _window->setWindowFlags(flags ^ Qt::WindowStaysOnTopHint);
        PlayerConfig::instance()->setTopHint(false);
    }
    else
    {
        _window->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
        PlayerConfig::instance()->setTopHint(true);
    }

    _window->show();
}

void TitleBar::_show_window_max_normal()
{
    if(_window->isMaximized())
    {
        _window->showNormal();
    }
    else
    {
        _window->showMaximized();
    }
}

void TitleBar::_select_theme(QAction *action)
{
    if(action == nullptr)
        return;

    _window->setStyleSheet("");

    if(action->objectName() == "theme_deep")
    {
        _window->setStyleSheet("QWidget{background-color:#606060;}");
    }
    else if(action->objectName() == "theme_light")
    {
        _window->setStyleSheet("QWidget{background-color:#F0F0F0;}");
    }

    PlayerConfig::instance()->setTheme(action->objectName());
}

void TitleBar::_show_about()
{
    if(_about == nullptr)
        QMessageBox::aboutQt(_window);
    else
        _about->exec();
}

void TitleBar::_init_control()
{
    _contextMenu = new QMenu(this);
    _icon = new QPushButton(this);
    _title = new QLabel("无标题", this);
    _content = new QWidget(this);
    _menu = new QPushButton(this);
    _min = new QPushButton(this);
    _max = new QPushButton(this);
    _close = new QPushButton(this);
}

void TitleBar::_init_layout()
{
    this->setMinimumHeight(40);
    this->setLayout(new QHBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);

    this->setStyleSheet("QPushButton{border:none;background-color:transparent;}"
                        "QPushButton:hover{background-color:#E0E0E0;}");

    _window->installEventFilter(this);

    // 默认主菜单
    _createMainMenu();

    // 上下文菜单
    _contextMenu->addAction("最大化", _window, &QMainWindow::showMaximized)->setObjectName("title_context_menu_max");
    _contextMenu->addAction("最小化", _window, &QMainWindow::showMinimized)->setObjectName("title_context_menu_min");
    _contextMenu->addAction("还原", _window, &QMainWindow::showNormal)->setObjectName("title_context_menu_restore");
    _contextMenu->addSeparator();
    auto action = _contextMenu->addAction("总在最前", this, &TitleBar::_bring_window_to_top);
    action->setObjectName("title_context_menu_top");
    action->setCheckable(true);
    _contextMenu->addSeparator();
    _contextMenu->addAction("关闭", _window, &QMainWindow::close);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, static_cast<void (QFrame::*)(const QPoint &)>(&TitleBar::customContextMenuRequested),
            this, &TitleBar::_show_context_menu);

    _icon->setFixedSize(50, 40);
    _icon->setIconSize(QSize(35, 35));
    _icon->setIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    this->layout()->addWidget(_icon);

    _title->setFixedWidth(120);
    _title->setMargin(10);
    this->layout()->addWidget(_title);

    _content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _content->setLayout(new QHBoxLayout(_content));
    _content->layout()->setMargin(0);
    _content->layout()->setSpacing(0);
    this->layout()->addWidget(_content);

    _menu->setFixedSize(60, 40);
    _menu->setIconSize(QSize(50, 50));
    _menu->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    connect(_menu, &QPushButton::pressed, this, &TitleBar::_show_main_menu);
    this->layout()->addWidget(_menu);

    _min->setFixedSize(60, 40);
    _min->setIconSize(QSize(50, 50));
    _min->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMinButton));
    connect(_min, &QPushButton::pressed, _window, &QMainWindow::showMinimized);
    this->layout()->addWidget(_min);

    _max->setFixedSize(60, 40);
    _max->setIconSize(QSize(50, 50));
    if(_window->isMaximized())
        _max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarNormalButton));
    else
        _max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    connect(_max, &QPushButton::pressed, this, &TitleBar::_show_window_max_normal);
    this->layout()->addWidget(_max);

    _close->setFixedSize(60, 40);
    _close->setIconSize(QSize(50, 50));
    _close->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(_close, &QPushButton::pressed, _window, &QMainWindow::close);
    this->layout()->addWidget(_close);

    _window->setFocus();
}

void TitleBar::_createMainMenu(QMenu *insertAtStart)
{
    if(_mainMenu != nullptr)
        delete _mainMenu;

    if(insertAtStart == nullptr)
    {
        _mainMenu = new QMenu(this);
    }
    else
    {
        _mainMenu = insertAtStart;
        _mainMenu->addSeparator();
    }

    auto themeMenu = _mainMenu->addMenu("主题");
    auto themeActionGroup = new QActionGroup(this);
    auto action = themeMenu->addAction("深色主题");
    action->setObjectName("theme_deep");
    action->setCheckable(true);
    themeActionGroup->addAction(action);
    action = themeMenu->addAction("浅色主题");
    action->setObjectName("theme_light");
    action->setCheckable(true);
    themeActionGroup->addAction(action);
    action = themeMenu->addAction("跟随系统");
    action->setObjectName("theme_follow");
    action->setCheckable(true);
    themeActionGroup->addAction(action);
    connect(themeActionGroup, SIGNAL(triggered(QAction *)), this, SLOT(_select_theme(QAction *)));

    auto theme = PlayerConfig::instance()->theme();
    if(!theme.isEmpty())
    {
        action = themeMenu->findChild<QAction*>(theme);
        if(action != nullptr)
        {
            action->trigger();
        }
    }

    _mainMenu->addAction("关于", this, &TitleBar::_show_about);
    _mainMenu->addAction("退出", _window, &QMainWindow::close);
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == _window)
        {
            if(_window->isMaximized())
            {
                _max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarNormalButton));
            }
            else
            {
                _max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMaxButton));
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}
