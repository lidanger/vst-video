#include "app.h"
#include "detailpage.h"
#include "loading.h"
#include "utils.h"
#include "webresource.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QMediaPlaylist>
#include <QtConcurrent>

DetailPage::DetailPage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player)
    : QFrame (parent)
    , _video(video)
    , _player(player)
{
    connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(_player_stateChanged(QMediaPlayer::State)));
    connect(_player->playlist(), SIGNAL(currentIndexChanged(int)), this, SLOT(_playlist_currentIndexChanged(int)));

    _init();
}

bool DetailPage::eventFilter(QObject *watched, QEvent *event)
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

void DetailPage::loadInfo(int index, const QString &name, const QString &api, const QString &id, bool play, qint64 time)
{
    _name->setText(name);

    // 检查是否一致
    if (WebResource::instance()->vDetail.id.value(index).toInt() == id.toInt())
    {
        // 图片
        QString file = WebResource::instance()->topic(api, id);
        if (!isFileExists(file))
        {
            file = WebResource::instance()->app.nopic;
        }

        QPixmap pixmap(file);
        _pic->setPixmap(pixmap);
        _pic->setAlignment(Qt::AlignCenter);

        // 分集
        _loadParts();

        // 介绍
        _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vDetail, index));

        if(play)
            _play_video(time);
    }
    else
    {
        Loading::instance()->show();
        auto word = api + "|" + id;
        QtConcurrent::run(this, &DetailPage::_requestDetailVideoInfo, word, play);
    }
}

void DetailPage::_init()
{
    this->setLayout(new QHBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    this->installEventFilter(this);

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

    _name = new QLabel(this);
    _name->setMinimumSize(151, 18);
    preview->layout()->addWidget(_name);

    _desc = new QTextEdit(this);
    _desc->setMinimumSize(151, 101);
    _desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _desc->setReadOnly(true);
    preview->layout()->addWidget(_desc);

    // 播放按钮
    auto play_buttons = new QGroupBox(this);
    play_buttons->setFixedHeight(75);
    play_buttons->setLayout(new QHBoxLayout(play_buttons));
    play_buttons->layout()->setMargin(0);
    preview->layout()->addWidget(play_buttons);

    _prev = new QPushButton(this);
    _prev->setFixedSize(32, 35);
    _prev->setStyleSheet("QPushButton{border-image:url(://img/front_out.svg) 0px 0px no-repeat;background:transparent;}"
                         "QPushButton:hover{border-image:url(://img/front_on.svg) -0px 0px no-repeat;background:transparent;}");
    connect(_prev, &QPushButton::pressed, this, &DetailPage::_play_prev);
    play_buttons->layout()->addWidget(_prev);

    _play = new QPushButton(this);
    _play->setFixedSize(33, 52);
    _play->setStyleSheet("QPushButton{border-image:url(://img/play_at.svg) 0px 0px no-repeat; background: transparent;}"
                         "QPushButton:hover{background:transparent;border-image:url(://img/play_on.svg) 0px 0px no-repeat;}");
    connect(_play, &QPushButton::pressed, this, [=](){_play_video();});
    play_buttons->layout()->addWidget(_play);

    _next = new QPushButton(this);
    _next->setFixedSize(32, 35);
    _next->setStyleSheet("QPushButton{border-image:url(://img/next_out.svg) 0px 0px no-repeat;background: transparent;}"
                         "QPushButton:hover{border-image:url(://img/next_on.svg) -0px 0px no-repeat;}");
    connect(_next, &QPushButton::pressed, this, &DetailPage::_play_next);
    play_buttons->layout()->addWidget(_next);

    // 分集列表
    _partList = new QListWidget(this);
    _partList->setMinimumSize(150, 150);
    _partList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _partList->setViewMode(QListWidget::IconMode);
    _partList->setGridSize(QSize(125, 40));
    _partList->setResizeMode(QListWidget::Adjust);
    _partList->setLayoutMode(QListWidget::SinglePass);
    _partList->setSpacing(10);
    _partList->setWrapping(true);
    _partList->setTextElideMode(Qt::ElideNone);
    _partList->setSelectionMode(QAbstractItemView::SingleSelection);
    _partList->setStyleSheet("QListWidget::item:selected{border:1px solid gray;border-radius:12px;background:#0081FF;}");
    connect(_partList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(_partsList_itemDoubleClicked(QListWidgetItem *)));
    this->layout()->addWidget(_partList);
}

void DetailPage::_requestDetailVideoInfo(QString word, bool play)
{
    QStringList v = word.split("|");
    if(v.size() != 2)
        return;

    QString api = v.value(0);
    QString id = v.value(1);

    WebResource::instance()->get_detail_videoinfo_by_id(api, id);

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestDetailVideoInfo", Q_ARG(QString, word), Q_ARG(bool, play));
}

void DetailPage::_endRequestDetailVideoInfo(QString word, bool play)
{
    Q_UNUSED(word)

    _pic->setPixmap(QPixmap(":/img/timg.jpeg"));
    //_name->clear();
    _desc->clear();
    _partList->clear();

    if(WebResource::instance()->vDetail.video.isEmpty())
        return;

    // 图片
    QString file = WebResource::instance()->topic(WebResource::instance()->vDetail.api, WebResource::instance()->vDetail.id.last());
    if (isFileExists(file))
    {
        QPixmap pixmap(file);
        _pic->setPixmap(pixmap);
        _pic->setAlignment(Qt::AlignCenter); // 图片居中
    }
    else
    {
        QtConcurrent::run(this, &DetailPage::_requestDetailImage, WebResource::instance()->vDetail.pic.value(0) + "|" + WebResource::instance()->vDetail.api + "|" + WebResource::instance()->vDetail.id.value(0));
    }

    // 介绍
    _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vDetail, WebResource::instance()->vDetail.id.length() - 1));

    // 分集
    _loadParts();

    Loading::instance()->hide();

    if(play)
        _play_video();
}

void DetailPage::_requestDetailImage(QString word)
{
    if (word.isEmpty())
        return;

    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    WebResource::instance()->UrlRequestImg(v.value(0), toHash(v.value(1)) + "_" + v.value(2));

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestDetailImage", Q_ARG(QString, word));
}

void DetailPage::_endRequestDetailImage(QString word)
{
    // 取关联数据
    QStringList v = word.split("|");

    // 设置预览图片
    QString file = WebResource::instance()->topic(v.value(1), v.value(2));
    if (!isFileExists(file))
    {
        file = WebResource::instance()->app.nopic;
    }

    QPixmap pixmap(file);
    _pic->setPixmap(pixmap);
    _pic->setAlignment(Qt::AlignCenter); // 图片居中
}

void DetailPage::_player_stateChanged(QMediaPlayer::State newState)
{
    if (WebResource::instance()->app.live)
    {
        _pic_video->setCurrentWidget(_pic);

        _play->setStyleSheet("QPushButton{border-image:url(://img/play_at.svg) 0px 0px no-repeat; background: transparent;}"
                             "QPushButton:hover{background:transparent;border-image:url(://img/play_on.svg) 0px 0px no-repeat;}");
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

            _play->setStyleSheet("QPushButton{border-image:url(://img/pause_out.svg) 0px 0px no-repeat; background: transparent;}"
                                 "QPushButton:hover{background:transparent;border-image:url(://img/pause_on.svg) 0px 0px no-repeat;}");
        }
        else
        {
            _pic_video->setCurrentWidget(_pic);

            _play->setStyleSheet("QPushButton{border-image:url(://img/play_at.svg) 0px 0px no-repeat; background: transparent;}"
                                 "QPushButton:hover{background:transparent;border-image:url(://img/play_on.svg) 0px 0px no-repeat;}");
        }
    }
}

void DetailPage::_playlist_currentIndexChanged(int index)
{
    _partList->setCurrentRow(index);
}

void DetailPage::_loadParts(int index)
{
    // 停止发信号
    _partList->blockSignals(true);

    _partList->clear();

    auto list = WebResource::instance()->vDetail.video.last().split("#");
    foreach (QString s, list)
    {
        // 第30集$https://index.m3u8$ckm3u8
        auto v = s.trimmed().split("$");

        QListWidgetItem *item = nullptr;
        if (v.size() == 3)
        {
            item = new QListWidgetItem(v.value(0));
            item->setData(Qt::UserRole, v.value(1));
        }
        else
        {
            item = new QListWidgetItem("高清");
            item->setData(Qt::UserRole, v.value(0));
        }

        item->setSizeHint(QSize(120, 40));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        _partList->addItem(item);
    }

    // 开启发信号
    _partList->blockSignals(false);

    // 设置选择项
    if(_partList->count() > 0 && index < _partList->count())
    {
        _partList->setCurrentRow(index);
    }
}

void DetailPage::_play_video(qint64 time)
{
    QStringList urls;
    QStringList names;
    for (int i = 0; i < _partList->count(); i++)
    {
        urls.push_back(_partList->item(i)->data(Qt::UserRole).toString());
        names.push_back(_partList->item(i)->text());
    }

    emit aboutToPlay(_name->text(), names, urls, _partList->currentRow(), time);
}

void DetailPage::_play_prev()
{
    int index = _partList->currentRow();
    if (index != 0)
    {
        _partList->setCurrentRow(index - 1);
    }
}

void DetailPage::_play_next()
{
    int index = _partList->currentRow();
    if (index < _partList->count() - 1)
    {
        _partList->setCurrentRow(index + 1);
    }
}

void DetailPage::_partsList_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);

    _play_video();
}
