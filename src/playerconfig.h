#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QColor>
#include <QSettings>
#include <QSize>
#include <QString>

// 忽略 GNU 语句表达式扩展警告
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

// 名称信息
typedef struct Notesinfo
{
    QString title;
    QString api;
    QString id;
    QString part;
    QString time;
} Notesinfo;
Q_DECLARE_METATYPE(Notesinfo);

class PlayerConfig
{
#define _getValue(groupname, keyname, defaultValue)                               \
    ({                                                                            \
        QSettings sets(_org_name, _app_name);                                     \
        sets.beginGroup(groupname);                                               \
        sets.value(keyname, defaultValue);                                        \
    })

#define _getKeys(groupname)                                                       \
    ({                                                                            \
        QSettings sets(_org_name, _app_name);                                     \
        sets.beginGroup(groupname);                                               \
        sets.childKeys();                                                         \
    })

#define _setValue(groupname, keyname, value)                                      \
    ({                                                                            \
        QSettings sets(_org_name, _app_name);                                     \
        sets.beginGroup(groupname);                                               \
        sets.setValue(keyname, value);                                            \
        sets.endGroup();                                                          \
    })

#define _removeKey(groupname, keyname)                                            \
    ({                                                                            \
        QSettings sets(_org_name, _app_name);                                     \
        sets.beginGroup(groupname);                                               \
        sets.remove(keyname);                                                     \
        sets.endGroup();                                                          \
    })

public:
    QVariant getValue(const QString &groupname, const QString &keyname, const QVariant &defaultValue = QVariant()) { return _getValue(groupname, keyname, defaultValue); }
    QStringList getKeys(const QString &groupname) { return _getKeys(groupname); }
    void setValue(const QString &groupname, const QString &keyname, const QVariant &value) { _setValue(groupname, keyname, value); }
    void removeKey(const QString &groupname, const QString &keyname) { _removeKey(groupname, keyname); }

public:
    static PlayerConfig *instance() { return _inst == nullptr ? new PlayerConfig() : _inst; }

public:
    // 播放记录
    void setCurrentVideo(const QString &video_name);
    QString currentVideo() { return _getValue("Player", "Current", "").toString(); }

    // 格式：片名=集数|位置|时间(20201203143935)|api|id
    void setVideoCurrentPosition(const QString &video_name, int part, qint64 pos, const QString &api, const QString &id);
    QString videoCurrentPosition(const QString &video_name) { return _getValue("Video", video_name, "").toString(); }

    // 只保留一百条。格式：时间(20201203143935)=片名
    QStringList history() { return _getKeys("History"); }
    QString historyItem(const QString &item_name) { return _getValue("History", item_name, "").toString(); }
    void clearHistory() { _removeKey("History", ""); }

    // 格式：片名|集数|位置=时间(20201203143935)|api|id
    void addFavorite(const QString &name, int part, qint64 pos, const QString &api, const QString &id);
    QStringList favorites() { return _getKeys("Favorite"); }
    QString favoriteItem(const QString &key) { return _getValue("Favorite", key, "").toString(); }
    void removeFavorite(const QString &key) { _removeKey("Favorite", key); }
    void clearFavorites() { _removeKey("Favorite", ""); }

    // 窗口状态
    void setWindowState(int state) { _setValue("Window", "State", state); }
    int windowState() { return _getValue("Window", "State", 0).toInt(); }

    void setWindowSize(const QSize &size);
    QSize windowSize();

    void setSidebarState(int state) { _setValue("Window", "SidebarState", state); }
    int sidebarState() { return _getValue("Window", "SidebarState", 1).toInt(); }

    void setSidebarWidth(int width) { _setValue("Window", "SidebarWidth", width); }
    int sidebarWidth() { return _getValue("Window", "SidebarWidth", 153).toInt(); }

    void setTheme(QString name) { _setValue("Window", "Theme", name); }
    QString theme() { return _getValue("Window", "Theme", "").toString(); }

    void setTopHint(bool tophint) { _setValue("Window", "TopHint", tophint); }
    bool topHint() { return _getValue("Window", "TopHint", false).toBool(); }

    // 快捷键
    void setShortcut(const QString &function_name, const QString &keys) { _setValue("Shotcut", function_name, keys); }
    QString shortcutKeys(const QString &function_name) { return _getValue("Shotcut", function_name, "").toString(); }

    // 资源
    // 过滤器。格式：xxx|yyy|...
    void setResourceFilters(const QString &filters) { _setValue("Resource", "Filters", filters); }
    QString resourceFilters() { return _getValue("Resource", "Filters", "").toString(); }

private:
    PlayerConfig();

private:
    static PlayerConfig *_inst;

private:
    QString _org_name;
    QString _app_name;
};

#endif // APPCONFIG_H
