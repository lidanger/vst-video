#ifndef PLAYERPAGE_H
#define PLAYERPAGE_H

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QTimer>
#include <QVideoWidget>

class PlayerPage : public QFrame
{
    Q_OBJECT

public:
    PlayerPage(QWidget* parent = nullptr);

public:
    void startPlayLive(int index);
    void startPlayVideo(const QString &name, const QStringList &part_names, const QStringList &part_urls, int index, qint64 time);
    void startPlayVideoFile(const QStringList &files);

    bool eventFilter(QObject *watched, QEvent *event);

    QVideoWidget *video() { return _video; }
    QMediaPlayer *player() { return _player; }

signals:
    void requestShowDetail();
    void requestShowPlayer();
    void requestShowFullScreen();
    void requestShowTitleBar(bool show);

private slots:
    void _name_linkActivated(const QString &link);
    void _switchPlayPause();
    void _openLocalFile();
    void _openUrl();
    void _showVideoInfo();
    void _addFavorite();
    void _player_mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void _player_stateChanged(QMediaPlayer::State state);
    // 播放器视频长度状态发生改变
    void _player_durationChange(qint64 playtime);
    void _player_positionChange(qint64 p);
    void _control_timeOut();
    void _volume_slider_valueChanged(int value);
    void _player_volumeChange(int value);
    void _hideVolumeSlider();
    void _showVolumeSlider();
    // 旋转镜像
    void _video_rotate_left();
    void _video_rotate_right();
    void _video_rotate_x();
    void _video_rotate_y();
    void _aboutToShowMenu();
    void _prevent_black_screen();
    // 重新载入视频
    void _reload_video();

private:
    void _initControl();
    void _initMenu();
    void _initLayout();

private:
    QStackedWidget *_video_container;
    QVideoWidget *_video;
    QMediaPlayer *_player;
    QMediaPlaylist *_playlist;
    QFrame *_control;
    QTimer *_panel_timer;
    QSlider *_progress;
    QPushButton *_play;
    QPushButton *_prev;
    QPushButton *_next;
    QPushButton *_volume;
    QTimer *_volume_slider_timer;
    QSlider *_volume_slider;
    QLabel *_volume_value;
    QLabel *_time;
    //QLabel *_name_label;
    QLabel *_name;
    QComboBox *_part;
    QLabel *_status;
    QPushButton *_setting;
    QMenu *_contextMenu;

    QList<QString> _video_names;

    QString _duration;
    QString _current_name;
    QString _current_api;
    QString _current_id;
    QString _last_name;

    QTimer *_play_timer;
};

#endif // PLAYERPAGE_H
