#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QFrame>
#include <QLabel>
#include <QMediaPlayer>
#include <QMenu>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextEdit>
#include <QVideoWidget>

class SearchPage : public QFrame
{
    Q_OBJECT

public:
    SearchPage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player);

public:
    void startSearch(const QString &keyword, const QString &source = QString());

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void itemSelectToPlay(int index, const QString &name, const QString &api, const QString &id);
    void requestShowDetail(int index, const QString &name, const QString &word, const QString &id);

private slots:
    void _tableView_currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void _tableView_doubleClicked(const QModelIndex &index);
    void _exploreMenu(const QPoint &pos);
    void _menu_action() {}
    void _endRequestPreviewImage(const QString &word);
    void _endRequestPreviewVideoInfo(const QString &word);
    void _player_stateChanged(QMediaPlayer::State newState);
    void _show_detail();

private:
    void _init();
    void _requestSearch(const QString &word, const QString &source = QString());
    void _endRequestSearch();
    void _requestPreviewImage(const QString &word);
    void _requestPreviewVideoInfo(const QString &word);

private:
    QTableView *_tableView;
    QStandardItemModel *_model;
    QMenu *_contextMenu;
    QString _searchString;

    QStackedWidget *_pic_video;
    QLabel *_pic;
    QLabel *_parts;
    QTextEdit *_desc;

    QVideoWidget *_video;
    QMediaPlayer *_player;
};

#endif // SEARCHLIST_H
