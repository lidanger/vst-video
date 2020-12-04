#include "favorites.h"
#include "playerconfig.h"
#include <QHeaderView>
#include <QMenu>
#include <QStandardItem>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>

Favorites::Favorites(QWidget *parent)
    : QDialog (parent)
{
    _init_control();
    _init_menu();
    _init_layout();
    _init_data();

    auto rect = parent->geometry();
    int x = rect.x() + rect.width() / 2 - this->width() / 2;
    int y = rect.y() + rect.height() / 2 - this->height() / 2 - 20;
    this->move(x, y);
}

void Favorites::_init_control()
{
    _table = new QTableView(this);
}

void Favorites::_init_menu()
{
    _menu = new QMenu(_table);
    _menu->addAction("删除", this, &Favorites::_remove);
    _menu->addAction("清空", this, &Favorites::_clear);
}

void Favorites::_init_layout()
{
    this->setWindowTitle("收藏夹");
    this->setMinimumSize(700, 450);
    this->setLayout(new QVBoxLayout(this));

    _table->setMinimumSize(150, 150);
    _table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_table, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_customContextMenuRequested(const QPoint &)));
    connect(_table, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(_doubleClicked(const QModelIndex &)));
    this->layout()->addWidget(_table);
}

void Favorites::_init_data()
{
    auto keys = PlayerConfig::instance()->favorites();
    if(keys.isEmpty())
        return;

    auto model = new QStandardItemModel();
    model->setColumnCount(3);
    model->setHorizontalHeaderItem(0, new QStandardItem("名称"));
    model->setHorizontalHeaderItem(1, new QStandardItem("分集"));
    model->setHorizontalHeaderItem(2, new QStandardItem("位置"));
    model->setHorizontalHeaderItem(3, new QStandardItem("时间"));
    _table->setModel(model);

    model->setRowCount(keys.size());

    for(int i = 0; i < keys.size(); i++)
    {
        auto key = keys[i];

        // 格式：片名|集数|位置=时间(20201203143935)|api|id
        auto items = key.split('|', QString::SkipEmptyParts);
        if(items.length() != 3)
            continue;

        auto name = items[0];
        auto part = items[1];
        auto pos = items[2];

        auto mss = pos.toLongLong() / 1000;
        if(mss > 3600)
            pos = QString("%1时%2分%3秒").arg(mss/3600).arg(mss%3600/60).arg(mss%60);
        if(mss > 60)
            pos = QString("%1分%2秒").arg(mss/60).arg(mss%60);
        else
            pos = QString("%1秒").arg(mss);

        auto time = PlayerConfig::instance()->favoriteItem(key);
        if(time.length() > 14)
        {
            time = time.left(14);
            time = QString("%1-%2-%3 %4:%5:%6")
                    .arg(time.left(4))
                    .arg(time.mid(4, 2))
                    .arg(time.mid(6, 2))
                    .arg(time.mid(8, 2))
                    .arg(time.mid(10, 2))
                    .arg(time.right(2));
        }

        model->setItem(i, 0, new QStandardItem(name));
        model->setItem(i, 1, new QStandardItem(QString::number(part.toInt() + 1)));
        model->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        model->setItem(i, 2, new QStandardItem(pos));
        model->item(i, 2)->setTextAlignment(Qt::AlignCenter);
        model->setItem(i, 3, new QStandardItem(time));
        model->item(i, 3)->setTextAlignment(Qt::AlignCenter);

        model->item(i)->setData(key);
    }

    _table->setColumnWidth(0, 300);
    _table->setColumnWidth(1, 100);
    _table->setColumnWidth(2, 100);
    _table->setColumnWidth(3, 145);

    if(model->rowCount() > 0)
        _table->selectRow(0);
}

void Favorites::_remove()
{
    auto row = _table->selectionModel()->selectedRows().first().row();
    auto model = static_cast<QStandardItemModel*>(_table->model());
    auto key = model->item(row)->data().toString();

    PlayerConfig::instance()->removeFavorite(key);
    model->removeRow(row);
}

void Favorites::_clear()
{
    PlayerConfig::instance()->clearFavorites();
    _table->model()->removeRows(0, _table->model()->rowCount());
}

void Favorites::_customContextMenuRequested(const QPoint &pos)
{
    if(_table->model() == nullptr || _table->model()->rowCount() == 0)
        return;

    auto row = _table->rowAt(pos.y());
    if(row == -1)
    {
        auto actions = _menu->findChildren<QAction*>();
        foreach(QAction *action, actions)
        {
            if(action->text() == "删除")
            {
                action->setVisible(false);
                break;
            }
        }
    }
    else
    {
        auto actions = _menu->findChildren<QAction*>();
        foreach(QAction *action, actions)
        {
            if(action->text() == "删除")
            {
                action->setVisible(true);
                break;
            }
        }
    }

    _menu->exec(QCursor::pos());
}

void Favorites::_doubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    auto model = static_cast<QStandardItemModel*>(_table->model());
    auto key = model->item(index.row())->data().toString();

    //格式：片名|集数|位置=时间(20201203143935)|api|id
    auto items = key.split('|', QString::SkipEmptyParts);
    if(items.length() != 3)
        return;

    auto name = items[0];
    auto part = items[1];
    auto pos = items[2];

    auto val = PlayerConfig::instance()->favoriteItem(key);
    items = val.split('|', QString::SkipEmptyParts);
    if(items.length() != 3)
        return;

    auto api = items[1];
    auto id = items[2];

    PlayerConfig::instance()->setCurrentVideo(name);
    PlayerConfig::instance()->setVideoCurrentPosition(name, part.toInt(), pos.toLongLong(), api, id);

    this->accept();
}
