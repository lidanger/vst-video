#ifndef CLASSIFYLIST_H
#define CLASSIFYLIST_H

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVideoWidget>

class BrowsePage : public QFrame
{
    Q_OBJECT

public:
    BrowsePage(QWidget* parent, QVideoWidget *video, QMediaPlayer *player);

public:
    void startBrowse(const QString &class_name, int sub_index);

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void itemSelectToPlay(int index, const QString &name, const QString &api, const QString &id);
    void requestShowDetail(int index, const QString &name, const QString &word, const QString &id);

private slots:
    void _exploreMenu(const QPoint &pos);
    void _page_prev();
    void _page_next();
    void _page_jmp();
    void _page_num_returnPressed();
    void _browseList_currentRowChanged(int key);
    void _browseList_itemDoubleClicked(QListWidgetItem *item);
    void _endRequestPreviewImage(const QString &word);
    void _endRequestPreviewVideoInfo(const QString &word);
    void _endRequestBrowseImage(const QString &word);
    void _endRequestPageInfo(int page);
    void _player_stateChanged(QMediaPlayer::State newState);
    void _show_detail();

private:
    void _init();
    void _requestSearch(const QString &word);
    void _endRequestSearch();
    void _requestPreviewImage(const QString &word);
    void _requestPreviewVideoInfo(const QString &word);
    void _getPageInfo(int page);
    void _createListWidget(QListWidget *listWidget, int key);
    void _setListWidgetImage(QListWidget *listWidget, const QString &word);
    void _requestBrowseImage(const QString &word);
    void _requestPageInfo(int page);

private:
    QListWidget *_browseList;
    QMenu *_contextMenu;

    QLabel *_page_info;
    QPushButton *_prev;
    QPushButton *_next;
    QLineEdit *_page_num;
    QPushButton *_jump;

    QStackedWidget *_pic_video;
    QLabel *_pic;
    QLabel *_parts;
    QTextEdit *_desc;

    QString _resource_name;
    int _resource_sub_index = -1;

    QVideoWidget *_video;
    QMediaPlayer *_player;
};

#endif // CLASSIFYLIST_H
