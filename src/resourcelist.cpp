#include "app.h"
#include "loading.h"
#include "playerconfig.h"
#include "resourcelist.h"
#include "utils.h"
#include "webresource.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QtConcurrent>

ResourceList::ResourceList(QMainWindow* parent)
    : QFrame (parent)
    , _window (parent)
{
    _init();
    reload();
}

void ResourceList::reload()
{
    _tree_model->removeRows(0, _tree_model->rowCount());
    _tree_model->setItem(0, 0, new QStandardItem("正在刷新..."));

    Loading::instance()->show();
    WebResource::instance()->sources.clear();

    // 视频分类
    auto ress = WebResource::instance()->getResources();
    for(int i = 0; i < ress.size(); i++)
    {
        auto name = ress.keys()[i];
        auto api = ress.values()[i];

        auto item = new QStandardItem(name);
        item->setData(false);
        _tree_model->setItem(i, 0, item);

        QtConcurrent::run(this, &ResourceList::_requestClassResources, name, api);
    }

    // 直播分类
    auto live = new QStandardItem(LIVE_CLASS_NAME);
    live->setData(true);
    _tree_model->setItem(ress.size(), 0, live);
    _tree->expand(live->index());

    QtConcurrent::run(this, &ResourceList::_requestLiveResources);
}

void ResourceList::_init()
{
    this->setLayout(new QHBoxLayout(this));
//    this->layout()->setMargin(0);
//    this->layout()->setSpacing(0);

    // 资源列表
    _tree = new QTreeView(this);
    _tree->setMinimumSize(100, 100);
    _tree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(_tree, SIGNAL(pressed(const QModelIndex &)), this, SLOT(_tree_pressed(const QModelIndex &)));
    this->layout()->addWidget(_tree);

    _tree_model = new QStandardItemModel(_tree);
    _tree->header()->hide();
    _tree->setModel(_tree_model);
    _tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ResourceList::_requestLiveResources()
{
    WebResource::instance()->getLives();

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, &ResourceList::_endRequestResources);
}

void ResourceList::_requestClassResources(const QString &name, const QString &api)
{
    WebResource::instance()->getResourceInfo(name, api);

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, &ResourceList::_endRequestResources);
}

void ResourceList::_endRequestResources()
{
    Loading::instance()->hide();

    QSet<QString> filters;
    auto val = PlayerConfig::instance()->resourceFilters();
    if(!val.isEmpty())
    {
        filters = val.split('|', QString::SkipEmptyParts).toSet();
    }

    foreach(auto source, WebResource::instance()->sources)
    {
        auto clss = _tree_model->findItems(source.name);
        if(clss.empty())
            continue;

        auto cls = clss.first();

        // 某类资源已添加就不再重复添加了
        if(cls->hasChildren())
            continue;

        foreach (auto item, source.items)
        {
            if(filters.contains(item.name))
                continue;

            cls->appendRow(new QStandardItem(item.name));
        }
    }
}

// 树形框项目被选择
void ResourceList::_tree_pressed(const QModelIndex &index)
{
    auto parent_index = index.parent().row();
    if(parent_index == -1)
        return;

    auto item = _tree_model->item(parent_index, 0);
    if(item == nullptr)
        return;

    auto live = item->data().toBool();

    emit resourceSelected(item->text(), index.row(), live);
}
