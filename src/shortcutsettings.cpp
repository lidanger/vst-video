#include "playerconfig.h"
#include "shortcutsettings.h"
#include "titlebar.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QMainWindow>
#include <QtDebug>

ShortcutSettings::ShortcutSettings(QWidget *parent)
    : QDialog(parent)
{
    _init();
    _init_data();

    auto rect = parent->geometry();
    int x = rect.x() + rect.width() / 2 - this->width() / 2;
    int y = rect.y() + rect.height() / 2 - this->height() / 2 - 20;
    this->move(x, y);
}

void ShortcutSettings::_init()
{
    this->setWindowTitle("快捷键设置");
    this->setMinimumSize(350, 500);
    this->setLayout(new QVBoxLayout(this));

    _shortcuts = new QListWidget(this);
    _shortcuts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _shortcuts->setSelectionMode(QAbstractItemView::NoSelection);
    this->layout()->addWidget(_shortcuts);

    // 按钮
    auto buttons_panel = new QFrame(this);
    buttons_panel->setFixedHeight(50);
    buttons_panel->setLayout(new QHBoxLayout(buttons_panel));
    buttons_panel->layout()->setSpacing(50);
    this->layout()->addWidget(buttons_panel);

    static_cast<QBoxLayout*>(buttons_panel->layout())->addStretch();

    _ok = new QPushButton("确定", this);
    _ok->setMinimumWidth(80);
    connect(_ok, &QPushButton::pressed, this, &ShortcutSettings::_save);
    buttons_panel->layout()->addWidget(_ok);

    _cancel= new QPushButton("取消", this);
    _cancel->setMinimumWidth(80);
    _cancel->setDefault(true);
    _cancel->setAutoDefault(true);
    connect(_cancel, &QPushButton::pressed, this, &QDialog::reject);
    buttons_panel->layout()->addWidget(_cancel);

    static_cast<QBoxLayout*>(buttons_panel->layout())->addStretch();
}

void ShortcutSettings::_init_data()
{
    if(this->parent() == nullptr)
        return;

    // 搜索主窗口的可配置项
    auto mainwindow = static_cast<QMainWindow*>(this->parent());

    // 窗口内的全局 action
    auto actions = mainwindow->findChildren<QAction*>(nullptr, Qt::FindDirectChildrenOnly);
    foreach(auto action, actions)
    {
        if(action->text().isEmpty())
            continue;

        auto item = new QListWidgetItem(_shortcuts);
        item->setSizeHint(QSize(100, 50));
        item->setData(Qt::UserRole, action->text());
        _shortcuts->addItem(item);

        auto widget = new QWidget(_shortcuts);
        widget->setLayout(new QHBoxLayout(widget));
        _shortcuts->setItemWidget(item, widget);

        auto label = new QLabel(action->text(), widget);
        label->setFixedWidth(100);
        widget->layout()->addWidget(label);

        auto edit = new QKeySequenceEdit(widget);
        edit->setKeySequence(action->shortcut());
        widget->layout()->addWidget(edit);
    }

    // 窗口内的直属按钮。。看起来只有标题栏有了
    auto titlebar = mainwindow->findChild<TitleBar*>();
    auto _buttons = titlebar->findChildren<QPushButton*>();
    foreach(auto button, _buttons)
    {
        auto txt = button->text().isEmpty() ? button->toolTip() : button->text();

        if(txt.isEmpty())
            continue;

        auto item = new QListWidgetItem(_shortcuts);
        item->setSizeHint(QSize(100, 50));
        item->setData(Qt::UserRole, txt);
        _shortcuts->addItem(item);

        auto widget = new QWidget(_shortcuts);
        widget->setLayout(new QHBoxLayout(widget));
        _shortcuts->setItemWidget(item, widget);

        auto label = new QLabel(txt, widget);
        label->setFixedWidth(100);
        widget->layout()->addWidget(label);

        auto edit = new QKeySequenceEdit(widget);
        edit->setKeySequence(button->shortcut());
        widget->layout()->addWidget(edit);
    }

    // 主菜单

}

void ShortcutSettings::_save()
{
    for(int i = 0; i < _shortcuts->count(); i++)
    {
        auto item = _shortcuts->item(i);

        auto name = item->data(Qt::UserRole).toString();
        if(name.isEmpty())
            continue;

        auto row = _shortcuts->itemWidget(item);
        if(row == nullptr)
            continue;

        auto edit = row->findChild<QKeySequenceEdit*>();
        if(edit == nullptr)
            continue;

        PlayerConfig::instance()->setShortcut(name, edit->keySequence().toString());
    }

    this->accept();
}
