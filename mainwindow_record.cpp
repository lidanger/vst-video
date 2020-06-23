#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webresource.h"

#include <QtConcurrent>

void MainWindow::initRecord()
{
    // 播放记录
    renotes();

    connect(ui->menu_notes, SIGNAL(triggered(QAction *)), this, SLOT(menu_action_notes_triggered(QAction *)));
}

// 加载播放记录
void MainWindow::renotes()
{
    ui->menu_notes->clear();

    QStringList keys = config.getKeys("notes");
    if (keys.size() == 0)
        return;

    for (int i = 0; i < keys.size(); i++)
    {
        //【片名】分集名称|pic|api|id|part|time
        QString key = config.getValue(keys.value(i)).toString();
        QStringList list = key.split("|");

        QAction *test = new QAction(list.value(0), this);
        test->setData(list);
        ui->menu_notes->addAction(test);
    }

    // 添加清除按钮
    ui->menu_notes->addSeparator();

    QAction *test = new QAction("清空记录", this);
    test->setData("clear");
    ui->menu_notes->addAction(test);
}

// 播放记录被选择
void MainWindow::menu_action_notes_triggered(QAction *action)
{
    if (action->data().toString() == "clear")
    {
        config.remove("notes");
        renotes();
        return;
    }

    // 禁止发信号
    ui->comboBox_name->blockSignals(true);
    ui->comboBox_part->blockSignals(true);

    // 清空原有播放信息
    ui->comboBox_name->clear();
    ui->comboBox_part->clear();
    ui->info_des->clear();

    // 添加所有播放记录到详情播放列表
    QString name, api, id;
    QStringList v;
    QStringList data;
    foreach(QAction *sub, ui->menu_notes->actions())
    {
        //【片名】分集名称
        //api
        //id
        //part
        //time
        data = sub->data().toStringList();

        v = data.value(0).split("】");
        name = v.first().mid(1);
        api = data.value(1);
        id = data.value(2);

        ui->comboBox_name->addItem(name, api + "|" + id);
    }

    // 开启发信号
    ui->comboBox_name->blockSignals(false);
    ui->comboBox_part->blockSignals(false);

    // 开始播放
    data = action->data().toStringList();
    v = data.value(0).split("】");
    name = v.first().mid(1);

    ui->comboBox_name->setCurrentText(name);

    app.live = false;
    show_loading(true);

    QString part = data.value(3);
    QString time = data.value(4);
    QtConcurrent::run(this, &MainWindow::requestDetailPlay, part.toInt(), time.toInt());
}

// 写播放记录
void MainWindow::MinWriteNotes(int part)
{
    if (ui->comboBox_name->currentIndex() == -1)
        return;

    Notesinfo note;
    note.title = QString("【%1】%2").arg(ui->comboBox_name->currentText()).arg(ui->comboBox_part->currentText());

    // api|id
    note.api = ui->comboBox_name->itemData(ui->comboBox_name->currentIndex()).toString();
    note.part = QString::number(part);
    note.time = QString::number(player->position());

    QString str = QString("%1|%2|%3|%4").arg(note.title).arg(note.api).arg(note.part).arg(note.time);

    config.set("notes", toHash(note.api), str);
}
