#include "app.h"
#include "browsepage.h"
#include "loading.h"
#include "utils.h"
#include "webresource.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QtConcurrent>

BrowsePage::BrowsePage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player)
    : QFrame (parent)
    , _video(video)
    , _player(player)
{
    connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(_player_stateChanged(QMediaPlayer::State)));

    _init();
}

void BrowsePage::startBrowse(const QString &class_name, int sub_index)
{
    _resource_name = class_name;
    _resource_sub_index = sub_index;
    _getPageInfo(1);
}

bool BrowsePage::eventFilter(QObject *watched, QEvent *event)
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

void BrowsePage::_init()
{
    this->setLayout(new QHBoxLayout(this));
    this->layout()->setMargin(0);
    this->layout()->setSpacing(0);
    this->installEventFilter(this);

    // 列表区
    auto list_area = new QFrame(this);
    list_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    list_area->setLayout(new QVBoxLayout(list_area));
    list_area->layout()->setMargin(0);
    list_area->layout()->setSpacing(0);
    this->layout()->addWidget(list_area);

    // 图片列表框
    _browseList = new QListWidget(this);
    _browseList->setMinimumSize(200, 100);
    _browseList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _browseList->setIconSize(QSize(210, 210));             // 设置图标大小
    _browseList->setGridSize(QSize(240, 240));             // 设置 item 大小
    _browseList->setResizeMode(QListView::Adjust);         // 自动适应布局
    _browseList->setViewMode(QListView::IconMode);         // 大图标模式
    _browseList->setMovement(QListView::Static);           // 禁止拖动
    _browseList->setSpacing(10);                           // 间距
    _browseList->horizontalScrollBar()->setDisabled(true); // 不显示横向滚动条
    _browseList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_browseList, SIGNAL(currentRowChanged(int)), this, SLOT(_browseList_currentRowChanged(int)));
    connect(_browseList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(_browseList_itemDoubleClicked(QListWidgetItem *)));
    connect(_browseList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_exploreMenu(const QPoint &)));
    _browseList->installEventFilter(this);
    list_area->layout()->addWidget(_browseList);

    // 上下文菜单
    _contextMenu = new QMenu(_browseList);
    _contextMenu->addAction("播放", this, [=](){_browseList_itemDoubleClicked(_browseList->currentItem());});
    _contextMenu->addSeparator();
    _contextMenu->addAction("查看详情", this, &BrowsePage::_show_detail);

    // 导航栏
    auto nav_bar = new QFrame(list_area);
    nav_bar->setLayout(new QHBoxLayout(nav_bar));
    list_area->layout()->addWidget(nav_bar);

    _page_info = new QLabel(nav_bar);
    _page_info->setText("当前第001页，共100 页");
    _page_info->setMinimumWidth(165);
    _page_info->setStyleSheet("background: transparent;");
    nav_bar->layout()->addWidget(_page_info);

    static_cast<QBoxLayout*>(nav_bar->layout())->addStretch();

    _prev = new QPushButton(nav_bar);
    _prev->setMinimumWidth(50);
    _prev->setText("上页");
    _prev->setShortcut(QKeySequence("PgUp"));
    connect(_prev, &QPushButton::pressed, this, &BrowsePage::_page_prev);
    nav_bar->layout()->addWidget(_prev);

    _next = new QPushButton(nav_bar);
    _next->setMinimumWidth(50);
    _next->setText("下页");
    _next->setShortcut(QKeySequence("PgDown"));
    connect(_next, &QPushButton::pressed, this, &BrowsePage::_page_next);
    nav_bar->layout()->addWidget(_next);

    _page_num = new QLineEdit(nav_bar);
    _page_num->setFixedWidth(100);
    connect(_page_num, &QLineEdit::returnPressed, this, &BrowsePage::_page_num_returnPressed);
    nav_bar->layout()->addWidget(_page_num);

    _jump = new QPushButton(nav_bar);
    _jump->setMinimumWidth(50);
    _jump->setText("跳转");
    connect(_jump, &QPushButton::pressed, this, &BrowsePage::_page_jmp);
    nav_bar->layout()->addWidget(_jump);

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

// 取分页信息
void BrowsePage::_getPageInfo(int page)
{
    _browseList->clear();
    _page_info->setText(QString("页数：%1/%2").arg(page).arg(WebResource::instance()->vBrowse.pagecount));
    _page_num->setText(QString::number(page));

    Loading::instance()->show();
    QtConcurrent::run(this, &BrowsePage::_requestPageInfo, page);
}

// 动态创建浏览器图片表项
void BrowsePage::_createListWidget(QListWidget *listWidget, int key)
{
    if (key < listWidget->count())
        return;

    if (key >= WebResource::instance()->vBrowse.id.size() || key >= WebResource::instance()->vBrowse.name.size())
        return;

    QString word = WebResource::instance()->vBrowse.pic.value(key) + "|" + WebResource::instance()->vBrowse.api + "|" + WebResource::instance()->vBrowse.id.value(key);

    QVBoxLayout *widgetLayout = new QVBoxLayout;
    widgetLayout->setMargin(0);
    widgetLayout->setSpacing(0);

    // 图标
    QString file = WebResource::instance()->topic(WebResource::instance()->vBrowse.api, WebResource::instance()->vBrowse.id.value(key));
    if (!isFileExists(file))
    {
        QtConcurrent::run(this, &BrowsePage::_requestBrowseImage, word);
        file = WebResource::instance()->app.nopic;
    }

    // 头像裁剪
    QPixmap pixmap(file);
    if (pixmap.width() > 227 || pixmap.height() > 227)
    {
        pixmap = pixmap.scaled(234, 234, Qt::KeepAspectRatio);
    }

    QLabel *imageLabel = new QLabel;
    //imageLabel->setScaledContents(true);  // 图片缩放
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter); // 图片居中

    widgetLayout->addWidget(imageLabel);

    // 标签
    QLabel *txtLabel = new QLabel;
    txtLabel->setFixedHeight(60);
    txtLabel->setWordWrap(true);
    txtLabel->setAlignment(Qt::AlignCenter); // 文本居中

    QString name = WebResource::instance()->vBrowse.name[key];
    if (name.size() > 30)
    {
        name = name.mid(0, 20) + "...";
        txtLabel->setToolTip(WebResource::instance()->vBrowse.name.value(key));
    }

    txtLabel->setText(name);

    widgetLayout->addWidget(txtLabel);

    // 添加列表项
    QWidget *widget = new QWidget;
    widget->setLayout(widgetLayout);

    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(240, 240));
    item->setData(Qt::UserRole, word);

    listWidget->addItem(item);

    listWidget->setSizeIncrement(240, 240); // 当用户重新定义窗口尺寸的时候，窗口会以此为基准
    listWidget->setItemWidget(item, widget);
}

void BrowsePage::_requestBrowseImage(const QString &word)
{
    if (word.isEmpty())
        return;

    QStringList v = word.split("|");

    //QMutexLocker locker(&_browse_locker);

    // 下载浏览图片
    WebResource::instance()->UrlRequestImg(v.value(0), toHash(v.value(1)) + "_" + v.value(2));

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestBrowseImage", Q_ARG(const QString &, word));
}

void BrowsePage::_requestPageInfo(int page)
{
    auto id = WebResource::instance()->sources[_resource_name].items[_resource_sub_index].id;
    auto api = WebResource::instance()->sources[_resource_name].api;

    WebResource::instance()->getvideo(4, api, id, QString::number(page));

    if(App::isStopped())
        return;

    QMetaObject::invokeMethod(this, "_endRequestPageInfo", Q_ARG(int, page));
}

// 浏览下载图片结束
void BrowsePage::_endRequestBrowseImage(const QString &word)
{
    QStringList v = word.split("|");

    QString file = WebResource::instance()->topic(v.value(1), v.value(2));
    if (isFileExists(file))
    {
        _setListWidgetImage(_browseList, word);
    }
}

void BrowsePage::_endRequestPageInfo(int page)
{
    Q_UNUSED(page)

    for (int i = 0; i < WebResource::instance()->vBrowse.id.size(); i++)
    {
        _createListWidget(_browseList, i);
    }

    if (_browseList->count() == 0)
    {
        Loading::instance()->hide();
        QMessageBox::warning(nullptr, "提示", "当前分类无资源!", QMessageBox::Yes);
    }
    else
    {
        // 默认选择第一个
        _browseList->setCurrentRow(0);
    }

    auto str = QString("页数：%1/%3，视频：%2/%4").arg(WebResource::instance()->vBrowse.page).arg(WebResource::instance()->vBrowse.pagesize).arg(WebResource::instance()->vBrowse.pagecount).arg(WebResource::instance()->vBrowse.recordcount);

    _page_info->setText(str);

    // 限定输入范围
    _page_num->setValidator(new QIntValidator(1, WebResource::instance()->vBrowse.pagecount.toInt(), this));

    Loading::instance()->hide();
}

void BrowsePage::_player_stateChanged(QMediaPlayer::State newState)
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

void BrowsePage::_show_detail()
{
    if(WebResource::instance()->vBrowse.id.isEmpty())
        return;

    auto index = _browseList->currentRow();

    auto name = WebResource::instance()->vBrowse.name.value(index);

    emit requestShowDetail(index, name, WebResource::instance()->vBrowse.api, WebResource::instance()->vBrowse.id.value(index));
}

// 设置浏览器图片表项
void BrowsePage::_setListWidgetImage(QListWidget *listWidget, const QString &word)
{
    // 查找对应列表项
    QListWidgetItem *pItem = nullptr;
    for (int i = 0; i < _browseList->count(); i++)
    {
        QString str = _browseList->item(i)->data(Qt::UserRole).toString();
        if(str != word)
            continue;

        pItem = _browseList->item(i);
    }

    if(pItem == nullptr)
        return;

    QWidget *widget = static_cast<QWidget *>(listWidget->itemWidget(pItem));
    if(widget->children().size() != 3)
        return;

    QStringList v = word.split("|");

    // 图标
    QString file = WebResource::instance()->topic(v.value(1), v.value(2));
    if (!isFileExists(file))
    {
        QtConcurrent::run(this, &BrowsePage::_requestBrowseImage, word);
        file = WebResource::instance()->app.nopic;
    }

    QPixmap pixmap(file);

    // 头像裁剪
    if (pixmap.width() > 227 || pixmap.height() > 227)
    {
        pixmap = pixmap.scaled(234, 234, Qt::KeepAspectRatio);
    }

    auto imageLabel = static_cast<QLabel *>(widget->children().value(1));
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter); // 图片居中
}

// 上页
void BrowsePage::_page_prev()
{
    int page = WebResource::instance()->vBrowse.page.toInt() - 1;
    if (page < 1)
    {
        return;
    }

    _getPageInfo(page);
}

// 下页
void BrowsePage::_page_next()
{
    int page = WebResource::instance()->vBrowse.page.toInt() + 1;
    if (page > WebResource::instance()->vBrowse.pagecount.toInt())
    {
        return;
    }

    _getPageInfo(page);
}

// 跳页
void BrowsePage::_page_jmp()
{
    _getPageInfo(_page_num->text().toInt());
}

void BrowsePage::_page_num_returnPressed()
{
    _getPageInfo(_page_num->text().toInt());
}

// 列表被选择
void BrowsePage::_browseList_currentRowChanged(int key)
{
    if (key == -1)
        return;

    QListWidgetItem *item = _browseList->item(key);
    QString word = item->data(Qt::UserRole).toString();
    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    // 清空原来的预览信息
    _pic->clear();
    _parts->clear();
    _desc->clear();

    // 检查选中内容的数据是否与当前视频分类信息一致
    if (WebResource::instance()->vBrowse.id.value(key).toInt() == v.value(2).toInt())
    {
        // 介绍
        _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vBrowse, key));

        // 集数
        QStringList list = WebResource::instance()->vBrowse.video.value(key).split("#");
        _parts->setText(QString::asprintf(" 共 %d 集", list.length()));

        // 照片
        QString file = WebResource::instance()->topic(WebResource::instance()->vBrowse.api, WebResource::instance()->vBrowse.id.value(key));
        if (isFileExists(file))
        {
            QPixmap pixmap(file);
            _pic->setPixmap(pixmap);
            _pic->setAlignment(Qt::AlignCenter);
        }
        else
            QtConcurrent::run(this, &BrowsePage::_requestPreviewImage, word);
    }
    // 分类数据已经包含了视频信息
    //else
    //    QtConcurrent::run(this, &MainWindow::requestPreviewVideoInfo, key);
}

void BrowsePage::_requestPreviewImage(const QString &word)
{
    if (word.isEmpty())
        return;

    QStringList v = word.split("|");
    if(v.size() != 3)
        return;

    //QMutexLocker locker(&_browse_locker);

    QString url = v.value(0);

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

// 线程搜索下载图片结束
void BrowsePage::_endRequestPreviewImage(const QString &word)
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
}

// 获取浏览影片数据
void BrowsePage::_requestPreviewVideoInfo(const QString &word)
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

    QMetaObject::invokeMethod(this, "_endRequestPreviewVideoInfo", Q_ARG(const QString &, word));
}

// 线程搜索影片详情结束
void BrowsePage::_endRequestPreviewVideoInfo(const QString &word)
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
    }
    else
        QtConcurrent::run(this, &BrowsePage::_requestPreviewImage, word);

    // 集数
    auto list = WebResource::instance()->vBrowse.video.first().split("#");
    _parts->setText(QString::asprintf(" 共 %d 集", list.length()));

    // 介绍
    _desc->setHtml(WebResource::instance()->todes(WebResource::instance()->vBrowse, 0));

    Loading::instance()->hide();
}

void BrowsePage::_browseList_itemDoubleClicked(QListWidgetItem *item)
{
    if(WebResource::instance()->vBrowse.id.isEmpty())
        return;

    auto index = _browseList->row(item);

    auto name = WebResource::instance()->vBrowse.name.value(index);

    emit itemSelectToPlay(index, name, WebResource::instance()->vBrowse.api, WebResource::instance()->vBrowse.id.value(index));
}

// 浏览器弹出菜单
void BrowsePage::_exploreMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    _contextMenu->exec(QCursor::pos());
}
