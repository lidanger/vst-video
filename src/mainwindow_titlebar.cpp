#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::initTitleBar()
{
    /*  自定义标题栏    */
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    FramelessHelper *pHelper = new FramelessHelper(this);
    pHelper->activateOn(this);           // 激活当前窗体
    pHelper->setTitleHeight(50);         // 设置窗体的标题栏高度，可拖动高度
    pHelper->setWidgetMovable(true);     // 设置窗体可移动
    pHelper->setWidgetResizable(true);   // 设置窗体可缩放
    pHelper->setOnlyTitleBarMove(false); // 设置是否只标题栏可拖动
    pHelper->setRubberBandOnMove(true);  // 设置橡皮筋效果-可移动
    // pHelper->setRubberBandOnResize(true);  // 设置橡皮筋效果-可缩放

    // 标题栏菜单关联
    ui->titlebar->setContextMenuPolicy(Qt::CustomContextMenu); // 鼠标右键点击控件时会发送一个customContextMenuRequested信号
    connect(ui->titlebar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(TitlebarMenu(const QPoint &)));

    // 关闭按钮
    ui->pushButton_close->setFixedSize(60, 40);
    ui->pushButton_close->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    ui->pushButton_close->setIconSize(QSize(50, 50));

    // 最大化按钮
    ui->pushButton_max->setFixedSize(60, 40);
    ui->pushButton_max->setIconSize(QSize(50, 50));
    on_pushButton_max_clicked();

    // 最小化按钮
    ui->pushButton_mini->setFixedSize(60, 40);
    ui->pushButton_mini->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMinButton));
    ui->pushButton_mini->setIconSize(QSize(50, 50));

    // 设置按钮
    ui->pushButton_seting->setFixedSize(60, 40);
    ui->pushButton_seting->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    ui->pushButton_seting->setIconSize(QSize(50, 50));

    // 搜索按钮
    ui->pushButton_search->setFixedSize(60, 40);
    ui->pushButton_search->setIconSize(QSize(20, 20));
    ui->pushButton_search->setShortcut(QKeySequence("F6"));
    ui->search_name->hide();

    ui->pushButton_playlist->setFixedSize(60, 40);
    ui->pushButton_playlist->setIconSize(QSize(20, 20));
    ui->pushButton_playlist->setShortcut(QKeySequence("F2"));

    ui->pushButton_full->setFixedSize(60, 40);
    ui->pushButton_full->setIconSize(QSize(20, 20));
    ui->pushButton_full->setShortcut(QKeySequence("F11"));

    this->setMouseTracking(true);
}

// 标题栏菜单
void MainWindow::TitlebarMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    if (this->isMaximized())
    {
        ui->action_max->setText("还原");
    }
    else
    {
        ui->action_max->setText("最大化");
    }

    ui->menu_titlebar->exec(QCursor::pos());
}

void MainWindow::on_pushButton_seting_clicked()
{
    renotes();

    ui->menu_seting->exec(QPoint(ui->pushButton_setting->pos().x(), ui->pushButton_seting->height()));
}

void MainWindow::on_action_exit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_pushButton_close_clicked()
{
    close();
}

void MainWindow::on_pushButton_mini_clicked()
{
    showMinimized();
}

void MainWindow::on_pushButton_max_clicked()
{
    if(this->isMaximized())
    {
        ui->pushButton_max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        this->showNormal();
    }
    else
    {
        ui->pushButton_max->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        this->showMaximized();
    }
}

void MainWindow::on_pushButton_search_pressed()
{
    if(ui->search_name->isVisible())
    {
        ui->pushButton_search->setIcon(QIcon(":/img/search.svg"));

        ui->search_name->hide();
        ui->tabWidget->setCurrentWidget(ui->tab_player);
        this->setFocus();
    }
    else
    {
        ui->pushButton_search->setIcon(QIcon(":/img/icon.png"));

        ui->search_name->show();
        ui->search_name->setFocus();
    }
}

// 控制条全屏按钮被单击
void MainWindow::on_pushButton_full_clicked()
{
    switchFullScreen(!isFullScreen());
}

// 切换列表显示
void MainWindow::on_pushButton_playlist_clicked()
{
    if (ui->box_source->isHidden())
    {
        ui->tabWidget->setStyleSheet("");
        ui->box_control->show();
        ui->box_source->show();
        //ui->box_info->show();
        //ui->box_page->show();
        //ui->tabWidget->findChildren<QTabBar *>().at(0)->show();
        // 取消置顶
        //hide();setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);show();
    }
    else
    {
        ui->box_source->hide();
        //ui->box_info->hide();
        //ui->box_page->hide();
        //ui->tabWidget->findChildren<QTabBar *>().at(0)->hide();
        ui->tabWidget->setStyleSheet("border:0;");
        // 窗口置顶
        //hide();setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);show();

        m_timer->start(3000);
    }
}
