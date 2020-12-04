#ifndef DETIALPAGE_H
#define DETIALPAGE_H

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVideoWidget>

class DetailPage : public QFrame
{
    Q_OBJECT

public:
    DetailPage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player);

public:
    void loadInfo(int index, const QString &name, const QString &api, const QString &id, bool play = true, qint64 time = 0);

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void aboutToPlay(const QString &name, const QStringList &part_names, const QStringList &part_urls, int index, qint64 time);

private slots:
    void _play_video(qint64 time = 0);
    void _play_prev();
    void _play_next();
    void _partsList_itemDoubleClicked(QListWidgetItem *item);
    void _endRequestDetailVideoInfo(QString _word, bool play = true);
    void _endRequestDetailImage(QString _word);
    void _player_stateChanged(QMediaPlayer::State newState);
    void _playlist_currentIndexChanged(int index);

private:
    void _init();
    void _requestDetailVideoInfo(QString _word, bool play = true);
    void _requestDetailImage(QString _word);
    void _loadParts(int index = 0);

private:
    QStackedWidget *_pic_video;
    QLabel *_pic;
    QLabel *_name;
    QTextEdit *_desc;
    QPushButton *_prev;
    QPushButton *_play;
    QPushButton *_next;
    QListWidget *_partList;

    QString _word;

    QVideoWidget *_video;
    QMediaPlayer *_player;
};

#endif // DETIALPAGE_H
