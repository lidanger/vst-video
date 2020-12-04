#include "utils.h"
#include "webresource.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QNetworkReply>
#include <QPixmap>
#include <QTimer>
#include <QThread>
#include <QMessageBox>
#include <QStandardPaths>

WebResource* WebResource::_inst = nullptr;

WebResource::WebResource()
    : _locker(QMutex::NonRecursive)
{
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    if(!cacheDir.exists())
        cacheDir.mkpath(cacheDir.absolutePath());

    // 检查资源文件
    if (!isFileExists(app.sourcePath))
    {
        QFile soucre(":/source/source.txt");
        soucre.copy(app.sourcePath);
        QFile(app.sourcePath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }

    if (!isFileExists(app.livePath))
    {
        QFile soucre(":/source/live.txt");
        soucre.copy(app.livePath);
        QFile(app.livePath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    }

    _codec = QTextCodec::codecForName("utf8");
}

QString WebResource::UrlRequestGet(const QString &url)
{
    QString ret;
    _threads.insert(QThread::currentThread());

    try
    {
        auto nam = new QNetworkAccessManager;

        auto request = QNetworkRequest(QUrl(url));
        request.setRawHeader("Content-Type", "application/json");
        auto reply = nam->get(request);

        QEventLoop eventloop;
        QTimer::singleShot(_web_request_timeout, &eventloop, SLOT(quit()));
        QObject::connect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
        eventloop.exec(QEventLoop::ExcludeUserInputEvents);

        if(reply->isFinished())
        {
            ret = _codec->toUnicode(reply->readAll());
        }

        reply->deleteLater();
        nam->deleteLater();
    }
    catch (int n)
    {
        qDebug() << "num=" << n << ",UrlRequestGet() error!" << endl;
        return "";
    }

    return ret;
}

void WebResource::UrlRequestImg(const QString &url, const QString &key)
{
    _threads.insert(QThread::currentThread());

    try
    {
        QString filename = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + key + ".jpg";

        if (!isFileExists(filename))
        {
            auto nam = new QNetworkAccessManager;
            auto reply = nam->get(QNetworkRequest(QUrl(url)));

            QEventLoop eventloop;
            QTimer::singleShot(_web_request_timeout, &eventloop, SLOT(quit()));
            QObject::connect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
            eventloop.exec(QEventLoop::ExcludeUserInputEvents);

            if(reply->isFinished())
            {
                QPixmap pic;
                pic.loadFromData(reply->readAll());
                pic.save(filename);
            }

            reply->deleteLater();
            nam->deleteLater();
        }
    }
    catch (int n)
    {
        qDebug() << "num=" << n << ",UrlRequestImg() error!" << endl;
    }
}

// 提交网页数据
QString WebResource::UrlRequestPost(const QString &url, const QString &data)
{
    QString ret;
    _threads.insert(QThread::currentThread());

    try
    {
        auto nam = new QNetworkAccessManager;

        auto qnr = QNetworkRequest(QUrl(url));
        qnr.setRawHeader("Content-Type", "application/json;charset=GBK");
        auto *reply = nam->post(qnr, data.toLocal8Bit());

        QEventLoop eventloop;
        QTimer::singleShot(_web_request_timeout, &eventloop, SLOT(quit()));
        QObject::connect(reply, SIGNAL(finished()), &eventloop, SLOT(quit()));
        eventloop.exec(QEventLoop::ExcludeUserInputEvents);

        if(reply->isFinished())
        {
            ret = _codec->toUnicode(reply->readAll());
        }

        reply->deleteLater();
        nam->deleteLater();
    }
    catch (int n)
    {
        qDebug() << "num=" << n << ",UrlRequestPost() error!" << endl;
    }

    return ret;
}

// xml 文本转 dom 对象
QDomElement WebResource::xmltoDom(const QString &xmlText)
{
    QDomDocument doc;
    doc.setContent(xmlText.toUtf8());

    QDomElement docElem = doc.documentElement();

    return docElem;
}

// dom 遍历 xml 获取影片信息
void WebResource::listDom(QDomElement &docElem, VideoInfo &cInfo)
{
    QDomNode node = docElem.firstChild();
    //if(node.toElement().isNull()){}
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            // 页面信息
            if (element.tagName() == "list")
            {
                cInfo.page = element.attribute("page");
                cInfo.pagecount = element.attribute("pagecount");
                cInfo.pagesize = element.attribute("pagesize");
                cInfo.recordcount = element.attribute("recordcount");
            }
            else if (element.tagName() == "last")
            {
                cInfo.last << element.text(); // 日期
            }
            else if (element.tagName() == "id")
            {
                cInfo.id << element.text(); // 影片 id
            }
            else if (element.tagName() == "tid")
            {
                cInfo.tid << element.text(); // 分类 ID
            }
            else if (element.tagName() == "name")
            {
                cInfo.name << element.text(); // 影片名称
            }
            else if (element.tagName() == "type")
            {
                cInfo.tname << element.text(); // 名称分类
            }
            else if (element.tagName() == "dd")
            {
                cInfo.video << element.text(); // 影片数据
            }
            else if (element.tagName() == "pic")
            {
                cInfo.pic << element.text(); // 影片图片
            }
            else if (element.tagName() == "lang")
            {
                cInfo.lang << element.text(); // 语言
            }
            else if (element.tagName() == "area")
            {
                cInfo.area << element.text(); // 地区
            }
            else if (element.tagName() == "year")
            {
                cInfo.year << element.text(); //年份
            }
            else if (element.tagName() == "state")
            {
                cInfo.state << element.text(); // 状态
            }
            else if (element.tagName() == "note")
            {
                cInfo.note << element.text(); // 标签
            }
            else if (element.tagName() == "des")
            {
                cInfo.des << element.text(); // 简介
            }
            else if (element.tagName() == "ty")
            {
                //分类信息
                NameInfo name;
                name.id = element.attribute("id");
                name.name = element.text();
                cInfo.type << name;
            }
            listDom(element, cInfo);
        }

        node = node.nextSibling();
    }
}

// DOM 遍历方式搜索显示影片信息
void WebResource::getvideo(int tp, const QString &api, const QString &word, const QString &page)
{
    QString url;

    switch (tp)
    {
    case 1:
        url = api + "?wd=" + word + "&pg=" + page;
        break;
    case 2:
        url = api + "?ac=videolist&ids=" + word;
        break;
    case 3:
        url = api;
        break;
    case 4:
        url = api + "?ac=videolist&t=" + word + "&pg=" + page;
        break;
    }

    auto done = UrlRequestGet(url);
    auto dom = xmltoDom(done);

    vBrowse.clear();    
    listDom(dom, vBrowse);
    vBrowse.api = api;
}

// DOM 遍历方式搜索显示影片信息
void WebResource::get_detail_videoinfo_by_id(const QString &api, const QString &word)
{
    auto url = api + "?ac=videolist&ids=" + word;
    auto done = UrlRequestGet(url);
    auto dom = xmltoDom(done);

    vDetail.clear();
    listDom(dom, vDetail);
    vDetail.api = api;
}

// 取所有资源类型
void WebResource::getResourceInfo(const QString &name, const QString &api)
{
    SourceInfo info;
    info.name = name;
    info.api = api;
    info.live = false;

    getvideo(3, info.api);

    info.items = vBrowse.type;

    sources[name] = info;
}

QMap<QString, QString> WebResource::getResources(const QString &pfile)
{
    QMap<QString, QString> clss;

    QFile file(pfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return clss;

    while(!file.atEnd())
    {
        QString line = file.readLine();
        line = line.toUtf8().trimmed();
        if (line.isEmpty())
            continue;

        auto strs = line.split(",");
        clss[strs[0]] = strs[1];
    }

    file.close();
    return clss;
}

// 取直播
void WebResource::getLives(const QString &pfile)
{
    QFile file(pfile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        SourceInfo info;
        info.name = LIVE_CLASS_NAME;
        info.live = true;

        for (int i = 0; !file.atEnd(); i++)
        {
            QByteArray line = file.readLine().trimmed();
            QString str(line);
            str = str.toUtf8();
            if (str != "")
            {
                QStringList list = str.split(",");
                NameInfo var;
                var.name = list.value(0);
                var.id = list.value(1);
                info.items.insert(i, var);
            }
        }

        file.close();

        if (info.items.size() > 0)
        {
            sources[info.name] = info;
        }
    }
}

// 搜索资源站
void WebResource::search(const QString &keyword, const QString &source)
{
    vSearch.clear();

    if (source.isEmpty())
    {
        foreach (SourceInfo it, sources)
        {
            auto url = it.api + "?wd=" + keyword;
            auto done = UrlRequestGet(url);
            auto dom = xmltoDom(done);

            VideoInfo info;
            listDom(dom, info);
            info.sname = it.name;
            info.api = it.api;

            vSearch.append(info);
        }
    }
    else
    {
        auto api = sources[source].api;
        auto url = api + "?wd=" + keyword;
        auto done = UrlRequestGet(url);
        auto dom = xmltoDom(done);

        VideoInfo info;
        listDom(dom, info);
        info.sname = sources[source].name;
        info.api = api;

        vSearch.append(info);
    }
}

// 组合简介信息
QString WebResource::todes(VideoInfo &cInfo, int index)
{
    QString str = QString("<h3>%1</h3><h4>%2 %3 %4 %5 %6 %7</h4><p>%8</p>")
                      .arg(cInfo.name.value(index))
                      .arg(cInfo.year.value(index))
                      .arg(cInfo.area.value(index))
                      .arg(cInfo.tname.value(index))
                      .arg(cInfo.lang.value(index))
                      .arg(cInfo.director.value(index))
                      .arg(cInfo.actor.value(index))
                      .arg(cInfo.des.value(index));
    return str;
}

// 调用默认程序打开 M3U8 格式
void WebResource::OpenM3u8(const QString &url)
{
    QFileInfo fi(url);
    QString tmp = "/tmp/" + fi.fileName();
    QFile file(tmp);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(localtom3u8(url).toUtf8());
        file.close();
    }
    open(tmp);
}

// 本地化 m3u8,返回内容
QString WebResource::localtom3u8(const QString &url)
{
    QString done = UrlRequestGet(url);
    QStringList lists = done.split("\n");
    QString m3u8 = "";

    foreach (QString list, lists)
    {
        // 判断是文件信息
        if (list != "" && list.mid(0, 1) != "#")
        {
            m3u8 += put_url(url, list.trimmed());
        }
        else
        {
            m3u8 += list.trimmed() + "\n";
        }
    }
    //qDebug() << m3u8;
    return m3u8;
}

QString WebResource::topic(const QString &api, const QString &id)
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + toHash(api) + "_" + id + ".jpg";
}

WebResource *WebResource::instance()
{
    if(_inst == nullptr)
        _inst = new WebResource;

    return _inst;
}

void WebResource::stopAll()
{
    foreach(auto thread, _threads)
    {
        if(thread->isRunning())
        {
            thread->quit();
        }
    }
}
