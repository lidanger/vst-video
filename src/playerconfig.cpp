#include "playerconfig.h"

#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QTime>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

PlayerConfig *PlayerConfig::_inst = nullptr;

PlayerConfig::PlayerConfig()
{
    auto app = QApplication::instance();
    _org_name = app->organizationName();
    _app_name = app->applicationName();
}

void PlayerConfig::setCurrentVideo(const QString &video_name)
{
    _setValue("Player", "Current", video_name);

    // 历史记录
    auto keys = _getKeys("History");
    if(!keys.isEmpty())
    {
        keys.sort();

        // 只保存最近 100 个
        if(keys.length() > 99)
        {
            _removeKey("History", keys.first());
        }
    }

    auto time = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    _setValue("History", time, video_name);
}

void PlayerConfig::setVideoCurrentPosition(const QString &video_name, int part, qint64 pos, const QString &api, const QString &id)
{
    if(video_name.isEmpty() || api.isEmpty() || id.isEmpty())
        return;

    // 格式：片名=集数|位置|时间(20201203143935)|api|id
    auto val = QString("%1|%2|%3|%4|%5")
            .arg(part)
            .arg(pos)
            .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"))
            .arg(api)
            .arg(id);

    _setValue("Video", video_name, val);
}

void PlayerConfig::addFavorite(const QString &name, int part, qint64 pos, const QString &api, const QString &id)
{
    if(name.isEmpty() || api.isEmpty() || id.isEmpty())
        return;

    // 格式：片名|集数|位置=时间(20201203143935)|api|id
    auto key = QString("%1|%2|%3")
            .arg(name)
            .arg(part)
            .arg(pos);
    auto val = QString("%1|%2|%3")
            .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"))
            .arg(api)
            .arg(id);

    _setValue("Favorite", key, val);
}

void PlayerConfig::setWindowSize(const QSize &size)
{
    _setValue("Window", "Width", size.width());
    _setValue("Window", "Height", size.height());
}

QSize PlayerConfig::windowSize()
{
    auto width = _getValue("Window", "Width", 1000).toInt();
    auto height = _getValue("Window", "Height", 600).toInt();
    return QSize(width, height);
}
