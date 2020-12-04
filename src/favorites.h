#ifndef READINGHISTORY_H
#define READINGHISTORY_H

#include <QDialog>
#include <QTableView>

class Favorites : public QDialog
{
    Q_OBJECT

public:
    Favorites(QWidget *parent = nullptr);

private:
    void _init_control();
    void _init_menu();
    void _init_layout();
    void _init_data();

private slots:
    void _remove();
    void _clear();
    void _customContextMenuRequested(const QPoint &pos);
    void _doubleClicked(const QModelIndex &index);

private:
    QTableView *_table;
    QMenu *_menu;
};

#endif // READINGHISTORY_H
