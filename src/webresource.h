#ifndef WEBRESOURCE_H
#define WEBRESOURCE_H

#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QEventLoop>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QQueue>
#include <QSettings>
#include <QStandardPaths>
#include <QTextCodec>

// 运行信息
typedef struct Appinfo
{
    bool showResourceBeforeFullScreen;

    QString sourcePath;

    QString livePath;

    QString nopic;

    Qt::WindowFlags Flags;

    bool live;
    bool local;

    Qt::WindowStates statesBeforeFullScreen;

    int angle;

    int mh;

    qreal mv;

    bool hideResource;

    bool isFullScreen;

    Appinfo()
    {
        auto app = QApplication::instance();
        QSettings sets(app->organizationName(), app->applicationName());
        QFileInfo fi(sets.fileName());

        sourcePath = fi.dir().filePath("source.txt");
        livePath = fi.dir().filePath("live.txt");

        nopic = ":/img/timg.jpeg";

        live = false;
        local = false;

        hideResource = false;
        isFullScreen = false;
    }

} AppInfo;
Q_DECLARE_METATYPE(AppInfo);

// 名称信息
typedef struct Nameinfo
{
    QString id;
    QString name;
} NameInfo;
Q_DECLARE_METATYPE(NameInfo);

// 资源信息
typedef struct SourceInfo
{
    QString name;          //资源名称
    QString api;           //接口地址
    QQueue<NameInfo> items; //分类信息
    bool live;             //是否直接源
} SourceInfo;
Q_DECLARE_METATYPE(SourceInfo);

// 视频信息
typedef struct VideoInfo
{
    QString sname;           //接口名称
    QString api;             //接口地址
    QString page;            //当前页数
    QString pagecount;       //总页数
    QString pagesize;        //页视频数量
    QString recordcount;     //总数量
    QList<QString> last;     //日期
    QList<QString> id;       //id
    QList<QString> tid;      //分类ID
    QList<QString> name;     //视频名称
    QList<QString> tname;    //分类名称
    QList<QString> pic;      //视频图片
    QList<QString> lang;     //语言
    QList<QString> area;     //地区
    QList<QString> year;     //年份
    QList<QString> note;     //视频标签
    QList<QString> actor;    //演员
    QList<QString> state;    //状态
    QList<QString> director; //导演
    QList<QString> video;    //视频信息 第01集$index.m3u8$ckm3u8#第02集$index.m3u8$ckm3u8#第03集$index.m3u8$ckm3u8
    QList<QString> des;      //视频简介
    QQueue<NameInfo> type;   //分类信息

    void clear()
    {
        api.clear();
        page.clear();
        pagecount.clear();
        pagesize.clear();
        recordcount.clear();
        type.clear();
        last.clear();
        id.clear();
        tid.clear();
        name.clear();
        tname.clear();
        pic.clear();
        video.clear();
        des.clear();
        lang.clear();
        area.clear();
        year.clear();
        note.clear();
        actor.clear();
        state.clear();
        director.clear();
    }

} VideoInfo;
Q_DECLARE_METATYPE(VideoInfo);

#define LIVE_CLASS_NAME "直播列表"

class WebResource : public QObject
{
    Q_OBJECT

private:
    WebResource();

public:
    // 取网页数据
    QString UrlRequestGet(const QString &url);

    // 下载图片文件
    void UrlRequestImg(const QString &url, const QString &key);

    // 提交网页数据
    QString UrlRequestPost(const QString &url, const QString &data);

    // xml 文本转 dom 对象
    QDomElement xmltoDom(const QString &xmlText);

    // dom 遍历 xml 获取影片信息
    void listDom(QDomElement &docElem, VideoInfo &cInfo);

    // DOM 遍历方式搜索显示影片信息
    void getvideo(int tp, const QString &api, const QString &word = "", const QString &page = "");

    // DOM 遍历方式搜索显示影片信息,用于详情页面
    void get_detail_videoinfo_by_id(const QString &api, const QString &word);

    // 取单个视频站资源信息
    void getResourceInfo(const QString &name, const QString &api);

    // 取视频站资源配置信息
    QMap<QString, QString> getResources(const QString &pfile);

    QMap<QString, QString> getResources() { return getResources(app.sourcePath); }

    // 取直播
    void getLives(const QString &pfile);

    void getLives() { getLives(app.livePath); }

    // 搜索资源站
    void search(const QString &keyword, const QString &source = QString());

    // 组合简介信息
    QString todes(VideoInfo &cInfo, int index);

    // 调用默认程序打开 M3U8 格式
    void OpenM3u8(const QString &url);

    // 本地化 m3u8,返回内容
    QString localtom3u8(const QString &url);

    // 图片路径
    QString topic(const QString &api, const QString &id);

    static WebResource* instance();

    void stopAll();

public:
    // 当前运行信息
    AppInfo app;

    // 资源分类信息
    QMap<QString, SourceInfo> sources;

    // 当前搜索资源信息
    QQueue<VideoInfo> vSearch;

    // 当前搜索列表中的单个资源（需要事先获取）或浏览分类列表中的单页资源信息
    VideoInfo vBrowse;

    // 详情中的当前资源信息
    VideoInfo vDetail;

private:
    static WebResource *_inst;
    int _web_request_timeout = 20000;
    QTextCodec *_codec;
    QMutex _locker;

    QSet<QThread*> _threads;
};

#endif // WEBRESOURCE_H
