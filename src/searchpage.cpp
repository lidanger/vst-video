#include "app.h"
#include "loading.h"
#include "searchpage.h"
#include "utils.h"
#include "webresource.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QtConcurrent>

SearchPage::SearchPage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player)
    : QFrame (parent)
    , _video(video)
    , _player(player)
{
    connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(_player_stateChanged(QMediaPlayer::State)));

    _init();
}

void SearchPage::startSearch(const QString &keyword, const QString &source)
{
    if (keyword.trimmed().isEmpty())
    {
        if(_model != nullptr)
            _model->removeRows(0, _model->rowCount());
    }
    else
    {
        if(_searchString != keyword || _model == nullptr || _model->rowCount() == 0)
        {
            _searchString = keyword;

            Loading::instance()->show();
            QtConcurrent::run(this, &SearchPage::_requestSearch, keyword, source);
        }
    }
}

bool SearchPage::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Hide)
    {
        if(watched == this)
        {
            _pic_video->setCurrentWidget(_pic);
        }
    }
    else if(event->type() == QEvent::Show)
    {
        _player_stateChanged(_player->state());
    }

    return QFrame::eventFilter(watched, event);
}

void SearchPage::_init()
{
    this->setLayout(new QHBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    this->installEventFilter(this);

    // 搜索表格列表视图
    _tableView = new QTableView(this);
    _tableView->setMinimumSize(200, 100);
    _tableView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    _tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    _tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _tableView->horizontalHeader()->setStretchLastSection(true);
    _tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    _tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_tableView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_exploreMenu(const QPoint &)));
    connect(_tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(_tableView_doubleClicked(const QModelIndex &)));
    this->layout()->addWidget(_tableView);

    // 数据模型
    _model = new QStandardItemModel();
    _model->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("视频名称")));
    //_model->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    _model->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("资源站")));
    _model->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("视频分类")));
    _model->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("视频ID")));
    _model->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("更新日期")));
    _tableView->setModel(_model);

    // 设置表格的各列的宽度值
    _tableView->setColumnWidth(0, 380);
    _tableView->setColumnWidth(1, 90);
    _tableView->setColumnWidth(2, 90);
    _tableView->setColumnWidth(3, 70);
    _tableView->setColumnWidth(4, 150);

    connect(_tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(_tableView_currentRowChanged(const QModelIndex &, const QModelIndex &)));

    // 右键菜单
    _contextMenu = new QMenu(_tableView);
    _contextMenu->addAction("播放", this, [=](){_tableView_doubleClicked(_tableView->currentIndex());});
    _contextMenu->addSeparator();
    _contextMenu->addAction("查看详情", this, &SearchPage::_show_detail);

    // 预览区
    auto preview = new QGroupBox(this);
    preview->setFixedWidth(300);
    preview->setLayout(new QVBoxLayout(preview));
    this->layout()->addWidget(preview);

    _pic_video = new QStackedWidget(this);
    _pic_video->setMinimumSize(151, 151);
    _pic_video->setMaximumHeight(preview->width() - preview->layout()->margin() * 2 - preview->layout()->spacing());
    preview->layout()->addWidget(_pic_video);

    _pic = new QLabel(this);
    _pic->setMinimumSize(151, 151);
    _pic->setMaximumHeight(preview->width() - preview->layout()->margin() * 2 - preview->layout()->spacing());
    _pic->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _pic->setMargin(0);
    _pic->setScaledContents(true);
    _pic->setPixmap(QPixmap(":/img/timg.jpeg"));
    _pic_video->addWidget(_pic);

    _parts = new QLabel(this);
    _parts->setMinimumWidth(151);
    _parts->setFixedHeight(18);
    _parts->setText("集数：");
    preview->layout()->addWidget(_parts);

    _desc = new QTextEdit(this);
    _desc->setMinimumSize(151, 101);
    _desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _desc->setReadOnly(true);
    preview->layout()->addWidget(_desc);
}

void SearchPage::_requestSearch(const QString &word, const QString &source)
{
    if (word.isEmpty())
        return;

    // 获取搜索影片数据
    QStringList v = word.split("|");
    WebResource::instance()->search(v.value(0), source);

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, &SearchPage::_endRequestSearch);
}

void SearchPage::_endRequestSearch()
{
    _model->removeRows(0, _model->rowCount());

    int row = 0;
    QString word;
    for (int i = 0; i < WebResource::instance()->vSearch.size(); i++)
    {
        for (int i2 = 0; i2 < WebResource::instance()->vSearch.value(i).id.size(); i2++, row++)
        {
            _model->setItem(row, 0, new QStandardItem(WebResource::instance()->vSearch.value(i).name.value(i2)));
            _model->item(row, 0)->setForeground(QBrush(QColor(255, 0, 0)));

            _model->setItem(row, 1, new QStandardItem(WebResource::instance()->vSearch.value(i).sname));
            _model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

            _model->setItem(row, 2, new QStandardItem(WebResource::instance()->vSearch.value(i).tname.value(i2)));
            _model->item(row, 2)->setTextAlignment(Qt::AlignCenter);

            _model->setItem(row, 3, new QStandardItem(WebResource::instance()->vSearch.value(i).id.value(i2)));
            _model->item(row, 3)->setTextAlignment(Qt::AlignCenter);

            _model->setItem(row, 4, new QStandardItem(WebResource::instance()->vSearch.value(i).last.value(i2)));
            _model->item(row, 4)->setTextAlignment(Qt::AlignCenter);

            // vSearch.value(i).pic.value(i2) 是空的
            word = WebResource::instance()->vSearch.value(i).pic.value(i2) + "|" + WebResource::instance()->vSearch.value(i).api + "|" + WebResource::instance()->vSearch.value(i).id.value(i2);

            _model->item(row, 0)->setData(word);
        }
    }

    // 默认选择第一个
    if(_model->rowCount() > 0)
        _tableView->selectRow(0);

    Loading::instance()->hide();
}

void SearchPage::_requestPreviewImage(const QString &word)
{
    if (word.isEmpty())
        return;

    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    QString url = v.value(0);
    // 搜索的图片地址是空的,需要从视频信息中获取
    if(_tableView->isVisible())
        url = WebResource::instance()->vBrowse.pic.first();

    if(url.isEmpty())
    {
        qDebug() << WebResource::instance()->vBrowse.name << " " << v.value(2) << "图片地址无效" << endl;
        return;
    }

    WebResource::instance()->UrlRequestImg(url, toHash(v.value(1)) + "_" + v.value(2));

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestPreviewImage", Q_ARG(const QString &, word));
}

void SearchPage::_endRequestPreviewImage(const QString &word)
{
    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    // 设置预览图片
    QString file = WebResource::instance()->topic(v.value(1), v.value(2));
    if (!isFileExists(file))
    {
        file = WebResource::instance()->app.nopic;
    }

    QPixmap pixmap(file);
    _pic->setPixmap(pixmap);
    _pic->setAlignment(Qt::AlignCenter);

    _pic_video->setCurrentWidget(_pic);
}

void SearchPage::_tableView_currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    if (current.row() == -1)
        return;

    QString word = _model->item(current.row(), 0)->data().toString();
    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    // 清空原来的预览信息
    _pic->clear();
    _parts->clear();
    _desc->clear();

    // 检查选中内容的数据是否与当前视频信息一致
    if (WebResource::instance()->vBrowse.id.value(current.row()).toInt() == v.value(2).toInt())
    {
        // 介绍
        _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vBrowse, current.row()));

        // 集数
        QStringList list = WebResource::instance()->vBrowse.video.first().split("#");
        _parts->setText(QString::asprintf(" 共 %d 集", list.length()));

        // 照片
        QString file = WebResource::instance()->topic(WebResource::instance()->vBrowse.api, WebResource::instance()->vBrowse.id.value(current.row()));
        if (isFileExists(file))
        {
            QPixmap pixmap(file);
            _pic->setPixmap(pixmap);
            _pic->setAlignment(Qt::AlignCenter);

            _pic_video->setCurrentWidget(_pic);
        }
        else
            QtConcurrent::run(this, &SearchPage::_requestPreviewImage, word);
    }
    else
    {
        Loading::instance()->show();
        QtConcurrent::run(this, &SearchPage::_requestPreviewVideoInfo, word);
    }
}

void SearchPage::_requestPreviewVideoInfo(const QString &word)
{
    if(word.isEmpty())
        return;

    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    //QMutexLocker locker(&_browse_locker);

    QString api = v.value(1);
    QString id = v.value(2);
    WebResource::instance()->getvideo(2, api, id);

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestPreviewVideoInfo", Q_ARG(const QString &, word));
}

void SearchPage::_endRequestPreviewVideoInfo(const QString &word)
{
    // 未请求到信息
    if(WebResource::instance()->vBrowse.id.isEmpty())
        return;

    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    if(v.value(1) != WebResource::instance()->vBrowse.api || v.value(2) != WebResource::instance()->vBrowse.id.first())
        return;

    // 图片
    QString file = WebResource::instance()->topic(v.value(1), v.value(2));
    if (isFileExists(file))
    {
        QPixmap pixmap(file);
        _pic->setPixmap(pixmap);
        _pic->setAlignment(Qt::AlignCenter);

        _pic_video->setCurrentWidget(_pic);
    }
    else
        QtConcurrent::run(this, &SearchPage::_requestPreviewImage, word);

    // 集数
    QStringList list = WebResource::instance()->vBrowse.video.first().split("#");
    _parts->setText(QString::asprintf(" 共 %d 集", list.length()));

    // 介绍
    _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vBrowse, 0));

    Loading::instance()->hide();
}

void SearchPage::_player_stateChanged(QMediaPlayer::State newState)
{
    if (WebResource::instance()->app.live)
    {
        _pic_video->setCurrentWidget(_pic);
    }
    else
    {
        if(newState == QMediaPlayer::PlayingState)
        {
            if(this->isVisible())
            {
                _pic_video->addWidget(_video);
                _pic_video->setCurrentWidget(_video);
            }
        }
        else
        {
            _pic_video->setCurrentWidget(_pic);
        }
    }
}

void SearchPage::_show_detail()
{
    if(_model->rowCount() == 0)
        return;

    auto index = _tableView->currentIndex().row();
    auto name = _model->item(index, 0)->text();

    auto v = _model->item(index, 0)->data().toString().split("|");

    emit requestShowDetail(index, name, v.at(1), v.at(2));
}

void SearchPage::_tableView_doubleClicked(const QModelIndex &index)
{
    if(_model->rowCount() == 0)
        return;

    auto name = _model->item(index.row(), 0)->text();

    auto v = _model->item(index.row(), 0)->data().toString().split("|");

    emit itemSelectToPlay(index.row(), name, v.at(1), v.at(2));
}

// 浏览器弹出菜单
void SearchPage::_exploreMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    _contextMenu->exec(QCursor::pos());
}
