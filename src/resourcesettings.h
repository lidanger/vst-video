#ifndef RESOURCESETTINGS_H
#define RESOURCESETTINGS_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>

class ResourceSettings : public QDialog
{
    Q_OBJECT

public:
    ResourceSettings(QWidget *parent = nullptr);

private:
    void _init();
    void _init_data();

private slots:
    void _save();

private:
    QTabWidget *_tabs;
    QPlainTextEdit *_classResouce;
    QPlainTextEdit *_liveResouce;
    QPlainTextEdit *_resourceFilter;
    QPushButton *_ok;
    QPushButton *_cancel;
};

#endif // RESOURCESETTINGS_H
