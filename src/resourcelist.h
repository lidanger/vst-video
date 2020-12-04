#ifndef RESOURCELIST_H
#define RESOURCELIST_H

#include <QFrame>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTreeView>

class ResourceList : public QFrame
{
    Q_OBJECT

public:
    ResourceList(QMainWindow* parent = nullptr);

public slots:
    void reload();

signals:
    void resourceSelected(const QString &class_name, int sub_index, bool live);

private slots:
    void _tree_pressed(const QModelIndex &index);

private:
    void _init();

    // 获取直播资源
    void _requestLiveResources();
    // 获取分类点播资源
    void _requestClassResources(const QString &name, const QString &api);
    void _endRequestResources();

private:
    QMainWindow *_window;
    QTreeView *_tree;
    QStandardItemModel *_tree_model;
};

#endif // RESOURCELIST_H
