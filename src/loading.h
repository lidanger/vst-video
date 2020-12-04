#ifndef LOADING_H
#define LOADING_H

#include <QDialog>
#include <QMainWindow>

class Loading : public QDialog
{
    Q_OBJECT

public:
    Loading(QWidget *parent = nullptr);

public:
    void show();

    static void setParent(QWidget *parent);
    static Loading* instance();

private:
    void _init();

private:
    static Loading *_inst;
};

#endif // LOADING_H
