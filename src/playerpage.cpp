#include "loading.h"
#include "playerconfig.h"
#include "playerpage.h"
#include "utils.h"
#include "webresource.h"

#include <QComboBox>
#include <QCompleter>
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMediaMetaData>
#include <QMessageBox>
#include <QStackedLayout>

PlayerPage::PlayerPage(QWidget* parent)
    : QFrame (parent)
{
    _initControl();
    _initLayout();
    _initMenu();
}

void PlayerPage::startPlayLive(int index)
{
    WebResource::instance()->app.live = true;
    WebResource::instance()->app.local = false;

    _player->stop();
    _playlist->clear();

    // 添加所有直播资源到播放列表
    foreach (auto info, WebResource::instance()->sources)
    {
        if(!info.name.contains("直播"))
            continue;

        foreach (auto var, info.items)
        {
            _playlist->addMedia(QUrl(var.id));
            _video_names.push_back(var.name);
        }
    }

    _progress->setEnabled(true);
    //_name_label->show();
    _name->setText(_video_names.value(index));
    _name->show();
    _part->hide();

    _video->setUpdatesEnabled(false);

    _playlist->setCurrentIndex(index);
    _player->play();

    _video->setUpdatesEnabled(false);

    _panel_timer->start(3000);
    _last_name = nullptr;
}

bool PlayerPage::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        auto ket = static_cast<QKeyEvent*>(event);

        if (ket->key() == Qt::Key_Escape)
        {
            _video->setCursor(Qt::ArrowCursor);

            if (_control->isHidden())
            {                
                _control->show();
            }

            emit requestShowTitleBar(true);
        }
        else if (ket->key() == Qt::Key_Enter || ket->key() == 16777220)
        {
            emit requestShowFullScreen();
            return true;
        }
        else if (ket->key() == Qt::Key_Space)
        {
            _switchPlayPause();
            return true;
        }
        else if (ket->key() == Qt::Key_Left)
        {
            if (!WebResource::instance()->app.live)
                _player->setPosition(_player->position() - 10000);
            return true;
        }
        else if (ket->key() == Qt::Key_Right)
        {
            if (!WebResource::instance()->app.live)
                _player->setPosition(_player->position() + 10000);
            return true;
        }
        else if (ket->key() == Qt::Key_Up)
        {
            if(watched != _part && watched != _contextMenu)
            {
                _player->setVolume(_player->volume() + 1);
                _showVolumeSlider();
                return true;
            }
        }
        else if (ket->key() == Qt::Key_Down)
        {
            if(watched != _part && watched != _contextMenu)
            {
                _player->setVolume(_player->volume() - 1);
                _showVolumeSlider();
                return true;
            }
        }
    }
    else if(event->type() == QEvent::Show)
    {
        if(watched == this)
        {
            _video_container->addWidget(_video);
        }

        this->setFocus();
    }
    else if(event->type() == QEvent::MouseButtonDblClick)
    {
        if(watched == _video)
        {
            emit requestShowPlayer();
        }

        this->setFocus();
    }
    else if (event->type() == QEvent::HoverEnter)
    {
        // 音量按钮鼠标移动事件
        if (watched == _volume)
        {
            _showVolumeSlider();
        }

        this->setFocus();
    }
    else if (event->type() == QEvent::MouseMove)
    {
        _panel_timer->start(3000);

        _video->setCursor(Qt::ArrowCursor);

        if (_control->isHidden())
        {
            _control->show();            
        }

        emit requestShowTitleBar(true);

        this->setFocus();
    }
    else if(event->type() == QEvent::Resize)
    {
        if(watched == this)
        {
            if(WebResource::instance()->app.isFullScreen)
                _control->hide();
            else
                _control->show();
        }
    }

    return QFrame::eventFilter(watched, event);
}

void PlayerPage::_name_linkActivated(const QString &link)
{
    Q_UNUSED(link)

    emit requestShowDetail();
}

void PlayerPage::_switchPlayPause()
{
    if(_player->state() == QMediaPlayer::PlayingState)
    {
        _player->pause();
    }
    else
    {
        _player->play();
    }
}

void PlayerPage::startPlayVideo(const QString &name, const QStringList &part_names, const QStringList &part_urls, int index, qint64 time)
{
    _current_name = name;
    _current_api = WebResource::instance()->vDetail.api;
    _current_id = WebResource::instance()->vDetail.id.first();

    WebResource::instance()->app.live = false;
    WebResource::instance()->app.local = false;

    //_name_label->show();
    _name->show();
    _part->show();
    _progress->setEnabled(true);

    _video->setUpdatesEnabled(true);

    _player->stop();

    _name->setText(QString("<a href='https://github.com/xymov/vst-video/'>%1</a>").arg(name));

    _playlist->blockSignals(true);
    _part->blockSignals(true);

    _part->clear();
    _playlist->clear();
    for(int i = 0; i < part_names.length(); i++)
    {
        _part->addItem(part_names[i]);
        _playlist->addMedia(QUrl(part_urls[i]));
    }

    _playlist->blockSignals(false);
    _part->blockSignals(false);

    // 设置分集输入自动选择
    auto completer = new QCompleter(_part->model(), this);
    completer->setFilterMode(Qt::MatchContains);
    _part->setCompleter(completer);

    _playlist->setCurrentIndex(index);
    _part->setCurrentIndex(index);

    _player->setPosition(time);
    _player->play();

    _video->setUpdatesEnabled(false);

    _panel_timer->start(3000);

    if(_last_name != name)
    {
        PlayerConfig::instance()->setCurrentVideo(name);
        _last_name = name;
    }
}

void PlayerPage::startPlayVideoFile(const QStringList &files)
{
    if (files.isEmpty())
        return;

    WebResource::instance()->app.live = false;
    WebResource::instance()->app.local = true;

    _player->stop();
    _playlist->clear();
    for (int i = 0; i < files.count(); i++)
    {
        _playlist->addMedia(QUrl::fromLocalFile(files.value(i)));
    }

    _player->play();
    _panel_timer->start(3000);
}

void PlayerPage::_initLayout()
{
    this->setLayout(new QVBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    this->installEventFilter(this);

    // 播放控件
    _video_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->layout()->addWidget(_video_container);

    _video->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _video->setStyleSheet("QVideoWidget{background:black;}");
    _video->setMouseTracking(true);
    _video->setAttribute(Qt::WA_OpaquePaintEvent);
    _video->setAspectRatioMode(Qt::KeepAspectRatio);
    _video->show();
    _video->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_video, static_cast<void (QWidget::*)(const QPoint &)>(&QVideoWidget::customContextMenuRequested), this, [=](const QPoint &){_contextMenu->exec(QCursor::pos());});
    _video->installEventFilter(this);
    _video_container->addWidget(_video);

//    //视频设备
//    GVI = new QGraphicsVideoItem();
//    GVI->setAspectRatioMode(Qt::KeepAspectRatio);
//    // 缩放 Qt::KeepAspectRatio,
//    // 铺满 Qt::IgnoreAspectRatio ，
//    // 拉伸 Qt::KeepAspectRatioByExpanding

//    //视频默认缩放
//    ui->action_videosize_KeepAspectRatio->setChecked(true);


//    //文字设备
//    GTI = new QGraphicsTextItem;
//    GTI->setDefaultTextColor(Qt::white);
//    QFont font = this->font();
//    font.setPixelSize(20);
//    GTI->setFont(font);

//    //添加进场景
//    scene = new QGraphicsScene();
//    scene->setBackgroundBrush(Qt::black);
//    scene->addItem(GVI);
//    scene->addItem(GTI);


//    // 绑定场景
   // QGraphicsView
//    // ui->view->fitInView(GVI,Qt::KeepAspectRatio);
//    ui->view->setScene(scene);
//    ui->view->horizontalScrollBar()->hide();  //不显示横向滚动条
//    ui->view->verticalScrollBar()->hide();   //不显示纵向滚动条
//    ui->view->setRenderHint(QPainter::SmoothPixmapTransform);  //平滑


//    //获取场景内部viewport区域,
//    viewWidget = ui->view->viewport();
//    //监视鼠标消息
//    viewWidget->installEventFilter(this);

//    player->setVideoOutput(GVI);
//    ui->view->show();

    // 初始化播放器
    _player->setVideoOutput(_video);
    _player->setVolume(100);                                    // 默认音量最大
    // 视频长度状态发生改变
    connect(_player, SIGNAL(durationChanged(qint64)), this, SLOT(_player_durationChange(qint64)));
    // 当前媒体的打开状态已更改
    connect(_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(_player_mediaStatusChanged(QMediaPlayer::MediaStatus)));
    // 当前媒体的播放状态已更改
    connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(_player_stateChanged(QMediaPlayer::State)));
    connect(_player, SIGNAL(positionChanged(qint64)), this, SLOT(_player_positionChange(qint64)));
    connect(_player, SIGNAL(volumeChanged(int)), this, SLOT(_player_volumeChange(int)));

    _playlist->playbackModeChanged(QMediaPlaylist::Sequential); // 顺序播放模式
    connect(_playlist, SIGNAL(currentIndexChanged(int)), _part, SLOT(setCurrentIndex(int)));
    _player->setPlaylist(_playlist);

    connect(_play_timer, &QTimer::timeout, this, &PlayerPage::_prevent_black_screen);
    // 每 1 分钟模拟一次鼠标事件
    _play_timer->setInterval(30 * 1000);

    // 控制面板
    _control->setFixedHeight(80);
    _control->setLayout(new QVBoxLayout(_control));
    _control->installEventFilter(this);
    this->layout()->addWidget(_control);

    // 控制板隐藏定时器
    _panel_timer->setSingleShot(false);
    connect(_panel_timer, &QTimer::timeout, this, &PlayerPage::_control_timeOut);

    // 播放进度条
    _progress->setFixedHeight(25);
    connect(_progress, &QSlider::sliderReleased, this, [=](){_player->setPosition(_progress->value());});
    _control->layout()->addWidget(_progress);

    // 播放按钮面板
    auto _control_buttons = new QFrame(this);
    _control_buttons->setFixedHeight(45);
    _control_buttons->setLayout(new QHBoxLayout(_control_buttons));
    _control_buttons->layout()->setMargin(0);
    _control_buttons->layout()->setSpacing(15);
    _control_buttons->setStyleSheet("QFrame{margin:0 5 6 5;}");
    _control->layout()->addWidget(_control_buttons);

    // 播放按钮
    _play->setToolTip("播放");
    _play->setIconSize(QSize(16, 16));
    _play->setFixedSize(20, 32);
    _play->setStyleSheet("QPushButton{border-image:url(://img/play_out.svg) 0px 0px no-repeat;}"
                         "QPushButton:hover{border-image:url(://img/play_on.svg) -0px 0px no-repeat;}");
    _play->setAutoDefault(true);
    _play->setDefault(true);
    connect(_play, &QPushButton::pressed, this, &PlayerPage::_switchPlayPause);
    _control_buttons->layout()->addWidget(_play);

    _prev->setToolTip("上一集");
    _prev->setIconSize(QSize(16, 16));
    _prev->setFixedSize(28, 32);
    _prev->setStyleSheet("QPushButton{border-image:url(://img/front_out.svg) 0px 0px no-repeat;}"
                         "QPushButton:hover{border-image:url(://img/front_on.svg) -0px 0px no-repeat;}"
                         "QPushButton:disabled{border-image:url(://img/front_off.svg) -0px 0px no-repeat;}");
    connect(_prev, &QPushButton::pressed, _playlist, &QMediaPlaylist::previous);
    _control_buttons->layout()->addWidget(_prev);

    _next->setToolTip("下一集");
    _next->setIconSize(QSize(16, 16));
    _next->setFixedSize(28, 32);
    _next->setStyleSheet("QPushButton{border-image:url(://img/next_out.svg) 0px 0px no-repeat;}"
                         "QPushButton:hover{border-image:url(://img/next_on.svg) -0px 0px no-repeat;}"
                         "QPushButton:disabled{border-image:url(://img/next_off.svg) -0px 0px no-repeat;}");
    connect(_next, &QPushButton::pressed, _playlist, &QMediaPlaylist::next);
    _control_buttons->layout()->addWidget(_next);

    // 音量
    _volume->setIconSize(QSize(16, 16));
    _volume->setFixedSize(30, 32);
    _volume->setStyleSheet("QPushButton{border-image:url(://img/volume-up_out.svg) 0px 0px no-repeat;filter:alpha(Opacity=80);-moz-opacity:0.5;opacity: 0.5;}"
                           "QPushButton:hover{border-image:url(://img/volume-up_on.svg) -0px 0px no-repeat;border:none;}");
    _volume->installEventFilter(this);
    _control_buttons->layout()->addWidget(_volume);

    // 音量调节
    _volume_slider->setMinimumSize(30, 100);
    _volume_slider->setMaximumSize(30, 100);
    _volume_slider->setMinimum(0);
    _volume_slider->setMaximum(100);
    _volume_slider->setPageStep(2);
    _volume_slider->setValue(100);
    _volume_slider->setTickPosition(QSlider::NoTicks);
    _volume_slider->hide();

    // 显示音量的标签
    _volume_value->setMinimumSize(100, 30);
    _volume_value->setStyleSheet("color:green;");
    _volume_value->hide();

    connect(_volume_slider, SIGNAL(valueChanged(int)), this, SLOT(_volume_slider_valueChanged(int)));

    // 音量显/隐定时器
    _volume_slider_timer->setInterval(3000);
    connect(_volume_slider_timer, &QTimer::timeout, this, &PlayerPage::_hideVolumeSlider);

    _volume_slider->installEventFilter(this);

    // 进度显示
    _time->setFixedSize(150, 40);
    _time->setText("00:00:00/00:00:00");
    _control_buttons->layout()->addWidget(_time);

    // 片名
    //_name_label->setFixedHeight(40);
    //_name_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //_name_label->hide();
    //_control_buttons->layout()->addWidget(_name_label);

    _name->setFixedHeight(40);
    _name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(_name, SIGNAL(linkActivated(const QString &)), this, SLOT(_name_linkActivated(const QString &)));
    _name->hide();
    _control_buttons->layout()->addWidget(_name);

    _part->setMinimumSize(120, 32);
    _part->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _part->setEditable(true);
    connect(_part, SIGNAL(currentIndexChanged(int)), _playlist, SLOT(setCurrentIndex(int)));
    _part->hide();
    _control_buttons->layout()->addWidget(_part);

    static_cast<QBoxLayout*>(_control_buttons->layout())->addStretch();

    _status->setFixedSize(100, 40);
    _control_buttons->layout()->addWidget(_status);

    _setting->setIconSize(QSize(16, 16));
    _setting->setFixedSize(32, 28);
    _setting->setStyleSheet("QPushButton{border-image:url(://img/setting_out.svg) 0px 0px no-repeat;}QPushButton::menu-indicator{image:none;}"
                            "QPushButton:hover{border-image:url(://img/setting_on.svg) -0px 0px no-repeat;border:none;}QPushButton::menu-indicator{image:none;}");
    _setting->setMenu(_contextMenu);
    _control_buttons->layout()->addWidget(_setting);
}

void PlayerPage::_initMenu()
{
    connect(_contextMenu, &QMenu::aboutToShow, this, &PlayerPage::_aboutToShowMenu);

    auto open = _contextMenu->addMenu("打开");
    open->addAction("本地文件", this, &PlayerPage::_openLocalFile);
    open->addAction("远程地址", this, &PlayerPage::_openUrl);

    _contextMenu->addSeparator();

    auto light = _contextMenu->addMenu("亮度");
    light->addAction("亮度 +", this, [=](){_video->setBrightness(_video->brightness() + 10);});
    light->addAction("亮度 -", this, [=](){_video->setBrightness(_video->brightness() - 10);});

    auto contrast = _contextMenu->addMenu("对比度");
    contrast->addAction("对比度 +", this, [=](){_video->setContrast(_video->contrast() + 10);});
    contrast->addAction("对比度 -", this, [=](){_video->setContrast(_video->contrast() - 10);});

    auto saturation = _contextMenu->addMenu("饱和度");
    saturation->addAction("饱和度 +", this, [=](){_video->setSaturation(_video->saturation() + 10);});
    saturation->addAction("饱和度 -", this, [=](){_video->setSaturation(_video->saturation() - 10);});

    _contextMenu->addSeparator();

    auto scale = _contextMenu->addMenu("缩放");
    auto group = new QActionGroup(scale);
    auto action = scale->addAction("填满", this, [=](){_video->setAspectRatioMode(Qt::IgnoreAspectRatio);});
    group->addAction(action);
    action = scale->addAction("缩放", this, [=](){_video->setAspectRatioMode(Qt::KeepAspectRatio);});
    group->addAction(action);
    action = scale->addAction("拉伸", this, [=](){_video->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);});
    group->addAction(action);

//    auto rotate = _contextMenu->addMenu("旋转");
//    rotate->addAction("顺时针旋转", this, &PlayerPage::_video_rotate_left);
//    rotate->addAction("逆时针旋转", this, &PlayerPage::_video_rotate_right);
//    rotate->addAction("水平镜像", this, &PlayerPage::_video_rotate_x);
//    rotate->addAction("垂直镜像", this, &PlayerPage::_video_rotate_y);

    _contextMenu->addSeparator();

    _contextMenu->addAction("视频信息", this, &PlayerPage::_showVideoInfo)->setObjectName("menu_video_info");

    _contextMenu->addAction("刷新", this, &PlayerPage::_reload_video)->setObjectName("menu_video_refresh");

    _contextMenu->addSeparator();

    _contextMenu->addAction("收藏", this, &PlayerPage::_addFavorite)->setObjectName("menu_video_favorite");
}

void PlayerPage::_openLocalFile()
{
    QString filter = "视频文件(*.mp4 *.avi *.wmv *.mpg *.mpeg *.m3u8 *.rmvb *.3gp *.mov *.swf *.flv);;音频文件(*.mp3 *. *.wav *.flac *.ogg *.ape *.cda *.mid *.voc);;所有文件(*.*)"; // 文件过滤器
    auto files = QFileDialog::getOpenFileNames(this, "选择多个文件", QDir::currentPath(), filter);
    if (files.count() == 0)
        return;

    _player->stop();

    _playlist->clear();

    for (int i = 0; i < files.count(); i++)
    {
        _playlist->addMedia(QUrl::fromLocalFile(files.value(i)));
    }

    _player->play();
}

void PlayerPage::_openUrl()
{
    bool ok = false;
    auto text = QInputDialog::getText(this, "打开URL", "请输入URL地址", QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        _player->setMedia(QUrl(text));
        _player->play();
    }
}

void PlayerPage::_showVideoInfo()
{
    if(_player->state() == QMediaPlayer::StoppedState)
        return;

    QString info;
    auto metaData = _player->availableMetaData();
    for(int i = 0; i < metaData.size(); i++)
    {
        if(metaData.at(i) == "PixelAspectRatio" || metaData.at(i) == "Resolution")
        {
            info += metaData.at(i) + ": " + QString::number(_player->metaData(metaData.at(i)).toSize().width()) + " X " + QString::number(_player->metaData(metaData.at(i)).toSize().height()) + "\n";
        }
        else
        {
            info += metaData.at(i) + ": " + _player->metaData(metaData.at(i)).toString() + "\n";
        }
    }

    QMessageBox dlg(QMessageBox::Information, "视频信息", info, QMessageBox::Ok, static_cast<QWidget*>(this->parent()));

    auto cover = _player->metaData(QMediaMetaData::ThumbnailImage).value<QImage>();
    if(!cover.isNull())
    {
        dlg.setIconPixmap(QPixmap::fromImage(cover));
    }

    dlg.exec();
}

void PlayerPage::_addFavorite()
{
    PlayerConfig::instance()->addFavorite(_current_name,
                                          _playlist->currentIndex(),
                                          _player->position(),
                                          WebResource::instance()->vDetail.api,
                                          WebResource::instance()->vDetail.id.first());
}

void PlayerPage::_player_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
    case QMediaPlayer::UnknownMediaStatus:
        _status->setText("状态未知");
        Loading::instance()->hide();
        break;
    case QMediaPlayer::NoMedia:
        _status->setText("无效视频");
        Loading::instance()->show();
        break;
    case QMediaPlayer::LoadingMedia:
        _status->setText("正在加载");
        _progress->setEnabled(false);
        Loading::instance()->show();
        _part->setCurrentIndex(_playlist->currentIndex());
        //qDebug() << "openUrl：" << _player->currentMedia().canonicalUrl().toString();
        break;
    case QMediaPlayer::LoadedMedia:
        _status->setText("准备就绪");
        Loading::instance()->hide();
        break;
    case QMediaPlayer::StalledMedia:
        _status->setText("正在缓冲");
        Loading::instance()->show();
        break;
    case QMediaPlayer::BufferingMedia:
        _status->setText("正在缓冲");
        break;
    case QMediaPlayer::BufferedMedia:
        _status->setText("正在播放");
        Loading::instance()->hide();
        break;
    case QMediaPlayer::EndOfMedia:
        _status->setText("播放结束");
        Loading::instance()->hide();
        break;
    case QMediaPlayer::InvalidMedia:
        _status->setText("加载失败");
        Loading::instance()->hide();
        break;
    }
}

void PlayerPage::_player_stateChanged(QMediaPlayer::State state)
{
    QString play = "QPushButton{border-image:url(:/img/play_out.svg) 0px 0px no-repeat;}QPushButton:hover{border-image:url(:/img/play_on.svg) -0px 0px no-repeat;}";
    QString pause = "QPushButton{border-image:url(:/img/pause_out.svg) 0px 0px no-repeat;}QPushButton:hover{border-image:url(:/img/pause_on.svg) -0px 0px no-repeat;}";

    switch (state)
    {
    case QMediaPlayer::PlayingState:
        _status->setText("正在播放");
        _play->setStyleSheet(pause);
        _play_timer->start();
        break;
    case QMediaPlayer::PausedState:
        _status->setText("播放暂停");
        _play->setStyleSheet(play);
        break;
    case QMediaPlayer::StoppedState:
        _status->setText("播放停止");
        _play->setStyleSheet(play);
        break;
    }
}

void PlayerPage::_player_durationChange(qint64 playtime)
{
    if(playtime == 0)
        return;

    _progress->setMaximum(static_cast<int>(playtime));
    QTime t(0, 0, 0);
    t = t.addMSecs(static_cast<int>(playtime));
    _duration = t.toString("hh:mm:ss");
    _progress->setEnabled(true);
}

void PlayerPage::_player_positionChange(qint64 p)
{
    if (WebResource::instance()->app.live)
        return;

    // 设置进度标签
    QTime t(0, 0, 0);
    t = t.addMSecs(static_cast<int>(p));
    auto timeElapsed = t.toString("HH:mm:ss");
    _time->setText(QString("%1/%2").arg(timeElapsed).arg(_duration));

    if (!_progress->isSliderDown())
    {
        _progress->setValue(static_cast<int>(p));

        // 设置提示文本
        _progress->setToolTip(timeElapsed);
    }

    // 10 秒记录一次播放记录
    if(!WebResource::instance()->app.live && p / 1000 % 10 == 0)
        PlayerConfig::instance()->setVideoCurrentPosition(_current_name,
                                                          _playlist->currentIndex(),
                                                          _player->position(),
                                                          _current_api,
                                                          _current_id);
}

void PlayerPage::_control_timeOut()
{
    if (_player->state() == QMediaPlayer::PlayingState)
    {
        _video->setCursor(Qt::BlankCursor);

        if(WebResource::instance()->app.hideResource || WebResource::instance()->app.isFullScreen)
        {
            _control->hide();
            emit requestShowTitleBar(false);
        }
    }
    else
    {
        _video->setCursor(Qt::ArrowCursor);

        _control->show();
        emit requestShowTitleBar(true);
    }
}

void PlayerPage::_volume_slider_valueChanged(int value)
{
    if(!_volume_slider->isVisible())
        return;

    _player->setVolume(value);

    _volume_value->move(_play->width() + _prev->width() + _next->width() + 15 * 3 - 8, _volume_slider->y() - 5 + (100 - value) * 80 / 100);

    if(!_volume_value->isVisible())
        _volume_value->show();

    _volume_value->setText(QString::asprintf("%d", value));

    // 重启隐藏定时器 3s
    _volume_slider_timer->start();
}

void PlayerPage::_player_volumeChange(int value)
{
    _volume_slider->setValue(value);
}

void PlayerPage::_hideVolumeSlider()
{
    _volume_slider->hide();
    _volume_value->hide();

    _volume_slider_timer->stop();
}

void PlayerPage::_showVolumeSlider()
{
    _volume_slider->move(_play->width() + _prev->width() + _next->width() + 15 * 3 + _volume->width() / 2, _video->height() - _volume_slider->height());

    _volume_slider->show();
    _volume_value->show();
    _volume_slider->setFocus();

    _volume_slider_timer->start();
}

/*  旋转镜像  */
void PlayerPage::_video_rotate_left()
{
//    qreal x = GVI->boundingRect().width() / 2.0;
//    qreal y = GVI->boundingRect().height() / 2.0;
//    WebResource::instance()->app.angle -= 90;
//    GVI->setTransform(QTransform().translate(x, y).rotate(WebResource::instance()->app.angle).translate(-x, -y));
}

void PlayerPage::_video_rotate_right()
{
//    qreal x = GVI->boundingRect().width() / 2.0;
//    qreal y = GVI->boundingRect().height() / 2.0;
//    WebResource::instance()->app.angle+= 90;
//    GVI->setTransform(QTransform().translate(x,y).rotate(app.angle).translate(-x,-y));
}

void PlayerPage::_video_rotate_x()
{
//    WebResource::instance()->app.mh = -WebResource::instance()->app.mh;
//    //旋转
//    qreal x = GVI->boundingRect().width() / 2.0;
//    qreal y = GVI->boundingRect().height() / 2.0;
//    GVI->setTransform(QTransform().translate(x, y).scale(WebResource::instance()->app.mh, 1).translate(-x, -y));
}

void PlayerPage::_video_rotate_y()
{
//    WebResource::instance()->app.mv = -WebResource::instance()->app.mv;
//    qreal x = GVI->boundingRect().width() / 2.0;
//    qreal y = GVI->boundingRect().height() / 2.0;
    //    GVI->setTransform(QTransform().translate(x, y).scale(1, WebResource::instance()->app.mv).translate(-x, -y));
}

void PlayerPage::_aboutToShowMenu()
{
    auto action = _contextMenu->findChild<QAction*>("menu_video_info");
    auto action2 = _contextMenu->findChild<QAction*>("menu_video_favorite");
    auto action3 = _contextMenu->findChild<QAction*>("menu_video_refresh");
    if(_player->state() == QMediaPlayer::StoppedState)
    {
        action->setVisible(false);
        action2->setVisible(false);
        action3->setVisible(false);
    }
    else
    {
        action->setVisible(true);
        if(WebResource::instance()->app.live || WebResource::instance()->app.local)
            action2->setVisible(false);
        else
            action2->setVisible(true);
        action3->setVisible(true);
    }
}

void PlayerPage::_prevent_black_screen()
{
    if(this->isActiveWindow() && _player->state() == QMediaPlayer::PlayingState)
        x11SendKey();
    //mouseMove(0, 0);
}

void PlayerPage::_initControl()
{
    _video_container = new QStackedWidget(this);
    _video = new QVideoWidget(_video_container);
    _player = new QMediaPlayer;
    _playlist = new QMediaPlaylist;
    _control = new QFrame(this);
    _panel_timer = new QTimer;
    _progress = new QSlider(Qt::Horizontal, this);
    _play = new QPushButton(this);
    _prev = new QPushButton(this);
    _next = new QPushButton(this);
    _volume = new QPushButton(this);
    _volume_slider = new QSlider(Qt::Vertical, this);
    _volume_value = new QLabel(this);
    _volume_slider_timer = new QTimer;
    _time = new QLabel(this);
    //_name_label = new QLabel("资源名称: ", this);
    _name = new QLabel("片名", this);
    _part = new QComboBox(this);
    _status = new QLabel("准备就绪", this);
    _setting = new QPushButton(this);

    _contextMenu = new QMenu(this);

    _play_timer = new QTimer;
}

void PlayerPage::_reload_video()
{
    auto time = _player->position();
    auto index = _playlist->currentIndex();

    _player->stop();

    _playlist->setCurrentIndex(index);
    _player->setPosition(time);

    _player->play();
}
