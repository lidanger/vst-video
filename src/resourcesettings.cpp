#include "playerconfig.h"
#include "resourcesettings.h"
#include "utils.h"
#include "webresource.h"

#include <QMessageBox>
#include <QStackedLayout>
#include <QStyleOptionFocusRect>
#include <QStylePainter>
#include <QtDebug>

ResourceSettings::ResourceSettings(QWidget *parent)
    : QDialog (parent)
{
    _init();
    _init_data();

    auto rect = parent->geometry();
    int x = rect.x() + rect.width() / 2 - this->width() / 2;
    int y = rect.y() + rect.height() / 2 - this->height() / 2 - 20;
    this->move(x, y);
}

void ResourceSettings::_init()
{
    this->setWindowTitle("资源设置");
    this->setMinimumSize(700, 500);
    this->setLayout(new QVBoxLayout(this));
    this->setStyleSheet("QPlainTextEdit{border:none;}");

    // 资源编辑器
    _tabs = new QTabWidget(this);
    this->layout()->addWidget(_tabs);

    _classResouce = new QPlainTextEdit(_tabs);
    _tabs->addTab(_classResouce, "点播");

    _liveResouce = new QPlainTextEdit(_tabs);
    _tabs->addTab(_liveResouce, "直播");

    _resourceFilter = new QPlainTextEdit(_tabs);
    _resourceFilter->setPlaceholderText("一行一个关键词");
    _tabs->addTab(_resourceFilter, "过滤");

    // 按钮
    auto buttons_panel = new QFrame(this);
    buttons_panel->setFixedHeight(50);
    buttons_panel->setLayout(new QHBoxLayout(buttons_panel));
    buttons_panel->layout()->setSpacing(50);
    this->layout()->addWidget(buttons_panel);

    static_cast<QBoxLayout*>(buttons_panel->layout())->addStretch();

    _ok = new QPushButton("确定", this);
    _ok->setMinimumWidth(80);
    connect(_ok, &QPushButton::pressed, this, &ResourceSettings::_save);
    buttons_panel->layout()->addWidget(_ok);

    _cancel= new QPushButton("取消", this);
    _cancel->setMinimumWidth(80);
    _cancel->setDefault(true);
    _cancel->setAutoDefault(true);
    connect(_cancel, &QPushButton::pressed, this, &QDialog::reject);
    buttons_panel->layout()->addWidget(_cancel);

    static_cast<QBoxLayout*>(buttons_panel->layout())->addStretch();
}

void ResourceSettings::_init_data()
{
    _classResouce->setPlainText(Readfile(WebResource::instance()->app.sourcePath));
    _liveResouce->setPlainText(Readfile(WebResource::instance()->app.livePath));

    // 过滤关键词格式：xxx|yyy|...
    auto val = PlayerConfig::instance()->resourceFilters();
    if(val.isEmpty())
        return;

    auto filters = val.split('|', QString::SkipEmptyParts);
    foreach(auto filter, filters)
    {
        _resourceFilter->appendPlainText(filter);
    }
}

void ResourceSettings::_save()
{
    if (!Writefile(WebResource::instance()->app.livePath, _liveResouce->toPlainText()))
    {
        QMessageBox::warning(this, "提示", "直播资源保存失败！", QMessageBox::Yes);
        return;
    }

    if (!Writefile(WebResource::instance()->app.sourcePath, _classResouce->toPlainText()))
    {
        QMessageBox::warning(this, "提示", "点播资源保存失败！", QMessageBox::Yes);
        return;
    }

    // 过滤关键词格式：xxx|yyy|...
    auto txt = _resourceFilter->toPlainText();
    QTextStream stream(&txt);
    QString filters;
    while(!stream.atEnd())
    {
        auto line = stream.readLine();
        filters.append(line).append('|');
    }

    if(!filters.isEmpty())
        PlayerConfig::instance()->setResourceFilters(filters);

    this->accept();
}
